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
        return text;
    }
}

std::string UiFormatter::format(const CommandResult& result) const {
    std::ostringstream out;

    for (const GameEvent& event : result.events) {
        out << formatTone(event.tone, event.message) << "\n";
    }

    return out.str();
}
