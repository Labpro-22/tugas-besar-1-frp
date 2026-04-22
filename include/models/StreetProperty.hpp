#pragma once
 
#include "Property.hpp"
#include <vector>
#include <string>
 
class GameContext;

class StreetProperty : public Property {
private:
    std::string colorGroup;
    int houseCost;
    int hotelCost;
    vector<int> rentLevels;   
    BuildingLevel buildingLevel;
public:
    StreetProperty(const std::string&      code,
                   const std::string&      name,
                   const std::string&      colorGroup,
                   int  purchasePrice,
                   int  mortgageValue,
                   int  houseCost,
                   int  hotelCost,
                   const std::vector<int>& rentLevels);
 
    int  calculateRent(const GameContext& ctx) const override;
    bool canBuild()    const override;
    int  getSellValue() const override;

    void buildHouse();
    void buildHotel(); // upgrade  HOUSE_4 → HOTEL
    void demolishBuildings();
    int getBuildingSellValue() const;
    string  getColorGroup()    const;
    BuildingLevel getBuildingLevel() const;
    int getHouseCost()     const;
    int getHotelCost()     const;
    int getBuildingCount() const;
    const std::vector<int>& getRentLevels() const;
 

};
