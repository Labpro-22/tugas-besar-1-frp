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

int minimumValidBid(int highestBid) {
    return (highestBid < 0) ? 0 : (highestBid + 1);
}
}

AuctionManager::AuctionManager(GameEngine& engine, Bank& bank,
                               TransactionLogger& logger)
    : engine(engine),
      bank(bank),
      logger(logger),
      auctionedProp(nullptr),
      highestBidder(nullptr),
      highestBid(-1),
      consecutivePasses(0),
      atLeastOneBid(false),
      auctionActive(false),
      currentAuctionIndex(0),
      auctionTriggerName("Bank") {}

std::vector<Player*> AuctionManager::buildAuctionOrder(
    Player* triggerPlayer, bool excludeBankrupt) const {
    const auto& allPlayers = engine.getPlayers();
    const auto& turnOrder = engine.getTurnManager().getTurnOrder();
    if (turnOrder.empty()) {
        return {};
    }

    int startOrderIndex = engine.getTurnManager().getCurrentOrderIndex();
    if (triggerPlayer != nullptr) {
        int triggerPlayerIndex = -1;
        for (int i = 0; i < static_cast<int>(allPlayers.size()); ++i) {
            if (allPlayers[i] == triggerPlayer) {
                triggerPlayerIndex = i;
                break;
            }
        }

        if (triggerPlayerIndex >= 0) {
            for (int i = 0; i < static_cast<int>(turnOrder.size()); ++i) {
                if (turnOrder[i] == triggerPlayerIndex) {
                    startOrderIndex = (i + 1) % static_cast<int>(turnOrder.size());
                    break;
                }
            }
        } else {
            startOrderIndex = (startOrderIndex + 1) % static_cast<int>(turnOrder.size());
        }
    }

    std::vector<Player*> order;
    order.reserve(turnOrder.size());
    for (int offset = 0; offset < static_cast<int>(turnOrder.size()); ++offset) {
        const int orderIndex = (startOrderIndex + offset) % static_cast<int>(turnOrder.size());
        const int playerIndex = turnOrder[orderIndex];
        if (playerIndex < 0 || playerIndex >= static_cast<int>(allPlayers.size())) {
            continue;
        }

        Player* candidate = allPlayers[playerIndex];
        if (!candidate) {
            continue;
        }
        if (candidate->isJailed()) {
            continue;
        }
        if (excludeBankrupt && candidate->isBankrupt()) {
            continue;
        }
        if (!excludeBankrupt || !candidate->isBankrupt()) {
            order.push_back(candidate);
        }
    }

    if (order.empty() && !excludeBankrupt) {
        for (Player* player : allPlayers) {
            if (player) {
                if (player->isJailed()) {
                    continue;
                }
                order.push_back(player);
            }
        }
    }
    return order;
}

void AuctionManager::resetState() {
    auctionedProp = nullptr;
    highestBidder = nullptr;
    highestBid = -1;
    consecutivePasses = 0;
    atLeastOneBid = false;
    auctionActive = false;
    currentAuctionIndex = 0;
    auctionTriggerName = "Bank";
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

    const int winningBid = std::max(0, highestBid);
    bank.receivePayment(*highestBidder, highestBid);
    bank.transferPropertyToPlayer(auctionedProp, *highestBidder);
    logger.logAuctionResult(highestBidder->getUsername(),
                            auctionedProp->getCode(), winningBid);
    engine.pushEvent(GameEventType::AUCTION, UiTone::SUCCESS,
        "Lelang Selesai",
        "Properti " + auctionedProp->getName() + " terjual kepada " +
            highestBidder->getUsername() + " dengan harga M" +
            std::to_string(winningBid) + ".\n" +
            highestBidder->getUsername() + " membayar M" +
            std::to_string(winningBid) + " dan sisa uangnya sekarang M" +
            std::to_string(highestBidder->getMoney()) + ".");
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
        if (!current || current->isBankrupt() || current->isJailed()) {
            currentAuctionIndex =
                (currentAuctionIndex + 1) % static_cast<int>(auctionOrder.size());
            continue;
        }

        const int totalActive = static_cast<int>(auctionOrder.size());
        const int passesNeeded = std::max(0, totalActive - 1);
        const bool mustBid = (!atLeastOneBid && consecutivePasses >= passesNeeded);
        const int shownHighestBid = std::max(0, highestBid);
        const int minBid = minimumValidBid(highestBid);

        std::ostringstream info;
        info << "Objek lelang: " << auctionedProp->getName()
             << " (" << auctionedProp->getCode() << ")\n"
             << "Pemicu lelang: " << auctionTriggerName << "\n"
             << "--\n\nPenawaran tertinggi saat ini: M" << shownHighestBid;
        if (highestBidder) {
            info << " (" << highestBidder->getUsername() << ")";
        }
        info << "\nBid minimum saat ini: M" << minBid
             << "\n\nGiliran " << current->getUsername()
             << " (Uang: M" << current->getMoney() << "):\n";
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
            options.push_back("BID_MIN");

            std::ostringstream promptMsg;
            promptMsg << "Properti: " << auctionedProp->getName()
                      << " (" << auctionedProp->getCode() << ")\n"
                      << "Pemicu: " << auctionTriggerName << "\n"
                      << "Penawaran tertinggi: M" << shownHighestBid;
            if (highestBidder) {
                promptMsg << " oleh " << highestBidder->getUsername();
            } else {
                promptMsg << " (belum ada penawaran)";
            }
            promptMsg << "\nBid minimum saat ini: M" << minBid;
            promptMsg << "\nGiliran: " << current->getUsername()
                      << " (Uang: M" << current->getMoney() << ")\n"
                      << "Pilih aksi: PASS / BID_MIN / BID <angka>";
            engine.pushPrompt(promptKey, promptMsg.str(), options);
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

        if (input.rfind("BID ", 0) == 0 || input == "BID_MIN") {
            int bidAmount = 0;
            if (input == "BID_MIN") {
                bidAmount = minBid;
            } else {
                try {
                    bidAmount = std::stoi(input.substr(4));
                } catch (const std::exception&) {
                    engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                        "Input Tidak Valid", "Format bid harus: BID <angka>.");
                    continue;
                }
            }

            if (bidAmount < minBid) {
                engine.pushEvent(GameEventType::AUCTION, UiTone::WARNING,
                    "Bid Terlalu Rendah",
                    "Bid harus minimal M" + std::to_string(minBid) + ".");
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
        auctionTriggerName =
            (triggerPlayer != nullptr) ? triggerPlayer->getUsername() : "Bank";
        auctionOrder = buildAuctionOrder(triggerPlayer, excludeBankrupt);
        auctionActive = true;
        currentAuctionIndex = 0;

        engine.pushEvent(GameEventType::AUCTION, UiTone::INFO,
            "Lelang Dimulai",
            "Pemicu: " + auctionTriggerName + "\n"
            "Objek: " + prop.getName() + " (" + prop.getCode() + ")\n\n"
            "Lelang dimulai!");
    }

    continueAuction();
}
