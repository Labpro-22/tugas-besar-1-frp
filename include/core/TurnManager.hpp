#pragma once
#include <vector>
using namespace std;

class TurnManager {
    private:
        vector<int> turnOrder;
        int currentIndex;
        int currentTurnNumber;
        bool extraTurnGranted;
    
    public:
        TurnManager();
        void initializeOrder(int nPlayers);
        int getCurrentPlayerIndex() const;
        void nextPlayer(const vector<bool>& bankruptFlags);
        void grantExtraTurn();
        int getTurnNumber() const;
        void removePlayer(int playerIndex);
        void resetExtraTurn();
        bool hasExtraTurn();

        const vector<int>& getTurnOrder() const;
        int getCurrentOrderIndex() const;
        void restoreState(const vector<int>& order,
                  int currentOrderIndex,
                  int turnNumber,
                  bool extraTurn = false);
};