#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/utils/GameException.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>

BankruptcyManager::BankruptcyManager(GameEngine& engine,
                                     Bank& bank,
                                     TransactionLogger& logger,
                                     PropertyManager& propertyManager,
                                     AuctionManager& auctionManager)
    : engine(engine),
      bank(bank),
      logger(logger),
      propertyManager(propertyManager),
      auctionManager(auctionManager) {}


int BankruptcyManager::computeMaxLiquidation(const Player& player) const {
    int total = player.getMoney();
    for (Property* prop : player.getOwnedProperties()) {
        if (prop->isMortgaged()) continue; 
        total += prop->getSellValue();     
    }
    return total;
}

void BankruptcyManager::runLiquidationPanel(Player& player, int obligation) {
    std::cout << "\n=== Panel Likuidasi ===\n";

    while (player.getMoney() < obligation) {
        std::cout << "Uang kamu saat ini: M" << player.getMoney()
                  << " | Kewajiban: M" << obligation << "\n";

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

        int optionNum = 1;
        std::vector<std::pair<int, Property*>> sellOptions;
        std::vector<std::pair<int, Property*>> mortgageOptions;

        if (!sellable.empty()) {
            std::cout << "\n[Jual ke Bank]\n";
            for (Property* prop : sellable) {
                int sellVal = prop->getSellValue();
                std::cout << optionNum << ". " << prop->getName()
                          << " (" << prop->getCode() << ")"
                          << " Harga Jual: M" << sellVal;
                if (prop->getType() == PropertyType::STREET) {
                    StreetProperty* sp = static_cast<StreetProperty*>(prop);
                    int bVal = sp->getBuildingSellValue();
                    if (bVal > 0) {
                        std::cout << " (termasuk bangunan: M" << bVal << ")";
                    }
                }
                std::cout << "\n";
                sellOptions.push_back({optionNum++, prop});
            }
        }

        if (!mortgageable.empty()) {
            std::cout << "\n[Gadaikan]\n";
            for (Property* prop : mortgageable) {
                std::cout << optionNum << ". " << prop->getName()
                          << " (" << prop->getCode() << ")"
                          << " Nilai Gadai: M" << prop->getMortgageValue()
                          << "\n";
                mortgageOptions.push_back({optionNum++, prop});
            }
        }

        if (sellOptions.empty() && mortgageOptions.empty()) {
            std::cout << "Tidak ada properti yang bisa dilikuidasi.\n";
            break;
        }

        std::cout << "\nPilih aksi (0 jika sudah cukup): ";
        int choice;
        std::cin >> choice;

        if (choice == 0) break;

        bool found = false;
        for (auto& [num, prop] : sellOptions) {
            if (num == choice) {
                int sellVal = prop->getSellValue();
                propertyManager.sellPropertyToBank(player, *prop);
                std::cout << prop->getName() << " terjual ke Bank. "
                          << "Kamu menerima M" << sellVal << ".\n";
                std::cout << "Uang kamu sekarang: M" << player.getMoney() << "\n";
                found = true;
                break;
            }
        }

        if (!found) {
            for (auto& [num, prop] : mortgageOptions) {
                if (num == choice) {
                    propertyManager.mortgageProperty(player, *prop);
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cout << "Pilihan tidak valid.\n";
        }
    }
}

void BankruptcyManager::transferAssetsToCreditor(Player& debtor,
                                                  Player& creditor) {
    std::cout << "\nPengalihan aset ke " << creditor.getUsername() << ":\n";

    int remainingMoney = debtor.getMoney();
    if (remainingMoney > 0) {
        std::cout << "- Uang tunai sisa : M" << remainingMoney << "\n";
        creditor.addMoney(remainingMoney);
        debtor.deductMoney(remainingMoney);
    }

    std::vector<Property*> props = debtor.getOwnedProperties();
    for (Property* prop : props) {
        std::cout << "- " << prop->getName()
                  << " (" << prop->getCode() << ") ";
        if (prop->isMortgaged()) {
            std::cout << "MORTGAGED [M]";
        } else if (prop->getType() == PropertyType::STREET) {
            StreetProperty* sp = static_cast<StreetProperty*>(prop);
            int lvl = sp->getBuildingCount();
            if (lvl == 0) std::cout << "OWNED";
            else if (lvl == 5) std::cout << "OWNED (hotel)";
            else std::cout << "OWNED (" << lvl << " rumah)";
        } else {
            std::cout << "OWNED";
        }
        std::cout << "\n";

        debtor.removeProperty(prop);
        prop->setOwner(&creditor);
        creditor.addProperty(prop);
    }

    logger.logAssetTransfer(debtor.getUsername(), creditor.getUsername(),
                            "Semua aset dialihkan akibat kebangkrutan");

    std::cout << creditor.getUsername()
              << " menerima semua aset " << debtor.getUsername() << ".\n";
}

void BankruptcyManager::returnAssetsToBank(Player& debtor) {
    std::cout << "\nUang sisa M" << debtor.getMoney()
              << " diserahkan ke Bank.\n";

    debtor.deductMoney(debtor.getMoney());

    std::vector<Property*> props = debtor.getOwnedProperties();
    for (Property* prop : props) {
        if (prop->getType() == PropertyType::STREET) {
            StreetProperty* sp = static_cast<StreetProperty*>(prop);
            if (sp->getBuildingLevel() != BuildingLevel::NONE) {
                sp->demolishBuildings();
                std::cout << "Bangunan di " << prop->getName()
                          << " dihancurkan.\n";
            }
        }
        bank.reclaim(prop);
    }

    std::cout << "Seluruh properti dikembalikan ke status BANK.\n";
}

void BankruptcyManager::auctionAllBankProperties(Player& debtor) {
    std::vector<Property*> toAuction;
    for (Property* prop : bank.getBankProperties()) {
        if (!prop->getOwner()) {
            toAuction.push_back(prop);
        }
    }

    if (toAuction.empty()) return;

    std::cout << "\nProperti akan dilelang satu per satu:\n";
    for (Property* prop : toAuction) {
        std::cout << "-> Lelang: " << prop->getName()
                  << " (" << prop->getCode() << ") ...\n";
        auctionManager.startAuction(*prop, &debtor, true);
    }
}


bool BankruptcyManager::canLiquidate(const Player& player,
                                      int obligation) const
{
    return computeMaxLiquidation(player) >= obligation;
}

void BankruptcyManager::handleDebt(Player& debtor, int obligation,
                                    Player* creditor)
{
    std::string creditorName = creditor ? creditor->getUsername() : "Bank";

    std::cout << "\nKamu tidak dapat membayar M" << obligation
              << " kepada " << creditorName << "!\n";
    std::cout << "Uang kamu       : M" << debtor.getMoney() << "\n";
    std::cout << "Total kewajiban : M" << obligation << "\n";
    std::cout << "Kekurangan      : M"
              << (obligation - debtor.getMoney()) << "\n";

    // Liquidation Estimation
    int maxLiquid = computeMaxLiquidation(debtor);
    std::cout << "\nEstimasi dana maksimum dari likuidasi:\n";

    const auto& props = debtor.getOwnedProperties();
    for (Property* prop : props) {
        if (prop->isMortgaged()) continue;
        std::cout << "  Jual " << prop->getName()
                  << " (" << prop->getCode() << ") -> M"
                  << prop->getSellValue() << "\n";
    }
    std::cout << "  Total potensi -> M" << maxLiquid << "\n";

    
    if (canLiquidate(debtor, obligation)) {
        std::cout << "\nDana likuidasi dapat menutup kewajiban.\n";
        std::cout << "Kamu wajib melikuidasi aset untuk membayar.\n";

        runLiquidationPanel(debtor, obligation);

        if (debtor.getMoney() >= obligation) {
            debtor.deductMoney(obligation);
            if (creditor) {
                creditor->addMoney(obligation);
                std::cout << "\nKewajiban M" << obligation
                          << " terpenuhi. Membayar ke "
                          << creditor->getUsername() << "...\n";
                std::cout << "Uang kamu        : M" << debtor.getMoney() << "\n";
                std::cout << "Uang " << creditor->getUsername()
                          << ": M" << creditor->getMoney() << "\n";
            } else {
                std::cout << "\nPajak M" << obligation
                          << " dibayarkan ke Bank.\n";
            }
            return; 
        }
    }

    
    std::cout << "\nTotal aset + uang tunai : M" << maxLiquid << "\n";
    std::cout << "Tidak cukup untuk menutup kewajiban M"
              << obligation << ".\n\n";

    std::cout << debtor.getUsername() << " dinyatakan BANGKRUT!\n";
    std::cout << "Kreditor: " << creditorName << "\n";

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

    std::cout << "\n" << debtor.getUsername()
              << " telah keluar dari permainan.\n";

    auto activePlayers = engine.getActivePlayers();
    std::cout << "Permainan berlanjut dengan "
              << activePlayers.size() << " pemain tersisa.\n";
}