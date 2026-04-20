#include "../../include/views/GameUI.hpp"

#include "../../include/utils/GameException.hpp"
#include "../../include/views/AnsiTheme.hpp"

#include <iostream>
#include <string>
#include <vector>

GameUI::GameUI(GameEngine& engine)
    : engine(engine),
      started(false) {}

void GameUI::displayWelcome() const {
    std::cout << AnsiTheme::clearScreen();
    std::cout << AnsiTheme::apply(AnsiTheme::bold(), "Nimonspoli CLI - MVP") << "\n";
    std::cout << "Ketik BANTUAN untuk melihat daftar perintah.\n";
    std::cout << "Ketik KELUAR untuk menutup program.\n\n";
}

void GameUI::bootstrapIfNeeded() {
    if (started) {
        return;
    }

    CommandResult startResult = engine.startNewGame(2, {"Pemain1", "Pemain2"});
    std::cout << formatter.format(startResult) << "\n";
    started = true;
}

void GameUI::run() {
    displayWelcome();
    bootstrapIfNeeded();

    std::string raw;
    while (true) {
        std::cout << AnsiTheme::apply(AnsiTheme::bold(), "> ");
        if (!std::getline(std::cin, raw)) {
            break;
        }

        try {
            Command cmd = parser.parse(raw);
            if (cmd.type == CommandType::EXIT) {
                std::cout << "Sampai jumpa!\n";
                break;
            }

            CommandResult result = engine.processCommand(cmd);
            std::cout << formatter.format(result) << "\n";

            if (cmd.type == CommandType::PRINT_BOARD) {
                std::cout << renderer.render(
                    engine.getBoard(),
                    engine.getPlayers(),
                    engine.getCurrentTurn(),
                    engine.getMaxTurn())
                          << "\n";
            }
        } catch (const GameException& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[ERROR] ") << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[FATAL] ") << e.what() << "\n";
        }
    }
}
