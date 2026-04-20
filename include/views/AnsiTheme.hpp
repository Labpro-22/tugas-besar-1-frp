#pragma once

#include <string>

class AnsiTheme {
public:
    static bool isColorEnabled();

    static std::string reset();
    static std::string bold();
    static std::string dim();

    static std::string info();
    static std::string success();
    static std::string warning();
    static std::string error();

    static std::string tileAction();
    static std::string tileUtility();

    // Monopoly color groups
    static std::string tileBrown();      // [CK] Coklat
    static std::string tileLightBlue();  // [BM] Biru Muda
    static std::string tilePink();       // [PK] Pink
    static std::string tileOrange();     // [OR] Orange
    static std::string tileRed();        // [MR] Merah
    static std::string tileYellow();     // [KN] Kuning
    static std::string tileGreen();      // [HJ] Hijau
    static std::string tileDarkBlue();   // [BT] Biru Tua

    static std::string apply(const std::string& color, const std::string& text);
    static std::string clearScreen();
};
