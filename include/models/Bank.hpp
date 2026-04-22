#pragma once

#include <vector>
#include <string>

class Player;
class Property;

class Bank {
private:
    std::vector<Property*> bankProperties;

public:
    Bank() = default;

    void paySalary(Player& player, int amount);
    void receivePayment(Player& player, int amount);
    void sendMoney(Player& player, int amount);

    void registerProperty(Property* prop);
    void transferPropertyToPlayer(Property* prop, Player& player);
    void reclaim(Property* prop);
    bool ownsProperty(const Property* prop) const;

    const std::vector<Property*>& getBankProperties() const;
};