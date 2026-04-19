#include "../../include/models/Tile.hpp"

Tile::Tile(int index, const string& code, const string& name) : index(index), code(code), name(name) {}

int Tile::getIndex() const{
    return index;
}

string Tile::getCode() const{
    return code;
}

string Tile::getName() const{
    return name;
}

bool Tile::isProperty() const{
    return false;
}