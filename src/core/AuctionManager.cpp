#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/utils/GameException.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>

AuctionManager::AuctionManager(GameEngine& engine, Bank& bank,
                               TransactionLogger& logger)
    : engine(engine),
      bank(bank),
      logger(logger),
      auctionedProp(nullptr),
      highestBidder(nullptr),
      highestBid(0),
      consecutivePasses(0),
      atLeastOneBid(false) {}


std::vector<Player*> AuctionManager::buildAuctionOrder(
    Player* triggerPlayer, bool excludeBankrupt) const
{
    std::vector<Player*> allActive = engine.getActivePlayers();
    std::vector<Player*> order;

    int triggerIdx = -1;
    for (int i = 0; i < static_cast<int>(allActive.size()); i++) {
        if (allActive[i] == triggerPlayer) {
            triggerIdx = i;
            break;
        }
    }

    if (triggerIdx == -1) {
        return allActive;
    }

    int n = static_cast<int>(allActive.size());
    for (int i = 1; i <= n; i++) {
        int idx = (triggerIdx + i) % n;
        order.push_back(allActive[idx]);
    }

    return order;
}

void AuctionManager::resetState() {
    auctionedProp      = nullptr;
    highestBidder      = nullptr;
    highestBid         = 0;
    consecutivePasses  = 0;
    atLeastOneBid      = false;
}

bool AuctionManager::processTurn(Player& current, int totalActivePlayers) {
    int passesNeeded = totalActivePlayers - 1;

    std::cout << "\nGiliran: " << current.getUsername() << "\n";
    std::cout << "Penawaran tertinggi saat ini: M" << highestBid;
    if (highestBidder) {
        std::cout << " (" << highestBidder->getUsername() << ")";
    }
    std::cout << "\n";
    std::cout << "Uang kamu: M" << current.getMoney() << "\n";


    bool mustBid = (!atLeastOneBid &&
                    consecutivePasses == totalActivePlayers - 1);

    if (mustBid) {
        std::cout << "[INFO] Kamu wajib melakukan bid minimal sekali!\n";
    }

    std::cout << "Aksi (PASS / BID <jumlah>): ";

    std::string input;
    std::getline(std::cin >> std::ws, input);

    if (input == "PASS" || input == "pass") {
        if (mustBid) {
            std::cout << "Kamu wajib melakukan bid! Tidak bisa PASS sekarang.\n";
            return processTurn(current, totalActivePlayers);
        }
        consecutivePasses++;
        std::cout << current.getUsername() << " memilih PASS.\n";

        if (consecutivePasses >= passesNeeded && atLeastOneBid) {
            return true; 
            // Auction is over
        }
        return false;

    } else if (input.size() > 4 &&
               (input.substr(0, 4) == "BID " || input.substr(0, 4) == "bid ")) {
        int bidAmount = 0;
        try {
            bidAmount = std::stoi(input.substr(4));
        } catch (...) {
            std::cout << "Input tidak valid. Masukkan BID <angka> atau PASS.\n";
            return processTurn(current, totalActivePlayers);
        }

        if (bidAmount <= highestBid) {
            std::cout << "Bid harus lebih tinggi dari bid tertinggi saat ini (M"
                      << highestBid << ").\n";
            return processTurn(current, totalActivePlayers);
        }

        if (!current.canAfford(bidAmount)) {
            std::cout << "Uang kamu tidak cukup untuk bid M" << bidAmount
                      << ". Uang kamu: M" << current.getMoney() << "\n";
            return processTurn(current, totalActivePlayers);
        }

        // Update highest bid
        highestBid     = bidAmount;
        highestBidder  = &current;
        atLeastOneBid  = true;
        consecutivePasses = 0; 

        logger.logAuctionBid(current.getUsername(),
                             auctionedProp->getCode(), bidAmount);

        std::cout << "Penawaran tertinggi: M" << highestBid
                  << " (" << highestBidder->getUsername() << ")\n";
        return false;

    } else {
        std::cout << "Input tidak valid. Ketik PASS atau BID <jumlah>.\n";
        return processTurn(current, totalActivePlayers);
    }
}

void AuctionManager::finalizeAuction() {
    if (!highestBidder || !auctionedProp) return;

    bank.receivePayment(*highestBidder, highestBid);
    bank.transferPropertyToPlayer(auctionedProp, *highestBidder);

    logger.logAuctionResult(highestBidder->getUsername(),
                            auctionedProp->getCode(), highestBid);

    std::cout << "\n=== Lelang Selesai! ===\n";
    std::cout << "Pemenang  : " << highestBidder->getUsername() << "\n";
    std::cout << "Harga akhir: M" << highestBid << "\n";
    std::cout << "Properti " << auctionedProp->getName()
              << " (" << auctionedProp->getCode() << ") kini dimiliki "
              << highestBidder->getUsername() << ".\n";
}


void AuctionManager::startAuction(Property& prop,
                                   Player* triggerPlayer,
                                   bool excludeBankrupt)
{
    resetState();
    auctionedProp = &prop;

    std::cout << "\nProperti " << prop.getName()
              << " (" << prop.getCode() << ") akan dilelang!\n";

    std::vector<Player*> order = buildAuctionOrder(triggerPlayer, excludeBankrupt);

    if (order.empty()) {
        std::cout << "Tidak ada pemain yang bisa ikut lelang.\n";
        return;
    }

    int totalActive = static_cast<int>(order.size());

    std::cout << "Urutan lelang dimulai dari pemain setelah "
              << (triggerPlayer ? triggerPlayer->getUsername() : "N/A")
              << ".\n";

    bool auctionDone = false;
    while (!auctionDone) {
        for (Player* current : order) {
            if (!current || current->isBankrupt()) continue;

            bool done = processTurn(*current, totalActive);
            if (done) {
                auctionDone = true;
                break;
            }
        }

        if (!auctionDone && !atLeastOneBid) {
            continue;
        }
    }
    
    finalizeAuction();
}