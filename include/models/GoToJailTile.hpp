#pragma once

#include "Tile.hpp"

class JailTile;

class GoToJailTile : public Tile {
private:
    JailTile& jailTile; 

public:
    GoToJailTile(int index, JailTile& jailTile,
                 const string& code = "PPJ",
                 const string& name = "Pergi ke Penjara");

    void onLand(Player& player, GameEngine& engine) override;
};
