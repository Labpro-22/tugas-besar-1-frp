#ifndef VIEWSGUI_LEADERBOARD_VIEW_HPP
#define VIEWSGUI_LEADERBOARD_VIEW_HPP

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "../core/Leaderboard.hpp"

namespace viewsGUI {
class LeaderboardView {
public:
    LeaderboardView(const sf::Font& headerFont, const sf::Font& bodyFont);

    bool loadAssets(const std::string& panelPath);
    void setPosition(sf::Vector2f position);

    void updateFromLeaderboard(const std::vector<Leaderboard>& rows);

    void render(sf::RenderWindow& window) const;

private:
    struct Row {
        int rank;
        std::string player;
        int cash;
        int asset;
        int propertyCount;
        int tokenIndex;
    };

    sf::Font m_headerFont;
    sf::Font m_bodyFont;

    sf::Texture m_panelTexture;
    sf::Sprite m_panelSprite;
    std::vector<sf::Texture> m_tokenTextures;

    std::vector<Row> m_rows;

    sf::Vector2f m_position;

    void drawHeaderText(sf::RenderWindow& window) const;
    void drawRowText(sf::RenderWindow& window, const Row& row, int rowIndex) const;
};
} // namespace viewsGUI

#endif
