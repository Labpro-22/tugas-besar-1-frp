#include "../../include/core/GameEngine.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Tile.hpp"
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

#include <algorithm>
#include <sstream>
#include <stdexcept>

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
    delete board;
    board = nullptr;

    for (Player* player : players) {
        delete player;
    }
    players.clear();
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

CommandResult GameEngine::startNewGame(int nPlayers, std::vector<std::string> names) {
    CommandResult result;
    result.commandName = "MULAI";

    for (Player* player : players) {
        delete player;
    }
    players.clear();

    if (!board) {
        board = new Board();
    }

    for (int i = 0; i < nPlayers && i < static_cast<int>(names.size()); ++i) {
        players.push_back(new Player(names[i], 1000));
    }

    turnManager.initializeOrder(static_cast<int>(players.size()));

    gameOver = false;

    if (cardManager) {
        cardManager->initializeDecks();
    }

    result.addEvent(
        GameEventType::SYSTEM,
        UiTone::SUCCESS,
        "Permainan Dimulai",
        "Permainan baru berhasil dibuat untuk " + std::to_string(players.size()) + " pemain."
    );
    return result;
}

CommandResult GameEngine::loadGame(const std::string& filename) {
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

CommandResult GameEngine::processCommand(const Command& cmd) {
    CommandResult result;

    switch (cmd.type) {
    case CommandType::HELP:
        result.commandName = "BANTUAN";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Daftar Perintah",
            "LEMPAR_DADU, ATUR_DADU X Y, CETAK_PAPAN, CETAK_LOG [N], SIMPAN <file>, MUAT <file>, AKHIRI_GILIRAN, KELUAR"
        );
        return result;

    case CommandType::ROLL_DICE: {
        result.commandName = "LEMPAR_DADU";
        const std::pair<int, int> roll = dice.rollRandom();
        result.addEvent(
            GameEventType::DICE,
            UiTone::INFO,
            "Hasil Dadu",
            std::to_string(roll.first) + " + " + std::to_string(roll.second) + " = " + std::to_string(dice.getTotal())
        );
        result.append(moveCurrentPlayer(dice.getTotal()));
        return result;
    }

    case CommandType::SET_DICE: {
        result.commandName = "ATUR_DADU";
        if (cmd.args.size() < 2) {
            throw GameException("ATUR_DADU membutuhkan 2 argumen angka.");
        }

        int d1 = 0;
        int d2 = 0;
        try {
            d1 = std::stoi(cmd.args[0]);
            d2 = std::stoi(cmd.args[1]);
        } catch (const std::exception&) {
            throw GameException("Argumen ATUR_DADU harus berupa angka bulat.");
        }

        const std::pair<int, int> roll = dice.setManual(d1, d2);
        result.addEvent(
            GameEventType::DICE,
            UiTone::INFO,
            "Dadu Manual",
            std::to_string(roll.first) + " + " + std::to_string(roll.second) + " = " + std::to_string(dice.getTotal())
        );
        result.append(moveCurrentPlayer(dice.getTotal()));
        return result;
    }

    case CommandType::SAVE:
        result.commandName = "SIMPAN";
        result.addEvent(
            GameEventType::SAVE_LOAD,
            UiTone::WARNING,
            "Belum Diimplementasikan",
            "Fitur SIMPAN akan diaktifkan setelah integrasi SaveLoadManager selesai."
        );
        return result;

    case CommandType::LOAD:
        result.commandName = "MUAT";
        throw GameException("MUAT belum diimplementasikan di fase ini.");

    case CommandType::PRINT_LOG:
        result.commandName = "CETAK_LOG";
        result.addEvent(
            GameEventType::LOG,
            UiTone::INFO,
            "Cetak Log",
            "Render log transaksi ditangani oleh layer UI (views)."
        );
        return result;

    case CommandType::PRINT_BOARD:
        result.commandName = "CETAK_PAPAN";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Cetak Papan",
            "Papan akan dirender oleh BoardRenderer pada layer UI."
        );
        return result;

    case CommandType::END_TURN:
        result.commandName = "AKHIRI_GILIRAN";
        return executeTurn();

    case CommandType::START_GAME:
        return startNewGame(2, {"Pemain1", "Pemain2"});

    case CommandType::EXIT:
        result.commandName = "KELUAR";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Keluar",
            "Permainan ditutup oleh pemain."
        );
        return result;

    case CommandType::UNKNOWN:
    default:
        throw GameException("Perintah tidak dikenali: " + cmd.raw);
    }
}

CommandResult GameEngine::executeTurn() {
    CommandResult result;
    result.commandName = "AKHIRI_GILIRAN";

    if (players.empty()) {
        throw GameException("executeTurn: tidak ada pemain aktif");
    }

    Player& current = getCurrentPlayer();
    if (cardManager) {
        cardManager->drawSkillCard(current);
        result.addEvent(
            GameEventType::CARD,
            UiTone::INFO,
            "Kartu Kemampuan",
            current.getUsername() + " mendapatkan 1 kartu kemampuan acak."
        );
    }

    checkWinCondition();
    if (gameOver) {
        result.addEvent(
            GameEventType::GAME_OVER,
            UiTone::SUCCESS,
            "Permainan Selesai",
            "Kondisi kemenangan telah terpenuhi."
        );
        return result;
    }

    turnManager.nextPlayer(buildBankruptFlags());
    result.addEvent(
        GameEventType::TURN,
        UiTone::INFO,
        "Giliran Berikutnya",
        "Sekarang giliran " + getCurrentPlayer().getUsername() + "."
    );
    return result;
}

CommandResult GameEngine::moveCurrentPlayer(int steps) {
    CommandResult result;
    result.commandName = "PINDAH";

    if (!board) {
        throw GameException("moveCurrentPlayer: board belum diinisialisasi");
    }
    if (board->size() <= 0) {
        throw GameException("moveCurrentPlayer: papan belum memiliki petak.");
    }

    Player& player = getCurrentPlayer();
    const int oldPos = player.getPosition();
    player.move(steps, board->size());

    result.addEvent(
        GameEventType::MOVEMENT,
        UiTone::INFO,
        "Pergerakan",
        player.getUsername() + " maju " + std::to_string(steps) + " petak."
    );

    if (player.getPosition() < oldPos) {
        awardPassGoSalary(player);
        result.addEvent(
            GameEventType::MONEY,
            UiTone::SUCCESS,
            "Lewat GO",
            player.getUsername() + " menerima gaji GO sebesar M" + std::to_string(goSalary) + "."
        );
    }

    Tile& landing = board->getTileByIndex(player.getPosition());
    result.addEvent(
        GameEventType::LANDING,
        UiTone::INFO,
        "Mendarat",
        "Bidak mendarat di " + landing.getName() + " (" + landing.getCode() + ")."
    );
    handleLanding(player, landing);
    return result;
}

void GameEngine::handleLanding(Player& p, Tile& t) {
    t.onLand(p, *this);
}

void GameEngine::checkWinCondition() {
    int activeCount = 0;
    for (Player* player : players) {
        if (player && !player->isBankrupt()) {
            activeCount++;
        }
    }

    if (activeCount <= 1) {
        endGame();
        return;
    }

    if (maxTurn > 0 && getCurrentTurn() >= maxTurn) {
        endGame();
    }
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
        if (p && p->getUsername() == name) {
            return p;
        }
    }
    return nullptr;
}

std::vector<Player*> GameEngine::getActivePlayers() const {
    std::vector<Player*> active;
    for (auto* p : players) {
        if (p && !p->isBankrupt()) {
            active.push_back(p);
        }
    }
    return active;
}

const std::vector<Player*>& GameEngine::getPlayers() const {
    return players;
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
    if (!board) {
        board = new Board();
    }
}

void GameEngine::handleJailTurn(Player& p) {
    // TODO: Implementasikan logika penjara:
    //   - jailTurns < 3: tawarkan pilihan lempar double atau bayar denda
    //   - jailTurns == 3: paksa bayar denda lalu lempar dadu normal
    (void)p;
}

void GameEngine::awardPassGoSalary(Player& p) {
    p.addMoney(goSalary);
}

std::vector<bool> GameEngine::buildBankruptFlags() const {
    std::vector<bool> flags(players.size(), false);
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        if (players[i]) {
            flags[i] = players[i]->isBankrupt();
        }
    }
    return flags;
}