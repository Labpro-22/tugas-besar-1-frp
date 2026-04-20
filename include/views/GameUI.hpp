#pragma once

#include "../../include/core/GameEngine.hpp"
#include "../../include/views/BoardRenderer.hpp"
#include "../../include/views/CommandParser.hpp"
#include "../../include/views/UiFormatter.hpp"

class GameUI {
public:
    explicit GameUI(GameEngine& engine);
    void run();

private:
    GameEngine& engine;
    CommandParser parser;
    BoardRenderer renderer;
    UiFormatter formatter;

    bool started;

    void displayWelcome() const;
    void bootstrapIfNeeded();
};
