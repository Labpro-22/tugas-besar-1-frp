#pragma once

#include "Tile.hpp"


class FreeParkingTile : public Tile {
public:
    explicit FreeParkingTile(int index,
                             const string& code = "BBP",
                             const string& name = "Bebas Parkir");

    void onLand(Player& player, GameEngine& engine) override;
};
