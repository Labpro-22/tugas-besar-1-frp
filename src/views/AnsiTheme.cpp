#include "../../include/views/AnsiTheme.hpp"

#include <cstdlib>


namespace {
const char* kReset = "\033[0m";
const char* kBold = "\033[1m";
const char* kDim = "\033[2m";
const char* kInfo = "\033[36m";
const char* kSuccess = "\033[32m";
const char* kWarning = "\033[33m";
const char* kError = "\033[31m";
const char* kTileAction = "\033[95m";
const char* kTileUtility = "\033[94m";
// Monopoly color groups
const char* kTileBrown     = "\033[38;5;94m";   // Coklat
const char* kTileLightBlue = "\033[38;5;117m";  // Biru Muda
const char* kTilePink      = "\033[38;5;212m";  // Pink
const char* kTileOrange    = "\033[38;5;208m";  // Orange
const char* kTileRed       = "\033[38;5;196m";  // Merah
const char* kTileYellow    = "\033[38;5;226m";  // Kuning
const char* kTileGreen     = "\033[38;5;40m";   // Hijau
const char* kTileDarkBlue  = "\033[38;5;21m";   // Biru Tua
} // namespace

bool AnsiTheme::isColorEnabled() {
    const char* noColor = std::getenv("NO_COLOR");
    return noColor == nullptr;
}

std::string AnsiTheme::reset() { return isColorEnabled() ? kReset : ""; }
std::string AnsiTheme::bold() { return isColorEnabled() ? kBold : ""; }
std::string AnsiTheme::dim() { return isColorEnabled() ? kDim : ""; }

std::string AnsiTheme::info() { return isColorEnabled() ? kInfo : ""; }
std::string AnsiTheme::success() { return isColorEnabled() ? kSuccess : ""; }
std::string AnsiTheme::warning() { return isColorEnabled() ? kWarning : ""; }
std::string AnsiTheme::error() { return isColorEnabled() ? kError : ""; }

std::string AnsiTheme::tileAction() { return isColorEnabled() ? kTileAction : ""; }
std::string AnsiTheme::tileUtility() { return isColorEnabled() ? kTileUtility : ""; }

std::string AnsiTheme::tileBrown()     { return isColorEnabled() ? kTileBrown : ""; }
std::string AnsiTheme::tileLightBlue() { return isColorEnabled() ? kTileLightBlue : ""; }
std::string AnsiTheme::tilePink()      { return isColorEnabled() ? kTilePink : ""; }
std::string AnsiTheme::tileOrange()    { return isColorEnabled() ? kTileOrange : ""; }
std::string AnsiTheme::tileRed()       { return isColorEnabled() ? kTileRed : ""; }
std::string AnsiTheme::tileYellow()    { return isColorEnabled() ? kTileYellow : ""; }
std::string AnsiTheme::tileGreen()     { return isColorEnabled() ? kTileGreen : ""; }
std::string AnsiTheme::tileDarkBlue()  { return isColorEnabled() ? kTileDarkBlue : ""; }

std::string AnsiTheme::apply(const std::string& color, const std::string& text) {
    if (!isColorEnabled()) {
        return text;
    }
    return color + text + reset();
}

std::string AnsiTheme::clearScreen() {
    if (!isColorEnabled()) {
        return std::string(60, '\n');
    }
    return "\033[2J\033[H";
}
