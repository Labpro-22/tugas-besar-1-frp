#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/RailroadProperty.hpp"
#include "../../include/models/UtilityProperty.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/utils/GameException.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>

PropertyManager::PropertyManager(GameEngine& engine, Bank& bank,
                                 TransactionLogger& logger)
    : engine(engine), bank(bank), logger(logger) {}


std::vector<StreetProperty*> PropertyManager::getColorGroup(
    const std::string& colorGroup) const
{
    std::vector<StreetProperty*> result;
    Board& board = engine.getBoard();
    auto propTiles = board.getAllPropertyTiles();
    for (auto* tile : propTiles) {
        Property* prop = &tile->getProperty();
        if (!prop || prop->getType() != PropertyType::STREET) continue;
        StreetProperty* sp = static_cast<StreetProperty*>(prop);
        if (sp->getColorGroup() == colorGroup) {
            result.push_back(sp);
        }
    }
    return result;
}

bool PropertyManager::hasMonopoly(const Player& player,
                                   const std::string& colorGroup) const
{
    auto group = getColorGroup(colorGroup);
    if (group.empty()) return false;
    for (auto* sp : group) {
        if (sp->getOwner() != &player) return false;
    }
    return true;
}

bool PropertyManager::hasBuildingsInColorGroup(
    const Player& player, const std::string& colorGroup) const
{
    auto group = getColorGroup(colorGroup);
    for (auto* sp : group) {
        if (sp->getOwner() == &player &&
            sp->getBuildingLevel() != BuildingLevel::NONE) {
            return true;
        }
    }
    return false;
}

void PropertyManager::sellAllBuildingsInColorGroup(
    Player& player, const std::string& colorGroup)
{
    auto group = getColorGroup(colorGroup);
    for (auto* sp : group) {
        if (sp->getOwner() != &player) continue;
        if (sp->getBuildingLevel() == BuildingLevel::NONE) continue;

        int proceeds = sp->getBuildingSellValue();
        sp->demolishBuildings();
        bank.sendMoney(player, proceeds);
        logger.logSellBuilding(player.getUsername(), sp->getCode(), proceeds);

        std::cout << "Bangunan " << sp->getName()
                  << " terjual. Kamu menerima M" << proceeds << ".\n";
    }
}

int PropertyManager::computeRent(const Property& prop,
                                  const GameContext& ctx) const
{
    return prop.calculateRent(ctx);
}

// Buy properties
bool PropertyManager::offerPurchase(Player& buyer, Property& prop) {
    int price = prop.getPurchasePrice();

    std::cout << "\nKamu mendarat di " << prop.getName()
              << " (" << prop.getCode() << ")!\n";
    std::cout << "+================================+\n";
    std::cout << "| Harga Beli : M" << price << "\n";
    std::cout << "| Nilai Gadai: M" << prop.getMortgageValue() << "\n";

    if (prop.getType() == PropertyType::STREET) {
        StreetProperty* sp = static_cast<StreetProperty*>(&prop);
        const auto& rentLevels = sp->getColorGroup(); // hanya untuk info
        std::cout << "| Color Group: " << sp->getColorGroup() << "\n";
    }
    std::cout << "+================================+\n";
    std::cout << "Uang kamu saat ini: M" << buyer.getMoney() << "\n";

    if (!buyer.canAfford(price)) {
        std::cout << "Uang kamu tidak cukup untuk membeli properti ini.\n";
        std::cout << "Properti ini akan masuk ke sistem lelang...\n";
        return false;
    }

    std::cout << "Apakah kamu ingin membeli properti ini seharga M"
              << price << "? (y/n): ";
    char choice;
    std::cin >> choice;

    if (choice != 'y' && choice != 'Y') {
        std::cout << "Properti ini akan masuk ke sistem lelang...\n";
        return false;
    }

    bank.receivePayment(buyer, price);
    bank.transferPropertyToPlayer(&prop, buyer);

    logger.logBuy(buyer.getUsername(), prop.getName(), prop.getCode(), price);

    std::cout << prop.getName() << " kini menjadi milikmu!\n";
    std::cout << "Uang tersisa: M" << buyer.getMoney() << "\n";
    return true;
}

// Get Railroad & Utility 
void PropertyManager::autoAcquire(Player& player, Property& prop) {
    std::string typeStr = (prop.getType() == PropertyType::RAILROAD)
                          ? "RAILROAD" : "UTILITY";

    bank.transferPropertyToPlayer(&prop, player);
    logger.logAutoAcquire(player.getUsername(), prop.getName(),
                          prop.getCode(), typeStr);

    if (prop.getType() == PropertyType::RAILROAD) {
        std::cout << "Kamu mendarat di " << prop.getName() << "!\n";
        std::cout << "Belum ada yang menginjaknya duluan, stasiun ini "
                     "kini menjadi milikmu!\n";
    } else {
        std::cout << "Kamu mendarat di " << prop.getName() << "!\n";
        std::cout << "Belum ada yang menginjaknya duluan, "
                  << prop.getName() << " kini menjadi milikmu!\n";
    }
}

// Pay Rent
void PropertyManager::payRent(Player& payer, Property& prop,
                               const GameContext& ctx)
{
    if (prop.isMortgaged()) {
        std::cout << "Kamu mendarat di " << prop.getName()
                  << ", milik " << prop.getOwner()->getUsername() << ".\n";
        std::cout << "Properti ini sedang digadaikan [M]. "
                     "Tidak ada sewa yang dikenakan.\n";
        return;
    }

    Player* owner = prop.getOwner();
    if (!owner || owner == &payer) return; 

    if (payer.isShieldActive()) {
        std::cout << "[SHIELD ACTIVE]: Efek ShieldCard melindungi kamu!\n";
        std::cout << "Tagihan sewa dibatalkan.\n";
        return;
    }

    int rent = computeRent(prop, ctx);

    std::cout << "\nKamu mendarat di " << prop.getName()
              << " (" << prop.getCode() << "), milik "
              << owner->getUsername() << "!\n";
    std::cout << "Sewa : M" << rent << "\n";
    std::cout << "Uang kamu : M" << payer.getMoney()
              << " -> M" << (payer.getMoney() - rent) << "\n";
    std::cout << "Uang " << owner->getUsername() << " : M"
              << owner->getMoney() << " -> M"
              << (owner->getMoney() + rent) << "\n";

    if (!payer.canAfford(rent)) {
        std::cout << "Kamu tidak mampu membayar sewa penuh! (M" << rent << ")\n";
        std::cout << "Uang kamu saat ini: M" << payer.getMoney() << "\n";
        throw InsufficientFundsException(payer.getUsername(), rent, payer.getMoney());
    }

    payer.deductMoney(rent);
    owner->addMoney(rent);

    std::string detail;
    if (prop.getType() == PropertyType::STREET) {
        StreetProperty* sp = static_cast<StreetProperty*>(&prop);
        int lvl = sp->getBuildingCount();
        if (lvl == 0) detail = "tanah kosong";
        else if (lvl == 5) detail = "hotel";
        else detail = std::to_string(lvl) + " rumah";
        if (prop.getFestivalMultiplier() > 1)
            detail += ", festival aktif x"
                   + std::to_string(prop.getFestivalMultiplier());
    } else if (prop.getType() == PropertyType::RAILROAD) {
        detail = "railroad";
    } else {
        detail = "utility";
    }

    logger.logRent(payer.getUsername(), owner->getUsername(),
                   rent, prop.getCode(), detail);
}

// Mortage Property
bool PropertyManager::mortgageProperty(Player& player, Property& prop) {
    if (!canMortgage(player, prop)) {
        if (prop.getType() == PropertyType::STREET) {
            StreetProperty* sp = static_cast<StreetProperty*>(&prop);
            if (hasBuildingsInColorGroup(player, sp->getColorGroup())) {
                std::cout << prop.getName()
                          << " tidak dapat digadaikan!\n";
                std::cout << "Masih terdapat bangunan di color group ["
                          << sp->getColorGroup() << "].\n";
                std::cout << "Jual semua bangunan color group ["
                          << sp->getColorGroup() << "]? (y/n): ";
                char choice;
                std::cin >> choice;
                if (choice != 'y' && choice != 'Y') return false;
                sellAllBuildingsInColorGroup(player, sp->getColorGroup());
                std::cout << "Lanjut menggadaikan " << prop.getName()
                          << "? (y/n): ";
                std::cin >> choice;
                if (choice != 'y' && choice != 'Y') return false;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    int mortgageValue = prop.getMortgageValue();
    prop.setStatus(OwnershipStatus::MORTGAGED);
    bank.sendMoney(player, mortgageValue);

    logger.logMortgage(player.getUsername(), prop.getCode(), mortgageValue);

    std::cout << prop.getName() << " berhasil digadaikan.\n";
    std::cout << "Kamu menerima M" << mortgageValue << " dari Bank.\n";
    std::cout << "Uang kamu sekarang: M" << player.getMoney() << "\n";
    std::cout << "Catatan: Sewa tidak dapat dipungut dari properti "
                 "yang digadaikan.\n";
    return true;
}

// Redeem Property
bool PropertyManager::redeemProperty(Player& player, Property& prop) {
    if (!canRedeem(player, prop)) return false;

    int redeemCost = prop.getPurchasePrice();

    if (!player.canAfford(redeemCost)) {
        std::cout << "Uang kamu tidak cukup untuk menebus "
                  << prop.getName() << ".\n";
        std::cout << "Harga tebus: M" << redeemCost
                  << " | Uang kamu: M" << player.getMoney() << "\n";
        return false;
    }

    bank.receivePayment(player, redeemCost);
    prop.setStatus(OwnershipStatus::OWNED);

    logger.logRedeem(player.getUsername(), prop.getCode(), redeemCost);

    std::cout << prop.getName() << " berhasil ditebus!\n";
    std::cout << "Kamu membayar M" << redeemCost << " ke Bank.\n";
    std::cout << "Uang kamu sekarang: M" << player.getMoney() << "\n";
    return true;
}

// Build Property 
bool PropertyManager::buildOnProperty(Player& player, StreetProperty& prop) {
    const std::string& colorGroup = prop.getColorGroup();

    if (!hasMonopoly(player, colorGroup)) {
        std::cout << "Kamu harus memiliki seluruh petak dalam color group ["
                  << colorGroup << "] untuk membangun.\n";
        return false;
    }

    if (prop.getBuildingLevel() == BuildingLevel::HOTEL) {
        std::cout << prop.getName()
                  << " sudah berstatus hotel (maksimal).\n";
        return false;
    }

    bool upgradeToHotel = false;
    int  cost           = 0;

    // Prerequisite to own a house
    if (prop.getBuildingLevel() == BuildingLevel::HOUSE_4) {
        auto group = getColorGroup(colorGroup);
        bool allHave4 = true;
        for (auto* sp : group) {
            if (sp->getBuildingLevel() != BuildingLevel::HOUSE_4) {
                allHave4 = false;
                break;
            }
        }
        if (!allHave4) {
            std::cout << "Semua petak di color group [" << colorGroup
                      << "] harus memiliki 4 rumah sebelum upgrade ke hotel.\n";
            return false;
        }
        upgradeToHotel = true;
        cost = prop.getHotelCost();
        std::cout << "Upgrade ke hotel? Biaya: M" << cost << " (y/n): ";
    } else {
        cost = prop.getHouseCost();
        std::cout << "Kamu membangun 1 rumah di " << prop.getName()
                  << ". Biaya: M" << cost << " (y/n): ";
    }

    char choice;
    std::cin >> choice;
    if (choice != 'y' && choice != 'Y') return false;

    if (!player.canAfford(cost)) {
        std::cout << "Uang kamu tidak cukup untuk membangun!\n";
        return false;
    }

    bank.receivePayment(player, cost);

    if (upgradeToHotel) {
        prop.buildHotel();
        logger.logBuild(player.getUsername(), prop.getCode(), "hotel", cost);
        std::cout << prop.getName() << " di-upgrade ke Hotel!\n";
    } else {
        prop.buildHouse();
        logger.logBuild(player.getUsername(), prop.getCode(), "rumah", cost);
        std::cout << "Kamu membangun 1 rumah di " << prop.getName()
                  << ". Biaya: M" << cost << "\n";
    }

    std::cout << "Uang tersisa: M" << player.getMoney() << "\n";
    return true;
}

// Sell Property to Bank
void PropertyManager::sellPropertyToBank(Player& player, Property& prop) {
    int sellValue = prop.getSellValue();
    if (prop.getType() == PropertyType::STREET) {
        StreetProperty* sp = static_cast<StreetProperty*>(&prop);
        if (sp->getBuildingLevel() != BuildingLevel::NONE) {
            sp->demolishBuildings();
        }
    }

    bank.sendMoney(player, sellValue);
    bank.reclaim(&prop);

    logger.log(player.getUsername(), "JUAL_PROPERTI",
               "Jual " + prop.getCode() + " ke Bank seharga M"
               + std::to_string(sellValue));
}


std::vector<Property*> PropertyManager::getMortgageableProperties(
    const Player& player) const
{
    std::vector<Property*> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (canMortgage(player, *prop)) result.push_back(prop);
    }
    return result;
}

std::vector<Property*> PropertyManager::getMortgagedProperties(
    const Player& player) const
{
    std::vector<Property*> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->isMortgaged()) result.push_back(prop);
    }
    return result;
}

std::map<std::string, std::vector<StreetProperty*>>
PropertyManager::getBuildableColorGroups(const Player& player) const
{
    std::map<std::string, std::vector<StreetProperty*>> result;
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->getType() != PropertyType::STREET) continue;
        StreetProperty* sp = static_cast<StreetProperty*>(prop);
        const std::string& cg = sp->getColorGroup();
        if (!hasMonopoly(player, cg)) continue;
        if (sp->getBuildingLevel() != BuildingLevel::HOTEL) {
            result[cg].push_back(sp);
        }
    }
    return result;
}

std::vector<StreetProperty*> PropertyManager::getBuildableTilesInGroup(
    const Player& player, const std::string& colorGroup) const
{
    auto group = getColorGroup(colorGroup);
    std::vector<StreetProperty*> result;

    int minLevel = 5; 
    for (auto* sp : group) {
        if (sp->getOwner() == &player) {
            int lvl = sp->getBuildingCount();
            minLevel = std::min(minLevel, lvl);
        }
    }

    for (auto* sp : group) {
        if (sp->getOwner() != &player) continue;
        if (sp->getBuildingLevel() == BuildingLevel::HOTEL) continue;
        if (sp->getBuildingCount() == minLevel) {
            result.push_back(sp);
        }
    }
    return result;
}

bool PropertyManager::canMortgage(const Player& player,
                                   const Property& prop) const
{
    if (prop.getOwner() != &player) return false;
    if (prop.isMortgaged()) return false;
    if (prop.getType() == PropertyType::STREET) {
        const StreetProperty* sp =
            static_cast<const StreetProperty*>(&prop);
        if (hasBuildingsInColorGroup(player, sp->getColorGroup()))
            return false;
    }
    return true;
}

bool PropertyManager::canRedeem(const Player& player,
                                 const Property& prop) const
{
    if (prop.getOwner() != &player) return false;
    if (!prop.isMortgaged()) return false;
    return true;
}

int PropertyManager::estimateLiquidationValue(const Player& player) const {
    int total = 0;
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->isMortgaged()) {
            continue;
        }
        total += prop->getSellValue();
    }
    return total;
}