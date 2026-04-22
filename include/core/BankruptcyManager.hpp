#pragma once

#include <string>
#include <vector>

class Player;
class Property;
class Bank;
class TransactionLogger;
class PropertyManager;
class AuctionManager;
class GameEngine;

class BankruptcyManager {
private:
    GameEngine& engine;
    Bank& bank;
    TransactionLogger& logger;
    PropertyManager& propertyManager;
    AuctionManager& auctionManager;

    Player* pendingDebtor;
    Player* pendingCreditor;
    int pendingObligation;

    int computeMaxLiquidation(const Player& player) const;
    void runLiquidationPanel(Player& player, int obligation);
    void transferAssetsToCreditor(Player& debtor, Player& creditor);
    void returnAssetsToBank(Player& debtor);
    void auctionAllBankProperties(Player& debtor);
    void clearPendingDebt();

public:
    BankruptcyManager(GameEngine& engine,
                      Bank& bank,
                      TransactionLogger& logger,
                      PropertyManager& propertyManager,
                      AuctionManager& auctionManager);

    void handleDebt(Player& debtor, int obligation, Player* creditor);
    bool canLiquidate(const Player& player, int obligation) const;
};
