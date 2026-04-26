#include "../../include/models/Player.hpp"
#include "../../include/models/Card.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/utils/GameException.hpp"

Player::Player(const string& username, int initialMoney)
    : username(username), money(initialMoney),
      position(0),
      status(PlayerStatus::ACTIVE),
      hasUsedSkillThisTurn_(false),
      shieldActive(false),
      hasJailFreeChanceCard_(false),
      jailTurns(0),
      consecutiveDoubles(0),
      discountPercent(0) {}

void Player::move(int steps, int boardSize) {
    if (isBankrupt()) throw PlayerBankruptException(username);
    position = (position + steps + boardSize) % boardSize;
}

void Player::setPosition(int pos) { position = pos; }
int  Player::getPosition()  const { return position; }

void Player::addMoney(int amount) {
    if (amount < 0) return;
    money += amount;
}

void Player::deductMoney(int amount) {
    if (amount < 0) return;
    money -= amount;
}

bool Player::canAfford(int amount) const { return money >= amount; }
int  Player::getMoney()             const { return money;           }

Player& Player::operator+=(int amount) {
    addMoney(amount);
    return *this;
}

Player& Player::operator-=(int amount) {
    deductMoney(amount);
    return *this;
}

bool Player::operator>(const Player& other) const {
    int tw = getTotalWealth();
    int ow = other.getTotalWealth();
    if (tw != ow) return tw > ow;

    if (countProperties() != other.countProperties())
        return countProperties() > other.countProperties();

    return countCards() > other.countCards();
}

bool Player::operator<(const Player& other) const {
    return other.operator>(*this);
}

void Player::addProperty(Property* prop) {
    if (!prop) return;
    ownedProperties.push_back(prop);
}

void Player::removeProperty(Property* prop) {
    for (auto it = ownedProperties.begin(); it != ownedProperties.end(); ++it) {
        if (*it == prop) {
            ownedProperties.erase(it);
            return;
        }
    }
}

const vector<Property*>& Player::getOwnedProperties() const {
    return ownedProperties;
}

int Player::countProperties() const {
    return static_cast<int>(ownedProperties.size());
}

void Player::addCard(std::shared_ptr<SkillCard> card) {
    if (static_cast<int>(handCards.size()) >= 3) {
        throw HandFullException(username);
    }
    handCards.push_back(card);
}

void Player::removeCard(int idx) {
    if (idx < 0 || idx >= static_cast<int>(handCards.size())) {
        throw InvalidCardIndexException(idx);
    }
    handCards.erase(handCards.begin() + idx);
}

int Player::countCards() const {
    return static_cast<int>(handCards.size());
}

const std::vector<std::shared_ptr<SkillCard>>& Player::getHandCards() const {
    return handCards;
}
bool Player::isBankrupt() const { return status == PlayerStatus::BANKRUPT; }
bool Player::isJailed() const { return status == PlayerStatus::JAILED; }
bool Player::isActive() const { return status == PlayerStatus::ACTIVE; }
PlayerStatus Player::getStatus() const { return status; }

void Player::setStatus(PlayerStatus s) { status = s; }

int  Player::getJailTurns() const { return jailTurns; }
void Player::setJailTurns(int t) { jailTurns = t;}
void Player::incrementJailTurns() { jailTurns++; }

int  Player::getConsecutiveDoubles() const { return consecutiveDoubles; }
void Player::setConsecutiveDoubles(int n) { consecutiveDoubles = n; }
void Player::incrementConsecutiveDoubles() { consecutiveDoubles++;}
void Player::resetConsecutiveDoubles() { consecutiveDoubles = 0; }

bool Player::isShieldActive() const { return shieldActive; }
void Player::setShieldActive(bool val) { shieldActive = val; }

bool Player::hasUsedSkillThisTurn() const { return hasUsedSkillThisTurn_; }
void Player::setUsedSkillThisTurn(bool val) { hasUsedSkillThisTurn_ = val;  }

int  Player::getDiscountPercent() const { return discountPercent; }
void Player::setDiscountPercent(int percent) { discountPercent = percent; }

int Player::getJailFreeCardCount() const { return hasJailFreeChanceCard_ ? 1 : 0; }
bool Player::hasJailFreeCard() const { return hasJailFreeChanceCard_; }
void Player::addJailFreeCard() { hasJailFreeChanceCard_ = true; }
bool Player::canStoreInSpecialInventorySlot(const Card& card) const {
    return card.getInventoryState() == InventoryCardState::CHANCE_SPECIAL;
}

bool Player::storeCardInSpecialInventorySlot(const Card& card) {
    if (!canStoreInSpecialInventorySlot(card)) {
        throw GameException(
            "Slot inventory terakhir hanya boleh diisi kartu kesempatan.");
    }
    if (hasJailFreeChanceCard_) {
        return false;
    }
    hasJailFreeChanceCard_ = true;
    return true;
}

bool Player::consumeJailFreeCard() {
    if (!hasJailFreeChanceCard_) return false;
    hasJailFreeChanceCard_ = false;
    return true;
}

int Player::getAssetValue() const {
    int total = 0;
    for (const Property* prop : ownedProperties) {
        if (!prop) {
            continue;
        }

        total += prop->getPurchasePrice();
        if (const auto* street = dynamic_cast<const StreetProperty*>(prop)) {
            total += street->getBuildingSellValue() * 2;
        }
    }
    return total;
}

int Player::getTotalWealth() const {
    int total = money;
    for (const Property* prop : ownedProperties) {
        if (prop) total += prop->getSellValue();
    }
    return total;
}

string Player::getUsername() const { return username; }
