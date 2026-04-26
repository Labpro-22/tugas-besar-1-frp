#include "../../include/core/Leaderboard.hpp"

#include "../../include/models/Player.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace {
struct LeaderboardRow {
    const Player* player = nullptr;
    int tokenIndex = -1;
};
} // namespace

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

bool Leaderboard::isTie(const Player& a, const Player& b) {
    return a.getMoney() == b.getMoney() &&
           a.countProperties() == b.countProperties() &&
           a.countCards() == b.countCards();
}

std::vector<Leaderboard> Leaderboard::generateRankings(const std::vector<Player*>& players) {
    std::vector<LeaderboardRow> rows;
    rows.reserve(players.size());

    for (size_t i = 0; i < players.size(); ++i) {
        const Player* player = players[i];
        if (!player) {
            continue;
        }

        rows.push_back(LeaderboardRow{
            player,
            static_cast<int>(i)});
    }

    std::sort(rows.begin(), rows.end(), [](const LeaderboardRow& a, const LeaderboardRow& b) {
        if (a.player->getMoney() != b.player->getMoney()) {
            return a.player->getMoney() > b.player->getMoney();
        }
        if (a.player->countProperties() != b.player->countProperties()) {
            return a.player->countProperties() > b.player->countProperties();
        }
        if (a.player->countCards() != b.player->countCards()) {
            return a.player->countCards() > b.player->countCards();
        }
        return a.player->getUsername() < b.player->getUsername();
    });

    std::vector<Leaderboard> ranked;
    ranked.reserve(rows.size());

    int currentRank = 0;
    for (size_t i = 0; i < rows.size(); ++i) {
        int rank = static_cast<int>(i) + 1;
        if (i > 0 && isTie(*rows[i].player, *rows[i - 1].player)) {
            rank = currentRank;
        }

        ranked.push_back(fromPlayer(*rows[i].player, rank, rows[i].tokenIndex));
        currentRank = rank;
    }

    return ranked;
}
