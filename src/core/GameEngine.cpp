#include "../../include/core/GameEngine.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/utils/Gamestateserializer.hpp"
#include "../../include/utils/Saveloadmanager.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"

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
#include <unordered_map>

namespace {
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
}

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
    SaveLoadManager saveLoad;
    saveLoad.loadInto(*this, filename);
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

GameSnapshot GameEngine::createSnapshot() const {
    GameSnapshot snapshot;

    snapshot.setCurrentTurn(turnManager.getTurnNumber());
    snapshot.setMaxTurn(maxTurn);
    snapshot.setNumPlayers(static_cast<int>(players.size()));

    unordered_map<const Player*, string> playerToName;

    for (const Player* player : players) {
        if (!player) continue;

        SavedPlayerState savedPlayer;
        savedPlayer.setUsername(player->getUsername());
        savedPlayer.setMoney(player->getMoney());
        savedPlayer.setStatus(toPlayerStatusString(*player));

        string positionCode = "GO";
        if (board != nullptr && board->size() > 0) {
            positionCode = board->getTileByIndex(player->getPosition()).getCode();
        }
        savedPlayer.setPositionCode(positionCode);

        // TODO: Mapping detail skill card ke SavedCardState menunggu API SkillCard final.

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

    // TODO: Mapping deck kartu kemampuan dari CardManager menunggu API final.
    snapshot.setDeck(SavedDeckState());

    // TODO: Mapping full log dari logger menunggu API TransactionLogger final.

    return snapshot;
}

void GameEngine::applySnapshot(const GameSnapshot& snapshot) {
    maxTurn = snapshot.getMaxTurn();
    gameOver = false;

    for (Player* p : players) {
        delete p;
    }
    players.clear();

    unordered_map<string, Player*> playersByName;
    playersByName.reserve(snapshot.getPlayers().size());

    for (const SavedPlayerState& saved : snapshot.getPlayers()) {
        Player* player = new Player(saved.getUsername(), saved.getMoney());

        PlayerStatus status = toPlayerStatusEnum(saved.getStatus());
        player->setStatus(status);
        player->setJailTurns(extractJailTurns(saved.getStatus()));

        if (board != nullptr && board->size() > 0) {
            player->setPosition(board->getIndexOf(saved.getPositionCode()));
        }

        // TODO: Restore kartu skill menunggu API SkillCard final.

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

    // TODO: Restore deck/log menunggu API CardManager + TransactionLogger final.
}

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