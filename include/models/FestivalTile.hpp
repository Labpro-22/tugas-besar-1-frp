#pragma once

#include "Tile.hpp"

class FestivalTile : public Tile {
public:
    explicit FestivalTile(int index, const string& code,
                          const string& name = "Festival");

    void onLand(Player& player, GameEngine& engine) override;
};
