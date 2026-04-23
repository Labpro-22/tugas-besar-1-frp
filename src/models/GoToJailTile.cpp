#include "../../include/models/GoToJailTile.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"

GoToJailTile::GoToJailTile(int index, JailTile& jailTile,
                           const string& code, const string& name)
    : Tile(index, code, name), jailTile(jailTile) {}

void GoToJailTile::onLand(Player& player, GameEngine& engine) {
    (void)jailTile;
    const bool jailed = engine.sendPlayerToJail(player, "PPJ");
    engine.getLogger().log(player.getUsername(), "PPJ",
        jailed
            ? "Mendarat di PPJ dan dipindahkan ke penjara."
            : "Mendarat di PPJ, kartu Bebas dari Penjara terpakai.");
    engine.pushEvent(GameEventType::MOVEMENT, jailed ? UiTone::ERROR : UiTone::INFO,
        "Pergi ke Penjara",
        jailed
            ? (player.getUsername() + " mendarat di PPJ! Bidak dipindah ke Penjara. Giliran berakhir.")
            : (player.getUsername() + " mendarat di PPJ tetapi menggunakan kartu Bebas dari Penjara."));
}
