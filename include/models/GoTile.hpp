#pragma once

#include "Tile.hpp"

class GoTile : public Tile{
private:
    int salary;

public:
    GoTile(int index, int salary);

    int getSalary() const;

    void onLand(Player& player, GameEngine& engine) override;
};