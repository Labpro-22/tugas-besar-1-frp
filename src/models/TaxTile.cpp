#include "../../include/models/TaxTile.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/utils/GameException.hpp"

TaxTile::TaxTile(int index, TaxType taxType, int flatAmount, int percentage) : Tile (index, taxType == TaxType::PPH ? "PPH" : "PBM", taxType == TaxType::PPH ? "Pajak Penghasilan" : "Pajak Barang Mewah"), taxType(taxType), flatAmount(flatAmount), percentage(percentage) {}

TaxType TaxTile::getTaxType() const{
    return taxType;
}

int TaxTile::getFlatAmount() const{
    return flatAmount;
}

int TaxTile::getPercentage() const{
    return percentage;
}

void TaxTile::handlePPH(Player& player, GameEngine& engine){
    // butuh class Bank, class BankruptcyManager

    (void)player;
    (void)engine;
    // int choice = engine.promptPPHChoice(flatAmount, percentage);
    // if (choice == 1){
    //     if (!player.canAfford(flatAmount)){
    //         engine.getBankruptcyManager().handleDebt(player, flatAmount, Creditor::BANK, engine);
    //     }
    //     else{
    //         engine.getBank().collectTax(player, flatAmount);
    //         engine.getLogger().log(engine.getCurrentTurn(), player.getUsername(), "PAJAK", "Bayar PPH flat M" + to_string(flatAmount));
    //     }
    // }
    // else{
    //     int wealth = calculateWealth(player);
    //     int tax = wealth * percentage/100;
    //     if (!player.canAfford(tax)){
    //         engine.getBankruptcyManager().handleDebt(player, tax, Creditor::BANK, engine);
    //     }
    //     else{
    //         engine.getBank().collectTax(player, tax);
    //         engine.getLogger().log(engine.getCurrentTurn(),
    //             player.getUsername(), "PAJAK",
    //             "Bayar PPH " + std::to_string(percentage) + "% = M"
    //             + std::to_string(tax) + " (kekayaan M"
    //             + std::to_string(wealth) + ")");
    //     }
    // }
}

void TaxTile::handlePBM(Player& player, GameEngine& engine){
    // butuh class Bank, class BankruptcyManager

    (void)player;
    (void)engine;
    // if (!player.canAfford(flatAmount)) {
    //     engine.getBankruptcyManager().handleDebt(
    //         player, flatAmount, Creditor::BANK, engine
    //     );
    // } else {
    //     engine.getBank().collectTax(player, flatAmount);
    //     engine.getLogger().log(engine.getCurrentTurn(),
    //         player.getUsername(), "PAJAK",
    //         "Bayar PBM M" + std::to_string(flatAmount));
    // }
}

int TaxTile::calculateWealth(const Player& player) const{
    int wealth = player.getMoney();

    for (const Property* prop : player.getOwnedProperties()){
        if (!prop) {
            throw GameException("Null property found in owned properties of '"
                + player.getUsername() + "'.");
        }

        wealth += prop->getPurchasePrice();

        if (prop->getType() == PropertyType::STREET){
            const auto* street = static_cast<const StreetProperty*>(prop);
            wealth += street->getBuildingSellValue() * 2;
        }
    }
    return wealth;
}

void TaxTile::onLand(Player& player, GameEngine& engine) {
    if (taxType == TaxType::PPH){
        handlePPH(player, engine);
    }
    else{
        handlePBM(player, engine);
    }
}