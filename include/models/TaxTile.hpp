#pragma once

#include "Tile.hpp"


enum class TaxType {
    PPH, // Pajak Penghasilan
    PBM  // Pajak Barang Mewah
};

class TaxTile : public Tile {
private:
    TaxType taxType;
    int flatAmount;  
    int percentage;  // Persentase PPH (0 untuk PBM)

    void handlePPH(Player& player, GameEngine& engine);
    void handlePBM(Player& player, GameEngine& engine);

    // uang tunai + harga beli semua properti + harga beli bangunan
    int calculateWealth(const Player& player) const;

public:
    TaxTile(int index, TaxType taxType, int flatAmount, int percentage = 0);
    
    TaxType getTaxType() const;
    int getFlatAmount() const;
    int getPercentage() const;

    void onLand(Player& player, GameEngine& engine) override;
};