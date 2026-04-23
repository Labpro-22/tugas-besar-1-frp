#include "../../include/viewsGUI/Button.hpp"

#include "../../include/viewsGUI/Theme.hpp"

namespace viewsGUI {
Button::Button(sf::Vector2f position, sf::Vector2f size, const std::string& textStr,
               const sf::Font& font)
    : m_isHovered(false), m_isEnabled(true), m_onClick(nullptr) {
    m_shape.setPosition(position);
    m_shape.setSize(size);
    m_shape.setOutlineThickness(2.0f);
    m_shape.setOutlineColor(Theme::BiruTua);

    m_normalColor = sf::Color(250, 250, 250);
    m_hoverColor = sf::Color(220, 230, 240);
    m_disabledColor = sf::Color(200, 200, 200);
    m_shape.setFillColor(m_normalColor);

    m_text.setFont(font);
    m_text.setString(textStr);
    m_text.setCharacterSize(16);
    m_text.setFillColor(Theme::TextDark);

    const sf::FloatRect textRect = m_text.getLocalBounds();
    m_text.setOrigin(textRect.left + textRect.width / 2.0f,
                     textRect.top + textRect.height / 2.0f);
    m_text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::setOnClick(std::function<void()> callback) { m_onClick = std::move(callback); }

void Button::setEnabled(bool enabled) {
    m_isEnabled = enabled;
    if (!m_isEnabled) {
        m_shape.setFillColor(m_disabledColor);
        m_text.setFillColor(sf::Color(130, 130, 130));
        return;
    }

    m_shape.setFillColor(m_normalColor);
    m_text.setFillColor(Theme::TextDark);
}

void Button::update(sf::Vector2f mousePos) {
    if (!m_isEnabled) {
        m_isHovered = false;
        return;
    }

    m_isHovered = m_shape.getGlobalBounds().contains(mousePos);
    m_shape.setFillColor(m_isHovered ? m_hoverColor : m_normalColor);
}

bool Button::handleMouseClick(sf::Vector2f mousePos) {
    if (!m_isEnabled || !m_shape.getGlobalBounds().contains(mousePos) || !m_onClick) {
        return false;
    }

    m_onClick();
    return true;
}

void Button::render(sf::RenderWindow& window) const {
    window.draw(m_shape);
    window.draw(m_text);
}
} // namespace viewsGUI
