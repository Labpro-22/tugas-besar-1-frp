#ifndef VIEWSGUI_DEBUG_DICE_POPUP_HPP
#define VIEWSGUI_DEBUG_DICE_POPUP_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <vector>

namespace viewsGUI {
class DebugDicePopup {
public:
    using ExecuteCallback = std::function<void(int die1, int die2)>;
    using CancelCallback = std::function<void()>;

    DebugDicePopup(sf::Vector2f windowSize, const sf::Font& font);

    void show(ExecuteCallback onExecute, CancelCallback onCancel, int initialDie1 = 1, int initialDie2 = 1);
    void hide();
    bool isVisible() const { return m_visible; }

    void update(sf::Vector2f mousePos);
    bool handleMousePressed(sf::Vector2f mousePos, sf::Mouse::Button button);
    bool handleMouseReleased(sf::Vector2f mousePos, sf::Mouse::Button button);
    void render(sf::RenderWindow& window) const;

private:
    struct NumberButton {
        sf::RectangleShape shape;
        sf::Text label;
        int value = 1;
        bool hovered = false;
        bool pressed = false;
    };

    bool m_visible;
    sf::Vector2f m_windowSize;
    sf::Font m_font;
    ExecuteCallback m_onExecute;
    CancelCallback m_onCancel;
    int m_selectedDie1;
    int m_selectedDie2;
    int m_pressedToken;
    bool m_executeHovered;
    bool m_cancelHovered;

    sf::RectangleShape m_backdrop;
    sf::RectangleShape m_panel;
    sf::Text m_titleText;
    sf::Text m_die1LabelText;
    sf::Text m_die2LabelText;
    sf::RectangleShape m_executeButton;
    sf::RectangleShape m_cancelButton;
    sf::Text m_executeText;
    sf::Text m_cancelText;
    std::vector<NumberButton> m_die1Buttons;
    std::vector<NumberButton> m_die2Buttons;

    void layout();
    void updateHover(sf::Vector2f mousePos);
    void updateVisuals();
    int hitToken(sf::Vector2f mousePos) const;

    static int clampDieValue(int value);
    static void centerText(sf::Text& text, const sf::FloatRect& bounds);
};
} // namespace viewsGUI

#endif
