#ifndef VIEWSGUI_THEME_HPP
#define VIEWSGUI_THEME_HPP

#include <SFML/Graphics.hpp>

namespace viewsGUI {
struct Theme {
    static inline const sf::Color Merah = sf::Color(173, 74, 74);
    static inline const sf::Color Pink = sf::Color(218, 143, 166);
    static inline const sf::Color Oranye = sf::Color(217, 119, 80);
    static inline const sf::Color BiruMuda = sf::Color(149, 158, 160);
    static inline const sf::Color TileAksi = sf::Color(243, 224, 207);
    static inline const sf::Color Coklat = sf::Color(123, 59, 13);
    static inline const sf::Color Kuning = sf::Color(229, 187, 105);
    static inline const sf::Color BiruTua = sf::Color(47, 79, 111);
    static inline const sf::Color Hijau = sf::Color(95, 143, 107);
    static inline const sf::Color Utilitas = sf::Color(183, 183, 183);

    static inline const sf::Color Background = sf::Color(230, 240, 235);
    static inline const sf::Color Border = sf::Color::Black;
    static inline const sf::Color TextDark = sf::Color(30, 30, 30);
    static inline const sf::Color Panel = sf::Color(40, 45, 50);
    static inline const sf::Color PanelSection = sf::Color(60, 65, 70);
};
} // namespace viewsGUI

#endif
