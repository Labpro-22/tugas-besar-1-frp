#include "../../include/core/GameEngine.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/core/EffectManager.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/CardTile.hpp"
#include "../../include/models/FestivalTile.hpp"
#include "../../include/models/SkillCards.hpp"
#include "../../include/models/FreeParkingTile.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/models/GoTile.hpp"
#include "../../include/models/GoToJailTile.hpp"
#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/RailroadProperty.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/TaxTile.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/UtilityProperty.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/utils/ConfigLoader.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/utils/Gamestateserializer.hpp"
#include "../../include/utils/Saveloadmanager.hpp"


#include "../../include/models/Player.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/core/Command.hpp"


#include "../../include/models/Bank.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/core/EffectManager.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/utils/Saveloadmanager.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;

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

std::string normalizeTileName(std::string raw) {
    std::replace(raw.begin(), raw.end(), '_', ' ');
    return raw;
}

std::string trimCopy(std::string raw) {
    auto isSpace = [](unsigned char c) {
        return std::isspace(c) != 0;
    };

    raw.erase(raw.begin(), std::find_if(raw.begin(), raw.end(),
        [&](unsigned char c) { return !isSpace(c); }));
    raw.erase(std::find_if(raw.rbegin(), raw.rend(),
        [&](unsigned char c) { return !isSpace(c); }).base(), raw.end());
    return raw;
}

std::string normalizePlayerNameKey(std::string raw) {
    raw = trimCopy(raw);
    std::transform(raw.begin(), raw.end(), raw.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return raw;
}

string toPlayerStatusString(const Player& p) {
    if (p.isBankrupt()) return "BANKRUPT";
    if (p.isJailed()) {
        const int jailTurns = p.getJailTurns();
        if (jailTurns > 0) {
            return "JAILED_" + to_string(jailTurns);
        }
        return "JAILED";
    }
    return "ACTIVE";
}

PlayerStatus toPlayerStatusEnum(const string& status) {
    if (status == "ACTIVE") return PlayerStatus::ACTIVE;
    if (status == "BANKRUPT") return PlayerStatus::BANKRUPT;
    if (status == "JAILED" || status.rfind("JAILED_", 0) == 0) return PlayerStatus::JAILED;
    throw SaveLoadException("Status pemain tidak dikenali: " + status);
}

int extractJailTurns(const string& status) {
    if (status == "JAILED") return 0;
    if (status.rfind("JAILED_", 0) != 0) return 0;
    const string num = status.substr(7);
    if (num.empty()) return 0;
    return stoi(num);
}

string toPropertyTypeString(PropertyType type) {
    switch (type) {
    case PropertyType::STREET: return "street";
    case PropertyType::RAILROAD: return "railroad";
    case PropertyType::UTILITY: return "utility";
    }
    throw SaveLoadException("Jenis properti tidak dikenali saat serialisasi");
}

OwnershipStatus toOwnershipStatusEnum(const string& status) {
    if (status == "BANK") return OwnershipStatus::BANK;
    if (status == "OWNED") return OwnershipStatus::OWNED;
    if (status == "MORTGAGED") return OwnershipStatus::MORTGAGED;
    throw SaveLoadException("Status properti tidak dikenali: " + status);
}

string toOwnershipStatusString(OwnershipStatus status) {
    switch (status) {
    case OwnershipStatus::BANK: return "BANK";
    case OwnershipStatus::OWNED: return "OWNED";
    case OwnershipStatus::MORTGAGED: return "MORTGAGED";
    }
    throw SaveLoadException("Status properti enum tidak dikenali");
}

std::string upperCopyText(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return value;
}

bool isDigitsOnly(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    return std::all_of(value.begin(), value.end(),
        [](unsigned char c) { return std::isdigit(c) != 0; });
}

struct SkillTargetCatalog {
    std::unordered_map<int, std::string> anyTileIdToCode;
    std::unordered_set<std::string> anyTileCodes;
    std::unordered_map<int, std::string> propertyIdToCode;
    std::unordered_set<std::string> propertyCodes;
};

SkillTargetCatalog loadSkillTargetCatalogFromConfig() {
    ConfigLoader loader;
    const std::vector<PropertyDef> propertyDefs = loader.loadProperties("config/property.txt");
    const std::vector<ActionTileDef> actionDefs = loader.loadActionTiles("config/aksi.txt");

    SkillTargetCatalog catalog;
    for (const PropertyDef& def : propertyDefs) {
        const std::string code = upperCopyText(def.getCode());
        catalog.propertyIdToCode[def.getId()] = code;
        catalog.propertyCodes.insert(code);
        catalog.anyTileIdToCode[def.getId()] = code;
        catalog.anyTileCodes.insert(code);
    }

    for (const ActionTileDef& def : actionDefs) {
        const std::string code = upperCopyText(def.getCode());
        catalog.anyTileIdToCode[def.getId()] = code;
        catalog.anyTileCodes.insert(code);
    }

    return catalog;
}

std::string normalizeTileTargetByConfig(const std::string& rawTarget,
                                        const std::unordered_map<int, std::string>& idToCode,
                                        const std::unordered_set<std::string>& validCodes,
                                        const std::string& sourceLabel) {
    const std::string trimmed = trimCopy(rawTarget);
    if (trimmed.empty()) {
        throw GameException("TARGET tidak boleh kosong.");
    }

    if (isDigitsOnly(trimmed)) {
        int targetId = 0;
        try {
            targetId = std::stoi(trimmed);
        } catch (const std::exception&) {
            throw GameException("TARGET ID tidak valid: " + trimmed + ".");
        }

        auto it = idToCode.find(targetId);
        if (it == idToCode.end()) {
            throw GameException(
                "TARGET ID '" + trimmed + "' tidak ditemukan pada " + sourceLabel + ".");
        }
        return it->second;
    }

    const std::string targetCode = upperCopyText(trimmed);
    if (validCodes.find(targetCode) == validCodes.end()) {
        throw GameException(
            "TARGET kode '" + targetCode + "' tidak ditemukan pada " + sourceLabel + ".");
    }
    return targetCode;
}

bool isJailFreeCardTypeName(const string& typeName) {
    return typeName == "GetOutOfJailCard";
}

bool isKnownSkillCardTypeName(const string& typeName) {
    return typeName == "MoveCard" ||
           typeName == "DiscountCard" ||
           typeName == "ShieldCard" ||
           typeName == "TeleportCard" ||
           typeName == "LassoCard" ||
           typeName == "DemolitionCard";
}

int findPlayerIndexByPointer(const std::vector<Player*>& players, const Player* target) {
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i] == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void appendForwardPath(std::vector<int>& path, int& index, int steps, int boardSize) {
    if (boardSize <= 0 || steps <= 0) {
        return;
    }
    for (int i = 0; i < steps; ++i) {
        index = (index + 1) % boardSize;
        path.push_back(index);
    }
}

void validatePlayerInventoryForSave(const Player& player) {
    const auto& hand = player.getHandCards();
    if (hand.size() > 3) {
        throw SaveLoadException(
            "Inventory skill card player " + player.getUsername() +
            " melebihi batas 3 kartu.");
    }

    for (const auto& card : hand) {
        if (!card) {
            throw SaveLoadException(
                "Inventory player " + player.getUsername() +
                " mengandung skill card null.");
        }
    }
}

void validateSavedInventoryForLoad(const SavedPlayerState& savedPlayer) {
    int skillCardCount = 0;
    bool hasJailFreeCard = false;

    for (size_t i = 0; i < savedPlayer.getCards().size(); ++i) {
        const SavedCardState& savedCard = savedPlayer.getCards()[i];
        const string& typeName = savedCard.getType();

        if (isJailFreeCardTypeName(typeName)) {
            if (hasJailFreeCard) {
                throw SaveLoadException(
                    "Player " + savedPlayer.getUsername() +
                    " memiliki lebih dari satu GetOutOfJailCard pada save.");
            }
            if (i + 1 != savedPlayer.getCards().size()) {
                throw SaveLoadException(
                    "GetOutOfJailCard untuk player " +
                    savedPlayer.getUsername() +
                    " harus berada di slot inventory terakhir pada save.");
            }
            if (!savedCard.getValue().empty() || !savedCard.getDuration().empty()) {
                throw SaveLoadException(
                    "GetOutOfJailCard untuk player " +
                    savedPlayer.getUsername() +
                    " tidak boleh memiliki value/duration pada save.");
            }
            hasJailFreeCard = true;
            continue;
        }

        if (!isKnownSkillCardTypeName(typeName)) {
            throw SaveLoadException(
                "Jenis skill card tidak dikenali pada save player " +
                savedPlayer.getUsername() + ": " + typeName);
        }

        if (hasJailFreeCard) {
            throw SaveLoadException(
                "Skill card untuk player " + savedPlayer.getUsername() +
                " tidak boleh berada setelah GetOutOfJailCard pada save.");
        }

        ++skillCardCount;
        if (skillCardCount > 3) {
            throw SaveLoadException(
                "Player " + savedPlayer.getUsername() +
                " memiliki lebih dari 3 skill card pada save.");
        }
    }
}
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

GameEngine::GameEngine()
    : board(nullptr),
      gameOver(false),
      gameStarted(false),
      turnActionTaken(false),
      diceRolledThisTurn(false),
      extraRollAllowedThisTurn(false),
      gameOverReason_(GameOverReason::NONE),
      maxTurn(0),
      initialBalance(1000),
      goSalary(200),
      jailFine(50),
      dice(6),
      bank(std::make_unique<Bank>()),
      logger(std::make_unique<TransactionLogger>()),
      cardManager(std::make_unique<CardManager>()),
      effectManager(std::make_unique<EffectManager>()),
      propertyManager(std::make_unique<PropertyManager>(*this, *bank, *logger)),
      auctionManager(std::make_unique<AuctionManager>(*this, *bank, *logger)),
      bankruptcyManager(std::make_unique<BankruptcyManager>(
          *this, *bank, *logger, *propertyManager, *auctionManager)) {}

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

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

CommandResult GameEngine::startNewGame(int nPlayers, std::vector<std::string> names) {
    CommandResult result;
    result.commandName = "MULAI";

    if (nPlayers < 2 || nPlayers > 4) {
        throw InvalidPlayerCountException(nPlayers);
    }
    if (static_cast<int>(names.size()) != nPlayers) {
        throw GameException("Jumlah nama pemain tidak sesuai.");
    }

    std::unordered_set<std::string> usedNames;
    std::vector<std::string> sanitizedNames;
    sanitizedNames.reserve(names.size());
    for (const std::string& rawName : names) {
        const std::string cleanName = trimCopy(rawName);
        if (cleanName.empty()) {
            throw InvalidPlayerNameException(rawName);
        }

        const std::string key = normalizePlayerNameKey(cleanName);
        if (!usedNames.insert(key).second) {
            throw DuplicatePlayerNameException(cleanName);
        }

        sanitizedNames.push_back(cleanName);
    }

    for (Player* player : players) {
        delete player;
    }
    players.clear();

    initBoard();

    for (int i = 0; i < nPlayers && i < static_cast<int>(sanitizedNames.size()); ++i) {
        players.push_back(new Player(sanitizedNames[i], initialBalance));
    }

    turnManager.initializeOrder(static_cast<int>(players.size()));

    gameOver = false;
    gameStarted = true;
    gameOverReason_ = GameOverReason::NONE;
    resetTurnActionFlags();

    if (logger) {
        logger->clear();
        logger->setCurrentTurn(turnManager.getTurnNumber());
    }

    if (cardManager) {
        cardManager->initializeDecks();
    }

    if (!players.empty()) {
        Player& first = getCurrentPlayer();
        if (effectManager) {
            effectManager->onTurnStart(first, *this);
        }
        if (cardManager && !first.isBankrupt()) {
            cardManager->drawSkillCard(first);
            if (cardManager->hasPendingSkillDrop(first)) {
                // Kondisi ini tidak diharapkan saat start game.
                cardManager->resolvePendingSkillDrop(first, 0);
            }
        }
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
    if (!players.empty()) {
        throw GameException("MUAT hanya tersedia sebelum permainan dimulai.");
    }

    if (board == nullptr) {
        initBoard();
    }

    SaveLoadManager saveLoad;
    saveLoad.loadInto(*this, filename);

    if (logger) {
        logger->setCurrentTurn(turnManager.getTurnNumber());
    }

    gameStarted = true;
    gameOverReason_ = GameOverReason::NONE;
    resetTurnActionFlags();

    CommandResult result;
    result.commandName = "MUAT";
    result.addEvent(
        GameEventType::SAVE_LOAD,
        UiTone::SUCCESS,
        "Berhasil Memuat",
        "State permainan berhasil dimuat dari file: " + filename);
    return result;
}

bool GameEngine::tryLoadGame(const std::string& filename, CommandResult& outResult, std::string& outError) {
    outError.clear();
    try {
        outResult = loadGame(filename);
        return true;
    } catch (const std::exception& e) {
        outResult = CommandResult{};
        outError = e.what();
        return false;
    }
}

bool GameEngine::canSaveAtTurnStart() const {
    return gameStarted && !players.empty() && !gameOver && !turnActionTaken;
}

SaveGameResult GameEngine::saveGame(const std::string& filename, bool overwrite) {
    SaveGameResult result;

    if (!canSaveAtTurnStart()) {
        result.status = SaveGameStatus::ERROR;
        result.message = "SIMPAN hanya boleh dilakukan di awal giliran sebelum aksi apapun.";
        return result;
    }

    std::string targetFilename = trimCopy(filename);
    if (targetFilename.empty()) {
        targetFilename = "game_save.nmp";
    }

    std::error_code ec;
    if (!overwrite && std::filesystem::exists(targetFilename, ec) && !ec) {
        result.status = SaveGameStatus::FILE_EXISTS;
        result.message = "File \"" + targetFilename + "\" sudah ada.";
        return result;
    }

    try {
        SaveLoadManager manager;
        manager.save(*this, targetFilename);
        if (logger) {
            logger->logSave(getCurrentPlayer().getUsername(), targetFilename);
        }
        result.status = SaveGameStatus::SUCCESS;
        result.message = "Permainan berhasil disimpan ke: " + targetFilename;
        return result;
    } catch (const std::exception& e) {
        result.status = SaveGameStatus::ERROR;
        result.message = e.what();
        return result;
    }
}

void GameEngine::run() {
    // Loop utama — berjalan hingga gameOver di-set oleh checkWinCondition/endGame
    while (!gameOver) {
        executeTurn();
    }
}

CommandResult GameEngine::processCommand(const Command& cmd) {
    CommandResult result;
    auto finalizeResult = [&]() -> CommandResult {
        flushEvents(result);
        return result;
    };

    if (players.empty() &&
        cmd.type != CommandType::START_GAME &&
        cmd.type != CommandType::LOAD &&
        cmd.type != CommandType::HELP &&
        cmd.type != CommandType::EXIT) {
        throw GameException("Game belum dimulai. Gunakan MULAI terlebih dahulu.");
    }
    if (!players.empty() && (cmd.type == CommandType::START_GAME || cmd.type == CommandType::LOAD)) {
        throw GameException("Perintah ini hanya tersedia sebelum permainan dimulai.");
    }

    if (logger) {
        logger->setCurrentTurn(turnManager.getTurnNumber());
    }

    auto upperCopy = [](std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return text;
    };

    auto resolvePropertyByCode = [&](const std::string& code) -> Property& {
        if (!board) {
            throw GameException("Board belum diinisialisasi");
        }
        Tile& tile = board->getTileByCode(upperCopy(code));
        if (!tile.isProperty()) {
            throw GameException("Tile bukan properti: " + code);
        }
        return static_cast<PropertyTile&>(tile).getProperty();
    };

    auto resolveDiceFlow = [&](CommandResult& flowResult) {
        Player& current = getCurrentPlayer();
        const bool rolledDouble = dice.isDouble();
        const int total = dice.getTotal();

        diceRolledThisTurn = true;
        turnActionTaken = true;
        extraRollAllowedThisTurn = false;

        if (logger) {
            string landed = "?";
            if (board && board->size() > 0) {
                const int nextPos = (current.getPosition() + total) % board->size();
                landed = board->getTileByIndex(nextPos).getCode();
            }
            logger->logDice(current.getUsername(), dice.getDie1(), dice.getDie2(), landed);
        }

        // Alur pemain yang sedang berada di penjara.
        if (current.isJailed()) {
            if (current.getJailTurns() >= 3) {
                Player* jailedPlayer = &current;
                if (!current.canAfford(jailFine)) {
                    flowResult.addEvent(
                        GameEventType::BANKRUPTCY,
                        UiTone::WARNING,
                        "Denda Penjara",
                        current.getUsername() +
                            " wajib membayar denda penjara M" +
                            std::to_string(jailFine) +
                            " pada percobaan ke-4. Jika uang tidak cukup, "
                            "likuidasi akan dijalankan.");
                    getBankruptcyManager().handleDebt(current, jailFine, nullptr);
                    if (hasPendingContinuation()) {
                        chainPendingContinuation([this, jailedPlayer, total, rolledDouble]() {
                            CommandResult resumed;
                            if (jailedPlayer->isBankrupt()) {
                                jailedPlayer->resetConsecutiveDoubles();
                                resumed.append(executeTurn());
                                return resumed;
                            }

                            jailedPlayer->setStatus(PlayerStatus::ACTIVE);
                            jailedPlayer->setJailTurns(0);
                            if (logger) {
                                logger->log(jailedPlayer->getUsername(), "BAYAR_DENDA",
                                    "Bayar denda penjara M" + std::to_string(jailFine));
                            }
                            resumed.addEvent(
                                GameEventType::MONEY,
                                UiTone::WARNING,
                                "Keluar Penjara",
                                jailedPlayer->getUsername() +
                                    " berhasil melunasi denda penjara M" +
                                    std::to_string(jailFine) +
                                    " dan keluar dari penjara.");
                            continueTurnAfterDiceResolution(
                                resumed, *jailedPlayer, total, rolledDouble);
                            return resumed;
                        });
                        return;
                    }

                    if (current.isBankrupt()) {
                        current.resetConsecutiveDoubles();
                        flowResult.append(executeTurn());
                        return;
                    }

                    current.setStatus(PlayerStatus::ACTIVE);
                    current.setJailTurns(0);
                    if (logger) {
                        logger->log(current.getUsername(), "BAYAR_DENDA",
                            "Bayar denda penjara M" + std::to_string(jailFine));
                    }
                    flowResult.addEvent(
                        GameEventType::MONEY,
                        UiTone::WARNING,
                        "Keluar Penjara",
                        current.getUsername() +
                            " berhasil melunasi denda penjara M" +
                            std::to_string(jailFine) +
                            " dan keluar dari penjara.");
                    continueTurnAfterDiceResolution(flowResult, current, total, rolledDouble);
                    return;
                }

                getBank().receivePayment(current, jailFine);
                current.setStatus(PlayerStatus::ACTIVE);
                current.setJailTurns(0);
                if (logger) {
                    logger->log(current.getUsername(), "BAYAR_DENDA",
                        "Bayar denda penjara M" + std::to_string(jailFine));
                }
                flowResult.addEvent(
                    GameEventType::MONEY,
                    UiTone::WARNING,
                    "Keluar Penjara",
                    current.getUsername() +
                        " wajib membayar denda M" + std::to_string(jailFine) +
                        " pada percobaan ke-4.");
                continueTurnAfterDiceResolution(flowResult, current, total, rolledDouble);
                return;
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

                flowResult.append(moveCurrentPlayer(total));
                if (!flowResult.prompts.empty() || hasPendingContinuation()) {
                    chainPendingContinuation([this]() {
                        CommandResult resumed;
                        Player& resumedPlayer = getCurrentPlayer();
                        resumedPlayer.resetConsecutiveDoubles();
                        resumed.append(executeTurn());
                        return resumed;
                    });
                    return;
                }
                current.resetConsecutiveDoubles();
                flowResult.append(executeTurn());
                return;
            }
        }

        continueTurnAfterDiceResolution(flowResult, current, total, rolledDouble);
    };

    switch (cmd.type) {
    case CommandType::HELP:
        result.commandName = "BANTUAN";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Daftar Perintah",
            "LEMPAR_DADU | PILIH_BUANG_KARTU <slot_1_3> | ATUR_DADU X Y | CETAK_PAPAN | CETAK_AKTA [KODE] | "
            "CETAK_PROPERTI | GADAI KODE | TEBUS KODE | BANGUN KODE | "
            "GUNAKAN_KEMAMPUAN IDX [TARGET] | BAYAR_DENDA (saat di penjara) | "
            "CETAK_LOG [N] | SIMPAN [FILE] | AKHIRI_GILIRAN | KELUAR"
        );
        flushEvents(result);
        return result;

    case CommandType::ROLL_DICE: {
        result.commandName = "LEMPAR_DADU";
        if (diceRolledThisTurn && !extraRollAllowedThisTurn) {
            throw GameException("LEMPAR_DADU hanya boleh 1x per giliran (kecuali saat mendapat double).");
        }
        if (extraRollAllowedThisTurn) {
            extraRollAllowedThisTurn = false;
        }
        const std::pair<int, int> roll = dice.rollRandom();
        result.addEvent(
            GameEventType::DICE,
            UiTone::INFO,
            "Hasil Dadu",
            "Mengocok dadu...\nHasil: " + std::to_string(roll.first) + " + " + std::to_string(roll.second) + " = " + std::to_string(dice.getTotal())
        );
        resolveDiceFlow(result);
        flushEvents(result);
        return result;
    }

    case CommandType::RESOLVE_SKILL_DROP:
        return handlePendingSkillDropPrompt();

    case CommandType::SET_DICE: {
        result.commandName = "ATUR_DADU";
        if (diceRolledThisTurn && !extraRollAllowedThisTurn) {
            throw GameException("ATUR_DADU hanya boleh 1x per giliran (kecuali saat mendapat double).");
        }
        if (extraRollAllowedThisTurn) {
            extraRollAllowedThisTurn = false;
        }
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
            "Dadu diatur secara manual.\nHasil: " + std::to_string(roll.first) + " + " + std::to_string(roll.second) + " = " + std::to_string(dice.getTotal())
        );
        resolveDiceFlow(result);
        flushEvents(result);
        return result;
    }

    case CommandType::SAVE:
    {
        result.commandName = "SIMPAN";
        const string filename = cmd.args.empty() ? "file_save.txt" : cmd.args[0];
        const SaveGameResult saveResult = saveGame(filename, true);
        if (saveResult.status != SaveGameStatus::SUCCESS) {
            throw GameException(saveResult.message.empty()
                                    ? "Gagal menyimpan permainan."
                                    : saveResult.message);
        }
        result.addEvent(
            GameEventType::SAVE_LOAD,
            UiTone::SUCCESS,
            "Berhasil Menyimpan",
            "Menyimpan permainan...\nPermainan berhasil disimpan ke: " + filename
        );
        flushEvents(result);
        return result;
    }

    case CommandType::LOAD:
    {
        const string filename = cmd.args.empty() ? "file_save.txt" : cmd.args[0];
        result = loadGame(filename);
        if (logger && !players.empty()) {
            logger->logLoad(getCurrentPlayer().getUsername(), filename);
        }
        // Change the success message loaded by loadGame() method to have exact wording.
        result.events.clear();
        result.addEvent(
            GameEventType::SAVE_LOAD,
            UiTone::SUCCESS,
            "Berhasil Memuat",
            "Memuat permainan...\n\nPermainan berhasil dimuat. Melanjutkan giliran " + getCurrentPlayer().getUsername() + "..."
        );
        return finalizeResult();
    }

    case CommandType::PRINT_DEED: {
        result.commandName = "CETAK_AKTA";

        const std::string promptKey = "cetak_akta_kode";
        if (cmd.args.empty() && !hasPromptAnswer(promptKey)) {
            pushPrompt(promptKey, "Masukkan kode petak:", {});
            setPendingContinuation([this]() {
                CommandResult resumed;
                Command cmd;
                cmd.type = CommandType::PRINT_DEED;
                cmd.args.push_back(consumePromptAnswer("cetak_akta_kode"));
                resumed.append(processCommand(cmd));
                return resumed;
            });
            return finalizeResult();
        }

        std::string targetCode;
        if (!cmd.args.empty()) {
            targetCode = cmd.args[0];
        } else {
            targetCode = consumePromptAnswer(promptKey);
        }
        targetCode = upperCopy(targetCode);

        if (!board) {
            throw GameException("Board belum diinisialisasi");
        }

        if (!board->hasTile(targetCode)) {
            result.addEvent(GameEventType::PROPERTY, UiTone::ERROR, "Not Found",
                "Petak \"" + targetCode + "\" tidak ditemukan atau bukan properti.");
            return finalizeResult();
        }

        Tile& tile = board->getTileByCode(targetCode);
        if (!tile.isProperty()) {
            result.addEvent(GameEventType::PROPERTY, UiTone::ERROR, "Not Found",
                "Petak \"" + targetCode + "\" tidak ditemukan atau bukan properti.");
            return finalizeResult();
        }

        Property& prop = static_cast<PropertyTile&>(tile).getProperty();
        std::ostringstream deed;

        deed << "+================================+\n";
        deed << "|        AKTA KEPEMILIKAN        |\n";

        if (prop.getType() == PropertyType::STREET) {
            const StreetProperty* sp = static_cast<const StreetProperty*>(&prop);
            std::string header = "|    [" + sp->getColorGroup() + "] " + upperCopy(sp->getName()) + " (" + sp->getCode() + ")";
            deed << header << std::string(std::max(0, 33 - (int)header.length()), ' ') << "|\n";
        } else if (prop.getType() == PropertyType::RAILROAD) {
            std::string header = "|    [STASIUN] " + upperCopy(prop.getName()) + " (" + prop.getCode() + ")";
            deed << header << std::string(std::max(0, 33 - (int)header.length()), ' ') << "|\n";
        } else {
            std::string header = "|    [PLN/PDAM] " + upperCopy(prop.getName()) + " (" + prop.getCode() + ")";
            deed << header << std::string(std::max(0, 33 - (int)header.length()), ' ') << "|\n";
        }

        deed << "+================================+\n";
        
        std::string hBeli = "| Harga Beli        : M" + std::to_string(prop.getPurchasePrice());
        deed << hBeli << std::string(std::max(0, 33 - (int)hBeli.length()), ' ') << "|\n";
        
        std::string nGadai = "| Nilai Gadai       : M" + std::to_string(prop.getMortgageValue());
        deed << nGadai << std::string(std::max(0, 33 - (int)nGadai.length()), ' ') << "|\n";
        
        deed << "+--------------------------------+\n";

        if (prop.getType() == PropertyType::STREET) {
            const StreetProperty* sp = static_cast<const StreetProperty*>(&prop);
            const std::vector<int>& rents = sp->getRentLevels();

            std::string rBase = "| Sewa (unimproved) : M" + std::to_string(rents[0]);
            deed << rBase << std::string(std::max(0, 33 - (int)rBase.length()), ' ') << "|\n";

            for (int i = 1; i <= 4; ++i) {
                std::string rHouse = "| Sewa (" + std::to_string(i) + " rumah)    : M" + std::to_string(rents[i]);
                deed << rHouse << std::string(std::max(0, 33 - (int)rHouse.length()), ' ') << "|\n";
            }
            std::string rHotel = "| Sewa (hotel)      : M" + std::to_string(rents[5]);
            deed << rHotel << std::string(std::max(0, 33 - (int)rHotel.length()), ' ') << "|\n";
            
            deed << "+--------------------------------+\n";

            std::string cHouse = "| Harga Rumah       : M" + std::to_string(sp->getHouseCost());
            deed << cHouse << std::string(std::max(0, 33 - (int)cHouse.length()), ' ') << "|\n";
            
            std::string cHotel = "| Harga Hotel       : M" + std::to_string(sp->getHotelCost());
            deed << cHotel << std::string(std::max(0, 33 - (int)cHotel.length()), ' ') << "|\n";

            int mult = sp->getFestivalMultiplier();
            if (mult > 1) {
                deed << "+--------------------------------+\n";
                const GameContext ctx(players, board, 0);
                std::string fest = "| Festival aktif x" + std::to_string(mult) + " (" + std::to_string(sp->getFestivalDuration()) + " turn)";
                deed << fest << std::string(std::max(0, 33 - (int)fest.length()), ' ') << "|\n";
                std::string rentActive = "| Sewa aktif        : M" + std::to_string(sp->calculateRent(ctx));
                deed << rentActive << std::string(std::max(0, 33 - (int)rentActive.length()), ' ') << "|\n";
            }
        } else if (prop.getType() == PropertyType::RAILROAD) {
            deed << "| Sewa bergantung pada jumlah    |\n";
            deed << "| stasiun yang dimiliki:         |\n";
            const RailroadProperty* rp = static_cast<const RailroadProperty*>(&prop);
            for (const auto& [count, rent] : rp->getRentByCount()) {
                std::string rStat = "|   " + std::to_string(count) + " stasiun -> M" + std::to_string(rent);
                deed << rStat << std::string(std::max(0, 33 - (int)rStat.length()), ' ') << "|\n";
            }
        } else {
            deed << "| Sewa = total_dadu x pengali    |\n";
            const UtilityProperty* up = static_cast<const UtilityProperty*>(&prop);
            for (const auto& [count, mult] : up->getMultiplierByCount()) {
                std::string rUtil = "|   " + std::to_string(count) + " utilitas -> x" + std::to_string(mult);
                deed << rUtil << std::string(std::max(0, 33 - (int)rUtil.length()), ' ') << "|\n";
            }
        }

        deed << "+================================+\n";

        std::string statusStr = "| Status : ";
        if (prop.isMortgaged()) {
            statusStr += "MORTGAGED [M]";
        } else if (prop.isBank()) {
            statusStr += "BANK (belum dimiliki)";
        } else {
            statusStr += "OWNED (" + prop.getOwner()->getUsername() + ")";
        }
        deed << statusStr << std::string(std::max(0, 33 - (int)statusStr.length()), ' ') << "|\n";
        deed << "+================================+";

        result.addEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Akta", deed.str());
        return finalizeResult();
    }

    case CommandType::PAY_JAIL_FINE: {
        result.commandName = "BAYAR_DENDA";
        Player& current = getCurrentPlayer();
        Player* jailedPlayer = &current;

        if (!current.isJailed()) {
            throw GameException("Kamu tidak sedang di penjara.");
        }
        if (diceRolledThisTurn) {
            throw GameException("BAYAR_DENDA harus dilakukan sebelum melempar dadu.");
        }
        if (!current.canAfford(jailFine)) {
            result.addEvent(
                GameEventType::BANKRUPTCY,
                UiTone::WARNING,
                "Denda Penjara",
                current.getUsername() +
                    " tidak punya cukup uang tunai untuk membayar denda penjara M" +
                    std::to_string(jailFine) +
                    ". Likuidasi akan dijalankan.");
            getBankruptcyManager().handleDebt(current, jailFine, nullptr);

            if (hasPendingContinuation()) {
                chainPendingContinuation([this, jailedPlayer]() {
                    CommandResult resumed;
                    if (jailedPlayer->isBankrupt()) {
                        resumed.append(executeTurn());
                        return resumed;
                    }

                    jailedPlayer->setStatus(PlayerStatus::ACTIVE);
                    jailedPlayer->setJailTurns(0);
                    if (logger) {
                        logger->log(jailedPlayer->getUsername(), "BAYAR_DENDA",
                            "Bayar denda penjara M" + std::to_string(jailFine));
                    }
                    resumed.addEvent(
                        GameEventType::MONEY,
                        UiTone::SUCCESS,
                        "Keluar Penjara",
                        jailedPlayer->getUsername() + " membayar denda M" +
                            std::to_string(jailFine) +
                            " dan kini bebas dari penjara.");
                    return resumed;
                });
                return finalizeResult();
            }

            if (current.isBankrupt()) {
                result.append(executeTurn());
                return finalizeResult();
            }
        }

        getBank().receivePayment(current, jailFine);
        current.setStatus(PlayerStatus::ACTIVE);
        current.setJailTurns(0);

        if (logger) {
            logger->log(current.getUsername(), "BAYAR_DENDA",
                "Bayar denda penjara M" + std::to_string(jailFine));
        }

        result.addEvent(
            GameEventType::MONEY,
            UiTone::SUCCESS,
            "Bebas Penjara",
            current.getUsername() + " membayar denda M" +
            std::to_string(jailFine) + " dan bebas dari penjara. "
            "Silakan LEMPAR_DADU."
        );
        return finalizeResult();
    }

    case CommandType::PRINT_PROPERTIES: {
        result.commandName = "CETAK_PROPERTI";
        const Player& p = getCurrentPlayer();
        const auto& owned = p.getOwnedProperties();

        if (owned.empty()) {
            result.addEvent(GameEventType::PROPERTY, UiTone::INFO,
                "Properti", "Kamu belum memiliki properti apapun.");
            return finalizeResult();
        }

        std::ostringstream out;
        out << "=== Properti Milik: " << p.getUsername() << " ===\n\n";

        std::map<std::string, std::vector<Property*>> groups;
        int totalPropertyWealth = 0;

        for (Property* prop : owned) {
            if (!prop) continue;
            totalPropertyWealth += prop->getPurchasePrice();

            std::string groupName;
            if (prop->getType() == PropertyType::STREET) {
                groupName = static_cast<const StreetProperty*>(prop)->getColorGroup();
                totalPropertyWealth += static_cast<const StreetProperty*>(prop)->getBuildingSellValue() * 2;
            } else if (prop->getType() == PropertyType::RAILROAD) {
                groupName = "STASIUN";
            } else {
                groupName = "UTILITAS";
            }
            groups[groupName].push_back(prop);
        }

        for (const auto& pair : groups) {
            out << "[" << pair.first << "]\n";
            for (Property* prop : pair.second) {
                std::string paddingName = std::string(std::max(0, 20 - (int)(prop->getName().length() + prop->getCode().length())), ' ');
                out << "  - " << prop->getName() << " (" << prop->getCode() << ") " << paddingName;

                if (prop->getType() == PropertyType::STREET) {
                    const StreetProperty* sp = static_cast<const StreetProperty*>(prop);
                    int lvl = sp->getBuildingCount();
                    if (sp->getBuildingLevel() == BuildingLevel::HOTEL) {
                        out << " Hotel   ";
                    } else if (lvl > 0) {
                        out << " " << lvl << " rumah ";
                    } else {
                        out << "         ";
                    }
                } else {
                    out << "         ";
                }

                out << " M" << prop->getPurchasePrice();
                
                if (prop->isMortgaged()) {
                    out << "   MORTGAGED [M]\n";
                } else {
                    out << "   OWNED\n";
                }
            }
            out << "\n";
        }

        out << "Total kekayaan properti: M" << totalPropertyWealth;

        result.addEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Properti", out.str());
        return finalizeResult();
    }

    case CommandType::PRINT_LOG:
    {
        result.commandName = "CETAK_LOG";
        if (!logger) {
            result.addEvent(GameEventType::LOG, UiTone::WARNING, "Logger", "TransactionLogger belum di-inject.");
            return finalizeResult();
        }

        if (!cmd.args.empty()) {
            const int n = std::stoi(cmd.args[0]);
            result.addEvent(GameEventType::LOG, UiTone::INFO, "Log", getTransactionLogReportLastN(n));
        } else {
            result.addEvent(GameEventType::LOG, UiTone::INFO, "Log", getTransactionLogReport());
        }
        return finalizeResult();
    }

    case CommandType::MORTGAGE: {
        result.commandName = "GADAI";
        turnActionTaken = true;
        if (cmd.args.empty()) {
            throw GameException("Usage: GADAI <KODE_PROPERTI>");
        }
        Property& prop = resolvePropertyByCode(cmd.args[0]);
        Player& current = getCurrentPlayer();
        const bool mortgaged = propertyManager->mortgageProperty(current, prop);
        if (hasPendingContinuation()) {
            return finalizeResult();
        }
        result.success = mortgaged;
        return finalizeResult();
    }

    case CommandType::REDEEM: {
        result.commandName = "TEBUS";
        turnActionTaken = true;
        if (cmd.args.empty()) {
            throw GameException("Usage: TEBUS <KODE_PROPERTI>");
        }
        Property& prop = resolvePropertyByCode(cmd.args[0]);
        Player& current = getCurrentPlayer();
        result.success = propertyManager->redeemProperty(current, prop);
        return finalizeResult();
    }

    case CommandType::BUILD: {
        result.commandName = "BANGUN";
        turnActionTaken = true;
        if (cmd.args.empty()) {
            throw GameException("Usage: BANGUN <KODE_PROPERTI_STREET>");
        }
        Property& prop = resolvePropertyByCode(cmd.args[0]);
        if (prop.getType() != PropertyType::STREET) {
            throw GameException("BANGUN hanya berlaku untuk properti STREET.");
        }
        Player& current = getCurrentPlayer();
        StreetProperty& street = static_cast<StreetProperty&>(prop);
        const bool built = propertyManager->buildOnProperty(current, street);
        if (hasPendingContinuation()) {
            return finalizeResult();
        }
        result.success = built;
        return finalizeResult();
    }

    case CommandType::USE_SKILL: {
        result.commandName = "GUNAKAN_KEMAMPUAN";

        if (diceRolledThisTurn) {
            throw GameException(
                "Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.");
        }

        Player& current = getCurrentPlayer();

        // Tampilkan daftar kartu jika tidak ada argumen
        const auto& hand = current.getHandCards();
        if (hand.empty()) {
            result.addEvent(GameEventType::CARD, UiTone::WARNING,
                "Kartu Kemampuan",
                "Kamu tidak memiliki kartu kemampuan di tangan.");
            return finalizeResult();
        }

        // Jika tidak ada argumen, tampilkan daftar kartu untuk dipilih
        if (cmd.args.empty()) {
            std::ostringstream list;
            list << "Daftar Kartu Kemampuan:\n";
            for (size_t i = 0; i < hand.size(); ++i) {
                list << (i + 1) << ". " << hand[i]->getTypeName()
                     << " - " << hand[i]->getDescription();
                if (hand[i]->getValue() > 0) {
                    list << " (nilai: " << hand[i]->getValue() << ")";
                }
                list << "\n";
            }
            list << "0. Batal\n\n";
            list << "Gunakan: GUNAKAN_KEMAMPUAN <nomor> [TARGET]";
            result.addEvent(GameEventType::CARD, UiTone::INFO,
                "Pilih Kartu", list.str());
            return finalizeResult();
        }

        if (current.hasUsedSkillThisTurn()) {
            throw GameException(
                "Kamu sudah menggunakan kartu kemampuan pada giliran ini! "
                "Penggunaan kartu dibatasi maksimal 1 kali dalam 1 giliran.");
        }

        int idx = 0;
        try {
            idx = std::stoi(cmd.args[0]) - 1;
        } catch (const std::exception&) {
            throw GameException("INDEX_KARTU harus berupa angka.");
        }

        if (idx == -1) {
            result.addEvent(GameEventType::CARD, UiTone::INFO, "Batal", "Batal menggunakan kartu kemampuan.");
            return finalizeResult();
        }

        if (idx < 0 || idx >= static_cast<int>(hand.size())) {
            throw GameException(
                "Nomor kartu tidak valid. Kamu punya " +
                std::to_string(hand.size()) + " kartu.");
        }

        const std::string cardType = hand[idx]->getTypeName();
        std::string target;
        if (cmd.args.size() >= 2) {
            target = cmd.args[1];
        }

        const bool needsTarget = (cardType == "TeleportCard" || cardType == "LassoCard" ||
                                  cardType == "DemolitionCard");

        if (needsTarget && trimCopy(target).empty()) {
            const std::string promptKey =
                "skill_target_" + current.getUsername() + "_" + std::to_string(idx + 1);
            if (!hasPromptAnswer(promptKey)) {
                PromptRequest prompt;
                prompt.id = promptKey;
                prompt.title = "TARGET KARTU";

                std::ostringstream message;
                if (cardType == "TeleportCard") {
                    message << "TeleportCard membutuhkan TARGET.\n"
                            << "Masukkan ID atau KODE petak tujuan.\n"
                            << "Referensi target valid: config/property.txt + config/aksi.txt.\n"
                            << "Contoh: 1 atau GO, 38 atau JKT.";
                    prompt.options.push_back(PromptOption{"SUBMIT", "Kirim Target"});
                    prompt.options.push_back(PromptOption{"cancel", "Batal"});
                } else if (cardType == "DemolitionCard") {
                    message << "DemolitionCard membutuhkan TARGET.\n"
                            << "Masukkan ID atau KODE properti lawan.\n"
                            << "Referensi target valid: config/property.txt.\n"
                            << "Contoh: 32 atau BDG.";
                    prompt.options.push_back(PromptOption{"SUBMIT", "Kirim Target"});
                    prompt.options.push_back(PromptOption{"cancel", "Batal"});
                } else {
                    message << "LassoCard: pilih pemain lawan yang berada di depanmu "
                            << "(jarak 1-12 petak).";

                    Board& boardRef = getBoard();
                    const int currentPos = current.getPosition();
                    std::vector<Player*> candidates;
                    for (Player* candidate : getActivePlayers()) {
                        if (!candidate || candidate == &current) {
                            continue;
                        }
                        if (candidate->isBankrupt() || candidate->isJailed()) {
                            continue;
                        }

                        const int dist = boardRef.distanceTo(currentPos, candidate->getPosition());
                        if (dist > 0 && dist <= 12) {
                            candidates.push_back(candidate);
                        }
                    }

                    if (candidates.empty()) {
                        result.addEvent(GameEventType::CARD, UiTone::WARNING,
                                        "Target Tidak Ada",
                                        "Tidak ada pemain lawan valid di depanmu "
                                        "(jarak 1-12 petak).");
                        result.success = false;
                        return finalizeResult();
                    }

                    for (Player* candidate : candidates) {
                        const int dist = boardRef.distanceTo(currentPos, candidate->getPosition());
                        std::ostringstream label;
                        label << candidate->getUsername()
                              << " (jarak " << dist << " petak)";
                        prompt.options.push_back(PromptOption{
                            candidate->getUsername(),
                            label.str()});
                    }
                    prompt.options.push_back(PromptOption{"cancel", "Batal"});
                }

                prompt.message = message.str();
                pushPrompt(prompt);

                setPendingContinuation([this, promptKey, idx]() {
                    CommandResult resumed;
                    if (!hasPromptAnswer(promptKey)) {
                        return resumed;
                    }

                    const std::string answer = consumePromptAnswer(promptKey);
                    const std::string normalizedAnswer = upperCopyText(trimCopy(answer));
                    if (normalizedAnswer.empty() || normalizedAnswer == "CANCEL" ||
                        normalizedAnswer == "CLOSE") {
                        resumed.addEvent(GameEventType::CARD, UiTone::INFO,
                                         "Batal", "Batal menggunakan kartu kemampuan.");
                        return resumed;
                    }

                    Command followUp;
                    followUp.type = CommandType::USE_SKILL;
                    followUp.raw = "GUNAKAN_KEMAMPUAN " + std::to_string(idx + 1) + " " + answer;
                    followUp.args = {std::to_string(idx + 1), answer};
                    resumed.append(processCommand(followUp));
                    return resumed;
                });

                return finalizeResult();
            }

            target = consumePromptAnswer(promptKey);
        }

        if (needsTarget) {
            target = trimCopy(target);
            if (target.empty()) {
                result.addEvent(GameEventType::CARD, UiTone::INFO,
                                "Batal", "Batal menggunakan kartu kemampuan.");
                return finalizeResult();
            }

            if (cardType == "TeleportCard" || cardType == "DemolitionCard") {
                const SkillTargetCatalog catalog = loadSkillTargetCatalogFromConfig();
                if (cardType == "TeleportCard") {
                    target = normalizeTileTargetByConfig(
                        target,
                        catalog.anyTileIdToCode,
                        catalog.anyTileCodes,
                        "config/property.txt dan config/aksi.txt");
                } else {
                    target = normalizeTileTargetByConfig(
                        target,
                        catalog.propertyIdToCode,
                        catalog.propertyCodes,
                        "config/property.txt");
                }
            } else if (cardType == "LassoCard") {
                Player* targetPlayer = getPlayerByName(target);
                if (!targetPlayer) {
                    const std::string upperTarget = upperCopyText(target);
                    for (Player* player : players) {
                        if (!player) {
                            continue;
                        }
                        if (upperCopyText(player->getUsername()) == upperTarget) {
                            targetPlayer = player;
                            break;
                        }
                    }
                }
                if (!targetPlayer) {
                    throw GameException("TARGET pemain '" + target + "' tidak ditemukan.");
                }
                target = targetPlayer->getUsername();
            }
        }

        turnActionTaken = true;
        const std::string cardName = hand[idx]->getTypeName();
        cardManager->useSkillCard(current, idx, *this, target);

        if (logger) {
            logger->logSkillCard(current.getUsername(), cardName,
                target.empty() ? "diaktifkan" : "-> " + target);
        }

        result.addEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Kartu Dipakai",
            cardName + " berhasil digunakan" +
            (target.empty() ? "." : " pada " + target + "."));
        return finalizeResult();
    }

    case CommandType::PRINT_BOARD:
        result.commandName = "CETAK_PAPAN";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Cetak Papan",
            "Papan akan dirender oleh BoardRenderer pada layer UI."
        );
        return finalizeResult();

    case CommandType::END_TURN:
        result.commandName = "AKHIRI_GILIRAN";
        turnActionTaken = true;
        result.append(executeTurn());
        return finalizeResult();

    case CommandType::START_GAME:
    {
        if (cmd.args.size() >= 3) {
            int n = std::stoi(cmd.args[0]);
            std::vector<std::string> names;
            for (size_t i = 1; i < cmd.args.size(); ++i) {
                names.push_back(cmd.args[i]);
            }
            return startNewGame(n, names);
        }
        return startNewGame(2, {"Pemain1", "Pemain2"});
    }

    case CommandType::EXIT:
        result.commandName = "KELUAR";
        result.addEvent(
            GameEventType::SYSTEM,
            UiTone::INFO,
            "Keluar",
            "Permainan ditutup oleh pemain."
        );
        return finalizeResult();

    case CommandType::UNKNOWN:
    default:
        throw GameException("Perintah tidak dikenali: " + cmd.raw);
    }
    return finalizeResult();
}

CommandResult GameEngine::executeTurn() {
    CommandResult result;
    result.commandName = "TRANSISI_GILIRAN";

    if (players.empty()) {
        throw GameException("executeTurn: tidak ada pemain aktif");
    }

    Player& current = getCurrentPlayer();
    if (effectManager && !current.isBankrupt()) {
        effectManager->onTurnEnd(current, *this);
    }

    checkWinCondition();
    if (gameOver) {
        std::string endReasonMessage =
            (gameOverReason_ == GameOverReason::MAX_TURN)
                ? "Permainan selesai karena batas giliran tercapai."
                : "Permainan selesai karena semua pemain lain telah bangkrut.";
        result.addEvent(
            GameEventType::GAME_OVER,
            UiTone::SUCCESS,
            "Permainan Selesai",
            endReasonMessage
        );
        flushEvents(result);
        return result;
    }

    turnManager.nextPlayer(buildBankruptFlags());

    Player& next = getCurrentPlayer();
    if (effectManager && !next.isBankrupt()) {
        effectManager->onTurnStart(next, *this);
    }

    resetTurnActionFlags();

    if (cardManager && !next.isBankrupt()) {
        std::shared_ptr<SkillCard> drawn = cardManager->drawSkillCard(next);
        if (cardManager->hasPendingSkillDrop(next)) {
            const std::string pendingUsername = next.getUsername();
            auto buildDropOptions = [&](const std::vector<std::string>& rawLabels) {
                std::vector<PromptOption> options;
                options.reserve(rawLabels.size());
                for (size_t i = 0; i < rawLabels.size(); ++i) {
                    options.push_back(PromptOption{
                        std::to_string(i),
                        std::to_string(i + 1) + ". " + rawLabels[i]});
                }
                return options;
            };

            result.addEvent(
                GameEventType::CARD,
                UiTone::INFO,
                "Kartu Kemampuan",
                "Kamu mendapatkan 1 kartu acak baru!\n"
                "Kartu yang didapat: " + drawn->getTypeName() + ".\n"
                "Inventory kartu kemampuan maksimal 3 kartu. Slot 4 khusus "
                "GetOutOfJailCard.\n"
                "Pilih 1 kartu kemampuan lama di slot 1-3 untuk dibuang. "
                "Kartu baru akan otomatis masuk menggantikannya."
            );

            const std::string promptKey = "skill_drop_" + pendingUsername;
            const std::vector<std::string> labels =
                cardManager->getPendingSkillDropOptions(next);

            PromptRequest prompt;
            prompt.id = promptKey;
            prompt.title = "KARTU KEMAMPUAN";
            prompt.message =
                "Kartu kemampuan penuh (maksimal 3). "
                "Slot 4 inventory khusus GetOutOfJailCard. "
                "Pilih 1 kartu kemampuan lama di slot 1-3 untuk dibuang. "
                "Kartu baru akan otomatis masuk menggantikannya. "
                "Kartu baru dan kartu kesempatan tidak bisa dipilih.";
            prompt.options = buildDropOptions(labels);
            pushPrompt(prompt);
            setPendingContinuation([this, pendingUsername]() {
                return handlePendingSkillDropPrompt(pendingUsername);
            });

            flushEvents(result);
            return result;
        }

        result.addEvent(
            GameEventType::CARD,
            UiTone::INFO,
            "Kartu Kemampuan",
            "Kamu mendapatkan 1 kartu acak baru!\n"
            "Kartu yang didapat: " + drawn->getTypeName() + "."
        );
    }

    if (logger) {
        logger->setCurrentTurn(turnManager.getTurnNumber());
    }

    result.addEvent(
        GameEventType::TURN,
        UiTone::INFO,
        "Giliran Berikutnya",
        "Sekarang giliran " + next.getUsername() + "."
    );
    flushEvents(result);
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
    const int boardSize = board->size();
    const int oldPos = player.getPosition();
    player.move(steps, boardSize);
    const bool crossedGo = (player.getPosition() < oldPos);
    Tile& landing = board->getTileByIndex(player.getPosition());
    const bool landedOnGoToJail = (landing.getCode() == "PPJ");

    result.addEvent(
        GameEventType::MOVEMENT,
        UiTone::INFO,
        "Pergerakan",
        "Memajukan Bidak " + player.getUsername() + " sebanyak " + std::to_string(steps) + " petak..."
    );

    if (crossedGo && !landedOnGoToJail && landing.getCode() != "GO") {
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
        "Bidak mendarat di: " + landing.getName() + "."
    );

    MovementPayload movement;
    movement.playerIndex = findPlayerIndexByPointer(players, &player);
    movement.playerName = player.getUsername();
    movement.fromIndex = oldPos;

    int pathCursor = oldPos;
    appendForwardPath(movement.path, pathCursor, steps, boardSize);
    const int landingIndex = pathCursor;

    handleLanding(player, landing);

    const int finalIndex = player.getPosition();
    if (finalIndex != landingIndex) {
        // Endpoint final disimpan di DTO movement.
        // Untuk Chance timed, lintasan detail follow-up dibentuk di GUI
        // setelah popup kartu selesai (4 detik).
        movement.path.push_back(finalIndex);
    }

    if (!movement.path.empty()) {
        movement.toIndex = movement.path.back();
    } else {
        movement.toIndex = finalIndex;
    }
    result.movement = movement;

    flushEvents(result);
    return result;
}

void GameEngine::continueTurnAfterDiceResolution(CommandResult& flowResult,
                                                 Player& current,
                                                 int totalSteps,
                                                 bool rolledDouble) {
    if (rolledDouble) {
        current.incrementConsecutiveDoubles();
        if (current.getConsecutiveDoubles() >= 3) {
            const bool jailed = sendPlayerToJail(current, "Triple Double");

            flowResult.addEvent(
                GameEventType::DICE,
                jailed ? UiTone::WARNING : UiTone::INFO,
                "Triple Double",
                current.getUsername() +
                    " melempar double 3 kali berturut-turut. " +
                    (jailed
                        ? "Bidak dipindah ke penjara dan giliran berakhir."
                        : "Kartu Bebas dari Penjara terpakai, jadi tidak dipenjara."));

            flowResult.append(executeTurn());
            return;
        }
    } else {
        current.resetConsecutiveDoubles();
    }

    flowResult.append(moveCurrentPlayer(totalSteps));
    if (!flowResult.prompts.empty() || hasPendingContinuation()) {
        chainPendingContinuation([this, rolledDouble]() {
            CommandResult resumed;
            Player& resumedPlayer = getCurrentPlayer();

            if (resumedPlayer.isJailed()) {
                resumedPlayer.resetConsecutiveDoubles();
                resumed.append(executeTurn());
                return resumed;
            }

            if (rolledDouble) {
                extraRollAllowedThisTurn = true;
                resumed.addEvent(
                    GameEventType::TURN,
                    UiTone::INFO,
                    "Double",
                    resumedPlayer.getUsername() +
                        " mendapatkan giliran tambahan karena melempar double.");
                return resumed;
            }

            resumed.append(executeTurn());
            return resumed;
        });
        return;
    }

    if (current.isJailed()) {
        current.resetConsecutiveDoubles();
        flowResult.append(executeTurn());
        return;
    }

    if (rolledDouble) {
        extraRollAllowedThisTurn = true;
        flowResult.addEvent(
            GameEventType::TURN,
            UiTone::INFO,
            "Double",
            current.getUsername() +
                " mendapatkan giliran tambahan karena melempar double.");
        return;
    }

    flowResult.append(executeTurn());
}

void GameEngine::handleLanding(Player& p, Tile& t) {
    t.onLand(p, *this);
}

bool GameEngine::sendPlayerToJail(Player& player, const std::string& source) {
    if (!board) {
        throw GameException("Board belum diinisialisasi");
    }

    player.setPosition(board->getIndexOf("PEN"));
    if (player.consumeJailFreeCard()) {
        player.setStatus(PlayerStatus::ACTIVE);
        player.setJailTurns(0);
        player.resetConsecutiveDoubles();
        pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Kartu Kesempatan",
            player.getUsername() +
                " menggunakan kartu Bebas dari Penjara saat efek \"" + source +
                "\". Pemain tidak jadi dipenjara.");
        if (logger) {
            logger->log(player.getUsername(), "KARTU_KESEMPATAN",
                "Gunakan Bebas dari Penjara (" + source + ")");
        }
        return false;
    }

    player.setStatus(PlayerStatus::JAILED);
    player.setJailTurns(0);
    player.resetConsecutiveDoubles();
    return true;
}

void GameEngine::checkWinCondition() {
    int activeCount = 0;
    for (Player* player : players) {
        if (player && !player->isBankrupt()) {
            activeCount++;
        }
    }

    if (activeCount <= 1) {
        gameOverReason_ = GameOverReason::BANKRUPTCY;
        endGame();
        return;
    }

    if (maxTurn > 0 && getCurrentTurn() >= maxTurn) {
        const bool exceededMaxTurn = (getCurrentTurn() > maxTurn);
        const bool isLastPlayerInRound =
            turnManager.orderSize() <= 1 ||
            turnManager.getCurrentOrderIndex() == (turnManager.orderSize() - 1);

        if (exceededMaxTurn || isLastPlayerInRound) {
            gameOverReason_ = GameOverReason::MAX_TURN;
            endGame();
        }
    }
}

void GameEngine::endGame() {
    gameOver = true;

    std::vector<Player*> active;
    for (Player* p : players) {
        if (p && !p->isBankrupt()) active.push_back(p);
    }

    // Tentukan pemenang berdasarkan mode akhir permainan:
    // - MAX_TURN: uang -> properti -> kartu
    // - BANKRUPTCY: kekayaan -> properti -> kartu (tetap kompatibel dengan behavior lama)
    auto betterPlayer = [this](const Player* a, const Player* b) {
        if (gameOverReason_ == GameOverReason::MAX_TURN) {
            if (a->getMoney() != b->getMoney()) {
                return a->getMoney() > b->getMoney();
            }
        } else {
            if (a->getTotalWealth() != b->getTotalWealth()) {
                return a->getTotalWealth() > b->getTotalWealth();
            }
        }

        if (a->countProperties() != b->countProperties()) {
            return a->countProperties() > b->countProperties();
        }
        if (a->countCards() != b->countCards()) {
            return a->countCards() > b->countCards();
        }

        // Tie-break deterministik untuk menjaga hasil sort konsisten.
        return a->getUsername() < b->getUsername();
    };

    auto sameRankAsTop = [this](const Player* p, const Player* top) {
        if (gameOverReason_ == GameOverReason::MAX_TURN) {
            return p->getMoney() == top->getMoney() &&
                   p->countProperties() == top->countProperties() &&
                   p->countCards() == top->countCards();
        }

        return p->getTotalWealth() == top->getTotalWealth() &&
               p->countProperties() == top->countProperties() &&
               p->countCards() == top->countCards();
    };

    std::vector<Player*> winners;
    if (active.size() == 1) {
        winners = active;
    } else if (!active.empty()) {
        std::sort(active.begin(), active.end(),
                  [&betterPlayer](Player* a, Player* b) {
                      return betterPlayer(a, b);
                  });
        Player* top = active[0];
        for (Player* p : active) {
            if (sameRankAsTop(p, top)) {
                winners.push_back(p);
            }
        }
    }

    // Rekap pemain → push ke event buffer (UI yang akan tampilkan)
    const bool isBankruptMode = (gameOverReason_ == GameOverReason::BANKRUPTCY);
    std::ostringstream recap;
    recap << (isBankruptMode
              ? "Mode: Bankruptcy (tanpa batas giliran)\n"
              : "Mode: Max Turn (batas " + std::to_string(maxTurn) + " giliran)\n");
    recap << "\nRekap pemain:\n";
    for (Player* p : players) {
        if (!p) continue;
        recap << "  " << p->getUsername();
        if (p->isBankrupt()) {
            recap << " [BANGKRUT]";
        } else {
            recap << " | Uang: M" << p->getMoney()
                  << " | Properti: " << p->countProperties()
                  << " | Kartu: " << p->countCards()
                  << " | Kekayaan: M" << p->getTotalWealth();
        }
        recap << "\n";
    }
    pushEvent(GameEventType::GAME_OVER, UiTone::INFO, "Rekap Akhir", recap.str());

    // Pemenang
    if (winners.empty()) {
        pushEvent(GameEventType::GAME_OVER, UiTone::WARNING,
            "Pemenang", "Tidak ada pemenang.");
    } else if (winners.size() == 1) {
        pushEvent(GameEventType::GAME_OVER, UiTone::SUCCESS,
            "Pemenang", "🏆 " + winners[0]->getUsername() + " MENANG!");
    } else {
        std::ostringstream seri;
        for (size_t i = 0; i < winners.size(); ++i) {
            if (i > 0) seri << ", ";
            seri << winners[i]->getUsername();
        }
        pushEvent(GameEventType::GAME_OVER, UiTone::SUCCESS,
            "Pemenang (Seri)", "🏆 " + seri.str());
    }
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

std::string GameEngine::getTransactionLogReport() const {
    if (!logger) {
        return "";
    }

    std::ostringstream out;
    out << "=== Log Transaksi ===\n";
    const auto& logs = logger->getAllLogs();
    for (const LogEntry& entry : logs) {
        out << entry.toString() << "\n";
    }
    return out.str();
}

std::string GameEngine::getTransactionLogReportLastN(int n) const {
    if (!logger) {
        return "";
    }

    std::ostringstream out;
    out << "=== Log Transaksi (" << n << " Terakhir) ===\n";
    const auto logs = logger->getLastN(n);
    for (const LogEntry& entry : logs) {
        out << entry.toString() << "\n";
    }
    return out.str();
}

bool GameEngine::isGameOver()    const { return gameOver;                    }
int  GameEngine::getMaxTurn()    const { return maxTurn;                     }
int  GameEngine::getGoSalary()   const { return goSalary;                    }
int  GameEngine::getJailFine()   const { return jailFine;                    }
int  GameEngine::getCurrentTurn() const{ return turnManager.getTurnNumber(); }

GameSnapshot GameEngine::createSnapshot() const {
    GameSnapshot snapshot;

    snapshot.setCurrentTurn(turnManager.getTurnNumber());
    snapshot.setMaxTurn(maxTurn);
    snapshot.setNumPlayers(static_cast<int>(players.size()));

    unordered_map<const Player*, string> playerToName;

    for (const Player* player : players) {
        if (!player) continue;
        validatePlayerInventoryForSave(*player);

        SavedPlayerState savedPlayer;
        savedPlayer.setUsername(player->getUsername());
        savedPlayer.setMoney(player->getMoney());
        savedPlayer.setStatus(toPlayerStatusString(*player));

        string positionCode = "GO";
        if (board != nullptr && board->size() > 0) {
            positionCode = board->getTileByIndex(player->getPosition()).getCode();
        }
        savedPlayer.setPositionCode(positionCode);

        // Serialize skill cards in hand
        for (const auto& card : player->getHandCards()) {
            if (!card) continue;
            std::string cardValue;
            std::string cardDuration;
            const std::string typeName = card->getTypeName();
            if (typeName == "MoveCard" || typeName == "DiscountCard") {
                cardValue = std::to_string(card->getValue());
            }
            if (typeName == "DiscountCard" && card->getDuration() > 0) {
                cardDuration = std::to_string(card->getDuration());
            }
            savedPlayer.addCard(SavedCardState(typeName, cardValue, cardDuration));
        }
        if (player->hasJailFreeCard()) {
            savedPlayer.addCard(SavedCardState("GetOutOfJailCard", "", ""));
        }

        snapshot.addPlayer(savedPlayer);
        playerToName[player] = player->getUsername();
    }

    const vector<int>& order = turnManager.getTurnOrder();
    for (int idx : order) {
        if (idx < 0 || idx >= static_cast<int>(players.size()) || players[idx] == nullptr) {
            throw SaveLoadException("Turn order mengandung index pemain yang tidak valid");
        }
        snapshot.addTurnOrder(players[idx]->getUsername());
    }

    if (!order.empty()) {
        const int currentPlayerIndex = turnManager.getCurrentPlayerIndex();
        if (currentPlayerIndex < 0 || currentPlayerIndex >= static_cast<int>(players.size()) ||
            players[currentPlayerIndex] == nullptr) {
            throw SaveLoadException("Current player index tidak valid");
        }
        snapshot.setActivePlayer(players[currentPlayerIndex]->getUsername());
    }

    if (board != nullptr) {
        const vector<PropertyTile*> propertyTiles = board->getAllPropertyTiles();
        for (const PropertyTile* pt : propertyTiles) {
            if (pt == nullptr) continue;

            const Property& prop = pt->getProperty();
            string ownerName = "BANK";
            if (prop.getOwner() != nullptr) {
                auto it = playerToName.find(prop.getOwner());
                if (it != playerToName.end()) {
                    ownerName = it->second;
                } else {
                    ownerName = prop.getOwner()->getUsername();
                }
            }

            string buildingState = "0";
            if (prop.getType() == PropertyType::STREET) {
                const auto& street = static_cast<const StreetProperty&>(prop);
                if (street.getBuildingLevel() == BuildingLevel::HOTEL) {
                    buildingState = "H";
                } else {
                    buildingState = to_string(street.getBuildingCount());
                }
            }

            snapshot.addProperty(SavedPropertyState(
                prop.getCode(),
                toPropertyTypeString(prop.getType()),
                ownerName,
                toOwnershipStatusString(prop.getStatus()),
                prop.getFestivalMultiplier(),
                prop.getFestivalDuration(),
                buildingState));
        }
    }

    // Serialize skill deck state from CardManager
    if (cardManager) {
        SavedDeckState deckState;
        for (const auto& typeName : cardManager->getSkillDeckStateForSave()) {
            deckState.addCardType(typeName);
        }
        snapshot.setDeck(deckState);
    } else {
        snapshot.setDeck(SavedDeckState());
    }

    // Serialize full transaction log
    if (logger) {
        for (const LogEntry& entry : logger->getAllLogs()) {
            snapshot.addLogEntry(SavedLogEntry(
                entry.getTurn(),
                entry.getUsername(),
                entry.getAction(),
                entry.getDetail()));
        }
    }

    return snapshot;
}

void GameEngine::applySnapshot(const GameSnapshot& snapshot) {
    maxTurn = snapshot.getMaxTurn();
    gameOver = false;
    gameOverReason_ = GameOverReason::NONE;

    for (Player* p : players) {
        delete p;
    }
    players.clear();

    unordered_map<string, Player*> playersByName;
    playersByName.reserve(snapshot.getPlayers().size());

    for (const SavedPlayerState& saved : snapshot.getPlayers()) {
        validateSavedInventoryForLoad(saved);

        Player* player = new Player(saved.getUsername(), saved.getMoney());

        PlayerStatus status = toPlayerStatusEnum(saved.getStatus());
        player->setStatus(status);
        player->setJailTurns(extractJailTurns(saved.getStatus()));

        if (board != nullptr && board->size() > 0) {
            player->setPosition(board->getIndexOf(saved.getPositionCode()));
        }

        // Restore skill cards from save
        if (cardManager) {
            for (const SavedCardState& savedCard : saved.getCards()) {
                if (savedCard.getType() == "GetOutOfJailCard") {
                    player->addJailFreeCard();
                    continue;
                }

                auto skillCard = cardManager->createSkillCardByType(savedCard.getType());
                if (!savedCard.getValue().empty()) {
                    try {
                        skillCard->setValue(std::stoi(savedCard.getValue()));
                    } catch (...) {}
                }
                if (!savedCard.getDuration().empty()) {
                    try {
                        skillCard->setDuration(std::stoi(savedCard.getDuration()));
                    } catch (...) {}
                }
                player->addCard(skillCard);
            }
        }

        players.push_back(player);
        playersByName[player->getUsername()] = player;
    }

    vector<int> restoredOrder;
    restoredOrder.reserve(snapshot.getTurnOrder().size());

    for (const string& username : snapshot.getTurnOrder()) {
        auto it = playersByName.find(username);
        if (it == playersByName.end()) {
            throw SaveLoadException("Turn order memuat username yang tidak ada: " + username);
        }

        int idx = -1;
        for (int i = 0; i < static_cast<int>(players.size()); ++i) {
            if (players[i] == it->second) {
                idx = i;
                break;
            }
        }
        if (idx < 0) {
            throw SaveLoadException("Gagal memetakan turn order untuk pemain: " + username);
        }
        restoredOrder.push_back(idx);
    }

    int activeOrderIndex = 0;
    if (!snapshot.getTurnOrder().empty()) {
        activeOrderIndex = -1;
        for (int i = 0; i < static_cast<int>(snapshot.getTurnOrder().size()); ++i) {
            if (snapshot.getTurnOrder()[i] == snapshot.getActivePlayer()) {
                activeOrderIndex = i;
                break;
            }
        }
        if (activeOrderIndex < 0) {
            throw SaveLoadException("Active player tidak ada di turn order");
        }
    }

    turnManager.restoreState(restoredOrder,
                             activeOrderIndex,
                             snapshot.getCurrentTurn(),
                             false);

    if (board == nullptr) {
        if (!snapshot.getProperties().empty()) {
            throw SaveLoadException("Board belum diinisialisasi saat apply properti snapshot");
        }
        return;
    }

    for (Player* p : players) {
        const vector<Property*> owned = p->getOwnedProperties();
        for (Property* prop : owned) {
            p->removeProperty(prop);
        }
    }

    for (const SavedPropertyState& savedProp : snapshot.getProperties()) {
        Tile& tile = board->getTileByCode(savedProp.getCode());
        if (!tile.isProperty()) {
            throw SaveLoadException("Tile bukan properti pada state properti: " + savedProp.getCode());
        }

        Property& prop = static_cast<PropertyTile&>(tile).getProperty();

        if (savedProp.getType() != toPropertyTypeString(prop.getType())) {
            throw SaveLoadException("Type properti tidak cocok untuk kode: " + savedProp.getCode());
        }

        if (savedProp.getOwner() == "BANK" || savedProp.getStatus() == "BANK") {
            prop.setOwner(nullptr);
            prop.setStatus(OwnershipStatus::BANK);
        } else {
            auto ownerIt = playersByName.find(savedProp.getOwner());
            if (ownerIt == playersByName.end()) {
                throw SaveLoadException("Owner properti tidak ditemukan: " + savedProp.getOwner());
            }

            Player* owner = ownerIt->second;
            prop.setOwner(owner);
            owner->addProperty(&prop);

            if (prop.getType() == PropertyType::STREET) {
                StreetProperty& street = static_cast<StreetProperty&>(prop);
                street.demolishBuildings();

                const string& b = savedProp.getBuildings();
                if (b == "H") {
                    for (int i = 0; i < 4; ++i) street.buildHouse();
                    street.buildHotel();
                } else {
                    int houses = stoi(b);
                    for (int i = 0; i < houses; ++i) street.buildHouse();
                }
            }

            prop.setStatus(toOwnershipStatusEnum(savedProp.getStatus()));
        }

        prop.setFestivalMultiplier(savedProp.getFestivalMult());
        prop.setFestivalDuration(savedProp.getFestivalDur());
    }

    // Restore skill deck state
    if (cardManager) {
        const SavedDeckState& deckState = snapshot.getDeck();
        if (!deckState.getCardTypes().empty()) {
            cardManager->loadSkillDeckState(deckState.getCardTypes());
        }
    }

    // Restore transaction log
    if (logger) {
        std::vector<LogEntry> restoredLogs;
        for (const SavedLogEntry& savedLog : snapshot.getLog()) {
            restoredLogs.emplace_back(
                savedLog.getTurn(),
                savedLog.getUsername(),
                savedLog.getActionType(),
                savedLog.getDetail());
        }
        logger->loadLogs(restoredLogs);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper private
// ─────────────────────────────────────────────────────────────────────────────

void GameEngine::initBoard() {
    delete board;
    board = new Board();

    ConfigLoader loader;
    const std::vector<PropertyDef> propertyDefs = loader.loadProperties("config/property.txt");
    const std::vector<ActionTileDef> actionDefs = loader.loadActionTiles("config/aksi.txt");
    const std::map<int, int> railroadRent = loader.loadRailroadConfig("config/railroad.txt");
    const std::map<int, int> utilityMultiplier = loader.loadUtilityConfig("config/utility.txt");
    const TaxConfig taxConfig = loader.loadTaxConfig("config/tax.txt");
    const SpecialConfig specialConfig = loader.loadSpecialConfig("config/special.txt");
    const MiscConfig miscConfig = loader.loadMiscConfig("config/misc.txt");

    goSalary = specialConfig.getGoSalary();
    jailFine = specialConfig.getJailFine();
    maxTurn = miscConfig.getMaxTurn();
    initialBalance = miscConfig.getInitialBalance();

    std::map<int, PropertyDef> propertyById;
    for (const PropertyDef& def : propertyDefs) {
        if (propertyById.find(def.getId()) != propertyById.end()) {
            throw GameException("Konfigurasi property tidak valid: id duplikat " +
                std::to_string(def.getId()) + ".");
        }
        propertyById[def.getId()] = def;
    }

    std::map<int, ActionTileDef> actionById;
    for (const ActionTileDef& def : actionDefs) {
        if (actionById.find(def.getId()) != actionById.end()) {
            throw GameException("Konfigurasi aksi tidak valid: id duplikat " +
                std::to_string(def.getId()) + ".");
        }
        actionById[def.getId()] = def;
    }

    int boardSize = 0;
    for (const auto& [id, _] : propertyById) {
        boardSize = std::max(boardSize, id);
    }
    for (const auto& [id, _] : actionById) {
        boardSize = std::max(boardSize, id);
    }
    if (boardSize <= 0) {
        throw GameException("Board config kosong. Tidak ada petak yang bisa diinisialisasi.");
    }

    JailTile* jailTile = nullptr;
    for (int id = 1; id <= boardSize; ++id) {
        const auto propertyIt = propertyById.find(id);
        const auto actionIt = actionById.find(id);

        if (propertyIt != propertyById.end() && actionIt != actionById.end()) {
            throw GameException(
                "Konfigurasi board tidak valid: id " + std::to_string(id) +
                " terdaftar sekaligus sebagai properti dan tile aksi.");
        }

        const int idx = id - 1;
        if (propertyIt != propertyById.end()) {
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
                throw GameException("Jenis properti tidak dikenal untuk kode '" +
                    def.getCode() + "'.");
            }

            board->addTile(std::make_unique<PropertyTile>(idx, property));
            continue;
        }

        if (actionIt == actionById.end()) {
            throw GameException(
                "Konfigurasi board tidak lengkap: tidak ada definisi petak untuk id " +
                std::to_string(id) + ".");
        }

        const ActionTileDef& action = actionIt->second;
        const std::string code = action.getCode();
        const std::string name = normalizeTileName(action.getName());

        if (code == "GO") {
            board->addTile(std::make_unique<GoTile>(idx, goSalary, code, name));
        } else if (code == "DNU") {
            board->addTile(std::make_unique<CardTile>(
                idx, code, CardDrawType::COMMUNITY, name));
        } else if (code == "KSP") {
            board->addTile(std::make_unique<CardTile>(
                idx, code, CardDrawType::CHANCE, name));
        } else if (code == "FES") {
            board->addTile(std::make_unique<FestivalTile>(idx, code, name));
        } else if (code == "PPH") {
            board->addTile(std::make_unique<TaxTile>(
                idx,
                TaxType::PPH,
                taxConfig.getPphFlat(),
                taxConfig.getPphPercentage(),
                code,
                name));
        } else if (code == "PBM") {
            board->addTile(std::make_unique<TaxTile>(
                idx,
                TaxType::PBM,
                taxConfig.getPbmFlat(),
                0,
                code,
                name));
        } else if (code == "PEN") {
            auto jail = std::make_unique<JailTile>(idx, code, name);
            jailTile = jail.get();
            board->addTile(std::move(jail));
        } else if (code == "PPJ") {
            if (!jailTile) {
                throw GameException("Konfigurasi papan tidak valid: tile PPJ muncul sebelum PEN.");
            }
            board->addTile(std::make_unique<GoToJailTile>(idx, *jailTile, code, name));
        } else if (code == "BBP") {
            board->addTile(std::make_unique<FreeParkingTile>(idx, code, name));
        } else {
            throw GameException(
                "Kode tile aksi tidak dikenali saat inisialisasi papan: '" +
                code + "'.");
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

void GameEngine::resetTurnActionFlags() {
    turnActionTaken = false;
    diceRolledThisTurn = false;
    extraRollAllowedThisTurn = false;
}

CommandResult GameEngine::handlePendingSkillDropPrompt(const std::string& pendingUsername) {
    CommandResult result;
    result.commandName = "PILIH_BUANG_KARTU";

    if (!cardManager) {
        throw GameException("CardManager belum diinisialisasi.");
    }

    Player* pendingPlayer = nullptr;
    if (!pendingUsername.empty()) {
        pendingPlayer = getPlayerByName(pendingUsername);
        if (pendingPlayer != nullptr && !cardManager->hasPendingSkillDrop(*pendingPlayer)) {
            pendingPlayer = nullptr;
        }
    }
    if (pendingPlayer == nullptr) {
        for (Player* player : players) {
            if (!player) {
                continue;
            }
            if (!cardManager->hasPendingSkillDrop(*player)) {
                continue;
            }
            if (pendingPlayer != nullptr) {
                throw GameException("Terdapat lebih dari satu pending drop kartu skill.");
            }
            pendingPlayer = player;
        }
    }

    if (pendingPlayer == nullptr) {
        result.addEvent(
            GameEventType::CARD,
            UiTone::INFO,
            "Kartu Kemampuan",
            "Tidak ada kartu skill yang perlu dibuang.");
        flushEvents(result);
        return result;
    }

    Player& playerToResolve = *pendingPlayer;
    const std::string promptKey = "skill_drop_" + playerToResolve.getUsername();
    const std::vector<std::string> labels =
        cardManager->getPendingSkillDropOptions(playerToResolve);

    auto requeuePrompt = [&]() {
        std::vector<PromptOption> options;
        options.reserve(labels.size());
        for (size_t i = 0; i < labels.size(); ++i) {
            options.push_back(PromptOption{
                std::to_string(i),
                std::to_string(i + 1) + ". " + labels[i]});
        }

        PromptRequest prompt;
        prompt.id = promptKey;
        prompt.title = "KARTU KEMAMPUAN";
        prompt.message =
            "Kartu kemampuan penuh (maksimal 3). "
            "Slot 4 inventory khusus GetOutOfJailCard. "
            "Pilih 1 kartu kemampuan lama di slot 1-3 untuk dibuang. "
            "Kartu baru akan otomatis masuk menggantikannya. "
            "Kartu baru dan kartu kesempatan tidak bisa dipilih.";
        prompt.options = std::move(options);
        pushPrompt(prompt);
        setPendingContinuation([this, pendingUsername]() {
            return handlePendingSkillDropPrompt(pendingUsername);
        });
    };

    if (!hasPromptAnswer(promptKey)) {
        requeuePrompt();
        flushEvents(result);
        return result;
    }

    const std::string rawAnswer = consumePromptAnswer(promptKey);
    int discardIndex = -1;
    try {
        discardIndex = std::stoi(rawAnswer);
    } catch (const std::exception&) {
        discardIndex = -1;
    }

    if (rawAnswer == "4") {
        result.addEvent(
            GameEventType::CARD,
            UiTone::WARNING,
            "Slot Tidak Valid",
            "Slot 4 inventory khusus GetOutOfJailCard. "
            "Kartu kemampuan baru hanya boleh ditukar dengan skill card "
            "di slot 1-3.");
        requeuePrompt();
        flushEvents(result);
        return result;
    }

    if (discardIndex < 0 || discardIndex >= static_cast<int>(labels.size())) {
        result.addEvent(
            GameEventType::CARD,
            UiTone::WARNING,
            "Input Tidak Valid",
            "Pilih slot skill 1-3 yang tersedia.");
        requeuePrompt();
        flushEvents(result);
        return result;
    }

    cardManager->resolvePendingSkillDrop(playerToResolve, discardIndex);
    result.addEvent(
        GameEventType::CARD,
        UiTone::SUCCESS,
        "Kartu Kemampuan",
        playerToResolve.getUsername() + " membuang kartu: " +
            labels[static_cast<size_t>(discardIndex)] + ".");

    if (logger) {
        logger->setCurrentTurn(turnManager.getTurnNumber());
    }

    result.addEvent(
        GameEventType::TURN,
        UiTone::INFO,
        "Giliran Berikutnya",
        "Sekarang giliran " + getCurrentPlayer().getUsername() + ".");

    flushEvents(result);
    return result;
}

// ── Event buffer API ──────────────────────────────────────────────────────────

void GameEngine::pushEvent(GameEventType type, UiTone tone,
                            const std::string& title, const std::string& msg,
                            const std::string& eventPayload) {
    pendingEvents_.push_back(GameEvent{type, tone, title, msg, eventPayload});
}

void GameEngine::pushPrompt(const PromptRequest& prompt) {
    pendingPrompts_.push_back(prompt);
}

void GameEngine::pushPrompt(const std::string& key, const std::string& msg,
                             const std::vector<std::string>& options, bool required) {
    pushPrompt(key, msg, options, required, "");
}

void GameEngine::pushPrompt(const std::string& key, const std::string& msg,
                             const std::vector<std::string>& options, bool required,
                             const std::string& title) {
    PromptRequest pr;
    pr.id      = key;
    pr.title   = title;
    pr.message = msg;
    pr.options.reserve(options.size());
    for (const std::string& option : options) {
        pr.options.push_back(PromptOption{option, option});
    }
    pr.required = required;
    pushPrompt(pr);
}

void GameEngine::flushEvents(CommandResult& result) {
    for (auto& ev : pendingEvents_) {
        result.events.push_back(ev);
    }
    pendingEvents_.clear();

    result.prompts.insert(
        result.prompts.end(), pendingPrompts_.begin(), pendingPrompts_.end());
    pendingPrompts_.clear();
}

void GameEngine::setPendingContinuation(
    const std::function<CommandResult()>& continuation) {
    pendingContinuation_ = continuation;
}

void GameEngine::chainPendingContinuation(
    const std::function<CommandResult()>& continuation) {
    if (!pendingContinuation_) {
        pendingContinuation_ = continuation;
        return;
    }

    std::function<CommandResult()> previous = pendingContinuation_;
    pendingContinuation_ = [this, previous, continuation]() {
        CommandResult result = previous();
        flushEvents(result);

        if (pendingContinuation_) {
            chainPendingContinuation(continuation);
            return result;
        }

        CommandResult followUp = continuation();
        result.append(followUp);
        return result;
    };
}

bool GameEngine::hasPendingContinuation() const {
    return static_cast<bool>(pendingContinuation_);
}

CommandResult GameEngine::resumePendingAction() {
    if (!pendingContinuation_) {
        throw GameException("Tidak ada aksi tertunda yang bisa dilanjutkan.");
    }

    std::function<CommandResult()> continuation = pendingContinuation_;
    pendingContinuation_ = nullptr;

    CommandResult result = continuation();
    if (result.commandName.empty()) {
        result.commandName = "LANJUTKAN_AKSI";
    }
    flushEvents(result);
    return result;
}

void GameEngine::clearPendingContinuation() {
    pendingContinuation_ = nullptr;
}

void GameEngine::setPromptAnswer(const std::string& key, const std::string& answer) {
    promptAnswers_[key] = answer;
}

std::string GameEngine::getPromptAnswer(const std::string& key) const {
    auto it = promptAnswers_.find(key);
    return (it != promptAnswers_.end()) ? it->second : "";
}

std::string GameEngine::consumePromptAnswer(const std::string& key) {
    auto it = promptAnswers_.find(key);
    if (it == promptAnswers_.end()) {
        return "";
    }

    std::string answer = it->second;
    promptAnswers_.erase(it);
    return answer;
}

bool GameEngine::hasPromptAnswer(const std::string& key) const {
    return promptAnswers_.find(key) != promptAnswers_.end();
}

void GameEngine::clearPromptAnswers() {
    promptAnswers_.clear();
}
