#pragma once

#include "Property.hpp"
#include <map>

class GameContext;

class RailroadProperty : public Property {
private:
    std::map<int,int> rentByCount_; // map jumlah railroad dimiliki → sewa
public:
    RailroadProperty(const std::string& code, const std::string& name, int purchasePrice, int mortgageValue,
                     const std::map<int,int>& rentByCount);
    int  calculateRent(const GameContext& ctx) const override;
    bool canBuild() const override;
    int  getSellValue() const override;
    const std::map<int,int>& getRentByCount() const;
};