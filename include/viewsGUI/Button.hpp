#ifndef VIEWSGUI_BUTTON_HPP
#define VIEWSGUI_BUTTON_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

namespace viewsGUI {
class Button {
public:
    Button(sf::Vector2f position, sf::Vector2f size, const std::string& text, const sf::Font& font);

    void render(sf::RenderWindow& window) const;
    void update(sf::Vector2f mousePos);
    bool handleMouseClick(sf::Vector2f mousePos);

    void setOnClick(std::function<void()> callback);
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_isEnabled; }

private:
    sf::RectangleShape m_shape;
    sf::Text m_text;
    bool m_isHovered;
    bool m_isEnabled;
    std::function<void()> m_onClick;

    sf::Color m_normalColor;
    sf::Color m_hoverColor;
    sf::Color m_disabledColor;
};
} // namespace viewsGUI

#endif
