#include "../../include/models/FreeParkingTile.hpp"


FreeParkingTile::FreeParkingTile(int index, const string& code, const string& name)
    : Tile(index, code, name) {}

void FreeParkingTile::onLand(Player& player, GameEngine& engine) {
    (void)player;
    (void)engine;
}
