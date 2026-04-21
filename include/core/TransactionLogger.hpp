#pragma once

#include "../models/LogEntry.hpp"
#include <vector>
#include <string>

class TransactionLogger {
private:
    std::vector<LogEntry> logs;
    int currentTurn;

public:
    TransactionLogger();

    void log(const std::string& username,
             const std::string& action,
             const std::string& detail);

    void setCurrentTurn(int turn);
    int  getCurrentTurn() const;


    void logDice(const std::string& username,
                 int die1, int die2,
                 const std::string& landedAt);

    void logBuy(const std::string& username,
                const std::string& propName,
                const std::string& propCode,
                int price);

    void logAutoAcquire(const std::string& username,
                        const std::string& propName,
                        const std::string& propCode,
                        const std::string& propType);

    void logRent(const std::string& payer,
                 const std::string& receiver,
                 int amount,
                 const std::string& propCode,
                 const std::string& detail);

    void logTax(const std::string& username,
                const std::string& taxType,
                int amount);

    void logBuild(const std::string& username,
                  const std::string& propCode,
                  const std::string& buildType,
                  int cost);

    void logSellBuilding(const std::string& username,
                         const std::string& propCode,
                         int proceeds);

    void logMortgage(const std::string& username,
                     const std::string& propCode,
                     int mortgageValue);

    void logRedeem(const std::string& username,
                   const std::string& propCode,
                   int redeemCost);

    void logAuctionBid(const std::string& username,
                       const std::string& propCode,
                       int bidAmount);

    void logAuctionResult(const std::string& winner,
                          const std::string& propCode,
                          int finalPrice);

    void logFestival(const std::string& username,
                     const std::string& propCode,
                     int oldRent, int newRent,
                     int duration);

    void logSkillCard(const std::string& username,
                      const std::string& cardType,
                      const std::string& effect);

    void logDrawCard(const std::string& username,
                     const std::string& cardDeck,
                     const std::string& cardText);

    void logBankruptcy(const std::string& username,
                       const std::string& creditor);

    void logAssetTransfer(const std::string& from,
                          const std::string& to,
                          const std::string& detail);

    void logSave(const std::string& username, const std::string& filename);
    void logLoad(const std::string& username, const std::string& filename);

    const std::vector<LogEntry>& getAllLogs() const;
    std::vector<LogEntry> getLastN(int n) const;
    void loadLogs(const std::vector<LogEntry>& entries);
    void clear();
};