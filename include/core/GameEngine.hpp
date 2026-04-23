#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../include/models/Dice.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/core/Command.hpp"
#include "../../include/core/CommandResult.hpp"

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

enum class GameOverReason {
    NONE,
    BANKRUPTCY,
    MAX_TURN
};

class GameEngine{
    private:
        Board* board;
        std::vector<Player*> players;
        bool gameOver;
        bool gameStarted;
        bool turnActionTaken;
        bool diceRolledThisTurn;
        bool extraRollAllowedThisTurn;
        GameOverReason gameOverReason_;
        int maxTurn;
        int initialBalance;
        int goSalary;
        int jailFine;
        TurnManager turnManager;
        Dice dice;

        // Owned core services (Facade internals).
        std::unique_ptr<Bank> bank;
        std::unique_ptr<TransactionLogger> logger;
        std::unique_ptr<CardManager> cardManager;
        std::unique_ptr<EffectManager> effectManager;
        std::unique_ptr<PropertyManager> propertyManager;
        std::unique_ptr<AuctionManager> auctionManager;
        std::unique_ptr<BankruptcyManager> bankruptcyManager;

        // ── Event buffer ───────────────────────────────────────────────────
        // Tile, Manager, dan kode lain di Core/Model layer TIDAK boleh
        // cout/cin langsung. Mereka push event/prompt ke buffer ini,
        // lalu engine flush ke CommandResult saat processCommand selesai.
        std::vector<GameEvent> pendingEvents_;
        std::vector<PromptRequest> pendingPrompts_;

        void initBoard();
        void handleJailTurn(Player& p);
        void awardPassGoSalary(Player& p);
        void continueTurnAfterDiceResolution(CommandResult& result, Player& player,
                                             int totalSteps, bool rolledDouble);
        std::vector<bool> buildBankruptFlags() const;
        void resetTurnActionFlags();
        CommandResult handlePendingSkillDropPrompt(const std::string& pendingUsername = "");
    
    public:
        GameEngine();
        ~GameEngine();
        GameEngine(const GameEngine&) = delete;
        GameEngine& operator=(const GameEngine&) = delete;

        // ── Event buffer API (dipakai oleh Tile & Manager) ────────────────
        void pushEvent(GameEventType type, UiTone tone,
                       const std::string& title, const std::string& msg,
                       const std::string& eventPayload = "");
        void pushPrompt(const PromptRequest& prompt);
        void pushPrompt(const std::string& key, const std::string& msg,
                        const std::vector<std::string>& options,
                        bool required);
        void pushPrompt(const std::string& key, const std::string& msg,
                        const std::vector<std::string>& options = {},
                        bool required = true,
                        const std::string& title = "");
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
        CommandResult startNewGame(int nPlayers, std::vector<std::string> names);
        CommandResult loadGame(const std::string& filename);
        void run();
        CommandResult processCommand(const Command& cmd);
        CommandResult executeTurn();
        CommandResult moveCurrentPlayer(int steps);
        void handleLanding(Player& p, Tile& t);
        bool sendPlayerToJail(Player& player, const std::string& source);
        void checkWinCondition();
        void endGame();

        //akses state
        Player& getCurrentPlayer();
        Player* getPlayerByName(const std::string& name);
        std::vector<Player*> getActivePlayers() const;
        const std::vector<Player*>& getPlayers() const;
    
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
        std::string getTransactionLogReport() const;
        std::string getTransactionLogReportLastN(int n) const;
    
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
