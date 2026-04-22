#include "../../include/models/GoTile.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"

GoTile::GoTile(int index, int salary, const string& code, const string& name)
    : Tile(index, code, name), salary(salary) {}

int GoTile::getSalary() const { return salary; }

void GoTile::onLand(Player& player, GameEngine& engine) {
    engine.getBank().paySalary(player, salary);
    engine.getLogger().log(player.getUsername(), "GO",
        "Berhenti di GO, terima gaji M" + std::to_string(salary));
    engine.pushEvent(GameEventType::MONEY, UiTone::SUCCESS,
        "Petak GO",
        player.getUsername() + " berhenti di GO dan menerima gaji M" +
        std::to_string(salary) + ". Uang: M" + std::to_string(player.getMoney()));
}
