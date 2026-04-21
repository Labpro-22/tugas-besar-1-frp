#include "../../include/models/CardTile.hpp"

CardTile::CardTile(int index, const string& code, CardDrawType drawType) : Tile(index, code, drawType == CardDrawType::CHANCE ? "Kesempatan" : "Dana Umum"), drawType(drawType) {}

CardDrawType CardTile::getDrawType() const{
    return drawType;
}

void CardTile::onLand(Player& player, GameEngine& engine) {
    // butuh class CardManager

    (void)player;
    (void)engine;
    // if (drawType == CardDrawType::CHANCE) {
    //     engine.getCardManager().drawChanceCard(player, engine);
    // } 
    // else {
    //     engine.getCardManager().drawCommunityCard(player, engine);
    // }
}