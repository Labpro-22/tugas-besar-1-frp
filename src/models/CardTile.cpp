#include "../../include/models/CardTile.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/core/GameEngine.hpp"

CardTile::CardTile(int index, const string& code, CardDrawType drawType,
                   const string& name)
    : Tile(index, code,
           name.empty()
               ? (drawType == CardDrawType::CHANCE ? "Kesempatan" : "Dana Umum")
               : name),
      drawType(drawType) {}

CardDrawType CardTile::getDrawType() const{
    return drawType;
}

void CardTile::onLand(Player& player, GameEngine& engine) {
    if (drawType == CardDrawType::CHANCE) {
        engine.getCardManager().drawChanceCard(player, engine);
    } else {
        engine.getCardManager().drawCommunityCard(player, engine);
    }
}
