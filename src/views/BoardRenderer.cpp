#include "../../include/views/BoardRenderer.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/views/AnsiTheme.hpp"

#include <array>
#include <sstream>
#include <vector>

namespace {
    std::array<std::array<int, 11>, 11> buildPerimeterIndex() {
        std::array<std::array<int, 11>, 11> map{};
        for (int r = 0; r < 11; ++r) {
            for (int c = 0; c < 11; ++c) {
                map[r][c] = -1;
            }
        }

        // Index mengikuti layout Monopoly standar (11x11, 40 petak total)
        // Bawah: Kanan ke Kiri
        int idx = 0;
        for (int c = 10; c >= 0; --c) map[10][c] = idx++;
        // Kiri: Bawah ke Atas
        for (int r = 9; r >= 1; --r) map[r][0] = idx++;
        // Atas: Kiri ke Kanan
        for (int c = 0; c < 11; ++c) map[0][c] = idx++;
        // Kanan: Atas ke Bawah
        for (int r = 1; r < 10; ++r) map[r][10] = idx++;

        return map;
    }

    const std::array<std::array<int, 11>, 11> kPerimeter = buildPerimeterIndex();

    // Helper untuk menghitung panjang tampilan (Display Width) karakter UTF-8
    // Mengamankan layout tabel agar tidak rusak gara-gara logo ⌂ (3 bytes) dan 🏟 (4 bytes)
    int getDisplayLength(const std::string& str) {
        int len = 0;
        for (size_t i = 0; i < str.length(); ++i) {
            unsigned char c = str[i];
            if (c >= 0xF0) { i += 3; len++; }      // 4-byte char
            else if (c >= 0xE0) { i += 2; len++; } // 3-byte char
            else if (c >= 0xC0) { i += 1; len++; } // 2-byte char
            else len++;                            // 1-byte char
        }
        return len;
    }
} // namespace

std::string BoardRenderer::fitCell(const std::string& text) const {
    int len = getDisplayLength(text);
    if (len >= cellWidth) {
        return text; 
    }
    return text + std::string(cellWidth - len, ' ');
}

std::string BoardRenderer::buildCellLabel(const Board& board, int boardIndex) const {
    const Tile& tile = board.getTileByIndex(boardIndex);
    std::string marker = "DF";
    std::string color;

    const PropertyTile* pt = dynamic_cast<const PropertyTile*>(&tile);
    if (pt) {
        const Property& prop = pt->getProperty();
        switch (prop.getType()) {
            case PropertyType::STREET: {
                const StreetProperty* sp = dynamic_cast<const StreetProperty*>(&prop);
                if (sp) {
                    std::string cg = sp->getColorGroup();
                    marker = cg;
                    if (cg == "CK")      color = AnsiTheme::tileBrown();
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
                marker = "AB";
                color = AnsiTheme::tileUtility();
                break;
            case PropertyType::RAILROAD:
                marker = "DF";
                color = AnsiTheme::tileAction();
                break;
        }
    } else {
        color = AnsiTheme::tileAction();
    }

    const std::string cellText = fitCell("[" + marker + "] " + tile.getCode());
    if (!color.empty()) {
        return AnsiTheme::apply(color, cellText);
    }
    return cellText;
}

std::string BoardRenderer::buildCellStatus(const Board& board, const std::vector<Player*>& players, int boardIndex) const {
    std::string leftStr = "";
    std::string rightStr = "";

    // 1. Ambil data kepemilikan
    const Tile& tile = board.getTileByIndex(boardIndex);
    const PropertyTile* pt = dynamic_cast<const PropertyTile*>(&tile);
    if (pt) {
        const Property& prop = pt->getProperty();
        if (prop.getOwner() != nullptr) {
            int ownerIdx = -1;
            for (size_t i = 0; i < players.size(); ++i) {
                if (players[i] == prop.getOwner()) ownerIdx = i + 1;
            }
            if (ownerIdx != -1) {
                leftStr = "P" + std::to_string(ownerIdx);
                const StreetProperty* sp = dynamic_cast<const StreetProperty*>(&prop);
                if (sp) {
                    const BuildingLevel lvl = sp->getBuildingLevel();
                    if (lvl == BuildingLevel::HOTEL) {
                        leftStr += " \xF0\x9F\x8F\x9F"; // 🏟 (UTF-8)
                    } else {
                        const int houses = static_cast<int>(lvl);
                        if (houses > 0) {
                            leftStr += " ";
                            for (int h = 0; h < houses; ++h) leftStr += "\xE2\x8C\x82"; // ⌂ (UTF-8)
                        }
                    }
                }
            }
        }
    }

    // Penanganan khusus penjara
    if (tile.getCode() == "PEN") {
        std::vector<std::string> inmates;
        std::vector<std::string> visitors;

        for (size_t i = 0; i < players.size(); ++i) {
            const Player* p = players[i];
            if (!p || p->isBankrupt() || p->getPosition() != 10) {
                continue;
            }

            if (p->isJailed()) {
                inmates.push_back(std::to_string(i + 1));
            } else {
                visitors.push_back(std::to_string(i + 1));
            }
        }

        auto join = [](const std::vector<std::string>& nums) {
            std::string s;
            for (size_t i = 0; i < nums.size(); ++i) {
                if (i > 0) s += ",";
                s += nums[i];
            }
            return s;
        };

        const std::string inList = join(inmates);
        const std::string vList = join(visitors);
        if (inList.empty() && vList.empty()) {
            return std::string(cellWidth, ' ');
        }

        // Coba format paling informatif dulu, lalu fallback yang lebih ringkas.
        std::vector<std::string> candidates;
        if (!inList.empty() && !vList.empty()) {
            candidates.push_back("IN:" + inList + " V:" + vList);
            candidates.push_back("IN:" + inList + "V:" + vList);
            candidates.push_back("I:" + inList + " V:" + vList);
            candidates.push_back("I:" + inList + "V:" + vList);
        }
        if (!inList.empty()) candidates.push_back("IN:" + inList);
        if (!vList.empty()) candidates.push_back("V:" + vList);

        for (const std::string& candidate : candidates) {
            if (getDisplayLength(candidate) <= cellWidth) {
                return fitCell(candidate);
            }
        }

        if (!inList.empty()) return fitCell("I:" + inList);
        return fitCell("V:" + vList);
    }

    // 2. Ambil token pemain
    for (size_t i = 0; i < players.size(); ++i) {
        const Player* p = players[i];
        if (p && !p->isBankrupt() && p->getPosition() == boardIndex) {
            rightStr += "(" + std::to_string(i + 1) + ")";
        }
    }

    // 3. Gabungkan posisi (Owner kiri, Bidak Kanan)
    int leftLen = getDisplayLength(leftStr);
    int rightLen = getDisplayLength(rightStr);

    if (leftLen == 0 && rightLen == 0) return std::string(cellWidth, ' ');

    if (leftLen + rightLen < cellWidth) {
        if (rightLen > 0 && leftLen > 0) {
            return leftStr + std::string(cellWidth - leftLen - rightLen, ' ') + rightStr;
        } else if (leftLen > 0) {
            return leftStr + std::string(cellWidth - leftLen, ' ');
        } else {
            // Jika cuma ada bidak tapi gak ada yang punya, di center sesuai Board.txt: `   (1)    `
            int pad = (cellWidth - rightLen) / 2;
            return std::string(pad, ' ') + rightStr + std::string(cellWidth - pad - rightLen, ' ');
        }
    }

    return leftStr + rightStr; 
}

std::string BoardRenderer::render(const Board& board, const std::vector<Player*>& players, int turn, int maxTurn) const {
    std::ostringstream out;

    if (board.size() < 40) {
        return AnsiTheme::apply(AnsiTheme::warning(), "Papan belum lengkap (minimal 40 petak).\n");
    }

    const std::string fullBorder = "+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+----------+";
    
    // --- MEMBANGUN LAYOUT TENGAH (Lebar persis 98 karakter) ---
    std::vector<std::string> center(27, std::string(98, ' '));
    center[3]  = std::string(36, ' ') + "==================================" + std::string(28, ' ');
    center[4]  = std::string(36, ' ') + "||          NIMONSPOLI          ||" + std::string(28, ' ');
    center[5]  = std::string(36, ' ') + "==================================" + std::string(28, ' ');

    std::string turnStr = "TURN " + std::to_string(turn) + " / " + std::to_string(maxTurn);
    int turnPad = (98 - turnStr.length()) / 2;
    center[7]  = std::string(turnPad, ' ') + turnStr + std::string(98 - turnPad - turnStr.length(), ' ');

    center[10] = std::string(36, ' ') + "----------------------------------" + std::string(28, ' ');
    center[11] = std::string(36, ' ') + "LEGENDA KEPEMILIKAN & STATUS      " + std::string(28, ' ');
    center[12] = std::string(36, ' ') + "P1-P4 : Properti milik Pemain 1-4 " + std::string(28, ' ');
    center[13] = std::string(36, ' ') + "\xE2\x8C\x82     : Rumah Level 1             " + std::string(28, ' ');
    center[14] = std::string(36, ' ') + "\xE2\x8C\x82\xE2\x8C\x82    : Rumah Level 2             " + std::string(28, ' ');
    center[15] = std::string(36, ' ') + "\xE2\x8C\x82\xE2\x8C\x82\xE2\x8C\x82   : Rumah Level 3             " + std::string(28, ' ');
    center[16] = std::string(36, ' ') + "\xF0\x9F\x8F\x9F    : Hotel (Maksimal)          " + std::string(28, ' ');
    center[17] = std::string(36, ' ') + "(1)-(4): Bidak (IN=Tahanan, V=Mampir)" + std::string(25, ' ');
    center[18] = std::string(36, ' ') + "----------------------------------" + std::string(28, ' ');
    center[19] = std::string(36, ' ') + "KODE WARNA:                       " + std::string(28, ' ');
    center[20] = std::string(36, ' ') + "[CK]=Coklat    [MR]=Merah         " + std::string(28, ' ');
    center[21] = std::string(36, ' ') + "[BM]=Biru Muda [KN]=Kuning        " + std::string(28, ' ');
    center[22] = std::string(36, ' ') + "[PK]=Pink      [HJ]=Hijau         " + std::string(28, ' ');
    center[23] = std::string(36, ' ') + "[OR]=Orange    [BT]=Biru Tua      " + std::string(28, ' ');
    center[24] = std::string(36, ' ') + "[DF]=Aksi      [AB]=Utilitas      " + std::string(28, ' ');

    // 1. Tulis Baris Atas (Row 0)
    out << fullBorder << "\n|";
    for (int c = 0; c < sideLength; ++c) out << buildCellLabel(board, kPerimeter[0][c]) << "|";
    out << "\n|";
    for (int c = 0; c < sideLength; ++c) out << buildCellStatus(board, players, kPerimeter[0][c]) << "|";
    out << "\n" << fullBorder << "\n";

    // 2. Tulis Baris Tengah (Row 1 to 9)
    for (int r = 1; r < 10; ++r) {
        int leftIdx = kPerimeter[r][0];
        int rightIdx = kPerimeter[r][10];

        // Label Kiri - Center Text - Label Kanan
        out << "|" << buildCellLabel(board, leftIdx) << "|"
            << center[(r - 1) * 3 + 0]
            << "|" << buildCellLabel(board, rightIdx) << "|\n";

        // Status Kiri - Center Text - Status Kanan
        out << "|" << buildCellStatus(board, players, leftIdx) << "|"
            << center[(r - 1) * 3 + 1]
            << "|" << buildCellStatus(board, players, rightIdx) << "|\n";

        // Border Bawah Kiri - Center Text - Border Bawah Kanan
        out << "+----------+" << center[(r - 1) * 3 + 2] << "+----------+\n";
    }

    // 3. Tulis Baris Bawah (Row 10)
    out << "|";
    for (int c = 0; c < sideLength; ++c) out << buildCellLabel(board, kPerimeter[10][c]) << "|";
    out << "\n|";
    for (int c = 0; c < sideLength; ++c) out << buildCellStatus(board, players, kPerimeter[10][c]) << "|";
    out << "\n" << fullBorder << "\n";

    return out.str();
}
