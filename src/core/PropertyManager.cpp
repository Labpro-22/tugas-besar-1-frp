#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/RailroadProperty.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/UtilityProperty.hpp"
#include "../../include/utils/GameException.hpp"

#include <algorithm>
#include <sstream>

PropertyManager::PropertyManager(GameEngine& engine, Bank& bank,
                                 TransactionLogger& logger)
    : engine(engine), bank(bank), logger(logger) {}

std::vector<StreetProperty*> PropertyManager::getColorGroup(
    const std::string& colorGroup) const {
    std::vector<StreetProperty*> result;
    for (auto* tile : engine.getBoard().getAllPropertyTiles()) {
        Property* prop = &tile->getProperty();
        if (prop->getType() != PropertyType::STREET) {
            continue;
        }

        auto* sp = static_cast<StreetProperty*>(prop);
        if (sp->getColorGroup() == colorGroup) {
            result.push_back(sp);
        }
    }
    return result;
}

bool PropertyManager::hasMonopoly(const Player& player,
                                  const std::string& colorGroup) const {
    auto group = getColorGroup(colorGroup);
    if (group.empty()) {
        return false;
    }

    for (auto* sp : group) {
        if (sp->getOwner() != &player) {
            return false;
        }
    }
    return true;
}

bool PropertyManager::hasBuildingsInColorGroup(
    const Player& player, const std::string& colorGroup) const {
    for (auto* sp : getColorGroup(colorGroup)) {
        if (sp->getOwner() == &player &&
            sp->getBuildingLevel() != BuildingLevel::NONE) {
            return true;
        }
    }
    return false;
}

void PropertyManager::sellAllBuildingsInColorGroup(
    Player& player, const std::string& colorGroup) {
    for (auto* sp : getColorGroup(colorGroup)) {
        if (sp->getOwner() != &player) {
            continue;
        }
        if (sp->getBuildingLevel() == BuildingLevel::NONE) {
            continue;
        }

        int proceeds = sp->getBuildingSellValue();
        sp->demolishBuildings();
        bank.sendMoney(player, proceeds);
        logger.logSellBuilding(player.getUsername(), sp->getCode(), proceeds);
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Jual Bangunan",
            sp->getName() + " terjual. Kamu menerima M" +
            std::to_string(proceeds) + ".");
    }
}

int PropertyManager::computeRent(const Property& prop,
                                 const GameContext& ctx) const {
    return prop.calculateRent(ctx);
}

bool PropertyManager::offerPurchase(Player& buyer, Property& prop) {
    const int basePrice = prop.getPurchasePrice();
    const int discountPercent = std::clamp(buyer.getDiscountPercent(), 0, 100);
    const bool hasDiscount = discountPercent > 0;
    const int discountedPrice =
        hasDiscount ? std::max(0, basePrice - ((basePrice * discountPercent) / 100)) : basePrice;
    const int price = discountedPrice;
    const std::string promptKey = "beli_" + prop.getCode();

    if (!engine.hasPromptAnswer(promptKey)) {
        std::ostringstream info;
        // Akta isn't fully drawn here in original either, let's keep it close but with exact words
        info << "Uang kamu saat ini: M" << buyer.getMoney();
        if (hasDiscount) {
            info << "\nDiscount aktif: " << discountPercent << "% (harga jadi M" << price << ")";
        }
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Mendarat di " + prop.getCode(), info.str());
    }

    if (!buyer.canAfford(price)) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::WARNING,
            "Tidak Cukup Uang", "Properti masuk lelang.");
        return false;
    }

    if (!engine.hasPromptAnswer(promptKey)) {
        std::ostringstream prompt;
        if (hasDiscount) {
            prompt << "Harga normal M" << basePrice << ", discount " << discountPercent
                   << "% -> M" << price
                   << ". Apakah kamu ingin membeli properti ini? (y/n):";
        } else {
            prompt << "Apakah kamu ingin membeli properti ini seharga M" << price << "? (y/n):";
        }
        engine.pushPrompt(
            promptKey,
            prompt.str(),
            {"y", "n"});
        engine.setPendingContinuation([this, &buyer, &prop]() {
            CommandResult resumed;
            if (!offerPurchase(buyer, prop)) {
                engine.getAuctionManager().startAuction(prop, &buyer, true);
            }
            return resumed;
        });
        return false;
    }

    const std::string answer = engine.consumePromptAnswer(promptKey);
    if (answer != "y" && answer != "Y") {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Tidak Jadi Beli", "Properti masuk sistem lelang...");
        return false;
    }

    bank.receivePayment(buyer, price);
    bank.transferPropertyToPlayer(&prop, buyer);
    logger.logBuy(buyer.getUsername(), prop.getName(), prop.getCode(), price);
    std::ostringstream successMessage;
    successMessage << prop.getName() << " kini menjadi milikmu!\n";
    if (hasDiscount) {
        successMessage << "Harga normal: M" << basePrice
                       << " | Discount " << discountPercent
                       << "% | Dibayar: M" << price << "\n";
    }
    successMessage << "Uang tersisa: M" << buyer.getMoney();
    engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
        "Beli Properti",
        successMessage.str());
    return true;
}

void PropertyManager::autoAcquire(Player& player, Property& prop) {
    std::string typeStr =
        (prop.getType() == PropertyType::RAILROAD) ? "RAILROAD" : "UTILITY";
    bank.transferPropertyToPlayer(&prop, player);
    logger.logAutoAcquire(player.getUsername(), prop.getName(),
                          prop.getCode(), typeStr);
    engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
        "Dapat " + typeStr,
        prop.getName() + " kini milik " + player.getUsername() +
        " (otomatis).");
}

void PropertyManager::payRent(Player& payer, Property& prop,
                              const GameContext& ctx) {
    if (prop.isMortgaged()) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Properti Digadai",
            prop.getName() + " sedang digadaikan [M]. Tidak ada sewa.");
        return;
    }

    Player* owner = prop.getOwner();
    if (!owner || owner == &payer) {
        return;
    }

    if (payer.isShieldActive()) {
        engine.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Shield Aktif",
            "ShieldCard melindungi dari sewa di " + prop.getCode() + ".");
        return;
    }

    int rent = computeRent(prop, ctx);
    if (!payer.canAfford(rent)) {
        throw InsufficientFundsException(
            payer.getUsername(), rent, payer.getMoney());
    }

    int payerBefore = payer.getMoney();
    int ownerBefore = owner->getMoney();
    payer.deductMoney(rent);
    owner->addMoney(rent);

    std::string detail;
    if (prop.getType() == PropertyType::STREET) {
        auto* sp = static_cast<const StreetProperty*>(&prop);
        int lvl = sp->getBuildingCount();
        if (lvl == 0) {
            detail = "tanah kosong";
        } else if (sp->getBuildingLevel() == BuildingLevel::HOTEL) {
            detail = "hotel";
        } else {
            detail = std::to_string(lvl) + " rumah";
        }

        if (prop.getFestivalMultiplier() > 1) {
            detail += ", festival x" +
                      std::to_string(prop.getFestivalMultiplier());
        }
    } else if (prop.getType() == PropertyType::RAILROAD) {
        detail = "railroad";
    } else {
        detail = "utility";
    }

    std::ostringstream msg;
    msg << "Kamu mendarat di " << prop.getName() << " (" << prop.getCode() << "), milik " << owner->getUsername() << "!\n";
    if (prop.getType() == PropertyType::STREET) {
        msg << "Kondisi: " << detail << "\n";
    }
    msg << "Sewa: M" << rent << "\n"
        << "Uang " << payer.getUsername() << " saat ini: M" << payerBefore << " -> M" << payer.getMoney() << "\n"
        << "Uang " << owner->getUsername() << " saat ini: M" << ownerBefore << " -> M" << owner->getMoney();
    engine.pushEvent(GameEventType::MONEY, UiTone::WARNING, "Bayar Sewa", msg.str());
    logger.logRent(payer.getUsername(), owner->getUsername(), rent, prop.getCode(), detail);
}

bool PropertyManager::mortgageProperty(Player& player, Property& prop) {
    if (prop.getOwner() != &player) {
        throw NotOwnerException(player.getUsername(), prop.getCode());
    }
    if (prop.isMortgaged()) {
        throw AlreadyMortgagedException(prop.getCode());
    }

    if (prop.getType() == PropertyType::STREET) {
        auto* sp = static_cast<StreetProperty*>(&prop);
        if (hasBuildingsInColorGroup(player, sp->getColorGroup())) {
            const std::string sellPromptKey = "jual_bangunan_" + prop.getCode();

            engine.pushEvent(GameEventType::PROPERTY, UiTone::WARNING,
                "Ada Bangunan",
                "Properti ini masih memiliki bangunan di atasnya!\n"
                "Kamu harus menjual seluruh bangunan dengan Color Group yang sama sebelum menggadaikan.");

            if (!engine.hasPromptAnswer(sellPromptKey)) {
                engine.pushPrompt(
                    sellPromptKey,
                    "Jual seluruh bangunan dengan Color Group " + sp->getColorGroup() + "? (y/n):",
                    {"y", "n"});
                engine.setPendingContinuation([this, &player, &prop]() {
                    CommandResult resumed;
                    mortgageProperty(player, prop);
                    return resumed;
                });
                return false;
            }

            const std::string sellAnswer = engine.consumePromptAnswer(sellPromptKey);
            if (sellAnswer != "y" && sellAnswer != "Y") {
                engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
                    "Gadai Dibatalkan",
                    "Aksi gadai dibatalkan!");
                return false;
            }

            sellAllBuildingsInColorGroup(player, sp->getColorGroup());
        }
    }

    int mv = prop.getMortgageValue();
    const std::string gadaiPromptKey = "gadai_confirm_" + prop.getCode();
    if (!engine.hasPromptAnswer(gadaiPromptKey)) {
        engine.pushPrompt(
            gadaiPromptKey,
            "Apakah kamu yakin ingin menggadaikan properti " + prop.getName() + " untuk mendapatkan uang tunai sebesar M" + std::to_string(mv) + "? (y/n):",
            {"y", "n"});
        engine.setPendingContinuation([this, &player, &prop]() {
            CommandResult resumed;
            mortgageProperty(player, prop);
            return resumed;
        });
        return false;
    }

    const std::string gadaiAnswer = engine.consumePromptAnswer(gadaiPromptKey);
    if (gadaiAnswer != "y" && gadaiAnswer != "Y") {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Gadai Dibatalkan",
            "Aksi gadai dibatalkan!");
        return false;
    }

    prop.setStatus(OwnershipStatus::MORTGAGED);
    bank.sendMoney(player, mv);
    logger.logMortgage(player.getUsername(), prop.getCode(), mv);
    engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
        "Gadai Berhasil",
        prop.getName() + " berhasil digadaikan!\n"
        "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
    return true;
}

bool PropertyManager::redeemProperty(Player& player, Property& prop) {
    if (prop.getOwner() != &player) {
        throw NotOwnerException(player.getUsername(), prop.getCode());
    }
    if (!prop.isMortgaged()) {
        throw NotMortgagedException(prop.getCode());
    }

    int cost = prop.getPurchasePrice();

    if (!player.canAfford(cost)) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::WARNING,
            "Uang Tidak Cukup",
            "Tebus: M" + std::to_string(cost) +
                " | Uang kamu: M" + std::to_string(player.getMoney()));
        return false;
    }

    bank.receivePayment(player, cost);
    prop.setStatus(OwnershipStatus::OWNED);
    logger.logRedeem(player.getUsername(), prop.getCode(), cost);
    engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
        "Tebus Berhasil",
        "Kamu berhasil menebus properti " + prop.getName() + " seharga M" + std::to_string(cost) + "!\n"
        "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
    return true;
}

bool PropertyManager::buildOnProperty(Player& player, StreetProperty& prop) {
    const std::string& cg = prop.getColorGroup();
    if (!hasMonopoly(player, cg)) {
        throw GameException("Harus memonopoli [" + cg + "] untuk membangun.");
    }
    if (prop.getBuildingLevel() == BuildingLevel::HOTEL) {
        throw MaxBuildingLevelException(prop.getCode());
    }

    const auto buildableTiles = getBuildableTilesInGroup(player, cg);
    const bool canBuildHere = std::any_of(
        buildableTiles.begin(), buildableTiles.end(),
        [&prop](const StreetProperty* candidate) {
            return candidate == &prop;
        });
    if (!canBuildHere) {
        throw GameException(
            "Pembangunan harus merata. Pilih petak dengan level bangunan terendah di color group [" +
            cg + "].");
    }

    bool upgradeToHotel = (prop.getBuildingLevel() == BuildingLevel::HOUSE_4);

    int cost = upgradeToHotel ? prop.getHotelCost() : prop.getHouseCost();
    const std::string promptKey = "bangun_" + prop.getCode();
    if (!engine.hasPromptAnswer(promptKey)) {
        std::string label = upgradeToHotel ? "hotel" : "rumah";
        engine.pushPrompt(
            promptKey,
            "Apakah kamu ingin membangun " + label + " di " + prop.getName() + " dengan biaya M" + std::to_string(cost) + "? (y/n):",
            {"y", "n"});
        engine.chainPendingContinuation([this, &player, &prop]() {
            CommandResult resumed;
            buildOnProperty(player, prop);
            return resumed;
        });
        return false;
    }

    const std::string answer = engine.consumePromptAnswer(promptKey);
    if (answer != "y" && answer != "Y") {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Bangun Dibatalkan",
            "Proses pembangunan dibatalkan!");
        return false;
    }

    if (!player.canAfford(cost)) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::WARNING,
            "Uang Tidak Cukup",
            "Butuh M" + std::to_string(cost) +
                " | Punya M" + std::to_string(player.getMoney()));
        return false;
    }

    bank.receivePayment(player, cost);
    if (upgradeToHotel) {
        prop.buildHotel();
        logger.logBuild(player.getUsername(), prop.getCode(), "hotel", cost);
        engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
            "Upgrade Hotel",
            "Satu hotel telah dibangun di " + prop.getName() + ".\n"
            "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
    } else {
        prop.buildHouse();
        logger.logBuild(player.getUsername(), prop.getCode(), "rumah", cost);
        engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS,
            "Bangun Rumah",
            "Satu rumah telah dibangun di " + prop.getName() + ".\n"
            "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
    }
    return true;
}

PropertyManager::BuildOption PropertyManager::getBuildOption(
    const Player& player, const StreetProperty& prop) const {
    if (prop.getOwner() != &player) {
        return BuildOption::NONE;
    }

    if (!hasMonopoly(player, prop.getColorGroup())) {
        return BuildOption::NONE;
    }

    if (prop.getBuildingLevel() == BuildingLevel::HOTEL) {
        return BuildOption::NONE;
    }

    const std::vector<StreetProperty*> buildableTiles =
        getBuildableTilesInGroup(player, prop.getColorGroup());
    const bool tileIsEligible = std::any_of(
        buildableTiles.begin(), buildableTiles.end(), [&prop](const StreetProperty* candidate) {
            return candidate != nullptr && candidate->getCode() == prop.getCode();
        });

    if (!tileIsEligible) {
        return BuildOption::NONE;
    }

    if (prop.getBuildingLevel() == BuildingLevel::HOUSE_4) {
        for (const StreetProperty* tile : getColorGroup(prop.getColorGroup())) {
            if (tile == nullptr || tile->getOwner() != &player ||
                tile->getBuildingLevel() != BuildingLevel::HOUSE_4) {
                return BuildOption::NONE;
            }
        }
        return BuildOption::HOTEL;
    }

    return BuildOption::HOUSE;
}

void PropertyManager::sellPropertyToBank(Player& player, Property& prop) {
    int sellValue = prop.getSellValue();
    if (prop.getType() == PropertyType::STREET) {
        auto* sp = static_cast<StreetProperty*>(&prop);
        if (sp->getBuildingLevel() != BuildingLevel::NONE) {
            sp->demolishBuildings();
        }
    }
    bank.sendMoney(player, sellValue);
    bank.reclaim(&prop);
    logger.log(player.getUsername(), "JUAL_PROPERTI",
               "Jual " + prop.getCode() + " seharga M" +
                   std::to_string(sellValue));
}

std::vector<Property*> PropertyManager::getMortgageableProperties(
    const Player& player) const {
    std::vector<Property*> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (canMortgage(player, *prop)) {
            result.push_back(prop);
        }
    }
    return result;
}

std::vector<Property*> PropertyManager::getMortgagedProperties(
    const Player& player) const {
    std::vector<Property*> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->isMortgaged()) {
            result.push_back(prop);
        }
    }
    return result;
}

std::map<std::string, std::vector<StreetProperty*>>
PropertyManager::getBuildableColorGroups(const Player& player) const {
    std::map<std::string, std::vector<StreetProperty*>> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->getType() != PropertyType::STREET) {
            continue;
        }

        auto* sp = static_cast<StreetProperty*>(prop);
        if (!hasMonopoly(player, sp->getColorGroup())) {
            continue;
        }
        if (sp->getBuildingLevel() != BuildingLevel::HOTEL) {
            result[sp->getColorGroup()].push_back(sp);
        }
    }
    return result;
}

std::vector<StreetProperty*> PropertyManager::getBuildableTilesInGroup(
    const Player& player, const std::string& colorGroup) const {
    auto group = getColorGroup(colorGroup);
    int minLevel = 5;
    for (auto* sp : group) {
        if (sp->getOwner() == &player) {
            minLevel = std::min(minLevel, sp->getBuildingCount());
        }
    }

    std::vector<StreetProperty*> result;
    for (auto* sp : group) {
        if (sp->getOwner() != &player) {
            continue;
        }
        if (sp->getBuildingLevel() == BuildingLevel::HOTEL) {
            continue;
        }
        if (sp->getBuildingCount() == minLevel) {
            result.push_back(sp);
        }
    }
    return result;
}

bool PropertyManager::canMortgage(const Player& player,
                                  const Property& prop) const {
    if (prop.getOwner() != &player || prop.isMortgaged()) {
        return false;
    }

    if (prop.getType() == PropertyType::STREET) {
        const auto* sp = static_cast<const StreetProperty*>(&prop);
        if (hasBuildingsInColorGroup(player, sp->getColorGroup())) {
            return false;
        }
    }
    return true;
}

bool PropertyManager::canRedeem(const Player& player,
                                const Property& prop) const {
    return prop.getOwner() == &player && prop.isMortgaged();
}

int PropertyManager::estimateLiquidationValue(const Player& player) const {
    int total = 0;
    for (Property* prop : player.getOwnedProperties()) {
        if (!prop || prop->isMortgaged()) {
            continue;
        }

        int bestValue = prop->getSellValue();
        if (canMortgage(player, *prop)) {
            bestValue = std::max(bestValue, prop->getMortgageValue());
        }
        total += bestValue;
    }
    return total;
}
