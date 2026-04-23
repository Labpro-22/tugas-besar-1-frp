#include "../../include/models/CommunityCards.hpp"

#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/models/Bank.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/GameException.hpp"

namespace {
void resolveBirthdayPayments(Player& beneficiary, GameEngine& game,
                             int amountPerPlayer, size_t startIndex) {
    if (beneficiary.isBankrupt()) {
        return;
    }

    std::vector<Player*> activePlayers = game.getActivePlayers();
    for (size_t i = startIndex; i < activePlayers.size(); ++i) {
        Player* other = activePlayers[i];
        if (!other || other == &beneficiary || other->isBankrupt()) {
            continue;
        }

        if (other->isShieldActive()) {
            game.pushEvent(GameEventType::CARD, UiTone::INFO,
                "Dana Umum",
                other->getUsername() +
                    " dilindungi ShieldCard dan tidak membayar hadiah ulang tahun.");
            continue;
        }

        if (!other->canAfford(amountPerPlayer)) {
            game.pushEvent(GameEventType::CARD, UiTone::WARNING,
                "Dana Umum",
                other->getUsername() + " harus membayar M" +
                    std::to_string(amountPerPlayer) + " kepada " +
                    beneficiary.getUsername() +
                    " karena kartu ulang tahun.");
            game.getBankruptcyManager().handleDebt(
                *other, amountPerPlayer, &beneficiary);
            if (game.hasPendingContinuation()) {
                game.chainPendingContinuation(
                    [creditor = &beneficiary, gamePtr = &game,
                     amountPerPlayer, nextIndex = i + 1]() {
                        CommandResult resumed;
                        resolveBirthdayPayments(*creditor, *gamePtr,
                                                amountPerPlayer, nextIndex);
                        return resumed;
                    });
                return;
            }
            continue;
        }

        const int payerBefore = other->getMoney();
        const int beneficiaryBefore = beneficiary.getMoney();
        other->deductMoney(amountPerPlayer);
        beneficiary.addMoney(amountPerPlayer);

        game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Dana Umum",
            other->getUsername() + " membayar M" +
                std::to_string(amountPerPlayer) + " kepada " +
                beneficiary.getUsername() + ". Uang: M" +
                std::to_string(payerBefore) + " -> M" +
                std::to_string(other->getMoney()) + ". " +
                beneficiary.getUsername() + ": M" +
                std::to_string(beneficiaryBefore) + " -> M" +
                std::to_string(beneficiary.getMoney()) + ".");
    }
}

void resolveElectionPayments(Player& payer, GameEngine& game,
                             int amountPerPlayer, size_t startIndex) {
    if (payer.isBankrupt()) {
        return;
    }

    std::vector<Player*> activePlayers = game.getActivePlayers();
    for (size_t i = startIndex; i < activePlayers.size(); ++i) {
        Player* other = activePlayers[i];
        if (!other || other == &payer || other->isBankrupt()) {
            continue;
        }

        if (payer.isShieldActive()) {
            game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
                "Dana Umum", "ShieldCard melindungi dari efek kartu nyaleg.");
            return;
        }

        if (!payer.canAfford(amountPerPlayer)) {
            game.pushEvent(GameEventType::CARD, UiTone::WARNING,
                "Dana Umum",
                payer.getUsername() + " harus membayar M" +
                    std::to_string(amountPerPlayer) + " kepada " +
                    other->getUsername() + " karena kartu nyaleg.");
            game.getBankruptcyManager().handleDebt(payer, amountPerPlayer, other);
            if (game.hasPendingContinuation()) {
                game.chainPendingContinuation(
                    [payerPtr = &payer, gamePtr = &game,
                     amountPerPlayer, nextIndex = i + 1]() {
                        CommandResult resumed;
                        resolveElectionPayments(*payerPtr, *gamePtr,
                                                amountPerPlayer, nextIndex);
                        return resumed;
                    });
                return;
            }
            if (payer.isBankrupt()) {
                return;
            }
            continue;
        }

        const int payerBefore = payer.getMoney();
        const int receiverBefore = other->getMoney();
        payer.deductMoney(amountPerPlayer);
        other->addMoney(amountPerPlayer);

        game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Dana Umum",
            payer.getUsername() + " membayar M" +
                std::to_string(amountPerPlayer) + " kepada " +
                other->getUsername() + ". Uang: M" +
                std::to_string(payerBefore) + " -> M" +
                std::to_string(payer.getMoney()) + ". " +
                other->getUsername() + ": M" +
                std::to_string(receiverBefore) + " -> M" +
                std::to_string(other->getMoney()) + ".");
    }
}
} // namespace

BirthdayCard::BirthdayCard(int amountPerPlayer)
    : ActionCard("Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain."),
      amountPerPlayer(amountPerPlayer) {}

void BirthdayCard::apply(Player& player, GameEngine& game) {
    resolveBirthdayPayments(player, game, amountPerPlayer, 0);
}

DoctorFeeCard::DoctorFeeCard(int fee)
    : ActionCard("Biaya dokter. Bayar M700."), fee(fee) {}

void DoctorFeeCard::apply(Player& player, GameEngine& game) {
    if (player.isShieldActive()) {
        game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Dana Umum", "ShieldCard melindungi dari biaya dokter.");
        return;
    }

    if (!player.canAfford(fee)) {
        game.pushEvent(GameEventType::CARD, UiTone::WARNING,
            "Dana Umum",
            player.getUsername() + " harus membayar biaya dokter sebesar M" +
                std::to_string(fee) + ".");
        game.getBankruptcyManager().handleDebt(player, fee, nullptr);
        return;
    }

    const int before = player.getMoney();
    player.deductMoney(fee);
    game.pushEvent(GameEventType::CARD, UiTone::WARNING,
        "Dana Umum",
        "Biaya dokter M" + std::to_string(fee) + " dibayar. Uang: M" +
            std::to_string(before) + " -> M" +
            std::to_string(player.getMoney()) + ".");
}

ElectionCard::ElectionCard(int amountPerPlayer)
    : ActionCard("Anda mau nyaleg. Bayar M200 kepada setiap pemain."),
      amountPerPlayer(amountPerPlayer) {}

void ElectionCard::apply(Player& player, GameEngine& game) {
    resolveElectionPayments(player, game, amountPerPlayer, 0);
}

ArisanCard::ArisanCard(int amount)
    : ActionCard("Arisan! Ambil M300 dari bank."), amount(amount) {}

void ArisanCard::apply(Player& player, GameEngine& game) {
    const int before = player.getMoney();
    game.getBank().sendMoney(player, amount);
    game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
        "Dana Umum",
        "Arisan! " + player.getUsername() + " menerima M" +
            std::to_string(amount) + " dari bank. Uang: M" +
            std::to_string(before) + " -> M" +
            std::to_string(player.getMoney()) + ".");
}

BegalCard::BegalCard(int amount)
    : ActionCard("Begal! Bayar M200 ke bank."), amount(amount) {}

void BegalCard::apply(Player& player, GameEngine& game) {
    if (player.isShieldActive()) {
        game.pushEvent(GameEventType::CARD, UiTone::SUCCESS,
            "Dana Umum", "ShieldCard melindungi dari kartu Begal.");
        return;
    }

    if (!player.canAfford(amount)) {
        game.pushEvent(GameEventType::CARD, UiTone::WARNING,
            "Dana Umum",
            player.getUsername() + " harus membayar M" +
                std::to_string(amount) + " ke bank karena kartu Begal.");
        game.getBankruptcyManager().handleDebt(player, amount, nullptr);
        return;
    }

    const int before = player.getMoney();
    game.getBank().receivePayment(player, amount);
    game.pushEvent(GameEventType::CARD, UiTone::WARNING,
        "Dana Umum",
        "Begal! " + player.getUsername() + " membayar M" +
            std::to_string(amount) + " ke bank. Uang: M" +
            std::to_string(before) + " -> M" +
            std::to_string(player.getMoney()) + ".");
}
