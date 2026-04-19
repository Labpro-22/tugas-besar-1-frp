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
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for(int i=nPlayers - 1; i > 0;--i){
        int j = std::rand() % (i+1);
        std::swap(turnOrder[i], turnOrder[j]);
    }
    currentIndex = 0;
    currentTurnNumber = 1;
    extraTurnGranted = false;
}

int TurnManager::getCurrentPlayerIndex() const{
    if(turnOrder.empty()){
        throw std::runtime_error("TurnManager: turnOrder is empty");
    }
    return turnOrder[currentIndex];
}

void TurnManager::nextPlayer(const std::vector<bool>& bankruptFlags){
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
    auto it = std::find(turnOrder.begin(),turnOrder.end(), playerIndex);
    if(it == turnOrder.end()) return;

    int removedPos = static_cast<int>(std::distance(turnOrder.begin(),it));
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

bool TurnManager::hasExtraTurn(){
    return extraTurnGranted;
}
