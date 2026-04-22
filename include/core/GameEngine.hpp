#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../../include/models/Dice.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/core/Command.hpp"
#include "../../include/core/CommandResult.hpp"
using namespace std;

class Player;
class Board;
class Tile;
class Property;
class Bank;
class AuctionManager;
class BankruptcyManager;
class PropertyManager;
class CardManager;
class EffectManager;
class TransactionLogger;
class SaveLoadManager;
class GameSnapshot;

class GameEngine{
    private:
        Board* board;
        vector<Player*> players;
        bool gameOver;
        bool gameStarted;
        bool turnActionTaken;
        bool diceRolledThisTurn;
        bool extraRollAllowedThisTurn;
        int maxTurn;
        int initialBalance;
        int goSalary;
        int jailFine;
        TurnManager turnManager;
        Dice dice;

        Bank* bank;
        AuctionManager* auctionManager;
        BankruptcyManager* bankruptcyManager;
        PropertyManager* propertyManager;
        CardManager* cardManager;
        EffectManager* effectManager;
        TransactionLogger* logger;

        // ── Event buffer ───────────────────────────────────────────────────
        // Tile, Manager, dan kode lain di Core/Model layer TIDAK boleh
        // cout/cin langsung. Mereka push event/prompt ke buffer ini,
        // lalu engine flush ke CommandResult saat processCommand selesai.
        vector<GameEvent>              pendingEvents_;
        vector<PromptRequest>          pendingPrompts_;

        void initBoard();
        void handleJailTurn(Player& p);
        void awardPassGoSalary(Player& p);
        vector<bool> buildBankruptFlags() const;
        void resetTurnActionFlags();
    
    public:
        GameEngine();
        ~GameEngine();
        GameEngine(const GameEngine&) = delete;
        GameEngine& operator=(const GameEngine&) = delete;

        void setBank(Bank* b);
        void setAuctionManager(AuctionManager* am);
        void setBankruptcyManager(BankruptcyManager* bm);
        void setPropertyManager(PropertyManager* pm);
        void setCardManager(CardManager* cm);
        void setEffectManager(EffectManager* em);
        void setTransactionLogger(TransactionLogger* tl);

        // ── Event buffer API (dipakai oleh Tile & Manager) ────────────────
        void pushEvent(GameEventType type, UiTone tone,
                       const std::string& title, const std::string& msg);
        void pushPrompt(const std::string& key, const std::string& msg,
                        const std::vector<std::string>& options = {},
                        bool required = true);
        // Flush semua pending event/prompt ke result, lalu bersihkan buffer
        void flushEvents(CommandResult& result);
        void setPendingContinuation(const std::function<CommandResult()>& continuation);
        void chainPendingContinuation(const std::function<CommandResult()>& continuation);
        bool hasPendingContinuation() const;
        CommandResult resumePendingAction();
        void clearPendingContinuation();
        // Ambil jawaban prompt dari UI (diisi oleh GameUI sebelum melanjutkan)
        void setPromptAnswer(const std::string& key, const std::string& answer);
        std::string getPromptAnswer(const std::string& key) const;
        std::string consumePromptAnswer(const std::string& key);
        bool hasPromptAnswer(const std::string& key) const;
        void clearPromptAnswers();

        //Siklus permainan
        CommandResult startNewGame(int nPlayers, vector<string> names);
        CommandResult loadGame(const string& filename);
        void run();
        CommandResult processCommand(const Command& cmd);
        CommandResult executeTurn();
        CommandResult moveCurrentPlayer(int steps);
        void handleLanding(Player& p, Tile& t);
        void checkWinCondition();
        void endGame();

        //akses state
        Player& getCurrentPlayer();
        Player* getPlayerByName(const string& name);
        vector<Player*> getActivePlayers() const;
        const vector<Player*>& getPlayers() const;
    
        Board& getBoard();
        Dice& getDice();
        TurnManager& getTurnManager();
        Bank& getBank();
        AuctionManager& getAuctionManager();
        BankruptcyManager& getBankruptcyManager();
        PropertyManager& getPropertyManager();
        CardManager& getCardManager();
        EffectManager& getEffectManager();
        TransactionLogger& getLogger();
    
        bool isGameOver() const;
        int getMaxTurn() const;
        int getGoSalary() const;
        int getJailFine() const;
        int getCurrentTurn() const;

        GameSnapshot createSnapshot() const;
        void applySnapshot(const GameSnapshot& snapshot);

private:
    // prompt answer store: key → answer string
    std::unordered_map<std::string, std::string> promptAnswers_;
    std::function<CommandResult()> pendingContinuation_;
};
