#include "../../include/viewsGUI/PopupBox.hpp"

#include "../../include/viewsGUI/Theme.hpp"

namespace viewsGUI {
PopupBox::PopupBox(sf::Vector2f windowSize, const sf::Font& font)
    : m_isVisible(false), m_font(font) {
    m_overlay.setSize(windowSize);
    m_overlay.setFillColor(sf::Color(0, 0, 0, 160));

    const sf::Vector2f boxSize(520.0f, 330.0f);
    m_box.setSize(boxSize);
    m_box.setOrigin(boxSize.x / 2.0f, boxSize.y / 2.0f);
    m_box.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
    m_box.setFillColor(Theme::Background);
    m_box.setOutlineThickness(4.0f);
    m_box.setOutlineColor(Theme::BiruTua);

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(22);
    m_titleText.setFillColor(Theme::TextDark);

    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(16);
    m_messageText.setFillColor(Theme::TextDark);
}

void PopupBox::centerTextX(sf::Text& text, float yPos) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top);
    text.setPosition(m_box.getPosition().x, yPos);
}

void PopupBox::showOptions(const std::string& title,
                           const std::string& message,
                           const std::vector<std::string>& options,
                           std::function<void(int)> onSelect) {
    m_isVisible = true;
    m_onSelect = std::move(onSelect);
    m_optionButtons.clear();

    m_titleText.setString(title);
    m_messageText.setString(message);

    const float boxTopY = m_box.getPosition().y - (m_box.getSize().y / 2.0f);
    centerTextX(m_titleText, boxTopY + 26.0f);
    centerTextX(m_messageText, boxTopY + 74.0f);

    const float btnWidth = 430.0f;
    const float btnHeight = 38.0f;
    const float startY = boxTopY + 126.0f;
    const float gapY = 10.0f;

    for (size_t i = 0; i < options.size(); ++i) {
        const float btnX = m_box.getPosition().x - (btnWidth / 2.0f);
        const float btnY = startY + static_cast<float>(i) * (btnHeight + gapY);

        std::string label = std::to_string(i) + ": " + options[i];
        auto button = std::make_shared<Button>(sf::Vector2f(btnX, btnY),
                                               sf::Vector2f(btnWidth, btnHeight),
                                               label,
                                               m_font);
        button->setOnClick([this, index = static_cast<int>(i)]() {
            this->hide();
            if (m_onSelect) {
                m_onSelect(index);
            }
        });
        m_optionButtons.push_back(button);
    }
}

void PopupBox::hide() {
    m_isVisible = false;
    m_optionButtons.clear();
    m_onSelect = nullptr;
}

void PopupBox::update(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return;
    }

    for (const auto& button : m_optionButtons) {
        button->update(mousePos);
    }
}

bool PopupBox::handleMouseClick(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return false;
    }

    for (const auto& button : m_optionButtons) {
        if (button->handleMouseClick(mousePos)) {
            return true;
        }
    }

    return true;
}

void PopupBox::render(sf::RenderWindow& window) const {
    if (!m_isVisible) {
        return;
    }

    window.draw(m_overlay);
    window.draw(m_box);
    window.draw(m_titleText);
    window.draw(m_messageText);

    for (const auto& button : m_optionButtons) {
        button->render(window);
    }
}
} // namespace viewsGUI
