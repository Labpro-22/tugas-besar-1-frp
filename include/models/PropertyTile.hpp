#pragma once

#include "Tile.hpp"
#include "Property.hpp"
#include <memory>

class PropertyTile : public Tile{
private:
    shared_ptr<Property> property;

public:
    PropertyTile(int index, shared_ptr<Property> property);

    Property& getProperty();
    const Property& getProperty() const;

    bool isProperty() const override;
    void onLand(Player& player, GameEngine& engine) override;
};