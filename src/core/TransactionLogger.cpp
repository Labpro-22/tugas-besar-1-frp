#include "../../include/core/TransactionLogger.hpp"

#include <sstream>
#include <algorithm>

TransactionLogger::TransactionLogger() : currentTurn(0) {}


void TransactionLogger::log(const std::string& username,
                             const std::string& action,
                             const std::string& detail) {
    logs.emplace_back(currentTurn, username, action, detail);
}

void TransactionLogger::setCurrentTurn(int turn) { currentTurn = turn; }
int  TransactionLogger::getCurrentTurn() const   { return currentTurn; }


void TransactionLogger::logDice(const std::string& username,
                                 int die1, int die2,
                                 const std::string& landedAt) {
    std::ostringstream oss;
    oss << "Lempar: " << die1 << "+" << die2 << "="
        << (die1 + die2) << " -> mendarat di " << landedAt;
    log(username, "DADU", oss.str());
}

void TransactionLogger::logBuy(const std::string& username,
                                const std::string& propName,
                                const std::string& propCode,
                                int price) {
    std::ostringstream oss;
    oss << "Beli " << propName << " (" << propCode << ") seharga M" << price;
    log(username, "BELI", oss.str());
}

void TransactionLogger::logAutoAcquire(const std::string& username,
                                        const std::string& propName,
                                        const std::string& propCode,
                                        const std::string& propType) {
    std::ostringstream oss;
    oss << propName << " (" << propCode << ") kini milik "
        << username << " (otomatis - " << propType << ")";
    log(username, propType, oss.str());
}

void TransactionLogger::logRent(const std::string& payer,
                                 const std::string& receiver,
                                 int amount,
                                 const std::string& propCode,
                                 const std::string& detail) {
    std::ostringstream oss;
    oss << "Bayar M" << amount << " ke " << receiver
        << " (" << propCode << ", " << detail << ")";
    log(payer, "SEWA", oss.str());
}

void TransactionLogger::logTax(const std::string& username,
                                const std::string& taxType,
                                int amount) {
    std::ostringstream oss;
    oss << "Bayar " << taxType << " M" << amount;
    log(username, "PAJAK", oss.str());
}

void TransactionLogger::logBuild(const std::string& username,
                                  const std::string& propCode,
                                  const std::string& buildType,
                                  int cost) {
    std::ostringstream oss;
    oss << "Bangun " << buildType << " di " << propCode << ". Biaya M" << cost;
    log(username, "BANGUN", oss.str());
}

void TransactionLogger::logSellBuilding(const std::string& username,
                                         const std::string& propCode,
                                         int proceeds) {
    std::ostringstream oss;
    oss << "Jual bangunan di " << propCode << ". Dapat M" << proceeds;
    log(username, "JUAL_BANGUNAN", oss.str());
}

void TransactionLogger::logMortgage(const std::string& username,
                                     const std::string& propCode,
                                     int mortgageValue) {
    std::ostringstream oss;
    oss << "Gadai " << propCode << ". Dapat M" << mortgageValue;
    log(username, "GADAI", oss.str());
}

void TransactionLogger::logRedeem(const std::string& username,
                                   const std::string& propCode,
                                   int redeemCost) {
    std::ostringstream oss;
    oss << "Tebus " << propCode << ". Bayar M" << redeemCost;
    log(username, "TEBUS", oss.str());
}

void TransactionLogger::logAuctionBid(const std::string& username,
                                       const std::string& propCode,
                                       int bidAmount) {
    std::ostringstream oss;
    oss << "Bid M" << bidAmount << " untuk " << propCode;
    log(username, "LELANG_BID", oss.str());
}

void TransactionLogger::logAuctionResult(const std::string& winner,
                                          const std::string& propCode,
                                          int finalPrice) {
    std::ostringstream oss;
    oss << winner << " memenangkan lelang " << propCode
        << " seharga M" << finalPrice;
    log(winner, "LELANG_HASIL", oss.str());
}

void TransactionLogger::logFestival(const std::string& username,
                                     const std::string& propCode,
                                     int oldRent, int newRent,
                                     int duration) {
    std::ostringstream oss;
    oss << propCode << ": sewa M" << oldRent
        << " -> M" << newRent
        << ", durasi " << duration << " giliran";
    log(username, "FESTIVAL", oss.str());
}

void TransactionLogger::logSkillCard(const std::string& username,
                                      const std::string& cardType,
                                      const std::string& effect) {
    std::ostringstream oss;
    oss << "Pakai " << cardType << " -> " << effect;
    log(username, "KARTU", oss.str());
}

void TransactionLogger::logDrawCard(const std::string& username,
                                     const std::string& cardDeck,
                                     const std::string& cardText) {
    std::ostringstream oss;
    oss << "Ambil kartu " << cardDeck << ": \"" << cardText << "\"";
    log(username, "KARTU", oss.str());
}

void TransactionLogger::logBankruptcy(const std::string& username,
                                       const std::string& creditor) {
    std::ostringstream oss;
    oss << username << " dinyatakan BANGKRUT. Kreditor: " << creditor;
    log(username, "BANGKRUT", oss.str());
}

void TransactionLogger::logAssetTransfer(const std::string& from,
                                          const std::string& to,
                                          const std::string& detail) {
    std::ostringstream oss;
    oss << "Aset dari " << from << " dialihkan ke " << to
        << ". Detail: " << detail;
    log(from, "AMBIL_ASET", oss.str());
}

void TransactionLogger::logSave(const std::string& username,
                                 const std::string& filename) {
    log(username, "SAVE", "Permainan disimpan ke: " + filename);
}

void TransactionLogger::logLoad(const std::string& username,
                                 const std::string& filename) {
    log(username, "LOAD", "Permainan dimuat dari: " + filename);
}

const std::vector<LogEntry>& TransactionLogger::getAllLogs() const {
    return logs;
}

std::vector<LogEntry> TransactionLogger::getLastN(int n) const {
    if (n <= 0) return {};
    int total = static_cast<int>(logs.size());
    int start = std::max(0, total - n);
    return std::vector<LogEntry>(logs.begin() + start, logs.end());
}

void TransactionLogger::loadLogs(const std::vector<LogEntry>& entries) {
    logs = entries;
}

void TransactionLogger::clear() {
    logs.clear();
    currentTurn = 0;
}