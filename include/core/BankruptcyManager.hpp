#pragma once

#include <cstddef>
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
    std::vector<Property*> returnAssetsToBank(Player& debtor);
    void auctionBankruptAssets(const std::vector<Property*>& properties,
                               std::size_t startIndex = 0);
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
