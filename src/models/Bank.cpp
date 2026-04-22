#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/utils/GameException.hpp"

#include <algorithm>

// Transaction
void Bank::paySalary(Player& player, int amount) {
    player.addMoney(amount);
}

void Bank::receivePayment(Player& player, int amount) {
    if (!player.canAfford(amount)) {
        throw InsufficientFundsException(
            player.getUsername(), amount, player.getMoney());
    }
    player.deductMoney(amount);
}

void Bank::sendMoney(Player& player, int amount) {
    player.addMoney(amount);
}


// Property Management
void Bank::registerProperty(Property* prop) {
    if (!prop) return;
    bankProperties.push_back(prop);
}

void Bank::transferPropertyToPlayer(Property* prop, Player& player) {
    if (!prop) return;

    auto it = std::find(bankProperties.begin(), bankProperties.end(), prop);
    if (it != bankProperties.end()) {
        bankProperties.erase(it);
    }

    prop->setOwner(&player);
    player.addProperty(prop);
}

void Bank::reclaim(Property* prop) {
    if (!prop) return;

    Player* prevOwner = prop->getOwner();
    if (prevOwner) {
        prevOwner->removeProperty(prop);
    }

    prop->setOwner(nullptr); 

    auto it = std::find(bankProperties.begin(), bankProperties.end(), prop);
    if (it == bankProperties.end()) {
        bankProperties.push_back(prop);
    }
}

bool Bank::ownsProperty(const Property* prop) const {
    auto it = std::find(bankProperties.begin(), bankProperties.end(), prop);
    return it != bankProperties.end();
}

const std::vector<Property*>& Bank::getBankProperties() const {
    return bankProperties;
}