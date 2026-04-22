#include "../../include/views/GameUI.hpp"

#include "../../include/core/CardManager.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/views/AnsiTheme.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

GameUI::GameUI(GameEngine& engine)
    : engine(engine), started(false) {}

void GameUI::displayWelcome() const {
    std::cout << AnsiTheme::clearScreen();
    std::cout << AnsiTheme::apply(AnsiTheme::bold(),
        "╔══════════════════════════════╗\n"
        "║     NIMONSPOLI CLI - FRP     ║\n"
        "╚══════════════════════════════╝\n");
    std::cout << "Ketik BANTUAN untuk daftar perintah.\n";
    std::cout << "Ketik KELUAR untuk menutup program.\n\n";
}

// ── Baca satu baris input dari terminal ───────────────────────────────────────
std::string GameUI::readLine(const std::string& prompt) const {
    if (!prompt.empty())
        std::cout << AnsiTheme::apply(AnsiTheme::warning(), prompt);
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// ── Normalise uppercase ───────────────────────────────────────────────────────
static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    return s;
}

// ── Isi semua pending prompt yang ada di engine dengan jawaban dari user ──────
void GameUI::resolvePrompts() {
    // Loop sampai tidak ada prompt lagi (satu prompt → satu loop)
    // Kita tidak tahu persis berapa prompt, jadi kita flush result dulu
    // dan cek apakah ada prompt di dalamnya.
    // Implementasi: kita jalankan mini-loop di sini setelah flushEvents.
    // Karena flushEvents sudah dipanggil di processCommand, kita cukup
    // tangani prompt yang ada di result terakhir lewat parameter.
}

// ── Handle satu PromptRequest: cetak ke terminal, baca jawaban ────────────────
void GameUI::handleSinglePrompt(const PromptRequest& prompt) {
    std::cout << AnsiTheme::apply(AnsiTheme::warning(),
        "\n[?] " + prompt.message + "\n");
    if (!prompt.options.empty()) {
        std::cout << AnsiTheme::apply(AnsiTheme::dim(), "    Opsi: ");
        for (size_t i = 0; i < prompt.options.size(); ++i) {
            if (i > 0) std::cout << " / ";
            std::cout << prompt.options[i];
        }
        std::cout << "\n";
    }
    std::cout << AnsiTheme::apply(AnsiTheme::bold(), "    >> ");
    std::string answer;
    std::getline(std::cin, answer);
    engine.setPromptAnswer(prompt.key, answer);
}

// ── Eksekusi command + resolve semua prompt yang muncul secara rekursif ───────
// Setiap kali engine membutuhkan input (ada prompt di result), kita
// minta user, set jawaban, lalu jalankan ulang command yang sama.
CommandResult GameUI::executeWithPrompts(const Command& cmd) {
    engine.clearPromptAnswers();
    engine.clearPendingContinuation();

    // Loop: jalankan command, jika ada prompt → jawab → ulangi
    // Maksimum 10 iterasi untuk hindari infinite loop
    for (int iteration = 0; iteration < 64; ++iteration) {
        CommandResult result = engine.processCommand(cmd);

        if (!result.prompt.has_value()) {
            // Tidak ada prompt → selesai
            return result;
        }

        // Ada prompt → tampilkan ke user, isi jawaban
        while (result.prompt.has_value()) {
            const PromptRequest& prompt = result.prompt.value();
            std::cout << formatter.format(result);
            handleSinglePrompt(prompt);

            if (!engine.hasPendingContinuation()) {
                break;
            }

            result = engine.resumePendingAction();
        }

        if (!result.prompt.has_value()) {
            return result;
        }
    }

    throw GameException("Terlalu banyak prompt berturut-turut dalam satu aksi.");
}

// ── Bootstrap: menu awal NEW / LOAD ──────────────────────────────────────────
void GameUI::bootstrapIfNeeded() {
    if (started) return;

    while (!started) {
        std::cout << "Menu awal [NEW/LOAD] (default NEW): ";
        std::string menuChoice;
        std::getline(std::cin, menuChoice);
        std::string normalized = toUpper(menuChoice);

        try {
            if (normalized == "LOAD" || normalized == "MUAT") {
                std::cout << "Nama file save (.txt, default file_save.txt): ";
                std::string filename;
                std::getline(std::cin, filename);
                if (filename.empty()) filename = "file_save.txt";

                CommandResult loadResult = engine.loadGame(filename);
                std::cout << formatter.format(loadResult) << "\n";
                started = true;
                return;
            }

            int nPlayers = 4;
            std::cout << "Jumlah pemain (2-4, default 4): ";
            std::string rawN;
            std::getline(std::cin, rawN);
            if (!rawN.empty()) {
                try {
                    int parsed = std::stoi(rawN);
                    if (parsed >= 2 && parsed <= 4) nPlayers = parsed;
                } catch (...) {}
            }

            std::vector<std::string> names;
            names.reserve(nPlayers);
            for (int i = 0; i < nPlayers; ++i) {
                std::cout << "Nama pemain " << (i + 1) << ": ";
                std::string name;
                std::getline(std::cin, name);
                if (name.empty()) name = "P" + std::to_string(i + 1);
                names.push_back(name);
            }

            CommandResult startResult = engine.startNewGame(nPlayers, names);
            std::cout << formatter.format(startResult) << "\n";
            started = true;

        } catch (const GameException& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[ERROR] ")
                      << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[FATAL] ")
                      << e.what() << "\n";
        }
    }
}

// ── Handler kartu ke-4 (drop interaktif) ─────────────────────────────────────
void GameUI::handlePendingSkillDrop() {
    if (engine.isGameOver() || engine.getPlayers().empty()) return;

    try {
        Player& current = engine.getCurrentPlayer();
        CardManager& cardManager = engine.getCardManager();
        if (!cardManager.hasPendingSkillDrop(current)) return;

        std::cout << "\nPemain " << current.getUsername() << " mendapatkan kartu ke-4! "
                  << "Wajib membuang 1 kartu kemampuan.\n";

        const auto options = cardManager.getPendingSkillDropOptions(current);
        std::cout << "Pilih kartu yang akan dibuang:\n";
        for (size_t i = 0; i < options.size(); ++i)
            std::cout << (i + 1) << ". " << options[i] << "\n";

        int choice = 0;
        while (choice < 1 || choice > static_cast<int>(options.size())) {
            std::cout << "Masukkan nomor kartu yang akan dibuang (1-"
                      << options.size() << "): ";
            std::string input;
            std::getline(std::cin, input);
            try { choice = std::stoi(input); } catch (...) { choice = 0; }
        }

        cardManager.resolvePendingSkillDrop(current, choice - 1);
        std::cout << "Kartu berhasil dibuang! Kamu memiliki 3 kartu di tangan.\n";

    } catch (const GameException& e) {
        std::cout << AnsiTheme::apply(AnsiTheme::error(), "[ERROR] ")
                  << e.what() << "\n";
    } catch (...) {}
}

// ── Status bar ────────────────────────────────────────────────────────────────
void GameUI::printStatusBar() const {
    if (engine.isGameOver() || engine.getPlayers().empty()) return;
    try {
        const Player& cur = engine.getCurrentPlayer();
        std::ostringstream bar;
        bar << "[ Giliran: " << cur.getUsername()
            << " | Uang: M" << cur.getMoney()
            << " | Turn: " << engine.getCurrentTurn()
            << "/" << engine.getMaxTurn();

        if (cur.isJailed()) {
            bar << " | DIPENJARA (percobaan "
                << (cur.getJailTurns() + 1) << "/4)"
                << " | Ketik BAYAR_DENDA atau LEMPAR_DADU";
        }

        if (cur.countCards() > 0) {
            bar << " | Kartu: ";
            const auto& cards = cur.getHandCards();
            for (size_t i = 0; i < cards.size(); ++i) {
                if (i > 0) bar << ", ";
                bar << (i + 1) << "." << cards[i]->getTypeName();
                if (cards[i]->getValue() > 0)
                    bar << "(" << cards[i]->getValue() << ")";
            }
        }
        bar << " ]";
        std::cout << AnsiTheme::apply(AnsiTheme::dim(), bar.str()) << "\n";
    } catch (...) {}
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void GameUI::run() {
    displayWelcome();
    bootstrapIfNeeded();

    std::string raw;
    while (true) {
        if (engine.isGameOver()) {
            // Flush event game over yang mungkin ada di buffer
            CommandResult dummy;
            engine.flushEvents(dummy);
            if (!dummy.events.empty())
                std::cout << formatter.format(dummy);
            std::cout << AnsiTheme::apply(AnsiTheme::bold(),
                "\nPermainan selesai. Ketik KELUAR untuk menutup.\n");
        }

        // Handle drop kartu ke-4 jika ada
        handlePendingSkillDrop();

        // Status bar
        printStatusBar();

        std::cout << AnsiTheme::apply(AnsiTheme::bold(), "> ");
        if (!std::getline(std::cin, raw)) break;
        if (raw.empty()) continue;

        try {
            Command cmd = parser.parse(raw);

            if (cmd.type == CommandType::EXIT) {
                std::cout << "Sampai jumpa!\n";
                break;
            }

            // Eksekusi command dengan resolve prompt otomatis
            CommandResult result = executeWithPrompts(cmd);
            std::cout << formatter.format(result) << "\n";

            // Render papan jika CETAK_PAPAN
            if (cmd.type == CommandType::PRINT_BOARD) {
                std::cout << renderer.render(
                    engine.getBoard(),
                    engine.getPlayers(),
                    engine.getCurrentTurn(),
                    engine.getMaxTurn()) << "\n";
            }

            // Flush event sisa (dari endGame dll)
            CommandResult leftover;
            engine.flushEvents(leftover);
            if (!leftover.events.empty())
                std::cout << formatter.format(leftover) << "\n";

        } catch (const GameException& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[ERROR] ")
                      << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cout << AnsiTheme::apply(AnsiTheme::error(), "[FATAL] ")
                      << e.what() << "\n";
        }
    }
}
