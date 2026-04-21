#include "../../include/models/GoToJailTile.hpp"
#include "../../include/models/JailTile.hpp"

GoToJailTile::GoToJailTile(int index, JailTile& jailTile) : Tile(index, "PPJ", "Pergi ke Penjara"), jailTile(jailTile) {}

void GoToJailTile::onLand(Player& player, GameEngine& engine){
    jailTile.sendToJail(player);

    /*butuh method GameEngine::endTurnImmediately()*/

    (void)engine;
    // engine.endTurnImmediately();
}