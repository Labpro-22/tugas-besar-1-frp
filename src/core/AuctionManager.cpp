#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {
std::string normalizeAuctionInput(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}
}

AuctionManager::AuctionManager(GameEngine& engine, Bank& bank,
                               TransactionLogger& logger)
    : engine(engine),
      bank(bank),
      logger(logger),
      auctionedProp(nullptr),
      highestBidder(nullptr),
      highestBid(0),
      consecutivePasses(0),
      atLeastOneBid(false),
      auctionActive(false),
      currentAuctionIndex(0) {}

std::vector<Player*> AuctionManager::buildAuctionOrder(
    Player* triggerPlayer, bool excludeBankrupt) const {
    (void)excludeBankrupt;

    std::vector<Player*> allActive = engine.getActivePlayers();
    int triggerIdx = -1;
    for (int i = 0; i < static_cast<int>(allActive.size()); ++i) {
        if (allActive[i] == triggerPlayer) {
            triggerIdx = i;
            break;
        }
    }

    if (triggerIdx == -1) {
        return allActive;
    }

    std::vector<Player*> order;
    const int n = static_cast<int>(allActive.size());
    for (int i = 1; i <= n; ++i) {
        order.push_back(allActive[(triggerIdx + i) % n]);
    }
    return order;
}

void AuctionManager::resetState() {
    auctionedProp = nullptr;
    highestBidder = nullptr;
    highestBid = 0;
    consecutivePasses = 0;
    atLeastOneBid = false;
    auctionActive = false;
    currentAuctionIndex = 0;
    auctionOrder.clear();
}

void AuctionManager::finalizeAuction() {
    if (!highestBidder || !auctionedProp) {
        engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
            "Lelang Berakhir",
            "Lelang selesai tanpa pemenang.");
        resetState();
        return;
    }

    bank.receivePayment(*highestBidder, highestBid);
    bank.transferPropertyToPlayer(auctionedProp, *highestBidder);
    logger.logAuctionResult(highestBidder->getUsername(),
                            auctionedProp->getCode(), highestBid);
    engine.pushEvent(GameEventType::AUCTION, UiTone::SUCCESS,
        "Lelang Selesai",
        "Properti " + auctionedProp->getName() + " terjual kepada " + highestBidder->getUsername() + " dengan harga M" + std::to_string(highestBid) + "!");
    resetState();
}

void AuctionManager::continueAuction() {
    if (!auctionActive || !auctionedProp) {
        return;
    }

    while (auctionActive) {
        if (auctionOrder.empty()) {
            engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                "Lelang Batal", "Tidak ada pemain yang dapat ikut lelang.");
            resetState();
            return;
        }

        if (currentAuctionIndex >= static_cast<int>(auctionOrder.size())) {
            currentAuctionIndex = 0;
        }

        Player* current = auctionOrder[currentAuctionIndex];
        if (!current || current->isBankrupt()) {
            currentAuctionIndex =
                (currentAuctionIndex + 1) % static_cast<int>(auctionOrder.size());
            continue;
        }

        const int totalActive = static_cast<int>(auctionOrder.size());
        const int passesNeeded = std::max(0, totalActive - 1);
        const bool mustBid = (!atLeastOneBid && consecutivePasses >= passesNeeded);

        std::ostringstream info;
        info << "--\n\nPenawaran tertinggi saat ini: M" << highestBid;
        if (highestBidder) {
            info << " (" << highestBidder->getUsername() << ")";
        }
        info << "\n\nGiliran " << current->getUsername() << " (Uang: M" << current->getMoney() << "):\n";
        if (mustBid) {
            info << "[INFO] Pemain ini wajib BID minimal sekali.\n";
        }
        engine.pushEvent(GameEventType::AUCTION, UiTone::INFO,
            "Giliran Lelang", info.str());

        const std::string promptKey =
            "lelang_" + current->getUsername() + "_" +
            std::to_string(currentAuctionIndex) + "_" +
            std::to_string(highestBid) + "_" +
            std::to_string(consecutivePasses);

        if (!engine.hasPromptAnswer(promptKey)) {
            std::vector<std::string> options;
            if (!mustBid) {
                options.push_back("PASS");
            }
            options.push_back("BID <jumlah>");

            engine.pushPrompt(
                promptKey,
                "Apakah ingin mengajukan tawaran? (B/P):",
                options);
            engine.setPendingContinuation([this]() {
                CommandResult resumed;
                continueAuction();
                return resumed;
            });
            return;
        }

        const std::string input =
            normalizeAuctionInput(engine.consumePromptAnswer(promptKey));

        if (input == "PASS") {
            if (mustBid) {
                engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                    "Tidak Bisa PASS", "Minimal harus ada satu bid di lelang ini.");
                continue;
            }

            ++consecutivePasses;
            engine.pushEvent(GameEventType::AUCTION, UiTone::INFO,
                "PASS", current->getUsername() + " memilih untuk pass.");

            if (atLeastOneBid && consecutivePasses >= passesNeeded) {
                finalizeAuction();
                return;
            }

            currentAuctionIndex =
                (currentAuctionIndex + 1) % static_cast<int>(auctionOrder.size());
            continue;
        }

        if (input.rfind("BID ", 0) == 0) {
            int bidAmount = 0;
            try {
                bidAmount = std::stoi(input.substr(4));
            } catch (const std::exception&) {
                engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                    "Input Tidak Valid", "Format bid harus: BID <angka>.");
                continue;
            }

            if (bidAmount <= highestBid) {
                engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                    "Bid Terlalu Rendah",
                    "Bid harus lebih besar dari M" +
                        std::to_string(highestBid) + ".");
                continue;
            }

            if (!current->canAfford(bidAmount)) {
                engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                    "Uang Tidak Cukup",
                    "Bid M" + std::to_string(bidAmount) +
                        " melebihi uang tunai pemain.");
                continue;
            }

            highestBid = bidAmount;
            highestBidder = current;
            atLeastOneBid = true;
            consecutivePasses = 0;
            logger.logAuctionBid(current->getUsername(),
                                 auctionedProp->getCode(), bidAmount);
            engine.pushEvent(GameEventType::AUCTION, UiTone::SUCCESS,
                "Bid Diterima",
                current->getUsername() + " mengajukan tawaran sebesar M" + std::to_string(bidAmount) + "!");

            currentAuctionIndex =
                (currentAuctionIndex + 1) % static_cast<int>(auctionOrder.size());
            continue;
        }

        engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
            "Input Tidak Valid", "Masukkan PASS atau BID <jumlah>.");
    }
}

void AuctionManager::startAuction(Property& prop,
                                  Player* triggerPlayer,
                                  bool excludeBankrupt) {
    if (!auctionActive) {
        resetState();
        auctionedProp = &prop;
        auctionOrder = buildAuctionOrder(triggerPlayer, excludeBankrupt);
        auctionActive = true;
        currentAuctionIndex = 0;

        engine.pushEvent(GameEventType::AUCTION, UiTone::INFO,
            "Lelang Dimulai",
            "Properti " + prop.getName() + " masuk sistem lelang...\n\nLelang dimulai!");
    }

    continueAuction();
}
