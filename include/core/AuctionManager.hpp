#pragma once

#include <vector>
#include <string>

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

    std::vector<Player*> buildAuctionOrder(Player* triggerPlayer,
                                           bool excludeBankrupt) const;


    void resetState();
    bool processTurn(Player& current, int totalActivePlayers);
    void finalizeAuction();

public:
    AuctionManager(GameEngine& engine, Bank& bank, TransactionLogger& logger);
    void startAuction(Property& prop,
                      Player*   triggerPlayer,
                      bool      excludeBankrupt = false);
};