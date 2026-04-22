#include "../../include/viewsGUI/PopupBox.hpp"

#include <iostream>

namespace viewsGUI {
PopupBox::PopupBox(sf::Vector2f windowSize, const sf::Font& font)
    : m_isVisible(false), m_font(font), m_windowSize(windowSize) {
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(58);
    m_titleText.setFillColor(sf::Color(53, 45, 36));

    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(30);
    m_messageText.setFillColor(sf::Color(53, 45, 36));
}

bool PopupBox::loadAssets(const std::string& uiDir) {
    bool success = true;

    const std::string base = (!uiDir.empty() && uiDir.back() == '/') ? uiDir : uiDir + "/";

    if (!m_overlayTexture.loadFromFile(base + "popup_overlay.png")) {
        std::cerr << "[ERROR] Gagal memuat popup overlay.\n";
        success = false;
    }
    if (!m_boxTexture.loadFromFile(base + "popup_box.png")) {
        std::cerr << "[ERROR] Gagal memuat popup box.\n";
        success = false;
    }

    m_overlay.setTexture(m_overlayTexture);
    m_box.setTexture(m_boxTexture);

    if (m_overlayTexture.getSize().x > 0 && m_overlayTexture.getSize().y > 0) {
        m_overlay.setScale(m_windowSize.x / static_cast<float>(m_overlayTexture.getSize().x),
                           m_windowSize.y / static_cast<float>(m_overlayTexture.getSize().y));
    }

    if (m_boxTexture.getSize().x > 0 && m_boxTexture.getSize().y > 0) {
        m_box.setOrigin(m_boxTexture.getSize().x / 2.0f, m_boxTexture.getSize().y / 2.0f);
    }
    m_box.setPosition(m_windowSize.x / 2.0f, m_windowSize.y / 2.0f);

    m_optionNormalPath = base + "popup_option_normal.png";
    m_optionHoverPath = base + "popup_option_hover.png";
    m_optionPressedPath = base + "popup_option_pressed.png";
    m_optionDisabledPath = base + "popup_option_disabled.png";

    return success;
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
    m_optionTexts.clear();

    m_titleText.setString(title);
    m_messageText.setString(message);

    const sf::FloatRect boxBounds = m_box.getGlobalBounds();
    const float boxTopY = boxBounds.top;
    centerTextX(m_titleText, boxTopY + 40.0f);
    centerTextX(m_messageText, boxTopY + 116.0f);

    const float btnWidth = 460.0f;
    const float btnHeight = 54.0f;
    const float startY = boxTopY + 188.0f;
    const float gapY = 14.0f;

    for (size_t i = 0; i < options.size(); ++i) {
        const float btnX = m_box.getPosition().x - (btnWidth / 2.0f);
        const float btnY = startY + static_cast<float>(i) * (btnHeight + gapY);

        auto button = std::make_shared<SpriteButton>();
        button->loadTextures(m_optionNormalPath,
                             m_optionHoverPath,
                             m_optionPressedPath,
                             m_optionDisabledPath);
        button->setPosition(sf::Vector2f(btnX, btnY));
        button->setOnClick([this, index = static_cast<int>(i)]() {
            this->hide();
            if (m_onSelect) {
                m_onSelect(index);
            }
        });

        m_optionButtons.push_back(button);

        sf::Text label(std::to_string(i) + ": " + options[i], m_font, 24);
        label.setFillColor(sf::Color(53, 45, 36));
        const sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        label.setPosition(btnX + btnWidth / 2.0f, btnY + btnHeight / 2.0f - 2.0f);
        m_optionTexts.push_back(label);
    }
}

void PopupBox::hide() {
    m_isVisible = false;
    m_optionButtons.clear();
    m_optionTexts.clear();
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

bool PopupBox::handleMousePressed(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return false;
    }

    for (const auto& button : m_optionButtons) {
        if (button->handleMousePressed(mousePos)) {
            return true;
        }
    }

    return true;
}

bool PopupBox::handleMouseReleased(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return false;
    }

    for (const auto& button : m_optionButtons) {
        if (button->handleMouseReleased(mousePos)) {
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
    for (const auto& text : m_optionTexts) {
        window.draw(text);
    }
}
} // namespace viewsGUI
