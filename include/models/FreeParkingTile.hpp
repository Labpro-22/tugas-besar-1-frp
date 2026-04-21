#pragma once

#include "Tile.hpp"


class FreeParkingTile : public Tile {
public:
    explicit FreeParkingTile(int index);

    void onLand(Player& player, GameEngine& engine) override;
};