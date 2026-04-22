#include "../../include/models/GoToJailTile.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"

GoToJailTile::GoToJailTile(int index, JailTile& jailTile,
                           const string& code, const string& name)
    : Tile(index, code, name), jailTile(jailTile) {}

void GoToJailTile::onLand(Player& player, GameEngine& engine) {
    jailTile.sendToJail(player);
    engine.getLogger().log(player.getUsername(), "PPJ",
        "Mendarat di PPJ dan dipindahkan ke penjara.");
    engine.pushEvent(GameEventType::MOVEMENT, UiTone::ERROR,
        "Pergi ke Penjara",
        player.getUsername() + " mendarat di PPJ! Bidak dipindah ke Penjara. "
        "Giliran berakhir.");
}
