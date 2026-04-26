#pragma once

#include <string>
#include <vector>
#include <memory>

using namespace std;

class Property;
class Card;
class SkillCard; 
enum class PlayerStatus {
    ACTIVE,
    JAILED,
    BANKRUPT
};

class Player {
private:
    string username;
    int money;
    int position;
    PlayerStatus status;

    vector<Property*> ownedProperties;
    vector<std::shared_ptr<SkillCard>> handCards;
    bool hasUsedSkillThisTurn_;
    bool shieldActive;
    bool hasJailFreeChanceCard_;
    int  jailTurns;
    int  consecutiveDoubles;
    int  discountPercent;
public:
    explicit Player(const string& username, int initialMoney = 0);
    void move(int steps, int boardSize);
    void setPosition(int pos);
    int  getPosition() const;

    void addMoney(int amount);
    void deductMoney(int amount);
    bool canAfford(int amount) const;
    int  getMoney() const;

    Player& operator+=(int amount);
    Player& operator-=(int amount);

    bool operator>(const Player& other) const;
    bool operator<(const Player& other) const;
    void addProperty(Property* prop);
    void removeProperty(Property* prop);

    const vector<Property*>& getOwnedProperties() const;
    int  countProperties() const;
    void addCard(std::shared_ptr<SkillCard> card);
    void removeCard(int idx);
    int  countCards() const;
    const vector<std::shared_ptr<SkillCard>>& getHandCards() const;

    bool isBankrupt() const;
    bool isJailed() const;
    bool isActive() const;
    PlayerStatus getStatus() const;
    void setStatus(PlayerStatus s);

    int  getJailTurns() const;
    void setJailTurns(int t);
    void incrementJailTurns();

    int  getConsecutiveDoubles() const;
    void setConsecutiveDoubles(int n);
    void incrementConsecutiveDoubles();
    void resetConsecutiveDoubles();

    bool isShieldActive() const;
    void setShieldActive(bool val);

    bool hasUsedSkillThisTurn() const;
    void setUsedSkillThisTurn(bool val);

    int  getDiscountPercent() const;
    void setDiscountPercent(int pct);

    int  getJailFreeCardCount() const;
    bool hasJailFreeCard() const;
    void addJailFreeCard();
    bool canStoreInSpecialInventorySlot(const Card& card) const;
    bool storeCardInSpecialInventorySlot(const Card& card);
    bool consumeJailFreeCard();

    int getAssetValue() const;
    int getTotalWealth() const;
    string getUsername() const;


};
