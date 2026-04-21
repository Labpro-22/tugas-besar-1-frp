#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../include/models/Dice.hpp"
#include "../../include/core/TurnManager.hpp"
using namespace std;

class Player;
class Board;
class Tile;
class Property;
class Command;
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
        int maxTurn;
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

        void initBoard();
        void handleJailTurn(Player& p);
        void awardPassGoSalary(Player& p);
        vector<bool> buildBankruptFlags() const;
    
    public:
        GameEngine();
        ~GameEngine();
        //Tujuannya agar tidak bisa di copy
        GameEngine(const GameEngine&) = delete;
        GameEngine& operator=(const GameEngine&) = delete;


        void setBank(Bank* b);
        void setAuctionManager(AuctionManager* am);
        void setBankruptcyManager(BankruptcyManager* bm);
        void setPropertyManager(PropertyManager* pm);
        void setCardManager(CardManager* cm);
        void setEffectManager(EffectManager* em);
        void setTransactionLogger(TransactionLogger* tl);

        //Siklus permainan
        void startNewGame(int nPlayers, vector<string> names);
        void loadGame(const string& filename);
        void run();
        void processCommand(const Command& cmd);
        void executeTurn();
        void moveCurrentPlayer(int steps);
        void handleLanding(Player& p, Tile& t);
        void checkWinCondition();
        void endGame();

        //akses state
        Player& getCurrentPlayer();
        Player* getPlayerByName(const string& name);
        vector<Player*> getActivePlayers() const;
    
        // Getter untuk Manager lain yang perlu mengakses engine
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

        // Snapshot boundary for save/load.
        GameSnapshot createSnapshot() const;
        void applySnapshot(const GameSnapshot& snapshot);

};