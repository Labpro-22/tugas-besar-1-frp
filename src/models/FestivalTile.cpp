#include "../../include/models/FestivalTile.hpp"

FestivalTile::FestivalTile(int index, const string& code) : Tile(index, code, "Festival"){}

void FestivalTile::onLand(Player& player, GameEngine& engine) {
    //butuh class EffectManager

    (void)player;
    (void)engine;
    // Property& chosen = engine.promptFestivalChoice(player);    
    // engine.getEffectManager().applyFestival(player, chosen);
}