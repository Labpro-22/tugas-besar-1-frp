#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"

JailTile::JailTile(int index) : Tile(index, "PEN", "Penjara") {}

int JailTile::countInmates(const vector<Player*>& players) const{
    int count = 0;
    for (const Player* p : players){
        if (p && p->isJailed()) ++count;
    }
    return count;
}

int JailTile::countVisitors(const vector<Player*>& players) const{
    int count = 0;
    for (const Player* p : players){
        if (p && !p->isJailed() && !p->isBankrupt() && p->getPosition() == index) ++count;
    }
    return count;
}

void JailTile::sendToJail(Player& player){
    player.setPosition(index);
    player.setStatus(PlayerStatus::JAILED);
    player.setJailTurns(0);
}

void JailTile::onLand(Player& player, GameEngine& engine) {
    (void)engine;
 
    if (!player.isJailed()) {}
}