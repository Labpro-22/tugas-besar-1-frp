#include "../../include/models/Board.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/utils/GameException.hpp"


void Board::validateIndex(int idx) const {
    if (tiles.empty()){
        throw BoardEmptyException();
    }
}

void Board::validateCode(const string& code) const {
    if (codeToIndex.find(code) == codeToIndex.end()){
        throw TileNotFoundException(code);
    }
}


void Board::addTile(unique_ptr<Tile> tile) {
    if (!tile) {
        throw NullTileException();
    }
    const string code = tile->getCode();
    const int         idx  = static_cast<int>(tiles.size());
    codeToIndex[code] = idx;
    tiles.push_back(move(tile));
}

Tile& Board::getTileByIndex(int idx) {
    validateIndex(idx);
    return *tiles[idx % static_cast<int>(tiles.size())];
}

const Tile& Board::getTileByIndex(int idx) const {
    validateIndex(idx);
    return *tiles[idx % static_cast<int>(tiles.size())];
}

Tile& Board::getTileByCode(const string& code) {
    validateCode(code);
    return *tiles[codeToIndex.at(code)];
}

const Tile& Board::getTileByCode(const string& code) const {
    validateCode(code);
    return *tiles[codeToIndex.at(code)];
}

int Board::getIndexOf(const string& code) const {
    validateCode(code);
    return codeToIndex.at(code);
}

int Board::size() const {
    return static_cast<int>(tiles.size());
}


int Board::distanceTo(int from, int to) const {
    const int n = size();
    if (n == 0) return 0;
    return (to - from + n) % n;
}


vector<PropertyTile*> Board::getAllPropertyTiles() const {
    vector<PropertyTile*> result;
    for (const auto& tile : tiles) {
        if (tile->isProperty()) {
            result.push_back(static_cast<PropertyTile*>(tile.get()));
        }
    }
    return result;
}

vector<PropertyTile*> Board::getPropertyTilesByColorGroup(
        const string& colorGroup) const {
    vector<PropertyTile*> result;
    for (const auto& tile : tiles) {
        if (!tile->isProperty()) continue;

        auto* pt = static_cast<PropertyTile*>(tile.get());
        const Property& prop = pt->getProperty();

        if (prop.getType() != PropertyType::STREET) continue;

        const auto* street = static_cast<const StreetProperty*>(&prop);
        if (street->getColorGroup() == colorGroup) {
            result.push_back(pt);
        }
    }
    return result;
}