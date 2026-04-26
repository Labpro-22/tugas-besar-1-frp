#pragma once

#include <string>
#include <vector>

class Player;
class Property;
class Bank;
class TransactionLogger;
class GameEngine;

class AuctionManager {
private:
    GameEngine& engine;
    Bank& bank;
    TransactionLogger& logger;

    Property* auctionedProp;
    Player* highestBidder;
    int highestBid;
    int consecutivePasses;
    bool atLeastOneBid;
    bool auctionActive;
    int currentAuctionIndex;
    std::string auctionTriggerName;
    std::vector<Player*> auctionOrder;

    std::vector<Player*> buildAuctionOrder(Player* triggerPlayer,
                                           bool excludeBankrupt) const;
    void resetState();
    void continueAuction();
    void finalizeAuction();

public:
    AuctionManager(GameEngine& engine, Bank& bank, TransactionLogger& logger);
    void startAuction(Property& prop,
                      Player* triggerPlayer,
                      bool excludeBankrupt = false);
};
