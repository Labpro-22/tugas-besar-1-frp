#pragma once

#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "../models/ActionCard.hpp"
#include "../models/Deck.hpp"
#include "../models/SkillCard.hpp"

class Player;
class GameEngine;

class CardManager {
private:
    Deck<ActionCard> chanceDeck;
    Deck<ActionCard> communityDeck;
    Deck<SkillCard> skillDeck;

    std::unordered_map<std::string, std::shared_ptr<SkillCard>> pendingSkillDraw;

    std::mt19937 rng;
    bool initialized;

    void randomizeSkillCardOnDraw(const std::shared_ptr<SkillCard>& card);

public:
    std::shared_ptr<SkillCard> createSkillCardByType(const std::string& typeName) const;

    CardManager();

    void initializeDecks();

    void drawChanceCard(Player& player, GameEngine& game);
    void drawCommunityCard(Player& player, GameEngine& game);

    std::shared_ptr<SkillCard> drawSkillCard(Player& player);

    bool hasPendingSkillDrop(const Player& player) const;
    std::shared_ptr<SkillCard> peekPendingSkillDraw(const Player& player) const;
    std::vector<std::string> getPendingSkillDropOptions(const Player& player) const;
    void resolvePendingSkillDrop(Player& player, int discardSkillIndex);

    void useSkillCard(Player& player, int idx, GameEngine& game,
                      const std::string& targetArg = "");

    std::vector<std::string> getSkillDeckStateForSave() const;
    void loadSkillDeckState(const std::vector<std::string>& cardTypes);
};
