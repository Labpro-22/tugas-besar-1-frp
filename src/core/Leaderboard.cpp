#include "../../include/core/Leaderboard.hpp"

#include "../../include/models/Player.hpp"

#include <utility>

Leaderboard::Leaderboard(int rank,
                         std::string playerName,
                         int cash,
                         int asset,
                         int propertyCount,
                         int skillCardCount,
                         int tokenIndex)
    : rank_(rank),
      playerName_(std::move(playerName)),
      cash_(cash),
      asset_(asset),
      propertyCount_(propertyCount),
      skillCardCount_(skillCardCount),
      tokenIndex_(tokenIndex) {}

Leaderboard Leaderboard::fromPlayer(const Player& player, int rank, int tokenIndex) {
    return Leaderboard(
        rank,
        player.getUsername(),
        player.getMoney(),
        player.getAssetValue(),
        player.countProperties(),
        player.countCards(),
        tokenIndex);
}
