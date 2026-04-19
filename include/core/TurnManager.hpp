#pragma once
#include <vector>

class TurnManager {
    private:
        std::vector<int> turnOrder;
        int currentIndex;
        int currentTurnNumber;
        bool extraTurnGranted;
    
    public:
        TurnManager();
        void initializeOrder(int nPlayers);
        int getCurrentPlayerIndex() const;
        void nextPlayer(const std::vector<bool>& bankruptFlags);
        void grantExtraTurn();
        int getTurnNumber() const;
        void removePlayer(int playerIndex);
        void resetExtraTurn();
        bool hasExtraTurn();
};