#include "../../include/models/ChanceCards.hpp"

#include "../../include/core/GameEngine.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/utils/GameException.hpp"

namespace {
bool shouldAwardPassGoOnForwardMove(const Board& board,
                                    int fromIndex,
                                    int toIndex) {
    if (!board.hasTile("GO")) {
        return false;
    }

    const int steps = board.distanceTo(fromIndex, toIndex);
    if (steps <= 0) {
        return false;
    }

    const int goIndex = board.getIndexOf("GO");
    int cursor = fromIndex;
    for (int i = 0; i < steps; ++i) {
        cursor = (cursor + 1) % board.size();
        if (cursor == goIndex) {
            return toIndex != goIndex;
        }
    }
    return false;
}
}

GoToNearestRailroadCard::GoToNearestRailroadCard()
    : ActionCard("Pergi ke stasiun terdekat.") {}

void GoToNearestRailroadCard::apply(Player& player, GameEngine& game) {
    Board& board = game.getBoard();

    const int current = player.getPosition();
    int bestDistance = board.size() + 1;
    int bestIndex = -1;

    for (PropertyTile* tile : board.getAllPropertyTiles()) {
        if (!tile) {
            continue;
        }

        const Property& property = tile->getProperty();
        if (property.getType() != PropertyType::RAILROAD) {
            continue;
        }

        const int idx = tile->getIndex();
        const int dist = board.distanceTo(current, idx);

        if (dist > 0 && dist < bestDistance) {
            bestDistance = dist;
            bestIndex = idx;
        }
    }

    if (bestIndex < 0) {
        throw GameException("No railroad tile found for GoToNearestRailroadCard.");
    }

    if (shouldAwardPassGoOnForwardMove(board, current, bestIndex)) {
        player.addMoney(game.getGoSalary());
        game.pushEvent(GameEventType::MONEY, UiTone::SUCCESS,
            "Lewat GO",
            player.getUsername() + " menerima gaji GO sebesar M" +
                std::to_string(game.getGoSalary()) + ".");
    }

    player.setPosition(bestIndex);
    Tile& landing = board.getTileByIndex(bestIndex);
    game.handleLanding(player, landing);
}

MoveBackCard::MoveBackCard(int steps)
    : ActionCard("Mundur 3 petak."), steps(steps) {}

void MoveBackCard::apply(Player& player, GameEngine& game) {
    Board& board = game.getBoard();
    player.move(-steps, board.size());

    Tile& landing = board.getTileByIndex(player.getPosition());
    game.handleLanding(player, landing);
}

GoToJailCard::GoToJailCard()
    : ActionCard("Masuk Penjara.") {}

void GoToJailCard::apply(Player& player, GameEngine& game) {
    const bool jailed = game.sendPlayerToJail(player, "Kartu Kesempatan");
    if (jailed) {
        game.pushEvent(GameEventType::MOVEMENT, UiTone::ERROR,
            "Kartu Kesempatan",
            player.getUsername() + " langsung dipindah ke penjara.");
    }
}

GetOutOfJailCard::GetOutOfJailCard()
    : ActionCard("Bebas dari Penjara.") {}

InventoryCardState GetOutOfJailCard::getInventoryState() const {
    return InventoryCardState::CHANCE_SPECIAL;
}

void GetOutOfJailCard::apply(Player& player, GameEngine& game) {
    if (!player.canStoreInSpecialInventorySlot(*this)) {
        throw GameException(
            "Slot inventory terakhir hanya boleh diisi kartu kesempatan.");
    }

    if (!player.storeCardInSpecialInventorySlot(*this)) {
        game.pushEvent(GameEventType::CARD, UiTone::INFO,
            "Kartu Kesempatan",
            "Kartu Bebas dari Penjara duplikat dibuang karena slot 4 sudah terisi.");
        return;
    }

    game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
        "Kartu Kesempatan",
        "Kartu Bebas dari Penjara disimpan pada slot 4 inventory.");
}

GoToNearestFestivalCard::GoToNearestFestivalCard()
    : ActionCard("Pergi ke Festival terdekat.") {}

void GoToNearestFestivalCard::apply(Player& player, GameEngine& game) {
    Board& board = game.getBoard();

    const int current = player.getPosition();
    int bestIndex = -1;

    // "Pergi ke Festival terdekat" diartikan sebagai bergerak maju
    // ke tile Festival pertama yang ditemui saat menyusuri papan.
    for (int step = 1; step <= board.size(); ++step) {
        const int idx = (current + step) % board.size();
        if (board.getTileByIndex(idx).getCode() != "FES") {
            continue;
        }
        bestIndex = idx;
        break;
    }

    if (bestIndex < 0) {
        throw GameException("No festival tile found for GoToNearestFestivalCard.");
    }

    if (shouldAwardPassGoOnForwardMove(board, current, bestIndex)) {
        player.addMoney(game.getGoSalary());
        game.pushEvent(GameEventType::MONEY, UiTone::SUCCESS,
            "Lewat GO",
            player.getUsername() + " menerima gaji GO sebesar M" +
                std::to_string(game.getGoSalary()) + ".");
    }

    player.setPosition(bestIndex);
    game.pushEvent(GameEventType::MOVEMENT, UiTone::INFO,
        "Kartu Kesempatan",
        player.getUsername() + " dipindahkan ke Festival terdekat.");

    Tile& landing = board.getTileByIndex(bestIndex);
    game.handleLanding(player, landing);
}
