#pragma once

#include "Tile.hpp"
#include <vector>

class JailTile : public Tile{
public:
    explicit JailTile(int index);

    int countInmates(const vector<Player*>& players) const;
    int countVisitors(const vector<Player*>& players) const;
    void sendToJail(Player& player);

    void onLand(Player& player, GameEngine& engine) override;
};