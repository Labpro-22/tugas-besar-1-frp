#pragma once

#include "Tile.hpp"
#include "PropertyTile.hpp"
#include <vector>
#include <map>
#include <memory>
#include <string>

class Board {
private:
    vector<unique_ptr<Tile>> tiles;
    map<string, int> codeToIndex;

    void validateIndex(int idx) const;
    void validateCode(const std::string& code) const;

public:
    Board() = default;

    void addTile(std::unique_ptr<Tile> tile);

    Tile&       getTileByIndex(int idx);
    const Tile& getTileByIndex(int idx) const;

    Tile&       getTileByCode(const std::string& code);
    const Tile& getTileByCode(const std::string& code) const;

    int  getIndexOf(const std::string& code) const;

    int  size() const;

    int  distanceTo(int from, int to) const;

    vector<PropertyTile*> getAllPropertyTiles() const;

    vector<PropertyTile*> getPropertyTilesByColorGroup(
        const std::string& colorGroup) const;
};