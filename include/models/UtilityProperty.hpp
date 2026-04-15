#pragma once

#include "Property.hpp"
#include <map>

class GameContext;

class UtilityProperty : public Property {
private:
    std::map<int,int> multiplierByCount_; //map jumlah utility → faktor pengali
public:
    UtilityProperty(const std::string& code, const std::string& name, int purchasePrice, int mortgageValue,
                    const std::map<int,int>& multiplierByCount);

    int  calculateRent(const GameContext& ctx) const override;
    bool canBuild() const override; 
    int  getSellValue() const override; 
    const std::map<int,int>& getMultiplierByCount() const;
};