#pragma once

#include "ActionCard.hpp"

class GoToNearestRailroadCard : public ActionCard {
public:
    GoToNearestRailroadCard();
    void apply(Player& player, GameEngine& game) override;
};

class MoveBackCard : public ActionCard {
private:
    int steps;

public:
    explicit MoveBackCard(int steps = 3);
    void apply(Player& player, GameEngine& game) override;
};

class GoToJailCard : public ActionCard {
public:
    GoToJailCard();
    void apply(Player& player, GameEngine& game) override;
};

class GetOutOfJailCard : public ActionCard {
public:
    GetOutOfJailCard();
    InventoryCardState getInventoryState() const override;
    void apply(Player& player, GameEngine& game) override;
};

class GoToNearestFestivalCard : public ActionCard {
public:
    GoToNearestFestivalCard();
    void apply(Player& player, GameEngine& game) override;
};
