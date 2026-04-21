#ifndef VIEWSGUI_BOARD_RENDERER_HPP
#define VIEWSGUI_BOARD_RENDERER_HPP

#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <vector>

class Board;

namespace viewsGUI {
class BoardRenderer {
public:
    BoardRenderer(float boardSize, const sf::Font& font);

    bool loadAssets(const std::string& assetDirectory);

    void render(sf::RenderWindow& window, const Board& board) const;

    sf::Vector2f getTilePosition(int index) const;
    sf::Vector2f getTileSize(int index) const;
    sf::Vector2f getTileCenter(int index) const;

private:
    float m_boardSize;
    float m_cornerSize;
    float m_tileSize;
    sf::Font m_font;
    sf::Texture m_centerTexture;
    sf::Sprite m_centerSprite;
    std::vector<sf::Texture> m_tileTextures;
    std::vector<sf::Sprite> m_tileSprites;

    sf::Color resolveHeaderColor(int index) const;
    void drawColorHeader(sf::RenderWindow& window, int index, sf::Color headerColor) const;
    void drawDynamicPrice(sf::RenderWindow& window,
                          int index,
                          const std::string& priceText) const;
};
} // namespace viewsGUI

#endif
