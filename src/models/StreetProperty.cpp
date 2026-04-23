#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/GameException.hpp"

StreetProperty::StreetProperty(const std::string&      code,
                               const std::string&      name,
                               const std::string&      colorGroup,
                               int  purchasePrice,
                               int  mortgageValue,
                               int  houseCost,
                               int  hotelCost,
                               const std::vector<int>& rentLevels)
    : Property(code, name, PropertyType::STREET, purchasePrice, mortgageValue),
      colorGroup(colorGroup),
      houseCost(houseCost),
      hotelCost(hotelCost),
      rentLevels(rentLevels),
      buildingLevel(BuildingLevel::NONE)
{
    if (rentLevels.size() != 6) {
        throw ConfigParseException(
            "StreetProperty '" + code + "' requires exactly 6 rent levels, got "
            + std::to_string(rentLevels.size()));
    }
}

int StreetProperty::calculateRent(const GameContext& ctx) const {
    if (!isOwned() || isMortgaged()) return 0;

    int lvl  = static_cast<int>(buildingLevel);
    int base = rentLevels[lvl];

    if (buildingLevel == BuildingLevel::NONE) {
        bool monopoly = false;
        const Board* board = ctx.getBoard();
        if (board != nullptr) {
            const auto groupTiles = board->getPropertyTilesByColorGroup(colorGroup);
            monopoly = !groupTiles.empty();
            for (const auto* tile : groupTiles) {
                if (!tile) {
                    monopoly = false;
                    break;
                }
                if (tile->getProperty().getOwner() != owner) {
                    monopoly = false;
                    break;
                }
            }
        }

        if (monopoly) base *= 2;
    }

    return base * festivalMultiplier;
}

bool StreetProperty::canBuild() const {
    return isOwned() && !isMortgaged() && buildingLevel != BuildingLevel::HOTEL;
}

int StreetProperty::getSellValue() const {
    int buildingValue = 0;
    if (buildingLevel == BuildingLevel::HOTEL) {
        buildingValue = hotelCost / 2;
    } else {
        int numHouses = static_cast<int>(buildingLevel);
        buildingValue = numHouses * (houseCost / 2);
    }
    return purchasePrice + buildingValue;
}

void StreetProperty::buildHouse() {
    if (!canBuild()) {
        throw BuildNotAllowedException(code);
    }
    if (buildingLevel == BuildingLevel::HOUSE_4) {
        throw MaxBuildingLevelException(code);
    }
    buildingLevel = static_cast<BuildingLevel>(static_cast<int>(buildingLevel) + 1);
}

void StreetProperty::buildHotel() {
    if (buildingLevel != BuildingLevel::HOUSE_4) {
        throw BuildNotAllowedException(
            code + " (need 4 houses before hotel)");
    }
    buildingLevel = BuildingLevel::HOTEL;
}

void StreetProperty::demolishBuildings() {
    buildingLevel = BuildingLevel::NONE;
}

int StreetProperty::getBuildingSellValue() const {
    if (buildingLevel == BuildingLevel::HOTEL) {
        return hotelCost / 2;
    }
    return static_cast<int>(buildingLevel) * (houseCost / 2);
}

std::string   StreetProperty::getColorGroup()    const { return colorGroup;   }
BuildingLevel StreetProperty::getBuildingLevel() const { return buildingLevel; }
int           StreetProperty::getHouseCost()     const { return houseCost;    }
int           StreetProperty::getHotelCost()     const { return hotelCost;    }

int StreetProperty::getBuildingCount() const {
    return static_cast<int>(buildingLevel);
}

const std::vector<int>& StreetProperty::getRentLevels() const {
    return rentLevels;
}
