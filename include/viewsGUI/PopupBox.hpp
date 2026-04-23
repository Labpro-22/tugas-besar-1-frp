#ifndef VIEWSGUI_POPUP_BOX_HPP
#define VIEWSGUI_POPUP_BOX_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "SpriteButton.hpp"

namespace viewsGUI {
class PopupBox {
public:
    PopupBox(sf::Vector2f windowSize, const sf::Font& font);
    bool loadAssets(const std::string& uiDir);

    void showOptions(const std::string& title,
                     const std::string& message,
                     const std::vector<std::string>& options,
                     std::function<void(int)> onSelect);
    void hide();

    bool isVisible() const { return m_isVisible; }

    void update(sf::Vector2f mousePos);
    bool handleMousePressed(sf::Vector2f mousePos);
    bool handleMouseReleased(sf::Vector2f mousePos);
    void render(sf::RenderWindow& window) const;

private:
    bool m_isVisible;
    sf::Font m_font;
    sf::Vector2f m_windowSize;

    sf::Texture m_overlayTexture;
    sf::Texture m_boxTexture;
    sf::Sprite m_overlay;
    sf::Sprite m_box;
    sf::Text m_titleText;
    sf::Text m_messageText;
    std::vector<sf::Text> m_optionTexts;

    std::vector<std::shared_ptr<SpriteButton>> m_optionButtons;
    std::function<void(int)> m_onSelect;

    std::string m_optionNormalPath;
    std::string m_optionHoverPath;
    std::string m_optionPressedPath;
    std::string m_optionDisabledPath;

    void centerTextX(sf::Text& text, float yPos);
};
} // namespace viewsGUI

#endif
