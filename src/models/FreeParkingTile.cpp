#include "../../include/models/FreeParkingTile.hpp"


FreeParkingTile::FreeParkingTile(int index) : Tile(index, "BBP", "Bebas Parkir") {}

void FreeParkingTile::onLand(Player& player, GameEngine& engine) {
    (void)player;
    (void)engine;
}