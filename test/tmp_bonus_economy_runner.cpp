#include <iostream>
#include <string>
#include <vector>

#include "core/AuctionManager.hpp"
#include "core/BankruptcyManager.hpp"
#include "core/GameEngine.hpp"
#include "core/PropertyManager.hpp"
#include "models/Bank.hpp"
#include "models/Board.hpp"
#include "models/ChanceCards.hpp"
#include "models/CommunityCards.hpp"
#include "models/GameContext.hpp"
#include "utils/Gamestateserializer.hpp"
#include "models/Player.hpp"
#include "models/Property.hpp"
#include "models/PropertyTile.hpp"
#include "models/StreetProperty.hpp"
#include "utils/GameException.hpp"

namespace {
void printResult(const std::string& name, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
}

bool messageContains(const std::vector<GameEvent>& events,
                     const std::string& titleContains,
                     const std::string& messageContainsText) {
    for (const GameEvent& ev : events) {
        if (ev.title.find(titleContains) != std::string::npos &&
            ev.message.find(messageContainsText) != std::string::npos) {
            return true;
        }
    }
    return false;
}

StreetProperty& getStreetByCode(GameEngine& engine, const std::string& code) {
    auto& tile = static_cast<PropertyTile&>(engine.getBoard().getTileByCode(code));
    return static_cast<StreetProperty&>(tile.getProperty());
}

bool buildWithYes(GameEngine& engine, Player& owner, StreetProperty& prop) {
    engine.setPromptAnswer("bangun_" + prop.getCode(), "y");
    return engine.getPropertyManager().buildOnProperty(owner, prop);
}

bool testDuplicateJailFreeCard() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    GetOutOfJailCard card;
    card.apply(*alice, engine);
    const int first = alice->getJailFreeCardCount();
    card.apply(*alice, engine);
    const int second = alice->getJailFreeCardCount();

    return first == 1 && second == 1 && alice->hasJailFreeCard();
}

bool testSendToJailUsesCard() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    alice->addJailFreeCard();
    const bool jailedFirst = engine.sendPlayerToJail(*alice, "UnitTest");
    const int penIndex = engine.getBoard().getIndexOf("PEN");

    if (jailedFirst) return false;
    if (alice->getPosition() != penIndex) return false;
    if (alice->isJailed()) return false;
    if (alice->hasJailFreeCard()) return false;

    const bool jailedSecond = engine.sendPlayerToJail(*alice, "UnitTest2");
    return jailedSecond && alice->isJailed() && alice->getPosition() == penIndex;
}

bool testGoToNearestFestival() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    Board& board = engine.getBoard();
    int fromIdx = -1;
    for (int i = 0; i < board.size(); ++i) {
        if (board.getTileByIndex(i).getCode() == "KSP") {
            fromIdx = i; // ambil KSP terakhir agar pasti wrap.
        }
    }
    if (fromIdx < 0) return false;

    int fesIdx = -1;
    int bestDist = board.size() + 1;
    for (int i = 0; i < board.size(); ++i) {
        if (board.getTileByIndex(i).getCode() != "FES") {
            continue;
        }
        const int dist = board.distanceTo(fromIdx, i);
        if (dist > 0 && dist < bestDist) {
            bestDist = dist;
            fesIdx = i;
        }
    }
    if (fesIdx < 0) return false;

    alice->setPosition(fromIdx);

    const int before = alice->getMoney();
    const int goSalary = engine.getGoSalary();

    GoToNearestFestivalCard card;
    card.apply(*alice, engine);

    if (alice->getPosition() != fesIdx) return false;

    const int expectedMoney = (fesIdx < fromIdx) ? before + goSalary : before;
    return alice->getMoney() == expectedMoney;
}

bool testCommunityArisanBegal() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    const int start = alice->getMoney();

    ArisanCard arisan(300);
    arisan.apply(*alice, engine);
    if (alice->getMoney() != start + 300) return false;

    BegalCard begal(200);
    begal.apply(*alice, engine);
    return alice->getMoney() == start + 100;
}

bool testJailFreeCardPersistsInSaveLoad() {
    GameEngine source;
    source.startNewGame(2, {"Alice", "Bob"});

    Player* aliceSource = source.getPlayerByName("Alice");
    if (!aliceSource) return false;

    aliceSource->addJailFreeCard();
    GameSnapshot snapshot = source.createSnapshot();

    GameEngine restored;
    restored.startNewGame(2, {"Tmp1", "Tmp2"});
    restored.applySnapshot(snapshot);

    Player* aliceRestored = restored.getPlayerByName("Alice");
    if (!aliceRestored) return false;

    return aliceRestored->hasJailFreeCard() &&
           aliceRestored->getJailFreeCardCount() == 1;
}

bool testBankruptcyRemovesCorrectDebtor() {
    GameEngine engine;
    engine.startNewGame(3, {"Alice", "Bob", "Charlie"});

    Player* alice = engine.getPlayerByName("Alice");
    Player* bob = engine.getPlayerByName("Bob");
    if (!alice || !bob) return false;

    bob->deductMoney(bob->getMoney());
    engine.getBankruptcyManager().handleDebt(*bob, 100, alice);

    const auto& order = engine.getTurnManager().getTurnOrder();
    const auto& players = engine.getPlayers();
    int bobIndex = -1;
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        if (players[i] == bob) {
            bobIndex = i;
            break;
        }
    }
    if (bobIndex < 0) return false;

    for (int idx : order) {
        if (idx == bobIndex) {
            return false;
        }
    }
    return bob->isBankrupt();
}

bool testRedeemUsesPurchasePriceForUtility() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    auto& tile = static_cast<PropertyTile&>(engine.getBoard().getTileByCode("PLN"));
    auto& prop = tile.getProperty();

    prop.setOwner(alice);
    alice->addProperty(&prop);
    prop.setStatus(OwnershipStatus::MORTGAGED);

    const int before = alice->getMoney();
    const int expectedCost = prop.getPurchasePrice();

    const bool ok = engine.getPropertyManager().redeemProperty(*alice, prop);
    if (!ok) return false;

    return alice->getMoney() == before - expectedCost && prop.getStatus() == OwnershipStatus::OWNED;
}

bool testStreetMonopolyRequiresFullGroupOwnership() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    StreetProperty& grt = getStreetByCode(engine, "GRT");
    engine.getBank().transferPropertyToPlayer(&grt, *alice);

    const GameContext ctx(engine.getPlayers(), &engine.getBoard(), 7);
    const int rent = grt.calculateRent(ctx);
    const int baseRent = grt.getRentLevels().at(0);

    // Alice hanya punya 1 dari 2 petak color group CK, jadi tidak boleh 2x.
    return rent == baseRent;
}

bool testBuildMustBeEvenlyDistributed() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    StreetProperty& grt = getStreetByCode(engine, "GRT");
    StreetProperty& tsk = getStreetByCode(engine, "TSK");
    engine.getBank().transferPropertyToPlayer(&grt, *alice);
    engine.getBank().transferPropertyToPlayer(&tsk, *alice);

    if (!buildWithYes(engine, *alice, grt)) return false;

    bool blockedByEvenRule = false;
    try {
        (void)buildWithYes(engine, *alice, grt);
    } catch (const GameException&) {
        blockedByEvenRule = true;
    }

    if (!blockedByEvenRule) return false;
    return buildWithYes(engine, *alice, tsk);
}

bool testHotelUpgradeStillPossibleAfterFirstHotelInGroup() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    if (!alice) return false;

    StreetProperty& grt = getStreetByCode(engine, "GRT");
    StreetProperty& tsk = getStreetByCode(engine, "TSK");
    engine.getBank().transferPropertyToPlayer(&grt, *alice);
    engine.getBank().transferPropertyToPlayer(&tsk, *alice);

    for (int i = 0; i < 4; ++i) {
        if (!buildWithYes(engine, *alice, grt)) return false;
        if (!buildWithYes(engine, *alice, tsk)) return false;
    }

    if (!buildWithYes(engine, *alice, grt)) return false;
    if (grt.getBuildingLevel() != BuildingLevel::HOTEL) return false;

    if (!buildWithYes(engine, *alice, tsk)) return false;
    return tsk.getBuildingLevel() == BuildingLevel::HOTEL;
}

bool testBankruptTransferKeepsMortgagedStatus() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player* alice = engine.getPlayerByName("Alice");
    Player* bob = engine.getPlayerByName("Bob");
    if (!alice || !bob) return false;

    StreetProperty& grt = getStreetByCode(engine, "GRT");
    engine.getBank().transferPropertyToPlayer(&grt, *bob);
    grt.setStatus(OwnershipStatus::MORTGAGED);

    bob->deductMoney(bob->getMoney());
    engine.getBankruptcyManager().handleDebt(*bob, 100, alice);

    return bob->isBankrupt() &&
           grt.getOwner() == alice &&
           grt.getStatus() == OwnershipStatus::MORTGAGED;
}

bool testLandingOnGoGetsSalaryOnlyOnce() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    Player& current = engine.getCurrentPlayer();
    current.setPosition(engine.getBoard().size() - 1);

    const int before = current.getMoney();
    engine.moveCurrentPlayer(1);

    return current.getMoney() == before + engine.getGoSalary();
}

bool testAuctionAllowsZeroBid() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    auto& tile = static_cast<PropertyTile&>(engine.getBoard().getTileByCode("GRT"));
    Property& prop = tile.getProperty();

    Player& current = engine.getCurrentPlayer();
    const std::string promptKey = "lelang_" + current.getUsername() + "_0_-1_0";
    engine.setPromptAnswer(promptKey, "BID 0");

    engine.getAuctionManager().startAuction(prop, nullptr, true);

    CommandResult out;
    engine.flushEvents(out);

    return messageContains(out.events, "Bid Diterima", "M0");
}

bool testMaxTurnNotEndedBeforeRoundCompletes() {
    GameEngine engine;
    engine.startNewGame(3, {"Alice", "Bob", "Charlie"});

    const int maxTurn = engine.getMaxTurn();
    if (maxTurn <= 0) {
        return true;
    }

    auto order = engine.getTurnManager().getTurnOrder();
    if (order.size() < 2) return false;

    engine.getTurnManager().restoreState(order, 0, maxTurn, false);
    engine.checkWinCondition();

    return !engine.isGameOver();
}

bool testMaxTurnWinnerUsesMoneyThenPropertyThenCard() {
    GameEngine engine;
    engine.startNewGame(2, {"Alice", "Bob"});

    const int maxTurn = engine.getMaxTurn();
    if (maxTurn <= 0) {
        return true;
    }

    Player* alice = engine.getPlayerByName("Alice");
    Player* bob = engine.getPlayerByName("Bob");
    if (!alice || !bob) return false;

    StreetProperty& jkt = getStreetByCode(engine, "JKT");
    StreetProperty& ikn = getStreetByCode(engine, "IKN");
    engine.getBank().transferPropertyToPlayer(&jkt, *alice);
    engine.getBank().transferPropertyToPlayer(&ikn, *alice);

    bob->addMoney(200); // Bob uang lebih banyak, walau wealth Alice lebih tinggi.

    auto order = engine.getTurnManager().getTurnOrder();
    if (order.empty()) return false;

    engine.getTurnManager().restoreState(
        order,
        static_cast<int>(order.size()) - 1,
        maxTurn,
        false);

    engine.checkWinCondition();
    if (!engine.isGameOver()) return false;

    CommandResult out;
    engine.flushEvents(out);

    bool bobWins = false;
    bool aliceWins = false;
    for (const GameEvent& ev : out.events) {
        if (ev.title.find("Pemenang") != std::string::npos) {
            if (ev.message.find("Bob") != std::string::npos) {
                bobWins = true;
            }
            if (ev.message.find("Alice") != std::string::npos) {
                aliceWins = true;
            }
        }
    }

    return bobWins && !aliceWins;
}
} // namespace

int main() {
    bool allOk = true;

    const bool t1 = testDuplicateJailFreeCard();
    printResult("Bonus duplicate GetOutOfJailCard discarded", t1);
    allOk = allOk && t1;

    const bool t2 = testSendToJailUsesCard();
    printResult("Bonus jail-free consumed on send-to-jail", t2);
    allOk = allOk && t2;

    const bool t3 = testGoToNearestFestival();
    printResult("Bonus GoToNearestFestival applies immediately", t3);
    allOk = allOk && t3;

    const bool t4 = testCommunityArisanBegal();
    printResult("Bonus community cards Arisan/Begal effect", t4);
    allOk = allOk && t4;

    const bool t5 = testJailFreeCardPersistsInSaveLoad();
    printResult("Bonus jail-free card persists in save/load", t5);
    allOk = allOk && t5;

    const bool t6 = testBankruptcyRemovesCorrectDebtor();
    printResult("Economy bankruptcy removes actual debtor", t6);
    allOk = allOk && t6;

    const bool t7 = testRedeemUsesPurchasePriceForUtility();
    printResult("Economy redeem utility uses purchase price", t7);
    allOk = allOk && t7;

    const bool t8 = testStreetMonopolyRequiresFullGroupOwnership();
    printResult("Economy street monopoly rent requires full group ownership", t8);
    allOk = allOk && t8;

    const bool t9 = testBuildMustBeEvenlyDistributed();
    printResult("Economy build enforces even distribution", t9);
    allOk = allOk && t9;

    const bool t10 = testHotelUpgradeStillPossibleAfterFirstHotelInGroup();
    printResult("Economy second hotel in group remains buildable", t10);
    allOk = allOk && t10;

    const bool t11 = testBankruptTransferKeepsMortgagedStatus();
    printResult("Economy bankrupt transfer preserves mortgaged status", t11);
    allOk = allOk && t11;

    const bool t12 = testLandingOnGoGetsSalaryOnlyOnce();
    printResult("Economy landing on GO pays salary once", t12);
    allOk = allOk && t12;

    const bool t13 = testAuctionAllowsZeroBid();
    printResult("Economy auction accepts first bid M0", t13);
    allOk = allOk && t13;

    const bool t14 = testMaxTurnNotEndedBeforeRoundCompletes();
    printResult("Economy max-turn ends after full round", t14);
    allOk = allOk && t14;

    const bool t15 = testMaxTurnWinnerUsesMoneyThenPropertyThenCard();
    printResult("Economy max-turn winner priority uses money first", t15);
    allOk = allOk && t15;

    return allOk ? 0 : 1;
}
