#include "../../include/core/GameEngine.hpp"
#include "../../include/utils/GameException.hpp"

// ─── Header dari Orang 1 & 2 (uncomment saat sudah tersedia) ─────────────────
// #include "../../include/models/Player.hpp"
// #include "../../include/models/Board.hpp"
// #include "../../include/core/Command.hpp"

// ─── Header dari Orang 4 & 5 (uncomment saat sudah tersedia) ─────────────────
// #include "../../include/models/Bank.hpp"
// #include "../../include/core/AuctionManager.hpp"
// #include "../../include/core/BankruptcyManager.hpp"
// #include "../../include/core/PropertyManager.hpp"
// #include "../../include/core/CardManager.hpp"
// #include "../../include/core/EffectManager.hpp"
// #include "../../include/core/TransactionLogger.hpp"
// #include "../../include/core/SaveLoadManager.hpp"

#include <iostream>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

GameEngine::GameEngine()
    : board(nullptr),
      gameOver(false),
      maxTurn(0),
      goSalary(200),
      jailFine(50),
      dice(6),
      bank(nullptr),
      auctionManager(nullptr),
      bankruptcyManager(nullptr),
      propertyManager(nullptr),
      cardManager(nullptr),
      effectManager(nullptr),
      logger(nullptr) {}

GameEngine::~GameEngine() {
    // Board dan players dimiliki oleh GameEngine — hapus jika diperlukan
    // delete board;
    // for (auto* p : players) delete p;
}

// ─────────────────────────────────────────────────────────────────────────────
// Dependency injection
// ─────────────────────────────────────────────────────────────────────────────

void GameEngine::setBank(Bank* b)                          { bank               = b;  }
void GameEngine::setAuctionManager(AuctionManager* am)     { auctionManager     = am; }
void GameEngine::setBankruptcyManager(BankruptcyManager* bm){ bankruptcyManager = bm; }
void GameEngine::setPropertyManager(PropertyManager* pm)   { propertyManager    = pm; }
void GameEngine::setCardManager(CardManager* cm)           { cardManager        = cm; }
void GameEngine::setEffectManager(EffectManager* em)       { effectManager      = em; }
void GameEngine::setTransactionLogger(TransactionLogger* tl){ logger            = tl; }

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void GameEngine::startNewGame(int nPlayers, std::vector<std::string> names) {
    // TODO: Inisialisasi board dari ConfigLoader (Orang 1)
    // TODO: Buat objek Player untuk setiap nama (Orang 1)
    // TODO: Distribusikan saldo awal ke setiap Player
    // TODO: initBoard() dengan definisi properti dari ConfigLoader

    // Acak urutan giliran via TurnManager
    turnManager.initializeOrder(nPlayers);

    gameOver = false;

    // TODO: catat ke TransactionLogger (Orang 4)
}

void GameEngine::loadGame(const std::string& filename) {
    // TODO: Panggil SaveLoadManager::load(filename) saat Orang 4 selesai
    // TODO: Pulihkan board, players, turnManager, dan state manager lain
    (void)filename;
    throw GameException("loadGame: belum diimplementasikan (menunggu SaveLoadManager)");
}

void GameEngine::run() {
    // Loop utama — berjalan hingga gameOver di-set oleh checkWinCondition/endGame
    while (!gameOver) {
        executeTurn();
    }
}

void GameEngine::processCommand(const Command& cmd) {
    // TODO: Dispatch ke handler berdasarkan cmd.type
    // Contoh switch akan ditambahkan saat Command & enum CommandType sudah ada (Orang 5)
    (void)cmd;
    throw GameException("processCommand: belum diimplementasikan (menunggu Command/CommandType)");
}

void GameEngine::executeTurn() {
    // TODO: Distribusi kartu skill di awal giliran (Orang 5)
    // TODO: Panggil UI untuk menerima input perintah pemain
    // TODO: Eksekusi perintah
    // TODO: checkWinCondition()
    // TODO: Jika tidak ada extra turn, lanjut ke pemain berikutnya
    throw GameException("executeTurn: belum diimplementasikan (menunggu Player & Board)");
}

void GameEngine::moveCurrentPlayer(int steps) {
    // TODO: Ambil pemain aktif
    // Player& p = getCurrentPlayer();
    // int oldPos = p.getPosition();
    // p.move(steps, board->size());

    // TODO: Deteksi apakah melewati GO (newPos < oldPos setelah wrap)
    // if (p.getPosition() < oldPos) awardPassGoSalary(p);

    // TODO: handleLanding(p, board->getTileByIndex(p.getPosition()))
    (void)steps;
    throw GameException("moveCurrentPlayer: belum diimplementasikan (menunggu Player & Board)");
}

void GameEngine::handleLanding(Player& p, Tile& t) {
    // Polimorfisme — setiap tile mengurus logikanya sendiri
    // t.onLand(p, *this);
    (void)p; (void)t;
    throw GameException("handleLanding: belum diimplementasikan (menunggu Tile hierarchy)");
}

void GameEngine::checkWinCondition() {
    // TODO: Hitung pemain aktif; jika <= 1, panggil endGame()
    // TODO: Jika maxTurn > 0 dan currentTurn >= maxTurn, panggil endGame()
    throw GameException("checkWinCondition: belum diimplementasikan (menunggu Player)");
}

void GameEngine::endGame() {
    gameOver = true;
    // TODO: Tentukan pemenang berdasarkan getTotalWealth() (Orang 1)
    // TODO: Tampilkan hasil akhir via GameUI (Orang 5)
}

// ─────────────────────────────────────────────────────────────────────────────
// Akses state
// ─────────────────────────────────────────────────────────────────────────────

Player& GameEngine::getCurrentPlayer() {
    int idx = turnManager.getCurrentPlayerIndex();
    if (idx < 0 || idx >= static_cast<int>(players.size())) {
        throw std::out_of_range("getCurrentPlayer: index pemain di luar batas");
    }
    return *players[idx];
}

Player* GameEngine::getPlayerByName(const std::string& name) {
    for (auto* p : players) {
        // TODO: Ganti dengan p->getUsername() == name saat Player.hpp tersedia
        (void)p; (void)name;
    }
    return nullptr;
}

std::vector<Player*> GameEngine::getActivePlayers() const {
    std::vector<Player*> active;
    for (auto* p : players) {
        // TODO: if (!p->isBankrupt()) active.push_back(p);
        (void)p;
    }
    return active;
}

Board& GameEngine::getBoard() {
    if (!board) throw GameException("Board belum diinisialisasi");
    return *board;
}

Dice&         GameEngine::getDice()             { return dice;             }
TurnManager&  GameEngine::getTurnManager()      { return turnManager;      }

Bank& GameEngine::getBank() {
    if (!bank) throw GameException("Bank belum di-inject");
    return *bank;
}
AuctionManager& GameEngine::getAuctionManager() {
    if (!auctionManager) throw GameException("AuctionManager belum di-inject");
    return *auctionManager;
}
BankruptcyManager& GameEngine::getBankruptcyManager() {
    if (!bankruptcyManager) throw GameException("BankruptcyManager belum di-inject");
    return *bankruptcyManager;
}
PropertyManager& GameEngine::getPropertyManager() {
    if (!propertyManager) throw GameException("PropertyManager belum di-inject");
    return *propertyManager;
}
CardManager& GameEngine::getCardManager() {
    if (!cardManager) throw GameException("CardManager belum di-inject");
    return *cardManager;
}
EffectManager& GameEngine::getEffectManager() {
    if (!effectManager) throw GameException("EffectManager belum di-inject");
    return *effectManager;
}
TransactionLogger& GameEngine::getLogger() {
    if (!logger) throw GameException("TransactionLogger belum di-inject");
    return *logger;
}

bool GameEngine::isGameOver()    const { return gameOver;                    }
int  GameEngine::getMaxTurn()    const { return maxTurn;                     }
int  GameEngine::getGoSalary()   const { return goSalary;                    }
int  GameEngine::getJailFine()   const { return jailFine;                    }
int  GameEngine::getCurrentTurn() const{ return turnManager.getTurnNumber(); }

// ─────────────────────────────────────────────────────────────────────────────
// Helper private
// ─────────────────────────────────────────────────────────────────────────────

void GameEngine::initBoard() {
    // TODO: Bangun Board dari std::vector<PropertyDef> hasil ConfigLoader (Orang 1 & 2)
}

void GameEngine::handleJailTurn(Player& p) {
    // TODO: Implementasikan logika penjara:
    //   - jailTurns < 3: tawarkan pilihan lempar double atau bayar denda
    //   - jailTurns == 3: paksa bayar denda lalu lempar dadu normal
    (void)p;
}

void GameEngine::awardPassGoSalary(Player& p) {
    // TODO: bank->paySalary(p, goSalary) saat Bank tersedia
    (void)p;
}

std::vector<bool> GameEngine::buildBankruptFlags() const {
    std::vector<bool> flags(players.size(), false);
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        // TODO: flags[i] = players[i]->isBankrupt();
    }
    return flags;
}