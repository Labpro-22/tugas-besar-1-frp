#include "../../include/models/PropertyTile.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/models/Player.hpp"

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
    Property& prop = *property;

    if (prop.isBank()) {
        if (prop.getType() == PropertyType::STREET) {
            const bool bought = engine.getPropertyManager().offerPurchase(player, prop);
            if (engine.hasPendingContinuation()) {
                return;
            }
            if (!bought) {
                engine.getAuctionManager().startAuction(prop, &player, true);
            }
        } else {
            engine.getPropertyManager().autoAcquire(player, prop);
        }
        return;
    }

    if (prop.getOwner() == nullptr || prop.getOwner() == &player) {
        return;
    }

    GameContext ctx(engine.getPlayers(), &engine.getBoard(), engine.getDice().getTotal());
    const int rent = prop.calculateRent(ctx);

    if (!player.isShieldActive() && !player.canAfford(rent)) {
        engine.getBankruptcyManager().handleDebt(player, rent, prop.getOwner());
        return;
    }

    engine.getPropertyManager().payRent(player, prop, ctx);
}
