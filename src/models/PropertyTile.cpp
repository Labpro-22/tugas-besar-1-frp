#include "../../include/models/PropertyTile.hpp"

PropertyTile::PropertyTile(int index, shared_ptr<Property> property) : Tile(index, property->getCode(), property->getName()), property(move(property)) {}

Property& PropertyTile::getProperty(){
    return *property;    
}

const Property& PropertyTile::getProperty() const{
    return *property;
}

bool PropertyTile::isProperty() const {
    return true;
}

void PropertyTile::onLand(Player& player, GameEngine& engine){
    (void)player; 
    (void)engine;
}
