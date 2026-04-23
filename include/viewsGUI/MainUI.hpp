#ifndef VIEWSGUI_MAIN_UI_HPP
#define VIEWSGUI_MAIN_UI_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <vector>

#include "AssetPanel.hpp"
#include "LeaderboardView.hpp"
#include "SpriteButton.hpp"

class Player;

namespace viewsGUI {
class MainUI {
public:
    MainUI(const sf::Font& titleFont, const sf::Font& bodyFont);

    bool loadAssets(const std::string& uiDir, const std::string& boardDir);

    void setOnRollDice(std::function<void()> callback);

    void updateData(const std::vector<Player*>& players,
                    const Player& currentPlayer,
                    const std::string& systemLog,
                    int currentTurn,
                    int maxTurn);

    void setRollEnabled(bool enabled);
    void setRollVisible(bool visible);

    void setPanelMode(AssetPanel::Mode mode);
    AssetPanel::Mode getPanelMode() const;

    void update(sf::Vector2f mousePos);
    bool handleMouseWheel(float delta, sf::Vector2f mousePos);
    bool handleMousePressed(sf::Vector2f mousePos);
    bool handleMouseReleased(sf::Vector2f mousePos);
    bool isRollDiceButtonHit(sf::Vector2f mousePos) const;

    void renderBackground(sf::RenderWindow& window) const;
    void renderOverlay(sf::RenderWindow& window) const;

private:
    sf::Font m_titleFont;
    sf::Font m_bodyFont;

    sf::Texture m_backgroundTexture;

    sf::Sprite m_backgroundSprite;

    SpriteButton m_rollDiceButton;
    sf::Text m_turnStatusText;
    SpriteButton m_tabAsset;
    SpriteButton m_tabLog;
    SpriteButton m_tabInventory;

    LeaderboardView m_leaderboardView;
    AssetPanel m_assetPanel;

    bool m_rollVisible;

    void syncTabActiveState();
};
} // namespace viewsGUI

#endif
