#include "../../include/models/TaxTile.hpp"
#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/utils/GameException.hpp"
#include <sstream>

TaxTile::TaxTile(int index, TaxType taxType, int flatAmount, int percentage,
                 const string& code, const string& name)
    : Tile(index,
           code.empty() ? (taxType == TaxType::PPH ? "PPH" : "PBM") : code,
           name.empty() ? (taxType == TaxType::PPH
                               ? "Pajak Penghasilan"
                               : "Pajak Barang Mewah")
                        : name),
      taxType(taxType), flatAmount(flatAmount), percentage(percentage) {}

TaxType TaxTile::getTaxType()    const { return taxType;    }
int     TaxTile::getFlatAmount() const { return flatAmount; }
int     TaxTile::getPercentage() const { return percentage; }

int TaxTile::calculateWealth(const Player& player) const {
    int wealth = player.getMoney();
    for (const Property* prop : player.getOwnedProperties()) {
        if (!prop) throw GameException(
            "Null property pada pemain '" + player.getUsername() + "'.");
        wealth += prop->getPurchasePrice();
        if (prop->getType() == PropertyType::STREET)
            wealth += static_cast<const StreetProperty*>(prop)
                          ->getBuildingSellValue() * 2;
    }
    return wealth;
}

void TaxTile::handlePPH(Player& player, GameEngine& engine) {
    if (player.isShieldActive()) {
        engine.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Shield Aktif", "ShieldCard melindungi dari PPH!");
        return;
    }

    const int taxFlat = flatAmount;
    const int wealth  = calculateWealth(player);
    const int taxPct  = (wealth * percentage) / 100;

    std::ostringstream msg;
    msg << "Kamu mendarat di " << getName() << " (" << getCode() << ")!\n"
        << "Kamu bebas memilih untuk membayar M" << taxFlat << " atau " << percentage << "% dari total kekayaan.\n"
        << "(Pilih sebelum jumlah kekayaanmu dihitung!)";
    const std::string promptKey = "pph_" + player.getUsername();

    if (!engine.hasPromptAnswer(promptKey)) {
        engine.pushEvent(GameEventType::TAX, UiTone::WARNING, "PPH", msg.str());
        engine.pushPrompt(promptKey,
            "Opsi PPH mana yang ingin kamu pilih? (1/2):", {"1", "2"});
        engine.setPendingContinuation([this, &player, &engine]() {
            CommandResult resumed;
            handlePPH(player, engine);
            return resumed;
        });
        return;
    }

    const std::string ans = engine.consumePromptAnswer(promptKey);
    if (ans != "1" && ans != "2") {
        engine.pushEvent(GameEventType::TAX, UiTone::WARNING,
            "Input Tidak Valid", "Masukkan 1 (flat) atau 2 (persentase).");
        engine.pushPrompt(promptKey,
            "Opsi PPH mana yang ingin kamu pilih? (1/2):", {"1", "2"});
        engine.setPendingContinuation([this, &player, &engine]() {
            CommandResult resumed;
            handlePPH(player, engine);
            return resumed;
        });
        return;
    }

    int choice = (ans == "2") ? 2 : 1;

    if (choice == 1) {
        if (!player.canAfford(taxFlat)) {
            engine.pushEvent(GameEventType::TAX, UiTone::ERROR,
                "Tidak Cukup",
                "Tidak bisa bayar flat M" + std::to_string(taxFlat) +
                ". Uang: M" + std::to_string(player.getMoney()));
            engine.getBankruptcyManager().handleDebt(player, taxFlat, nullptr);
            return;
        }
        int before = player.getMoney();
        engine.getBank().receivePayment(player, taxFlat);
        engine.pushEvent(GameEventType::TAX, UiTone::SUCCESS, "Bayar PPH Flat",
            "Pajak sebesar M" + std::to_string(taxFlat) + " telah dibayar!\n"
            "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
        engine.getLogger().logTax(player.getUsername(), "PPH flat", taxFlat);
    } else {
        int totalProperti = 0;
        int totalBangunan = 0;
        for (const Property* prop : player.getOwnedProperties()) {
            totalProperti += prop->getPurchasePrice();
            if (prop->getType() == PropertyType::STREET) {
                totalBangunan += static_cast<const StreetProperty*>(prop)->getBuildingSellValue() * 2;
            }
        }

        std::ostringstream detail;
        detail << "Rincian Kekayaan:\n"
               << "- Uang tunai : M" << player.getMoney() << "\n"
               << "- Properti   : M" << totalProperti << "\n"
               << "- Bangunan   : M" << totalBangunan << "\n"
               << "Total Kekayaan : M" << wealth;
        engine.pushEvent(GameEventType::TAX, UiTone::INFO, "Rincian Kekayaan",
            detail.str());

        if (!player.canAfford(taxPct)) {
            engine.pushEvent(GameEventType::TAX, UiTone::ERROR,
                "Tidak Cukup",
                "Tidak bisa bayar M" + std::to_string(taxPct) +
                ". Uang: M" + std::to_string(player.getMoney()));
            engine.getBankruptcyManager().handleDebt(player, taxPct, nullptr);
            return;
        }
        int before = player.getMoney();
        engine.getBank().receivePayment(player, taxPct);
        engine.pushEvent(GameEventType::TAX, UiTone::SUCCESS,
            "Bayar PPH " + std::to_string(percentage) + "%",
            "Pajak " + std::to_string(percentage) + "% sebesar M" + std::to_string(taxPct) + " telah dibayar!\n"
            "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
        engine.getLogger().logTax(player.getUsername(),
            "PPH " + std::to_string(percentage) + "%", taxPct);
    }
}

void TaxTile::handlePBM(Player& player, GameEngine& engine) {
    if (player.isShieldActive()) {
        engine.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Shield Aktif", "ShieldCard melindungi dari PBM!");
        return;
    }

    engine.pushEvent(GameEventType::TAX, UiTone::WARNING, "PBM",
        "Kamu mendarat di Pajak Barang Mewah (PBM)!");

    if (!player.canAfford(flatAmount)) {
        engine.pushEvent(GameEventType::TAX, UiTone::ERROR,
            "Tidak Cukup",
            "Tidak bisa bayar M" + std::to_string(flatAmount) +
            ". Uang: M" + std::to_string(player.getMoney()));
        engine.getBankruptcyManager().handleDebt(player, flatAmount, nullptr);
        return;
    }

    int before = player.getMoney();
    engine.getBank().receivePayment(player, flatAmount);
    engine.pushEvent(GameEventType::TAX, UiTone::SUCCESS, "Bayar PBM",
        "Pajak sebesar M" + std::to_string(flatAmount) + " telah dibayar!\n"
        "Uang kamu saat ini: M" + std::to_string(player.getMoney()));
    engine.getLogger().logTax(player.getUsername(), "PBM", flatAmount);
}

void TaxTile::onLand(Player& player, GameEngine& engine) {
    if (taxType == TaxType::PPH) handlePPH(player, engine);
    else                         handlePBM(player, engine);
}
