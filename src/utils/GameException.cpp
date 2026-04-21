#include "../../include/models/GameContext.hpp"
#include "../../include/utils/GameException.hpp"

GameContext::GameContext(const std::vector<Player*>& allPlayers,
                         const Board* board, int diceTotal) : allPlayers_(allPlayers), board_(board), diceTotal_(diceTotal) {}

const std::vector<Player*>& GameContext::getAllPlayers() const { return allPlayers_;}
const Board* GameContext::getBoard() const { return board_;}
int GameContext::getDiceTotal()  const { return diceTotal_;}