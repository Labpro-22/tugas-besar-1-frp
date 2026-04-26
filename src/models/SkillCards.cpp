#include "../../include/models/SkillCards.hpp"

#include "../../include/core/EffectManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/StreetProperty.hpp"
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

MoveCard::MoveCard(int value)
    : SkillCard("Maju sejumlah petak.", value, 0) {}

void MoveCard::apply(Player& player, GameEngine& game) {
    if (value <= 0) {
        throw GameException("MoveCard value is not set.");
    }

    Board& board = game.getBoard();
    const int oldPos = player.getPosition();
    player.move(value, board.size());

    if (shouldAwardPassGoOnForwardMove(board, oldPos, player.getPosition())) {
        player.addMoney(game.getGoSalary());
    }

    Tile& landing = board.getTileByIndex(player.getPosition());
    game.handleLanding(player, landing);
}

std::string MoveCard::getTypeName() const {
    return "MoveCard";
}

DiscountCard::DiscountCard(int value)
    : SkillCard("Dapatkan diskon pembelian properti.", value, 0) {}

void DiscountCard::apply(Player& player, GameEngine& game) {
    if (value <= 0) {
        throw GameException("DiscountCard value is not set.");
    }

    duration = 1;
    game.getEffectManager().applyDiscount(player, value);
}

std::string DiscountCard::getTypeName() const {
    return "DiscountCard";
}

ShieldCard::ShieldCard()
    : SkillCard("Kebal tagihan atau sanksi selama 1 giliran.", 0, 1) {}

void ShieldCard::apply(Player& player, GameEngine& game) {
    (void)game;
    player.setShieldActive(true);
}

std::string ShieldCard::getTypeName() const {
    return "ShieldCard";
}

TeleportCard::TeleportCard()
    : SkillCard("Pindah ke petak manapun.", 0, 0) {}

void TeleportCard::setTargetCode(const std::string& code) {
    targetCode = code;
}

void TeleportCard::apply(Player& player, GameEngine& game) {
    if (targetCode.empty()) {
        throw GameException("TeleportCard target tile is not set.");
    }

    Board& board = game.getBoard();
    const int oldPos = player.getPosition();
    const int targetIdx = board.getIndexOf(targetCode);

    if (shouldAwardPassGoOnForwardMove(board, oldPos, targetIdx)) {
        player.addMoney(game.getGoSalary());
    }

    player.setPosition(targetIdx);
    Tile& landing = board.getTileByIndex(targetIdx);
    game.handleLanding(player, landing);
}

std::string TeleportCard::getTypeName() const {
    return "TeleportCard";
}

LassoCard::LassoCard()
    : SkillCard("Tarik lawan di depanmu ke posisimu.", 0, 0) {}

void LassoCard::setTargetUsername(const std::string& username) {
    targetUsername = username;
}

void LassoCard::apply(Player& player, GameEngine& game) {
    if (targetUsername.empty()) {
        throw GameException("LassoCard target player is not set.");
    }

    Player* target = game.getPlayerByName(targetUsername);
    if (!target || target == &player) {
        throw GameException("Invalid LassoCard target player.");
    }

    if (target->isJailed() || target->isBankrupt()) {
        throw GameException("LassoCard target cannot be JAILED or BANKRUPT.");
    }

    Board& board = game.getBoard();
    const int dist = board.distanceTo(player.getPosition(), target->getPosition());
    if (dist <= 0 || dist > 12) {
        throw GameException("LassoCard target must be ahead within 1 turn and max distance 12.");
    }

    target->setPosition(player.getPosition());
}

std::string LassoCard::getTypeName() const {
    return "LassoCard";
}

DemolitionCard::DemolitionCard()
    : SkillCard("Hancurkan bangunan lawan pada satu properti.", 0, 0) {}

void DemolitionCard::setTargetPropertyCode(const std::string& code) {
    targetPropertyCode = code;
}

void DemolitionCard::apply(Player& player, GameEngine& game) {
    if (targetPropertyCode.empty()) {
        throw GameException("DemolitionCard target property is not set.");
    }

    Board& board = game.getBoard();
    Tile& tile = board.getTileByCode(targetPropertyCode);
    if (!tile.isProperty()) {
        throw GameException("DemolitionCard target tile is not a property.");
    }

    PropertyTile& propertyTile = static_cast<PropertyTile&>(tile);
    Property& property = propertyTile.getProperty();

    if (property.getType() != PropertyType::STREET) {
        throw GameException("DemolitionCard can only target STREET properties.");
    }

    if (!property.getOwner() || property.getOwner() == &player) {
        throw GameException("DemolitionCard target must be owned by an opponent.");
    }

    if (property.isMortgaged()) {
        throw GameException("DemolitionCard cannot target MORTGAGED property.");
    }

    StreetProperty& street = static_cast<StreetProperty&>(property);
    if (street.getBuildingLevel() == BuildingLevel::NONE) {
        return;
    }

    street.demolishBuildings();
}

std::string DemolitionCard::getTypeName() const {
    return "DemolitionCard";
}
