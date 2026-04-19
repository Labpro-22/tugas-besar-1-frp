#pragma once
#include <stdexcept>
#include <string>
using namespace std;

class GameException : public std::runtime_error {
public:
    explicit GameException(const string& message) : std::runtime_error(message) {}
};

// exception for conf

// file config not found or ga bisa dibuka
class ConfigFileNotFoundException : public GameException {
public:
    explicit ConfigFileNotFoundException(const string& filename)
        : GameException("Config file not found: " + filename) {}        
};

// format barid di file config ga sesuai
class ConfigParseException : public GameException {
public: 
    explicit ConfigParseException(const string& message)
        : GameException("Config parse error: " + message) {}
};

// exception for player

// invalid ops buat player bangkrut
class PlayerBankruptException : public GameException {
public: 
    explicit PlayerBankruptException(const string& username)
        : GameException("Player " + username + " is bankrupt.") {}
};

// not enough saldo buat transaksi
class InsufficientFundsException : public GameException {
public:
    explicit InsufficientFundsException(const string& username, int required, int available)
        : GameException("Player " + username + " has insufficient funds. Required: " + to_string(required) + ", has  " + to_string(available)) {}
};

// kartu full
class HandFullException : public GameException {
public:
    explicit HandFullException(const string& username)
        : GameException("Player " + username + "already has max skill cards.") {}
};

// index kartu invalid
class InvalidCardIndexException : public GameException {
public:
    explicit InvalidCardIndexException(int index)
        : GameException("Invalid card index: " + to_string(index)) {}
};

// exception for property

class BuildNotAllowedException : public GameException {
public:
    explicit BuildNotAllowedException(const std::string& propCode)
        : GameException("Building not allowed on property: " + propCode) {}
};
 
class MaxBuildingLevelException : public GameException {
public:
    explicit MaxBuildingLevelException(const std::string& propCode)
        : GameException("Property '" + propCode + "' is already at max building level") {}
};
 

class AlreadyMortgagedException : public GameException {
public:
    explicit AlreadyMortgagedException(const std::string& propCode)
        : GameException("Property '" + propCode + "' is already mortgaged") {}
};

// properti ga digadai tapi mau dilunasi
class NotMortgagedException : public GameException {
public:
    explicit NotMortgagedException(const std::string& propCode)
        : GameException("Property '" + propCode + "' is not mortgaged") {}
};
 
// masih ada bangunan saat mau digadai
class HasBuildingsException : public GameException {
public:
    explicit HasBuildingsException(const std::string& propCode)
        : GameException("Property '" + propCode + "' still has buildings; demolish first") {}
};
 
class NotOwnerException : public GameException {
public:
    explicit NotOwnerException(const std::string& username, const std::string& propCode)
        : GameException("Player '" + username + "' does not own property '" + propCode + "'") {}
};

// exception for Dice (cek nilai input dadu ketika setManual(), rentang [1,6])
class InvalidDiceValueException : public GameException{
    public:
        explicit InvalidDiceValueException(int value, int faces) : GameException("Invalid dice value: " + std::to_string(value) + ". Dice Value Must be between 1 and 6"){}
};

