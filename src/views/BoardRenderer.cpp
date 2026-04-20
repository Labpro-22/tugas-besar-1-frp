#include "../../include/views/BoardRenderer.hpp"

#include "../../include/models/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/views/AnsiTheme.hpp"

#include <array>
#include <iomanip>
#include <sstream>

namespace {
std::array<std::array<int, 11>, 11> buildPerimeterIndex() {
    std::array<std::array<int, 11>, 11> map{};
    for (int r = 0; r < 11; ++r) {
        for (int c = 0; c < 11; ++c) {
            map[r][c] = -1;
        }
    }

    int idx = 0;
    for (int c = 0; c < 11; ++c) map[0][c] = idx++;
    for (int r = 1; r < 10; ++r) map[r][10] = idx++;
    for (int c = 10; c >= 0; --c) map[10][c] = idx++;
    for (int r = 9; r >= 1; --r) map[r][0] = idx++;

    return map;
}

const std::array<std::array<int, 11>, 11> kPerimeter = buildPerimeterIndex();
} // namespace

std::string BoardRenderer::fitCell(const std::string& text) const {
    if (text.size() >= static_cast<size_t>(cellWidth)) {
        return text.substr(0, cellWidth);
    }
    return text + std::string(cellWidth - text.size(), ' ');
}

std::string BoardRenderer::buildCellLabel(const Board& board,
                                          const std::vector<Player*>& players,
                                          int boardIndex) const {
    const Tile& tile = board.getTileByIndex(boardIndex);
    std::ostringstream label;
    label << tile.getCode();

    std::string tokens;
    for (size_t i = 0; i < players.size(); ++i) {
        const Player* p = players[i];
        if (p && !p->isBankrupt() && p->getPosition() == boardIndex) {
            if (!tokens.empty()) {
                tokens += ",";
            }
            tokens += std::to_string(i + 1);
        }
    }
    if (!tokens.empty()) {
        label << "(" << tokens << ")";
    }

    // Warnai label sesuai kategori
    std::string color;
    // Cek PropertyTile
    const PropertyTile* pt = dynamic_cast<const PropertyTile*>(&tile);
    if (pt) {
        const Property& prop = pt->getProperty();
        switch (prop.getType()) {
            case PropertyType::STREET: {
                // Cek colorGroup
                const StreetProperty* sp = dynamic_cast<const StreetProperty*>(&prop);
                if (sp) {
                    std::string cg = sp->getColorGroup();
                    if (cg == "CK") color = AnsiTheme::tileBrown();
                    else if (cg == "BM") color = AnsiTheme::tileLightBlue();
                    else if (cg == "PK") color = AnsiTheme::tilePink();
                    else if (cg == "OR") color = AnsiTheme::tileOrange();
                    else if (cg == "MR") color = AnsiTheme::tileRed();
                    else if (cg == "KN") color = AnsiTheme::tileYellow();
                    else if (cg == "HJ") color = AnsiTheme::tileGreen();
                    else if (cg == "BT") color = AnsiTheme::tileDarkBlue();
                }
                break;
            }
            case PropertyType::UTILITY:
                color = AnsiTheme::tileUtility();
                break;
            case PropertyType::RAILROAD:
                color = AnsiTheme::tileUtility();
                break;
        }
    } else if (tile.getCode().rfind("DF", 0) == 0) {
        // Tile aksi
        color = AnsiTheme::tileAction();
    }
    // Fallback: tidak berwarna jika tidak dikenali
    std::string cellText = fitCell(label.str());
    if (!color.empty()) {
        return AnsiTheme::apply(color, cellText);
    } else {
        return cellText;
    }
}

std::string BoardRenderer::render(const Board& board,
                                  const std::vector<Player*>& players,
                                  int turn,
                                  int maxTurn) const {
    std::ostringstream out;

    out << AnsiTheme::apply(AnsiTheme::bold(), "=== PAPAN NIMONSPOLI (11x11) ===") << "\n";
    out << "TURN " << turn << " / " << maxTurn << "\n";

    if (board.size() < 40) {
        out << AnsiTheme::apply(AnsiTheme::warning(),
                                "Papan belum lengkap (butuh minimal 40 petak untuk render 11x11).")
            << "\n";
        return out.str();
    }

    const std::string border = "+" + std::string(cellWidth, '-') + "+";
    const std::string rowBorder = [&]() {
        std::string b;
        for (int i = 0; i < sideLength; ++i) {
            b += border;
        }
        return b;
    }();

    for (int r = 0; r < sideLength; ++r) {
        out << rowBorder << "\n";
        for (int c = 0; c < sideLength; ++c) {
            out << "|";
            if (r == 0 || r == sideLength - 1 || c == 0 || c == sideLength - 1) {
                const int idx = kPerimeter[r][c];
                out << buildCellLabel(board, players, idx);
            } else if (r == 5 && c >= 3 && c <= 7) {
                static const std::array<std::string, 5> title = {
                    "NIMON", "SPOLI", "CLI  ", "MVP  ", "2026 "
                };
                out << fitCell(title[c - 3]);
            } else {
                out << std::string(cellWidth, ' ');
            }
        }
        out << "|\n";
    }
    out << rowBorder << "\n";

    out << AnsiTheme::apply(AnsiTheme::dim(), "Legenda token: (1..n) = indeks pemain dalam daftar internal") << "\n";

    return out.str();
}
