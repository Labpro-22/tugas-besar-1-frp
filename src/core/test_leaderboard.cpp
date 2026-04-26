#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "core/Leaderboard.hpp"
#include "models/Player.hpp"
#include "models/SkillCards.hpp"
#include "models/StreetProperty.hpp"

namespace {

StreetProperty makeStreetProperty(const std::string& code) {
    return StreetProperty(
        code,
        code,
        "TEST",
        100,
        50,
        50,
        100,
        {10, 20, 30, 40, 50, 60});
}

void test_leaderboard_normalRanking_byMoneyDescending() {
    Player andi("Andi", 1200);
    Player budi("Budi", 900);
    Player caca("Caca", 500);

    std::vector<Player*> players = {&budi, &caca, &andi};
    const std::vector<Leaderboard> ranked = Leaderboard::generateRankings(players);

    assert(ranked.size() == 3);
    assert(ranked[0].getPlayerName() == "Andi");
    assert(ranked[0].getRank() == 1);
    assert(ranked[1].getPlayerName() == "Budi");
    assert(ranked[1].getRank() == 2);
    assert(ranked[2].getPlayerName() == "Caca");
    assert(ranked[2].getRank() == 3);
    std::cout << "[PASS] Leaderboard normal: uang berbeda tersortir menurun\n";
}

void test_leaderboard_tieRanking_sameMoneyPropertyCard_sameRank() {
    Player alice("Alice", 1000);
    Player bob("Bob", 1000);
    Player chika("Chika", 800);

    StreetProperty pAlice = makeStreetProperty("PA");
    StreetProperty pBob = makeStreetProperty("PB");
    alice.addProperty(&pAlice);
    bob.addProperty(&pBob);

    alice.addCard(std::make_shared<ShieldCard>());
    bob.addCard(std::make_shared<ShieldCard>());

    std::vector<Player*> players = {&bob, &alice, &chika};
    const std::vector<Leaderboard> ranked = Leaderboard::generateRankings(players);

    assert(ranked.size() == 3);
    assert(ranked[0].getPlayerName() == "Alice");
    assert(ranked[1].getPlayerName() == "Bob");
    assert(ranked[0].getRank() == 1);
    assert(ranked[1].getRank() == 1);
    assert(ranked[2].getPlayerName() == "Chika");
    assert(ranked[2].getRank() == 3);
    std::cout << "[PASS] Leaderboard tie: uang/properti/kartu sama menghasilkan rank identik\n";
}

void test_leaderboard_tieBreaker_propertyCount_lowerRankWhenFewerProperties() {
    Player dion("Dion", 1000);
    Player erik("Erik", 1000);

    StreetProperty d1 = makeStreetProperty("D1");
    StreetProperty d2 = makeStreetProperty("D2");
    StreetProperty e1 = makeStreetProperty("E1");
    dion.addProperty(&d1);
    dion.addProperty(&d2);
    erik.addProperty(&e1);

    std::vector<Player*> players = {&erik, &dion};
    const std::vector<Leaderboard> ranked = Leaderboard::generateRankings(players);

    assert(ranked.size() == 2);
    assert(ranked[0].getPlayerName() == "Dion");
    assert(ranked[0].getRank() == 1);
    assert(ranked[0].getPropertyCount() == 2);
    assert(ranked[0].getTokenIndex() == 1);

    assert(ranked[1].getPlayerName() == "Erik");
    assert(ranked[1].getRank() == 2);
    assert(ranked[1].getPropertyCount() == 1);
    assert(ranked[1].getTokenIndex() == 0);
    std::cout << "[PASS] Leaderboard tie-breaker: properti lebih sedikit turun peringkat\n";
}

} // namespace

void run_leaderboard_tests() {
    std::cout << "\n=== Unit Test: Leaderboard ===\n";
    test_leaderboard_normalRanking_byMoneyDescending();
    test_leaderboard_tieRanking_sameMoneyPropertyCard_sameRank();
    test_leaderboard_tieBreaker_propertyCount_lowerRankWhenFewerProperties();
}

