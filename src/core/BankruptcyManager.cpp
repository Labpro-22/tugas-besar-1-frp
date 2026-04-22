#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"

#include <sstream>
#include <utility>

BankruptcyManager::BankruptcyManager(GameEngine& engine, Bank& bank,
                                     TransactionLogger& logger,
                                     PropertyManager& propertyManager,
                                     AuctionManager& auctionManager)
    : engine(engine),
      bank(bank),
      logger(logger),
      propertyManager(propertyManager),
      auctionManager(auctionManager),
      pendingDebtor(nullptr),
      pendingCreditor(nullptr),
      pendingObligation(0) {}

int BankruptcyManager::computeMaxLiquidation(const Player& player) const {
    int total = player.getMoney();
    for (Property* prop : player.getOwnedProperties()) {
        if (!prop || prop->isMortgaged()) {
            continue;
        }

        int bestValue = prop->getSellValue();
        if (propertyManager.canMortgage(player, *prop)) {
            bestValue = std::max(bestValue, prop->getMortgageValue());
        }
        total += bestValue;
    }
    return total;
}

void BankruptcyManager::clearPendingDebt() {
    pendingDebtor = nullptr;
    pendingCreditor = nullptr;
    pendingObligation = 0;
}

void BankruptcyManager::runLiquidationPanel(Player& player, int obligation) {
    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
        "Panel Likuidasi",
        "Uang: M" + std::to_string(player.getMoney()) +
            " | Kewajiban: M" + std::to_string(obligation));

    const auto& owned = player.getOwnedProperties();
    std::vector<Property*> sellable;
    std::vector<Property*> mortgageable;
    for (Property* prop : owned) {
        if (!prop->isMortgaged()) {
            sellable.push_back(prop);
        }
        if (propertyManager.canMortgage(player, *prop)) {
            mortgageable.push_back(prop);
        }
    }

    if (sellable.empty() && mortgageable.empty()) {
        engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::ERROR,
            "Tidak Ada Aset", "Tidak ada properti yang bisa dilikuidasi.");
        return;
    }

    std::vector<std::string> options;
    std::vector<std::pair<std::string, Property*>> actions;
    std::ostringstream list;

    list << "[Jual ke Bank]\n";
    for (Property* prop : sellable) {
        options.push_back("JUAL_" + prop->getCode());
        actions.push_back({"JUAL", prop});

        list << "  " << actions.size() << ". " << prop->getName()
             << " (" << prop->getCode() << ") -> M" << prop->getSellValue();
        if (prop->getType() == PropertyType::STREET) {
            int buildingValue =
                static_cast<StreetProperty*>(prop)->getBuildingSellValue();
            if (buildingValue > 0) {
                list << " (incl. bangunan M" << buildingValue << ")";
            }
        }
        list << "\n";
    }

    list << "[Gadaikan]\n";
    for (Property* prop : mortgageable) {
        options.push_back("GADAI_" + prop->getCode());
        actions.push_back({"GADAI", prop});
        list << "  " << actions.size() << ". " << prop->getName()
             << " (" << prop->getCode() << ") -> M"
             << prop->getMortgageValue() << "\n";
    }

    list << "  0. Selesai (jika uang sudah cukup)";

    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
        "Pilihan Likuidasi", list.str());

    const std::string promptKey = "likuidasi_" + player.getUsername();
    if (!engine.hasPromptAnswer(promptKey)) {
        engine.pushPrompt(promptKey, "Pilih aksi (nomor atau 0):", options);
        engine.setPendingContinuation([this]() {
            CommandResult resumed;
            if (pendingDebtor) {
                handleDebt(*pendingDebtor, pendingObligation, pendingCreditor);
            }
            return resumed;
        });
        return;
    }

    int choice = -1;
    const std::string answer = engine.consumePromptAnswer(promptKey);
    try {
        choice = std::stoi(answer);
    } catch (const std::exception&) {
        for (size_t i = 0; i < options.size(); ++i) {
            if (answer == options[i]) {
                choice = static_cast<int>(i) + 1;
                break;
            }
        }
        if (answer == "SELESAI") {
            choice = 0;
        }
    }

    if (choice == 0) {
        if (player.getMoney() < obligation) {
            engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
                "Likuidasi Belum Cukup",
                "Dana belum cukup untuk melunasi kewajiban. Lanjutkan likuidasi.");
        }
        return;
    }

    if (choice < 1 || choice > static_cast<int>(actions.size())) {
        engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
            "Pilihan Tidak Valid",
            "Pilih nomor aksi yang tersedia.");
        return;
    }

    auto [actionType, prop] = actions[choice - 1];
    if (actionType == "JUAL") {
        int sellValue = prop->getSellValue();
        propertyManager.sellPropertyToBank(player, *prop);
        engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::INFO,
            "Jual Properti",
            prop->getName() + " terjual. Dapat M" +
                std::to_string(sellValue) + ". Uang sekarang: M" +
                std::to_string(player.getMoney()));
    } else {
        propertyManager.mortgageProperty(player, *prop);
        if (engine.hasPendingContinuation()) {
            engine.chainPendingContinuation([this]() {
                CommandResult resumed;
                if (pendingDebtor) {
                    handleDebt(*pendingDebtor, pendingObligation, pendingCreditor);
                }
                return resumed;
            });
        }
    }
}

void BankruptcyManager::transferAssetsToCreditor(Player& debtor,
                                                 Player& creditor) {
    std::ostringstream detail;
    detail << "Pengalihan ke " << creditor.getUsername() << ":\n";

    int remainingMoney = debtor.getMoney();
    if (remainingMoney > 0) {
        detail << "- Uang tunai: M" << remainingMoney << "\n";
        creditor.addMoney(remainingMoney);
        debtor.deductMoney(remainingMoney);
    }

    std::vector<Property*> props = debtor.getOwnedProperties();
    for (Property* prop : props) {
        detail << "- " << prop->getName() << " (" << prop->getCode() << ")";
        if (prop->isMortgaged()) {
            detail << " MORTGAGED [M]";
        }
        detail << "\n";

        debtor.removeProperty(prop);
        prop->setOwner(&creditor);
        creditor.addProperty(prop);
    }

    logger.logAssetTransfer(debtor.getUsername(), creditor.getUsername(),
                            "Semua aset dialihkan akibat bangkrut");
    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
        "Bangkrut", "Player " + debtor.getUsername() + " bangkrut!\n"
        "Semua properti dan sisa uang " + debtor.getUsername() + " telah diserahkan kepada " + creditor.getUsername() + ".");
}

void BankruptcyManager::returnAssetsToBank(Player& debtor) {
    std::ostringstream detail;
    detail << "Uang sisa M" << debtor.getMoney() << " diserahkan ke Bank.\n";
    debtor.deductMoney(debtor.getMoney());

    std::vector<Property*> props = debtor.getOwnedProperties();
    for (Property* prop : props) {
        if (prop->getType() == PropertyType::STREET) {
            auto* sp = static_cast<StreetProperty*>(prop);
            if (sp->getBuildingLevel() != BuildingLevel::NONE) {
                sp->demolishBuildings();
                detail << "Bangunan di " << prop->getName()
                       << " dihancurkan.\n";
            }
        }
        bank.reclaim(prop);
    }

    logger.logAssetTransfer(debtor.getUsername(), "BANK", "Aset dikembalikan ke Bank (bangkrut)");
    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
        "Bangkrut", "Player " + debtor.getUsername() + " bangkrut!\n"
        "Semua properti " + debtor.getUsername() + " telah disita oleh Bank dan akan dilelang.\n"
        "Bangunan yang ada di properti tersebut telah dihancurkan.");
}

void BankruptcyManager::auctionAllBankProperties(Player& debtor) {
    std::vector<Property*> toAuction;
    for (Property* prop : bank.getBankProperties()) {
        if (!prop->getOwner()) {
            toAuction.push_back(prop);
        }
    }

    if (toAuction.empty()) {
        return;
    }

    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::INFO,
        "Lelang Aset",
        std::to_string(toAuction.size()) +
            " properti bank tersedia untuk dilelang.");

    for (Property* prop : toAuction) {
        auctionManager.startAuction(*prop, &debtor, true);
        if (engine.hasPendingContinuation()) {
            engine.chainPendingContinuation([this, &debtor]() {
                CommandResult resumed;
                auctionAllBankProperties(debtor);
                return resumed;
            });
            return;
        }
    }
}

bool BankruptcyManager::canLiquidate(const Player& player, int obligation) const {
    return computeMaxLiquidation(player) >= obligation;
}

void BankruptcyManager::handleDebt(Player& debtor, int obligation,
                                   Player* creditor) {
    pendingDebtor = &debtor;
    pendingCreditor = creditor;
    pendingObligation = obligation;

    const std::string creditorName = creditor ? creditor->getUsername() : "Bank";

    std::ostringstream summary;
    summary << "Tidak bisa bayar M" << obligation << " ke " << creditorName
            << "\nUang: M" << debtor.getMoney()
            << "\nKewajiban: M" << obligation
            << "\nKekurangan: M" << (obligation - debtor.getMoney())
            << "\n\nEstimasi likuidasi maksimum:\n";

    int maxLiquid = computeMaxLiquidation(debtor);
    for (Property* prop : debtor.getOwnedProperties()) {
        if (!prop || prop->isMortgaged()) {
            continue;
        }

        summary << "  " << prop->getName() << " -> jual M"
                << prop->getSellValue();
        if (propertyManager.canMortgage(debtor, *prop)) {
            summary << ", gadai M" << prop->getMortgageValue();
        }
        summary << "\n";
    }
    summary << "  Total potensi: M" << maxLiquid;

    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::ERROR,
        "Tidak Cukup Bayar", summary.str());

    while (debtor.getMoney() < obligation && canLiquidate(debtor, obligation)) {
        engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::WARNING,
            "Wajib Likuidasi",
            "Dana likuidasi cukup menutup kewajiban. Lanjutkan panel likuidasi.");
        runLiquidationPanel(debtor, obligation);
        if (engine.hasPendingContinuation()) {
            return;
        }
    }

    if (debtor.getMoney() >= obligation) {
        debtor.deductMoney(obligation);
        if (creditor) {
            creditor->addMoney(obligation);
            engine.pushEvent(GameEventType::MONEY, UiTone::SUCCESS,
                "Lunas",
                debtor.getUsername() + " berhasil membayar M" +
                    std::to_string(obligation) + " ke " + creditorName + ".");
        } else {
            engine.pushEvent(GameEventType::MONEY, UiTone::SUCCESS,
                "Pajak Lunas",
                "M" + std::to_string(obligation) + " dibayarkan ke Bank.");
        }
        clearPendingDebt();
        return;
    }

    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::ERROR,
        "BANGKRUT",
        debtor.getUsername() + " dinyatakan BANGKRUT.\nKreditor: " +
            creditorName);
    logger.logBankruptcy(debtor.getUsername(), creditorName);

    if (creditor) {
        transferAssetsToCreditor(debtor, *creditor);
    } else {
        returnAssetsToBank(debtor);
        auctionAllBankProperties(debtor);
    }

    debtor.setStatus(PlayerStatus::BANKRUPT);
    engine.getTurnManager().removePlayer(
        engine.getTurnManager().getCurrentPlayerIndex());

    engine.pushEvent(GameEventType::BANKRUPTCY, UiTone::ERROR,
        "Keluar Permainan",
        debtor.getUsername() + " keluar. Tersisa " +
            std::to_string(engine.getActivePlayers().size()) +
            " pemain aktif.");
    clearPendingDebt();
}
