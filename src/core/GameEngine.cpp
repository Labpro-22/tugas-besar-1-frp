#include "../../include/core/GameEngine.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/CardTile.hpp"
#include "../../include/models/FestivalTile.hpp"
#include "../../include/models/FreeParkingTile.hpp"
#include "../../include/models/GoTile.hpp"
#include "../../include/models/GoToJailTile.hpp"
#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/RailroadProperty.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/TaxTile.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/UtilityProperty.hpp"
#include "../../include/utils/ConfigLoader.hpp"
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
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {
std::string normalizeColorGroup(const std::string& raw) {
    if (raw == "COKLAT") return "CK";
    if (raw == "BIRU_MUDA") return "BM";
    if (raw == "MERAH_MUDA") return "PK";
    if (raw == "ORANGE") return "OR";
    if (raw == "MERAH") return "MR";
    if (raw == "KUNING") return "KN";
    if (raw == "HIJAU") return "HJ";
    if (raw == "BIRU_TUA") return "BT";
    return raw;
}

const std::vector<std::string> kBoardOrder = {
    "GO",  "GRT", "DNU", "TSK", "PPH", "GBR", "BGR", "FES", "DPK", "BKS",
    "PEN", "MGL", "PLN", "SOL", "YOG", "STB", "MAL", "DNU", "SMG", "SBY",
    "BBP", "MKS", "KSP", "BLP", "MND", "TUG", "PLB", "PKB", "PAM", "MED",
    "PPJ", "BDG", "DEN", "FES", "MTR", "GUB", "KSP", "JKT", "PBM", "IKN"
};
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

GameEngine::GameEngine()
    : board(nullptr),
      gameOver(false),
      maxTurn(0),
            initialBalance(1000),
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

    initBoard();

    for (int i = 0; i < nPlayers && i < static_cast<int>(names.size()); ++i) {
        players.push_back(new Player(names[i], initialBalance));
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

    auto resolveDiceFlow = [&](CommandResult& flowResult) {
        Player& current = getCurrentPlayer();
        const int movingPlayerIndex = turnManager.getCurrentPlayerIndex();
        const std::string movingPlayerName = current.getUsername();
        const bool rolledDouble = dice.isDouble();
        const int total = dice.getTotal();

        auto buildMovementPath = [&](int fromIndex, int steps, int finalIndex) {
            if (!board || board->size() <= 0 || steps <= 0) {
                return;
            }

            std::vector<int> path;
            path.reserve(static_cast<size_t>(steps) + 1);
            const int boardSize = board->size();

            for (int i = 1; i <= steps; ++i) {
                path.push_back((fromIndex + i) % boardSize);
            }

            if (finalIndex >= 0 && !path.empty() && finalIndex != path.back()) {
                path.push_back(finalIndex);
            }

            if (path.empty()) {
                return;
            }

            flowResult.movement = MovementPayload{
                movingPlayerIndex,
                movingPlayerName,
                fromIndex,
                finalIndex,
                path
            };
        };

        // Alur pemain yang sedang berada di penjara.
        if (current.isJailed()) {
            if (current.getJailTurns() >= 3) {
                if (!current.canAfford(jailFine)) {
                    current.setStatus(PlayerStatus::BANKRUPT);
                    flowResult.addEvent(
                        GameEventType::BANKRUPTCY,
                        UiTone::ERROR,
                        "Bangkrut di Penjara",
                        current.getUsername() +
                            " tidak mampu membayar denda penjara sebesar M" +
                            std::to_string(jailFine) + ".");
                    current.resetConsecutiveDoubles();
                    flowResult.append(executeTurn());
                    return;
                }

                current.deductMoney(jailFine);
                current.setStatus(PlayerStatus::ACTIVE);
                current.setJailTurns(0);
                flowResult.addEvent(
                    GameEventType::MONEY,
                    UiTone::WARNING,
                    "Keluar Penjara",
                    current.getUsername() +
                        " wajib membayar denda M" + std::to_string(jailFine) +
                        " pada percobaan ke-4.");
            } else {
                if (!rolledDouble) {
                    current.incrementJailTurns();
                    current.resetConsecutiveDoubles();
                    flowResult.addEvent(
                        GameEventType::TURN,
                        UiTone::WARNING,
                        "Masih Dipenjara",
                        current.getUsername() +
                            " gagal mendapatkan double dan tetap di penjara "
                            "pada giliran ini.");
                    flowResult.append(executeTurn());
                    return;
                }

                current.setStatus(PlayerStatus::ACTIVE);
                current.setJailTurns(0);
                current.resetConsecutiveDoubles();
                flowResult.addEvent(
                    GameEventType::SYSTEM,
                    UiTone::SUCCESS,
                    "Double Penjara",
                    current.getUsername() +
                        " mendapatkan double dan keluar dari penjara.");

                const int oldPos = current.getPosition();
                flowResult.append(moveCurrentPlayer(total));
                buildMovementPath(oldPos, total, current.getPosition());
                current.resetConsecutiveDoubles();
                flowResult.append(executeTurn());
                return;
            }
        }

        if (rolledDouble) {
            current.incrementConsecutiveDoubles();
            if (current.getConsecutiveDoubles() >= 3) {
                if (!board) {
                    throw GameException("processCommand: board belum diinisialisasi");
                }

                current.setPosition(board->getIndexOf("PEN"));
                current.setStatus(PlayerStatus::JAILED);
                current.setJailTurns(0);
                current.resetConsecutiveDoubles();

                flowResult.addEvent(
                    GameEventType::DICE,
                    UiTone::WARNING,
                    "Triple Double",
                    current.getUsername() +
                        " melempar double 3 kali berturut-turut. Bidak langsung "
                        "dipindah ke penjara dan giliran berakhir.");

                flowResult.append(executeTurn());
                return;
            }
        } else {
            current.resetConsecutiveDoubles();
        }

        const int oldPos = current.getPosition();
        flowResult.append(moveCurrentPlayer(total));
        buildMovementPath(oldPos, total, current.getPosition());

        if (current.isJailed()) {
            current.resetConsecutiveDoubles();
            flowResult.append(executeTurn());
            return;
        }

        if (rolledDouble) {
            flowResult.addEvent(
                GameEventType::TURN,
                UiTone::INFO,
                "Double",
                current.getUsername() +
                    " mendapatkan giliran tambahan karena melempar double.");
            return;
        }

        flowResult.append(executeTurn());
    };

    switch (cmd.type) {
    case CommandType::HELP:
        result.commandName = "BANTUAN";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Daftar Perintah",
            "LEMPAR_DADU, PILIH_BUANG_KARTU <index_0_3>, ATUR_DADU X Y, CETAK_PAPAN, CETAK_LOG [N], SIMPAN <file>, MUAT <file>, KELUAR"
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
        resolveDiceFlow(result);
        return result;
    }

    case CommandType::RESOLVE_SKILL_DROP: {
        result.commandName = "PILIH_BUANG_KARTU";
        if (!cardManager) {
            throw GameException("CardManager belum di-inject untuk menyelesaikan pending kartu skill.");
        }

        Player& current = getCurrentPlayer();
        if (!cardManager->hasPendingSkillDrop(current)) {
            throw GameException("Tidak ada pending kartu skill untuk pemain saat ini.");
        }

        if (cmd.args.empty()) {
            throw GameException("PILIH_BUANG_KARTU membutuhkan argumen index 0..3.");
        }

        int discardIndex = -1;
        try {
            discardIndex = std::stoi(cmd.args[0]);
        } catch (const std::exception&) {
            throw GameException("Argumen PILIH_BUANG_KARTU harus berupa angka 0..3.");
        }

        cardManager->resolvePendingSkillDrop(current, discardIndex);
        result.addEvent(
            GameEventType::CARD,
            UiTone::SUCCESS,
            "Kartu Skill",
            current.getUsername() + " telah menyelesaikan pemilihan kartu yang dibuang.");
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
        resolveDiceFlow(result);
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
        result.success = false;
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::WARNING,
            "Perintah Dinonaktifkan",
            "Giliran kini berpindah otomatis setelah aksi dadu selesai diproses."
        );
        return result;

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
    result.commandName = "TRANSISI_GILIRAN";

    if (players.empty()) {
        throw GameException("executeTurn: tidak ada pemain aktif");
    }

    Player& current = getCurrentPlayer();
    if (cardManager && !current.isBankrupt()) {
        cardManager->drawSkillCard(current);
        if (cardManager->hasPendingSkillDrop(current)) {
            result.addEvent(
                GameEventType::CARD,
                UiTone::WARNING,
                "Kartu Kemampuan",
                current.getUsername() + " harus memilih 1 kartu untuk dibuang (maksimal 3 kartu di tangan)."
            );

            PromptRequest prompt;
            prompt.key = "SKILL_DROP";
            prompt.message = "Pilih index kartu yang dibuang (0..2 dari tangan, 3 = kartu baru).";
            prompt.options = cardManager->getPendingSkillDropOptions(current);
            prompt.required = true;
            result.prompt = prompt;
        } else {
            result.addEvent(
                GameEventType::CARD,
                UiTone::INFO,
                "Kartu Kemampuan",
                current.getUsername() + " mendapatkan 1 kartu kemampuan acak."
            );
        }
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
    const bool crossedGo = (player.getPosition() < oldPos);
    Tile& landing = board->getTileByIndex(player.getPosition());
    const bool landedOnGoToJail = (landing.getCode() == "PPJ");

    result.addEvent(
        GameEventType::MOVEMENT,
        UiTone::INFO,
        "Pergerakan",
        player.getUsername() + " maju " + std::to_string(steps) + " petak."
    );

    if (crossedGo && !landedOnGoToJail) {
        awardPassGoSalary(player);
        result.addEvent(
            GameEventType::MONEY,
            UiTone::SUCCESS,
            "Lewat GO",
            player.getUsername() + " menerima gaji GO sebesar M" + std::to_string(goSalary) + "."
        );
    }

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
    delete board;
    board = new Board();

    ConfigLoader loader;
    const std::vector<PropertyDef> propertyDefs = loader.loadProperties("config/property.txt");
    const std::map<int, int> railroadRent = loader.loadRailroadConfig("config/railroad.txt");
    const std::map<int, int> utilityMultiplier = loader.loadUtilityConfig("config/utility.txt");
    const TaxConfig taxConfig = loader.loadTaxConfig("config/tax.txt");
    const SpecialConfig specialConfig = loader.loadSpecialConfig("config/special.txt");
    const MiscConfig miscConfig = loader.loadMiscConfig("config/misc.txt");

    goSalary = specialConfig.getGoSalary();
    jailFine = specialConfig.getJailFine();
    maxTurn = miscConfig.getMaxTurn();
    initialBalance = miscConfig.getInitialBalance();

    std::unordered_map<std::string, PropertyDef> propertyByCode;
    for (const PropertyDef& def : propertyDefs) {
        propertyByCode[def.getCode()] = def;
    }

    JailTile* jailTile = nullptr;
    for (int idx = 0; idx < static_cast<int>(kBoardOrder.size()); ++idx) {
        const std::string& code = kBoardOrder[idx];

        const auto propertyIt = propertyByCode.find(code);
        if (propertyIt != propertyByCode.end()) {
            const PropertyDef& def = propertyIt->second;
            std::shared_ptr<Property> property;

            if (def.getTypeName() == "STREET") {
                property = std::make_shared<StreetProperty>(
                    def.getCode(),
                    def.getName(),
                    normalizeColorGroup(def.getColorGroup()),
                    def.getPurchasePrice(),
                    def.getMortgageValue(),
                    def.getHouseCost(),
                    def.getHotelCost(),
                    def.getRentLevels());
            } else if (def.getTypeName() == "RAILROAD") {
                property = std::make_shared<RailroadProperty>(
                    def.getCode(),
                    def.getName(),
                    def.getPurchasePrice(),
                    def.getMortgageValue(),
                    railroadRent);
            } else if (def.getTypeName() == "UTILITY") {
                property = std::make_shared<UtilityProperty>(
                    def.getCode(),
                    def.getName(),
                    def.getPurchasePrice(),
                    def.getMortgageValue(),
                    utilityMultiplier);
            } else {
                throw GameException("Jenis properti tidak dikenal untuk kode '" + code + "'.");
            }

            board->addTile(std::make_unique<PropertyTile>(idx, property));
            continue;
        }

        if (code == "GO") {
            board->addTile(std::make_unique<GoTile>(idx, goSalary));
        } else if (code == "DNU") {
            board->addTile(std::make_unique<CardTile>(idx, code, CardDrawType::COMMUNITY));
        } else if (code == "KSP") {
            board->addTile(std::make_unique<CardTile>(idx, code, CardDrawType::CHANCE));
        } else if (code == "FES") {
            board->addTile(std::make_unique<FestivalTile>(idx, code));
        } else if (code == "PPH") {
            board->addTile(std::make_unique<TaxTile>(
                idx,
                TaxType::PPH,
                taxConfig.getPphFlat(),
                taxConfig.getPphPercentage()));
        } else if (code == "PBM") {
            board->addTile(std::make_unique<TaxTile>(
                idx,
                TaxType::PBM,
                taxConfig.getPbmFlat()));
        } else if (code == "PEN") {
            auto jail = std::make_unique<JailTile>(idx);
            jailTile = jail.get();
            board->addTile(std::move(jail));
        } else if (code == "PPJ") {
            if (!jailTile) {
                throw GameException("Konfigurasi papan tidak valid: tile PPJ muncul sebelum PEN.");
            }
            board->addTile(std::make_unique<GoToJailTile>(idx, *jailTile));
        } else if (code == "BBP") {
            board->addTile(std::make_unique<FreeParkingTile>(idx));
        } else {
            throw GameException("Kode tile tidak dikenali saat inisialisasi papan: '" + code + "'.");
        }
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