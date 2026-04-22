#include "../../include/models/FestivalTile.hpp"
#include "../../include/core/EffectManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/GameContext.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include <sstream>

FestivalTile::FestivalTile(int index, const std::string& code,
                           const std::string& name)
    : Tile(index, code, name) {}

void FestivalTile::onLand(Player& player, GameEngine& engine) {
    std::vector<Property*> candidates;
    for (Property* prop : player.getOwnedProperties())
        if (prop && !prop->isMortgaged()) candidates.push_back(prop);

    if (candidates.empty()) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO,
            "Festival", "Tidak ada properti yang bisa diperkuat.");
        return;
    }

    GameContext ctx(engine.getPlayers(), &engine.getBoard(),
                    engine.getDice().getTotal());

    const std::string promptKey = "festival_" + player.getUsername();
    if (!engine.hasPromptAnswer(promptKey)) {
        std::ostringstream msg;
        msg << "Kamu mendarat di Festival!\n"
            << "Pilih salah satu properti yang kamu miliki untuk menyelenggarakan festival!\n"
            << "(Harga sewa properti ini akan menjadi 2x lipat pada kunjungan berikutnya)\n\n"
            << "Daftar propertimu:\n";
        for (Property* p : candidates) {
            msg << "- " << p->getName() << " (" << p->getCode() << ")\n";
        }
        engine.pushEvent(GameEventType::PROPERTY, UiTone::INFO, "Festival", msg.str());

        // Kumpulkan semua opsi kode untuk prompt
        std::vector<std::string> opts;
        for (Property* p : candidates) {
            opts.push_back(p->getCode());
        }

        engine.pushPrompt(promptKey,
            "Masukkan nama/id propertimu:",
            opts);
        engine.setPendingContinuation([this, &player, &engine]() {
            CommandResult resumed;
            onLand(player, engine);
            return resumed;
        });
        return;
    }

    const std::string raw = engine.consumePromptAnswer(promptKey);
    Property* chosen = nullptr;
    for (Property* p : candidates) {
        if (p->getCode() == raw) {
            chosen = p;
            break;
        }
    }

    if (!chosen) {
        engine.pushEvent(GameEventType::PROPERTY, UiTone::WARNING,
            "Input Tidak Valid", "Kode properti tidak ditemukan di daftar milikmu.");
        
        std::vector<std::string> opts;
        for (Property* p : candidates) opts.push_back(p->getCode());

        engine.pushPrompt(promptKey,
            "Masukkan nama/id propertimu:",
            opts);
        engine.setPendingContinuation([this, &player, &engine]() {
            CommandResult resumed;
            onLand(player, engine);
            return resumed;
        });
        return;
    }

    const int oldRent = chosen->calculateRent(ctx);
    engine.getEffectManager().applyFestival(player, *chosen);
    const int newRent = chosen->calculateRent(ctx);

    std::ostringstream result;
    result << "Festival diadakan di " << chosen->getCode() << "! Harga sewa di properti tersebut telah naik!";
    engine.pushEvent(GameEventType::PROPERTY, UiTone::SUCCESS, "Festival", result.str());

    engine.getLogger().logFestival(player.getUsername(),
        chosen->getCode(), oldRent, newRent, chosen->getFestivalDuration());
}
