#include "../../include/models/GoTile.hpp"

GoTile::GoTile(int index, int salary) : Tile(index, "GO", "Petak Mulai"), salary(salary) {}

int GoTile::getSalary() const{
    return salary;
}

void GoTile::onLand(Player& player, GameEngine& engine) {
    /*butuh 
    class Bank, 
    class TransactionLogger,
    method GameEngine::getBank(),
    method GameEngine::getLogger(),
    method GameEngine::getCurrentTurn(),*/

    (void)player;
    (void)engine;
    // engine.getBank().paySalary(player, salary);

    // engine.getLogger().log(
    //     engine.getCurrentTurn(),
    //     player.getUsername(),
    //     "GO",
    //     "Berhenti di GO, terima gaji M" + to_string(salary)
    // );
}
