#include <cassert>
#include <iostream>
#include <vector>

#include "core/CardManager.hpp"
#include "models/Player.hpp"
#include "utils/GameException.hpp"

namespace {

std::vector<std::string> handTypeNames(const Player& player) {
    std::vector<std::string> names;
    const auto& hand = player.getHandCards();
    for (const auto& card : hand) {
        if (card) {
            names.push_back(card->getTypeName());
        }
    }
    return names;
}

void test_cardmanager_drawSkillCard_addsToHand() {
    CardManager cm;
    cm.initializeDecks();

    Player player("card_tester", 1000);
    cm.drawSkillCard(player);

    assert(player.countCards() == 1);
    assert(!cm.hasPendingSkillDrop(player));
    std::cout << "[PASS] CardManager::drawSkillCard menambah kartu ke tangan saat slot tersedia\n";
}

void test_cardmanager_drawSkillCard_createsPendingAtFourthCard() {
    CardManager cm;
    cm.initializeDecks();

    Player player("card_tester", 1000);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);

    assert(player.countCards() == 3);
    assert(cm.hasPendingSkillDrop(player));

    const std::vector<std::string> options = cm.getPendingSkillDropOptions(player);
    assert(options.size() == 3);
    std::cout << "[PASS] Kartu ke-4 masuk mekanisme pending drop tanpa mengisi slot 4 inventory\n";
}

void test_cardmanager_resolvePendingDrop_swapExistingCard() {
    CardManager cm;
    cm.initializeDecks();

    Player player("card_tester", 1000);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    const std::vector<std::string> before = handTypeNames(player);

    cm.drawSkillCard(player);
    assert(cm.hasPendingSkillDrop(player));

    cm.resolvePendingSkillDrop(player, 1);

    assert(!cm.hasPendingSkillDrop(player));
    assert(player.countCards() == 3);
    assert(handTypeNames(player) != before);
    std::cout << "[PASS] resolvePendingSkillDrop menukar salah satu skill card di slot 1-3\n";
}

void test_cardmanager_resolvePendingDrop_replaceHandCard() {
    CardManager cm;
    cm.initializeDecks();

    Player player("card_tester", 1000);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);

    assert(cm.hasPendingSkillDrop(player));
    cm.resolvePendingSkillDrop(player, 0);

    assert(!cm.hasPendingSkillDrop(player));
    assert(player.countCards() == 3);
    const auto& hand = player.getHandCards();
    for (const auto& card : hand) {
        assert(card != nullptr);
    }
    std::cout << "[PASS] resolvePendingSkillDrop dapat mengganti salah satu kartu tangan\n";
}

void test_cardmanager_resolvePendingDrop_invalidIndexThrows() {
    CardManager cm;
    cm.initializeDecks();

    Player player("card_tester", 1000);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);
    cm.drawSkillCard(player);

    bool threw = false;
    try {
        cm.resolvePendingSkillDrop(player, 3);
    } catch (const InvalidCardIndexException&) {
        threw = true;
    }

    assert(threw);
    assert(cm.hasPendingSkillDrop(player));
    std::cout << "[PASS] resolvePendingSkillDrop melempar InvalidCardIndexException untuk slot di luar 1-3\n";
}

void test_cardmanager_saveLoadState_roundTrip() {
    CardManager cm;

    const std::vector<std::string> expected = {
        "MoveCard",
        "ShieldCard",
        "TeleportCard"
    };

    cm.loadSkillDeckState(expected);
    const std::vector<std::string> actual = cm.getSkillDeckStateForSave();

    assert(actual == expected);
    std::cout << "[PASS] State deck skill dapat di-load dan diambil kembali untuk save\n";
}

} // namespace

void run_card_tests() {
    std::cout << "\n=== Unit Test: Card ===\n";
    test_cardmanager_drawSkillCard_addsToHand();
    test_cardmanager_drawSkillCard_createsPendingAtFourthCard();
    test_cardmanager_resolvePendingDrop_swapExistingCard();
    test_cardmanager_resolvePendingDrop_replaceHandCard();
    test_cardmanager_resolvePendingDrop_invalidIndexThrows();
    test_cardmanager_saveLoadState_roundTrip();
}
