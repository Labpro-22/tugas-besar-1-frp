#pragma once
#include <vector>

class Player;
class Board;

class GameContext {
public:
    GameContext(const std::vector<Player*>& allPlayers,
                const Board* board, int diceTotal);

    const std::vector<Player*>& getAllPlayers() const;
    const Board* getBoard() const;
    int getDiceTotal()  const;
 
private:
    const std::vector<Player*>& allPlayers_;
    const Board* board_;
    int diceTotal_;
};