#include "../../include/core/TurnManager.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

TurnManager::TurnManager()
    : currentIndex(0), currentTurnNumber(1), extraTurnGranted(false) {}

void TurnManager::initializeOrder(int nPlayers) {
    turnOrder.clear();
    for (int i = 0; i < nPlayers; ++i) {
        turnOrder.push_back(i);
    }
    // Fisher-Yates shuffle
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = nPlayers - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        std::swap(turnOrder[i], turnOrder[j]);
    }
    currentIndex       = 0;
    currentTurnNumber  = 1;
    extraTurnGranted   = false;
}

int TurnManager::getCurrentPlayerIndex() const {
    if (turnOrder.empty()) {
        throw std::runtime_error("TurnManager: turnOrder kosong");
    }
    return turnOrder[currentIndex];
}

void TurnManager::nextPlayer(const std::vector<bool>& bankruptFlags) {
    if (turnOrder.empty()) return;

    // Jika masih ada extra turn, habiskan dulu
    if (extraTurnGranted) {
        extraTurnGranted = false;
        return;
    }

    int size = static_cast<int>(turnOrder.size());
    // Simpan posisi awal agar tidak loop selamanya jika semua bangkrut
    int start = currentIndex;

    do {
        currentIndex = (currentIndex + 1) % size;

        // Setiap kali kembali ke index 0 berarti satu putaran penuh selesai
        if (currentIndex == 0) {
            currentTurnNumber++;
        }
    } while (bankruptFlags[turnOrder[currentIndex]] && currentIndex != start);
}

void TurnManager::grantExtraTurn() {
    extraTurnGranted = true;
}

int TurnManager::getTurnNumber() const {
    return currentTurnNumber;
}

void TurnManager::removePlayer(int playerIndex) {
    auto it = std::find(turnOrder.begin(), turnOrder.end(), playerIndex);
    if (it == turnOrder.end()) return;

    int removedPos = static_cast<int>(std::distance(turnOrder.begin(), it));
    turnOrder.erase(it);

    // Sesuaikan currentIndex agar tidak loncat / out-of-bounds
    if (turnOrder.empty()) {
        currentIndex = 0;
        return;
    }
    if (removedPos < currentIndex) {
        // Elemen sebelum kursor dihapus, geser kursor mundur
        currentIndex--;
    } else if (removedPos == currentIndex) {
        // Pemain yang sedang aktif dihapus: wrap jika perlu
        currentIndex = currentIndex % static_cast<int>(turnOrder.size());
    }
    // removedPos > currentIndex: tidak perlu adjust
}

void TurnManager::resetExtraTurn() {
    extraTurnGranted = false;
}

bool TurnManager::hasExtraTurn() const {
    return extraTurnGranted;
}

int TurnManager::orderSize() const {
    return static_cast<int>(turnOrder.size());
}