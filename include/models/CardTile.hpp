#pragma once

#include "Tile.hpp"


enum class CardDrawType {
    CHANCE,    // Kartu Kesempatan
    COMMUNITY  // Kartu Dana Umum
};

class CardTile : public Tile {
private:
    CardDrawType drawType;

public:
    CardTile(int index, const string& code, CardDrawType drawType,
             const string& name = "");
    
    CardDrawType getDrawType() const;

    void onLand(Player& player, GameEngine& engine) override;
};
