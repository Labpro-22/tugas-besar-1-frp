#pragma once

#include <string>

class Player;
class GameEngine;

enum class InventoryCardState {
    NONE,
    SKILL,
    CHANCE_SPECIAL
};

class Card {
protected:
    std::string description;

public:
    explicit Card(const std::string& description);
    virtual ~Card() = default;

    virtual void apply(Player& player, GameEngine& game) = 0;
    virtual InventoryCardState getInventoryState() const;

    std::string getDescription() const;
};
