#include <cassert>
#include <iostream>
#include <set>
#include "models/Dice.hpp"
#include "core/TurnManager.hpp"
#include "../../include/utils/GameException.hpp"

// ─── Dice Tests ───────────────────────────────────────────────────────────────

void test_dice_rollRandom_range() {
    Dice dice;
    for (int i = 0; i < 500; ++i) {
        std::pair<int, int> roll = dice.rollRandom();
        int d1 = roll.first;
        int d2 = roll.second;
        assert(d1 >= 1 && d1 <= 6);
        assert(d2 >= 1 && d2 <= 6);
        assert(dice.getTotal() == d1 + d2);
    }
    std::cout << "[PASS] Dice::rollRandom berada dalam rentang 1-6\n";
}

void test_dice_setManual() {
    Dice dice;
    std::pair<int, int> roll = dice.setManual(3, 5);
    int d1 = roll.first;
    int d2 = roll.second;
    assert(d1 == 3 && d2 == 5);
    assert(dice.getDie1() == 3 && dice.getDie2() == 5);
    assert(dice.getTotal() == 8);
    assert(!dice.isDouble());
    std::cout << "[PASS] Dice::setManual menyimpan nilai dengan benar\n";
}

void test_dice_isDouble() {
    Dice dice;
    dice.setManual(4, 4);
    assert(dice.isDouble());
    dice.setManual(2, 5);
    assert(!dice.isDouble());
    std::cout << "[PASS] Dice::isDouble berfungsi dengan benar\n";
}

void test_dice_setManual_invalid() {
    Dice dice;
    bool threw = false;
    try {
        dice.setManual(0, 3);
    } catch (const GameException&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        dice.setManual(3, 7);
    } catch (const GameException&) {
        threw = true;
    }
    assert(threw);
    std::cout << "[PASS] Dice::setManual melempar GameException untuk nilai di luar rentang\n";
}

// ─── TurnManager Tests ────────────────────────────────────────────────────────

void test_turnmanager_initializeOrder() {
    TurnManager tm;
    tm.initializeOrder(4);
    // Setiap index 0-3 harus muncul tepat sekali
    std::set<int> seen;
    for (int i = 0; i < 4; ++i) {
        int idx = tm.getCurrentPlayerIndex();
        seen.insert(idx);
        assert(idx >= 0 && idx < 4);
        tm.nextPlayer({false, false, false, false});
    }
    assert(seen.size() == 4);
    std::cout << "[PASS] TurnManager::initializeOrder menghasilkan permutasi lengkap\n";
}

void test_turnmanager_nextPlayer_wraps() {
    TurnManager tm;
    tm.initializeOrder(3);
    std::vector<bool> noBankrupt = {false, false, false};

    int firstPlayer = tm.getCurrentPlayerIndex();
    tm.nextPlayer(noBankrupt);
    tm.nextPlayer(noBankrupt);
    tm.nextPlayer(noBankrupt); // Kembali ke awal putaran
    assert(tm.getCurrentPlayerIndex() == firstPlayer);
    std::cout << "[PASS] TurnManager::nextPlayer wrap-around berfungsi\n";
}

void test_turnmanager_turnNumber_increments() {
    TurnManager tm;
    tm.initializeOrder(2);
    std::vector<bool> noBankrupt = {false, false};

    assert(tm.getTurnNumber() == 1);
    tm.nextPlayer(noBankrupt); // Pemain 2 giliran
    tm.nextPlayer(noBankrupt); // Kembali ke pemain 1 → turn 2
    assert(tm.getTurnNumber() == 2);
    std::cout << "[PASS] TurnManager::getTurnNumber bertambah setiap putaran penuh\n";
}

void test_turnmanager_extraTurn() {
    TurnManager tm;
    tm.initializeOrder(3);
    std::vector<bool> noBankrupt = {false, false, false};

    int currentBefore = tm.getCurrentPlayerIndex();
    tm.grantExtraTurn();
    assert(tm.hasExtraTurn());
    tm.nextPlayer(noBankrupt); // Seharusnya tidak pindah, tapi konsumsi extra turn
    // Extra turn harus sudah direset
    assert(!tm.hasExtraTurn());
    // Pemain masih sama karena giliran tambahan dikonsumsi
    assert(tm.getCurrentPlayerIndex() == currentBefore);
    std::cout << "[PASS] TurnManager::grantExtraTurn tidak berpindah pemain\n";
}

void test_turnmanager_removePlayer() {
    TurnManager tm;
    tm.initializeOrder(4);
    std::vector<bool> noBankrupt = {false, false, false, false};

    // Catat pemain aktif lalu hapus yang pertama
    int toRemove = tm.getCurrentPlayerIndex();
    tm.nextPlayer(noBankrupt); // Pindah ke pemain berikutnya dulu
    tm.removePlayer(toRemove);

    // Pastikan toRemove tidak muncul lagi setelah rotasi penuh
    for (int i = 0; i < 3; ++i) {
        assert(tm.getCurrentPlayerIndex() != toRemove);
        tm.nextPlayer({false, false, false, false});
    }
    std::cout << "[PASS] TurnManager::removePlayer menghapus pemain dari rotasi\n";
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Unit Test: Dice ===\n";
    test_dice_rollRandom_range();
    test_dice_setManual();
    test_dice_isDouble();
    test_dice_setManual_invalid();

    std::cout << "\n=== Unit Test: TurnManager ===\n";
    test_turnmanager_initializeOrder();
    test_turnmanager_nextPlayer_wraps();
    test_turnmanager_turnNumber_increments();
    test_turnmanager_extraTurn();
    test_turnmanager_removePlayer();

    std::cout << "\n✅ Semua test berhasil!\n";
    return 0;
}