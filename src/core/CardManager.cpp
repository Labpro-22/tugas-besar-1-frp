#include "../../include/core/CardManager.hpp"

#include "../../include/core/GameEngine.hpp"
#include "../../include/core/TransactionLogger.hpp"
#include "../../include/models/ChanceCards.hpp"
#include "../../include/models/CommunityCards.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/SkillCards.hpp"
#include "../../include/utils/GameException.hpp"

CardManager::CardManager()
    : rng(std::random_device{}()), initialized(false) {}

void CardManager::initializeDecks() {
    chanceDeck.clear();
    communityDeck.clear();
    skillDeck.clear();
    pendingSkillDraw.clear();

    // Chance deck (tanpa salinan)
    chanceDeck.addCard(std::make_shared<GoToNearestRailroadCard>());
    chanceDeck.addCard(std::make_shared<MoveBackCard>(3));
    chanceDeck.addCard(std::make_shared<GoToJailCard>());
    chanceDeck.addCard(std::make_shared<GetOutOfJailCard>());

    // Community deck (tanpa salinan)
    communityDeck.addCard(std::make_shared<BirthdayCard>(100));
    communityDeck.addCard(std::make_shared<DoctorFeeCard>(700));
    communityDeck.addCard(std::make_shared<ElectionCard>(200));

    // Skill deck (sesuai jumlah lembar di spesifikasi)
    for (int i = 0; i < 4; ++i) skillDeck.addCard(std::make_shared<MoveCard>(0));
    for (int i = 0; i < 3; ++i) skillDeck.addCard(std::make_shared<DiscountCard>(0));
    for (int i = 0; i < 2; ++i) skillDeck.addCard(std::make_shared<ShieldCard>());
    for (int i = 0; i < 2; ++i) skillDeck.addCard(std::make_shared<TeleportCard>());
    for (int i = 0; i < 2; ++i) skillDeck.addCard(std::make_shared<LassoCard>());
    for (int i = 0; i < 2; ++i) skillDeck.addCard(std::make_shared<DemolitionCard>());

    chanceDeck.shuffle();
    communityDeck.shuffle();
    skillDeck.shuffle();

    initialized = true;
}

void CardManager::randomizeSkillCardOnDraw(const std::shared_ptr<SkillCard>& card) {
    if (!card) {
        return;
    }

    const std::string type = card->getTypeName();
    if (type == "MoveCard" && card->getValue() <= 0) {
        std::uniform_int_distribution<int> dist(1, 12);
        card->setValue(dist(rng));
    } else if (type == "DiscountCard" && card->getValue() <= 0) {
        std::uniform_int_distribution<int> stepDist(1, 5);
        card->setValue(stepDist(rng) * 5);
    }
}

void CardManager::drawChanceCard(Player& player, GameEngine& game) {
    if (!initialized) initializeDecks();

    std::shared_ptr<ActionCard> card = chanceDeck.draw();
    std::string cardCode;
    if (card && dynamic_cast<const GoToNearestRailroadCard*>(card.get()) != nullptr) {
        cardCode = "CHANCE_NEAREST_RAILROAD";
    } else if (card && dynamic_cast<const MoveBackCard*>(card.get()) != nullptr) {
        cardCode = "CHANCE_MOVE_BACK";
    } else if (card && dynamic_cast<const GoToJailCard*>(card.get()) != nullptr) {
        cardCode = "CHANCE_GO_TO_JAIL";
    } else if (card && dynamic_cast<const GetOutOfJailCard*>(card.get()) != nullptr) {
        cardCode = "CHANCE_GET_OUT_OF_JAIL";
    }

    game.pushEvent(GameEventType::CARD, UiTone::INFO,
        "Kartu Kesempatan",
        "Kamu mendarat di Kesempatan!\n"
        "Pemain " + player.getUsername() + " mengambil kartu Kesempatan:\n\"" +
            card->getDescription() + "\"",
        cardCode);
    game.getLogger().logDrawCard(player.getUsername(), "Kesempatan",
                                 card->getDescription());
    card->apply(player, game);
    chanceDeck.discard(card);
}

void CardManager::drawCommunityCard(Player& player, GameEngine& game) {
    if (!initialized) initializeDecks();

    std::shared_ptr<ActionCard> card = communityDeck.draw();
    std::string cardCode;
    if (card && dynamic_cast<const BirthdayCard*>(card.get()) != nullptr) {
        cardCode = "CARD_BIRTHDAY";
    } else if (card && dynamic_cast<const DoctorFeeCard*>(card.get()) != nullptr) {
        cardCode = "CARD_DOCTOR";
    } else if (card && dynamic_cast<const ElectionCard*>(card.get()) != nullptr) {
        cardCode = "CARD_ELECTION";
    }

    game.pushEvent(GameEventType::CARD, UiTone::INFO,
        "Dana Umum",
        "Kamu mendarat di Dana Umum!\n"
        "Pemain " + player.getUsername() + " mengambil kartu Dana Umum:\n\"" +
            card->getDescription() + "\"",
        cardCode);
    game.getLogger().logDrawCard(player.getUsername(), "Dana Umum",
                                 card->getDescription());
    card->apply(player, game);
    communityDeck.discard(card);
}

std::shared_ptr<SkillCard> CardManager::drawSkillCard(Player& player) {
    if (!initialized) initializeDecks();

    if (hasPendingSkillDrop(player)) {
        throw GameException("Player masih punya pending drop kartu skill ke-4.");
    }

    std::shared_ptr<SkillCard> drawn = skillDeck.draw();
    randomizeSkillCardOnDraw(drawn);

    if (player.countCards() < 3) {
        player.addCard(drawn);
        return drawn;
    }

    // Tidak prompt di manager, simpan pending agar UI memutuskan kartu mana yang dibuang.
    pendingSkillDraw[player.getUsername()] = drawn;
    return drawn;
}

bool CardManager::hasPendingSkillDrop(const Player& player) const {
    return pendingSkillDraw.find(player.getUsername()) != pendingSkillDraw.end();
}

std::vector<std::string> CardManager::getPendingSkillDropOptions(const Player& player) const {
    std::vector<std::string> options;

    const auto& hand = player.getHandCards();
    for (const auto& card : hand) {
        if (card) {
            options.push_back(card->getTypeName());
        }
    }

    auto it = pendingSkillDraw.find(player.getUsername());
    if (it != pendingSkillDraw.end() && it->second) {
        options.push_back(it->second->getTypeName());
    }

    return options;
}

void CardManager::resolvePendingSkillDrop(Player& player, int discardIndexFromFour) {
    auto it = pendingSkillDraw.find(player.getUsername());
    if (it == pendingSkillDraw.end() || !it->second) {
        throw GameException("Tidak ada pending kartu skill ke-4 untuk pemain ini.");
    }

    if (discardIndexFromFour < 0 || discardIndexFromFour > 3) {
        throw InvalidCardIndexException(discardIndexFromFour);
    }

    std::shared_ptr<SkillCard> drawn = it->second;
    pendingSkillDraw.erase(it);

    if (discardIndexFromFour == 3) {
        skillDeck.discard(drawn);
        return;
    }

    const auto& hand = player.getHandCards();
    if (discardIndexFromFour >= static_cast<int>(hand.size())) {
        throw InvalidCardIndexException(discardIndexFromFour);
    }

    std::shared_ptr<SkillCard> discarded = hand[discardIndexFromFour];
    player.removeCard(discardIndexFromFour);
    skillDeck.discard(discarded);
    player.addCard(drawn);
}

void CardManager::useSkillCard(Player& player, int idx, GameEngine& game,
                               const std::string& targetArg) {
    if (player.hasUsedSkillThisTurn()) {
        throw GameException("Skill card hanya boleh dipakai sekali per giliran.");
    }

    const auto& hand = player.getHandCards();
    if (idx < 0 || idx >= static_cast<int>(hand.size())) {
        throw InvalidCardIndexException(idx);
    }

    std::shared_ptr<SkillCard> card = hand[idx];
    if (!card) {
        throw GameException("Skill card tidak valid.");
    }

    const std::string type = card->getTypeName();
    if (type == "TeleportCard") {
        std::shared_ptr<TeleportCard> t = std::static_pointer_cast<TeleportCard>(card);
        t->setTargetCode(targetArg);
    } else if (type == "LassoCard") {
        std::shared_ptr<LassoCard> l = std::static_pointer_cast<LassoCard>(card);
        l->setTargetUsername(targetArg);
    } else if (type == "DemolitionCard") {
        std::shared_ptr<DemolitionCard> d = std::static_pointer_cast<DemolitionCard>(card);
        d->setTargetPropertyCode(targetArg);
    }

    card->apply(player, game);

    player.removeCard(idx);
    player.setUsedSkillThisTurn(true);
    skillDeck.discard(card);
}

std::vector<std::string> CardManager::getSkillDeckStateForSave() const {
    std::vector<std::string> state;

    const auto& drawPile = skillDeck.getDrawPile();
    for (const auto& card : drawPile) {
        if (card) {
            state.push_back(card->getTypeName());
        }
    }

    return state;
}

std::shared_ptr<SkillCard> CardManager::createSkillCardByType(const std::string& typeName) const {
    if (typeName == "MoveCard") return std::make_shared<MoveCard>(0);
    if (typeName == "DiscountCard") return std::make_shared<DiscountCard>(0);
    if (typeName == "ShieldCard") return std::make_shared<ShieldCard>();
    if (typeName == "TeleportCard") return std::make_shared<TeleportCard>();
    if (typeName == "LassoCard") return std::make_shared<LassoCard>();
    if (typeName == "DemolitionCard") return std::make_shared<DemolitionCard>();

    throw GameException("Unknown skill card type in save/load: " + typeName);
}

void CardManager::loadSkillDeckState(const std::vector<std::string>& cardTypes) {
    skillDeck.clear();

    for (const std::string& typeName : cardTypes) {
        skillDeck.addCard(createSkillCardByType(typeName));
    }

    initialized = true;
}
