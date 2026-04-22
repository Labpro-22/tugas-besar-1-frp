#include "../../include/core/TurnManager.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

TurnManager::TurnManager() : currentIndex(0), currentTurnNumber(1), extraTurnGranted(false){}

void TurnManager::initializeOrder(int nPlayers){
    turnOrder.clear();
    for (int i=0; i < nPlayers; ++i){
        turnOrder.push_back(i);
    }

    // Pakai algoritma Fisher-Yates Shuffle
    srand(static_cast<unsigned int>(time(nullptr)));
    for(int i=nPlayers - 1; i > 0;--i){
        int j = rand() % (i+1);
        swap(turnOrder[i], turnOrder[j]);
    }
    currentIndex = 0;
    currentTurnNumber = 1;
    extraTurnGranted = false;
}

int TurnManager::getCurrentPlayerIndex() const{
    if(turnOrder.empty()){
        throw runtime_error("TurnManager: turnOrder is empty");
    }
    return turnOrder[currentIndex];
}

void TurnManager::nextPlayer(const vector<bool>& bankruptFlags){
    if (turnOrder.empty()) return;

    // kondisi ada extra turn
    if(extraTurnGranted){
        extraTurnGranted = false;
        return;
    }

    int size = static_cast<int>(turnOrder.size());

    int start = currentIndex;

    do{
        currentIndex = (currentIndex + 1) % size;

        // tiap satu putran penuh maka currentTurnNumber bertmabah
        if (currentIndex == 0){
            currentTurnNumber++;
        }
    } while (bankruptFlags[turnOrder[currentIndex]] && currentIndex != start);
}

void TurnManager::grantExtraTurn(){
    extraTurnGranted = true;
}

int TurnManager::getTurnNumber() const{
    return currentTurnNumber;
}

void TurnManager::removePlayer(int playerIndex){
    auto it = find(turnOrder.begin(),turnOrder.end(), playerIndex);
    if(it == turnOrder.end()) return;

    int removedPos = static_cast<int>(distance(turnOrder.begin(),it));
    turnOrder.erase(it);

    //currentIndex disesuaikan agar tidak lompat
    if(turnOrder.empty()){
        currentIndex = 0;
        return;
    }
    if (removedPos < currentIndex){
        currentIndex--;
    }else if(removedPos == currentIndex){
        currentIndex = currentIndex % static_cast<int>(turnOrder.size());
    }
}

void TurnManager::resetExtraTurn(){
    extraTurnGranted = false;
}

bool TurnManager::hasExtraTurn() const {
    return extraTurnGranted;
}

const vector<int>& TurnManager::getTurnOrder() const {
    return turnOrder;
}

int TurnManager::getCurrentOrderIndex() const {
    return currentIndex;
}

void TurnManager::restoreState(const vector<int>& order,
                               int currentOrderIndex,
                               int turnNumber,
                               bool extraTurn) {
    if (order.empty()) {
        throw runtime_error("TurnManager::restoreState - turn order cannot be empty");
    }
    if (currentOrderIndex < 0 || currentOrderIndex >= static_cast<int>(order.size())) {
        throw runtime_error("TurnManager::restoreState - current order index out of range");
    }
    if (turnNumber < 1) {
        throw runtime_error("TurnManager::restoreState - turn number must be >= 1");
    }

    turnOrder = order;
    currentIndex = currentOrderIndex;
    currentTurnNumber = turnNumber;
    extraTurnGranted = extraTurn;
}

int TurnManager::orderSize() const {
    return static_cast<int>(turnOrder.size());
}
