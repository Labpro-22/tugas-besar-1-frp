#ifndef VIEWSGUI_ACTION_PANEL_HPP
#define VIEWSGUI_ACTION_PANEL_HPP

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <vector>

#include "Button.hpp"

namespace viewsGUI {
class ActionPanel {
public:
    ActionPanel(sf::Vector2f position, sf::Vector2f size, const sf::Font& font);

    void update(sf::Vector2f mousePos);
    void handleMouseClick(sf::Vector2f mousePos);
    void render(sf::RenderWindow& window) const;

    void updatePlayerInfo(const std::string& playerName, int money, int turn, int maxTurn);
    void setSystemMessage(const std::string& message);

    std::shared_ptr<Button> getRollDiceBtn() const { return m_btnRollDice; }
    std::shared_ptr<Button> getBuildBtn() const { return m_btnBuild; }
    std::shared_ptr<Button> getMortgageBtn() const { return m_btnMortgage; }
    std::shared_ptr<Button> getUseCardBtn() const { return m_btnUseCard; }
    std::shared_ptr<Button> getSaveBtn() const { return m_btnSave; }
    std::shared_ptr<Button> getLoadBtn() const { return m_btnLoad; }

private:
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Font m_font;

    sf::RectangleShape m_background;
    sf::RectangleShape m_statusBox;
    sf::RectangleShape m_logBox;

    sf::Text m_titleText;
    sf::Text m_playerInfoText;
    sf::Text m_turnInfoText;
    sf::Text m_logText;

    std::vector<std::shared_ptr<Button>> m_buttons;
    std::shared_ptr<Button> m_btnRollDice;
    std::shared_ptr<Button> m_btnBuild;
    std::shared_ptr<Button> m_btnMortgage;
    std::shared_ptr<Button> m_btnUseCard;
    std::shared_ptr<Button> m_btnSave;
    std::shared_ptr<Button> m_btnLoad;

    void initLayout();
};
} // namespace viewsGUI

#endif
