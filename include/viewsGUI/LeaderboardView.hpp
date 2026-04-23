#ifndef VIEWSGUI_LEADERBOARD_VIEW_HPP
#define VIEWSGUI_LEADERBOARD_VIEW_HPP

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

class Player;

namespace viewsGUI {
class LeaderboardView {
public:
    LeaderboardView(const sf::Font& headerFont, const sf::Font& bodyFont);

    bool loadAssets(const std::string& panelPath);
    void setPosition(sf::Vector2f position);

    void updateFromPlayers(const std::vector<Player*>& players);

    void render(sf::RenderWindow& window) const;

private:
    struct Row {
        std::string player;
        int cash;
        int assets;
        int total;
    };

    sf::Font m_headerFont;
    sf::Font m_bodyFont;

    sf::Texture m_panelTexture;
    sf::Sprite m_panelSprite;

    std::vector<Row> m_rows;

    sf::Vector2f m_position;

    void drawRowText(sf::RenderWindow& window, const Row& row, int rowIndex) const;
};
} // namespace viewsGUI

#endif
