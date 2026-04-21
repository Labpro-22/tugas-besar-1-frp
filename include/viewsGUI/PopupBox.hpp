#ifndef VIEWSGUI_POPUP_BOX_HPP
#define VIEWSGUI_POPUP_BOX_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Button.hpp"

namespace viewsGUI {
class PopupBox {
public:
    PopupBox(sf::Vector2f windowSize, const sf::Font& font);

    void showOptions(const std::string& title,
                     const std::string& message,
                     const std::vector<std::string>& options,
                     std::function<void(int)> onSelect);
    void hide();

    bool isVisible() const { return m_isVisible; }

    void update(sf::Vector2f mousePos);
    bool handleMouseClick(sf::Vector2f mousePos);
    void render(sf::RenderWindow& window) const;

private:
    bool m_isVisible;
    sf::Font m_font;

    sf::RectangleShape m_overlay;
    sf::RectangleShape m_box;
    sf::Text m_titleText;
    sf::Text m_messageText;

    std::vector<std::shared_ptr<Button>> m_optionButtons;
    std::function<void(int)> m_onSelect;

    void centerTextX(sf::Text& text, float yPos);
};
} // namespace viewsGUI

#endif
