#pragma once

#include "../../include/core/CommandResult.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/views/BoardRenderer.hpp"
#include "../../include/views/CommandParser.hpp"
#include "../../include/views/UiFormatter.hpp"

class GameUI {
public:
    explicit GameUI(GameEngine& engine);
    void run();

private:
    GameEngine&   engine;
    CommandParser parser;
    BoardRenderer renderer;
    UiFormatter   formatter;
    bool          started;

    void displayWelcome() const;
    void bootstrapIfNeeded();
    void resolvePrompts();
    void handlePendingSkillDrop();
    void printStatusBar() const;

    // Baca satu baris input dari terminal
    std::string readLine(const std::string& prompt = "") const;

    // Tampilkan satu PromptRequest ke terminal dan isi jawaban ke engine
    void handleSinglePrompt(const PromptRequest& prompt);

    // Jalankan command, resolve semua prompt yang muncul, kembalikan result akhir
    CommandResult executeWithPrompts(const Command& cmd);
};
