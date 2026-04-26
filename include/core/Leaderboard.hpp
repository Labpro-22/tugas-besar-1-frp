#pragma once

#include <string>

class Player;

class Leaderboard {
public:
    static Leaderboard fromPlayer(const Player& player, int rank, int tokenIndex);

    int getRank() const { return rank_; }
    const std::string& getPlayerName() const { return playerName_; }
    int getCash() const { return cash_; }
    int getAsset() const { return asset_; }
    int getPropertyCount() const { return propertyCount_; }
    int getSkillCardCount() const { return skillCardCount_; }
    int getTokenIndex() const { return tokenIndex_; }

private:
    Leaderboard(int rank,
                std::string playerName,
                int cash,
                int asset,
                int propertyCount,
                int skillCardCount,
                int tokenIndex);

    int rank_ = 0;
    std::string playerName_;
    int cash_ = 0;
    int asset_ = 0;
    int propertyCount_ = 0;
    int skillCardCount_ = 0;
    int tokenIndex_ = -1;
};
