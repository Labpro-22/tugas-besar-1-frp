#pragma once

#include <string>

struct Leaderboard {
    int rank = 0;
    std::string playerName;
    int cash = 0;
    int asset = 0;
    int propertyCount = 0;
    int tokenIndex = -1;
};
