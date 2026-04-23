#include "../../include/viewsGUI/DebugDicePopup.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace {
constexpr int kTokenNone = 0;
constexpr int kTokenExecute = 1;
constexpr int kTokenCancel = 2;
constexpr int kTokenDie1Base = 100;
constexpr int kTokenDie2Base = 200;
} // namespace

namespace viewsGUI {
DebugDicePopup::DebugDicePopup(sf::Vector2f windowSize, const sf::Font& font)
    : m_visible(false),
      m_windowSize(windowSize),
      m_font(font),
      m_selectedDie1(1),
      m_selectedDie2(1),
      m_pressedToken(kTokenNone),
      m_executeHovered(false),
      m_cancelHovered(false) {
    m_backdrop.setSize(m_windowSize);
    m_backdrop.setFillColor(sf::Color(0, 0, 0, 140));

    m_panel.setSize(sf::Vector2f(760.0f, 440.0f));
    m_panel.setFillColor(sf::Color(248, 236, 214));
    m_panel.setOutlineThickness(3.0f);
    m_panel.setOutlineColor(sf::Color(92, 70, 46));

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(42);
    m_titleText.setFillColor(sf::Color(53, 45, 36));
    m_titleText.setString("DEBUG: ATUR DADU");

    m_die1LabelText.setFont(m_font);
    m_die1LabelText.setCharacterSize(30);
    m_die1LabelText.setFillColor(sf::Color(53, 45, 36));
    m_die1LabelText.setString("Dadu 1");

    m_die2LabelText.setFont(m_font);
    m_die2LabelText.setCharacterSize(30);
    m_die2LabelText.setFillColor(sf::Color(53, 45, 36));
    m_die2LabelText.setString("Dadu 2");

    m_executeButton.setSize(sf::Vector2f(200.0f, 62.0f));
    m_executeButton.setOutlineThickness(2.0f);
    m_executeButton.setOutlineColor(sf::Color(70, 52, 33));

    m_cancelButton.setSize(sf::Vector2f(200.0f, 62.0f));
    m_cancelButton.setOutlineThickness(2.0f);
    m_cancelButton.setOutlineColor(sf::Color(70, 52, 33));

    m_executeText.setFont(m_font);
    m_executeText.setCharacterSize(30);
    m_executeText.setFillColor(sf::Color(53, 45, 36));
    m_executeText.setString("EKSEKUSI");

    m_cancelText.setFont(m_font);
    m_cancelText.setCharacterSize(30);
    m_cancelText.setFillColor(sf::Color(53, 45, 36));
    m_cancelText.setString("BATAL");

    m_die1Buttons.reserve(6);
    m_die2Buttons.reserve(6);
    for (int value = 1; value <= 6; ++value) {
        NumberButton d1;
        d1.shape.setSize(sf::Vector2f(68.0f, 56.0f));
        d1.shape.setOutlineThickness(2.0f);
        d1.shape.setOutlineColor(sf::Color(70, 52, 33));
        d1.label.setFont(m_font);
        d1.label.setCharacterSize(30);
        d1.label.setFillColor(sf::Color(53, 45, 36));
        d1.label.setString(std::to_string(value));
        d1.value = value;
        m_die1Buttons.push_back(d1);

        NumberButton d2 = d1;
        d2.value = value;
        m_die2Buttons.push_back(d2);
    }

    layout();
    updateVisuals();
}

void DebugDicePopup::show(ExecuteCallback onExecute,
                          CancelCallback onCancel,
                          int initialDie1,
                          int initialDie2) {
    m_onExecute = std::move(onExecute);
    m_onCancel = std::move(onCancel);
    m_selectedDie1 = clampDieValue(initialDie1);
    m_selectedDie2 = clampDieValue(initialDie2);
    m_pressedToken = kTokenNone;
    m_executeHovered = false;
    m_cancelHovered = false;
    m_visible = true;
    updateVisuals();
}

void DebugDicePopup::hide() {
    m_visible = false;
    m_pressedToken = kTokenNone;
    m_executeHovered = false;
    m_cancelHovered = false;
}

void DebugDicePopup::update(sf::Vector2f mousePos) {
    if (!m_visible) {
        return;
    }
    updateHover(mousePos);
    updateVisuals();
}

bool DebugDicePopup::handleMousePressed(sf::Vector2f mousePos, sf::Mouse::Button button) {
    if (!m_visible) {
        return false;
    }

    if (button != sf::Mouse::Left) {
        return true;
    }

    m_pressedToken = hitToken(mousePos);
    updateVisuals();
    return true;
}

bool DebugDicePopup::handleMouseReleased(sf::Vector2f mousePos, sf::Mouse::Button button) {
    if (!m_visible) {
        return false;
    }

    if (button != sf::Mouse::Left) {
        return true;
    }

    const int releasedToken = hitToken(mousePos);
    const int activatedToken = (releasedToken == m_pressedToken) ? releasedToken : kTokenNone;
    m_pressedToken = kTokenNone;

    if (activatedToken >= kTokenDie1Base && activatedToken < kTokenDie1Base + 6) {
        m_selectedDie1 = (activatedToken - kTokenDie1Base) + 1;
    } else if (activatedToken >= kTokenDie2Base && activatedToken < kTokenDie2Base + 6) {
        m_selectedDie2 = (activatedToken - kTokenDie2Base) + 1;
    } else if (activatedToken == kTokenExecute) {
        if (m_onExecute) {
            m_onExecute(m_selectedDie1, m_selectedDie2);
        }
    } else if (activatedToken == kTokenCancel) {
        if (m_onCancel) {
            m_onCancel();
        }
    }

    updateVisuals();
    return true;
}

void DebugDicePopup::render(sf::RenderWindow& window) const {
    if (!m_visible) {
        return;
    }

    window.draw(m_backdrop);
    window.draw(m_panel);
    window.draw(m_titleText);
    window.draw(m_die1LabelText);
    window.draw(m_die2LabelText);

    for (const NumberButton& button : m_die1Buttons) {
        window.draw(button.shape);
        window.draw(button.label);
    }
    for (const NumberButton& button : m_die2Buttons) {
        window.draw(button.shape);
        window.draw(button.label);
    }

    window.draw(m_executeButton);
    window.draw(m_cancelButton);
    window.draw(m_executeText);
    window.draw(m_cancelText);
}

void DebugDicePopup::layout() {
    m_backdrop.setSize(m_windowSize);

    const sf::Vector2f panelSize = m_panel.getSize();
    const float panelLeft = (m_windowSize.x - panelSize.x) * 0.5f;
    const float panelTop = (m_windowSize.y - panelSize.y) * 0.5f;
    m_panel.setPosition(panelLeft, panelTop);

    const sf::FloatRect panelRect(panelLeft, panelTop, panelSize.x, panelSize.y);

    centerText(m_titleText, sf::FloatRect(panelRect.left, panelRect.top + 24.0f, panelRect.width, 54.0f));

    m_die1LabelText.setPosition(panelRect.left + 52.0f, panelRect.top + 126.0f);
    m_die2LabelText.setPosition(panelRect.left + 52.0f, panelRect.top + 232.0f);

    const float buttonStartX = panelRect.left + 222.0f;
    const float dieRow1Y = panelRect.top + 116.0f;
    const float dieRow2Y = panelRect.top + 222.0f;
    const float dieButtonGapX = 12.0f;

    for (size_t i = 0; i < m_die1Buttons.size(); ++i) {
        NumberButton& button = m_die1Buttons[i];
        const float x = buttonStartX + static_cast<float>(i) * (button.shape.getSize().x + dieButtonGapX);
        button.shape.setPosition(x, dieRow1Y);
        centerText(button.label, button.shape.getGlobalBounds());
    }

    for (size_t i = 0; i < m_die2Buttons.size(); ++i) {
        NumberButton& button = m_die2Buttons[i];
        const float x = buttonStartX + static_cast<float>(i) * (button.shape.getSize().x + dieButtonGapX);
        button.shape.setPosition(x, dieRow2Y);
        centerText(button.label, button.shape.getGlobalBounds());
    }

    const float actionY = panelRect.top + panelRect.height - 98.0f;
    m_executeButton.setPosition(panelRect.left + 182.0f, actionY);
    m_cancelButton.setPosition(panelRect.left + panelRect.width - 182.0f - m_cancelButton.getSize().x, actionY);
    centerText(m_executeText, m_executeButton.getGlobalBounds());
    centerText(m_cancelText, m_cancelButton.getGlobalBounds());
}

void DebugDicePopup::updateHover(sf::Vector2f mousePos) {
    for (NumberButton& button : m_die1Buttons) {
        button.hovered = button.shape.getGlobalBounds().contains(mousePos);
    }
    for (NumberButton& button : m_die2Buttons) {
        button.hovered = button.shape.getGlobalBounds().contains(mousePos);
    }
    m_executeHovered = m_executeButton.getGlobalBounds().contains(mousePos);
    m_cancelHovered = m_cancelButton.getGlobalBounds().contains(mousePos);
}

void DebugDicePopup::updateVisuals() {
    const sf::Color baseColor(246, 239, 225);
    const sf::Color hoverColor(236, 224, 196);
    const sf::Color selectedColor(220, 190, 139);
    const sf::Color selectedHoverColor(232, 206, 158);
    const sf::Color pressedColor(189, 150, 95);

    for (size_t i = 0; i < m_die1Buttons.size(); ++i) {
        NumberButton& button = m_die1Buttons[i];
        const int token = kTokenDie1Base + static_cast<int>(i);
        button.pressed = (m_pressedToken == token);
        const bool selected = (m_selectedDie1 == button.value);
        sf::Color fill = selected ? selectedColor : baseColor;
        if (button.hovered) {
            fill = selected ? selectedHoverColor : hoverColor;
        }
        if (button.pressed) {
            fill = pressedColor;
        }
        button.shape.setFillColor(fill);
        centerText(button.label, button.shape.getGlobalBounds());
    }

    for (size_t i = 0; i < m_die2Buttons.size(); ++i) {
        NumberButton& button = m_die2Buttons[i];
        const int token = kTokenDie2Base + static_cast<int>(i);
        button.pressed = (m_pressedToken == token);
        const bool selected = (m_selectedDie2 == button.value);
        sf::Color fill = selected ? selectedColor : baseColor;
        if (button.hovered) {
            fill = selected ? selectedHoverColor : hoverColor;
        }
        if (button.pressed) {
            fill = pressedColor;
        }
        button.shape.setFillColor(fill);
        centerText(button.label, button.shape.getGlobalBounds());
    }

    const sf::Color actionBase(236, 224, 196);
    const sf::Color actionHover(226, 208, 171);
    const sf::Color actionPressed(189, 150, 95);

    sf::Color executeFill = m_executeHovered ? actionHover : actionBase;
    sf::Color cancelFill = m_cancelHovered ? actionHover : actionBase;
    if (m_pressedToken == kTokenExecute) {
        executeFill = actionPressed;
    }
    if (m_pressedToken == kTokenCancel) {
        cancelFill = actionPressed;
    }
    m_executeButton.setFillColor(executeFill);
    m_cancelButton.setFillColor(cancelFill);
}

int DebugDicePopup::hitToken(sf::Vector2f mousePos) const {
    for (size_t i = 0; i < m_die1Buttons.size(); ++i) {
        if (m_die1Buttons[i].shape.getGlobalBounds().contains(mousePos)) {
            return kTokenDie1Base + static_cast<int>(i);
        }
    }
    for (size_t i = 0; i < m_die2Buttons.size(); ++i) {
        if (m_die2Buttons[i].shape.getGlobalBounds().contains(mousePos)) {
            return kTokenDie2Base + static_cast<int>(i);
        }
    }
    if (m_executeButton.getGlobalBounds().contains(mousePos)) {
        return kTokenExecute;
    }
    if (m_cancelButton.getGlobalBounds().contains(mousePos)) {
        return kTokenCancel;
    }
    return kTokenNone;
}

int DebugDicePopup::clampDieValue(int value) {
    return std::max(1, std::min(6, value));
}

void DebugDicePopup::centerText(sf::Text& text, const sf::FloatRect& bounds) {
    const sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + (textBounds.width * 0.5f),
                   textBounds.top + (textBounds.height * 0.5f));
    text.setPosition(bounds.left + (bounds.width * 0.5f),
                     bounds.top + (bounds.height * 0.5f));
}
} // namespace viewsGUI
