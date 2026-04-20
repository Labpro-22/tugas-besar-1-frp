#pragma once

#include <string>
#include <vector>

class Board;
class Player;

class BoardRenderer {
public:
    std::string render(const Board& board,
                       const std::vector<Player*>& players,
                       int turn,
                       int maxTurn) const;

private:
    static constexpr int sideLength = 11;
    static constexpr int cellWidth = 10;

    std::string fitCell(const std::string& text) const;
    std::string buildCellLabel(const Board& board,
                               const std::vector<Player*>& players,
                               int boardIndex) const;
};
