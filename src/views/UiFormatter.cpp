#include "../../include/views/UiFormatter.hpp"

#include "../../include/views/AnsiTheme.hpp"

#include <sstream>

std::string UiFormatter::formatTone(UiTone tone, const std::string& text) const {
    switch (tone) {
    case UiTone::SUCCESS:
        return AnsiTheme::apply(AnsiTheme::success(), text);
    case UiTone::WARNING:
        return AnsiTheme::apply(AnsiTheme::warning(), text);
    case UiTone::ERROR:
        return AnsiTheme::apply(AnsiTheme::error(), text);
    case UiTone::INFO:
    default:
        return AnsiTheme::apply(AnsiTheme::info(), text);
    }
}

std::string UiFormatter::format(const CommandResult& result) const {
    std::ostringstream out;

    out << AnsiTheme::apply(AnsiTheme::bold(), "=== " + result.commandName + " ===") << "\n";

    for (const GameEvent& event : result.events) {
        const std::string head = "[" + event.title + "] ";
        out << formatTone(event.tone, head + event.message) << "\n";
    }

    if (result.prompt.has_value()) {
        const PromptRequest& prompt = result.prompt.value();
        out << formatTone(UiTone::WARNING, "[INPUT] " + prompt.message) << "\n";
        if (!prompt.options.empty()) {
            out << AnsiTheme::apply(AnsiTheme::dim(), "Pilihan:");
            for (size_t i = 0; i < prompt.options.size(); ++i) {
                out << " " << prompt.options[i];
                if (i + 1 < prompt.options.size()) {
                    out << ",";
                }
            }
            out << "\n";
        }
    }

    return out.str();
}
