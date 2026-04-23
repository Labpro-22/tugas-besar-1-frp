#pragma once

#include <vector>
#include <string>
#include <map>

class Player;
class Property;
class StreetProperty;
class Bank;
class TransactionLogger;
class GameEngine;
class GameContext;
class Board;

class PropertyManager {
private:
    GameEngine& engine;
    Bank& bank;
    TransactionLogger& logger;

    std::vector<StreetProperty*> getColorGroup(const std::string& colorGroup) const;

    bool hasMonopoly(const Player& player, const std::string& colorGroup) const;
    bool hasBuildingsInColorGroup(const Player& player,
                                  const std::string& colorGroup) const;
    void sellAllBuildingsInColorGroup(Player& player,
                                      const std::string& colorGroup);
    int  computeRent(const Property& prop,const GameContext& ctx) const;

public:
    enum class BuildOption {
        NONE,
        HOUSE,
        HOTEL
    };

    PropertyManager(GameEngine& engine, Bank& bank, TransactionLogger& logger);

    bool offerPurchase(Player& buyer, Property& prop);
    void autoAcquire(Player& player, Property& prop);
    void payRent(Player& payer, Property& prop, const GameContext& ctx);
    bool mortgageProperty(Player& player, Property& prop);
    bool redeemProperty(Player& player, Property& prop);
    bool buildOnProperty(Player& player, StreetProperty& prop);
    BuildOption getBuildOption(const Player& player, const StreetProperty& prop) const;
    void sellPropertyToBank(Player& player, Property& prop);


    std::vector<Property*> getMortgageableProperties(const Player& player) const;
    std::vector<Property*> getMortgagedProperties(const Player& player) const;
    std::map<std::string, std::vector<StreetProperty*>>
        getBuildableColorGroups(const Player& player) const;
    std::vector<StreetProperty*> getBuildableTilesInGroup(
        const Player& player, const std::string& colorGroup) const;

    bool canMortgage(const Player& player, const Property& prop) const;
    bool canRedeem(const Player& player, const Property& prop) const;
    int estimateLiquidationValue(const Player& player) const;
};