#ifndef VIEWSGUI_SPRITE_BUTTON_HPP
#define VIEWSGUI_SPRITE_BUTTON_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>

namespace viewsGUI {
class SpriteButton {
public:
    SpriteButton();

    bool loadTextures(const std::string& normalPath,
                      const std::string& hoverPath,
                      const std::string& pressedPath,
                      const std::string& disabledPath);

    void setPosition(sf::Vector2f position);
    void setScale(sf::Vector2f scale);
    void setVisible(bool visible);
    void setEnabled(bool enabled);

    void setOnClick(std::function<void()> callback);

    void update(sf::Vector2f mousePos);
    bool handleMousePressed(sf::Vector2f mousePos);
    bool handleMouseReleased(sf::Vector2f mousePos);

    void render(sf::RenderWindow& window) const;

    bool isVisible() const { return m_visible; }
    bool isEnabled() const { return m_enabled; }
    bool contains(sf::Vector2f point) const;

private:
    sf::Texture m_normalTexture;
    sf::Texture m_hoverTexture;
    sf::Texture m_pressedTexture;
    sf::Texture m_disabledTexture;

    sf::Sprite m_sprite;

    bool m_visible;
    bool m_enabled;
    bool m_hovered;
    bool m_pressed;

    std::function<void()> m_onClick;

    void applyVisualState();
};
} // namespace viewsGUI

#endif
