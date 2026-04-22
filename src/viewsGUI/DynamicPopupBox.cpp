#include "../../include/viewsGUI/DynamicPopupBox.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace {
// Layout tuning point untuk DynamicPopupBox (1920x1080).
// Ubah angka-angka di sini untuk geser posisi/ukuran komponen popup.
struct PopupLayout {
    static constexpr float kPopupWidth = 900.0f;
    static constexpr float kPopupHeight = 843.14f;

    static constexpr float kHeaderTop = 22.0f;

    static constexpr float kCardOffsetX = 42.0f;
    static constexpr float kCardOffsetY = 124.0f;
    static constexpr float kCardWidth = 542.0f;
    static constexpr float kCardHeight = 680.0f;
    static constexpr float kCardTextPaddingX = 28.0f;
    static constexpr float kCardTextMaxWidth = kCardWidth - (2.0f * kCardTextPaddingX);

    static constexpr float kRibbonMarginX = 20.0f;
    static constexpr float kRibbonTop = 24.0f;
    static constexpr float kRibbonHeight = 34.0f;

    static constexpr float kPropertyTitleY = 72.0f;
    static constexpr float kPropertyPriceY = 150.0f;
    static constexpr float kPropertyRentStartY = 198.0f;
    static constexpr float kPropertyRentStepY = 35.0f;
    static constexpr float kPropertyDescriptionY = 538.0f;

    static constexpr float kGenericTitleY = 82.0f;
    static constexpr float kGenericDescriptionY = 170.0f;

    static constexpr float kMinimizeIconSize = 30.0f;
    static constexpr float kMinimizeIconRight = 42.0f;
    static constexpr float kMinimizeIconTop = 22.0f;

    static constexpr float kButtonsX = 622.0f;
    static constexpr float kButtonsAreaTop = 196.0f;
    static constexpr float kButtonsAreaHeight = 518.0f;
    static constexpr float kButtonWidth = 240.0f;
    static constexpr float kButtonHeight = 76.0f;
    static constexpr float kButtonGapY = 18.0f;

    static constexpr float kMinimizedWidth = 220.0f;
    static constexpr float kMinimizedHeight = 72.0f;
    static constexpr float kMinimizedRight = 30.0f;
    static constexpr float kMinimizedBottom = 26.0f;
};

std::string ensureTrailingSlash(const std::string& dir) {
    if (dir.empty()) {
        return std::string();
    }
    if (dir.back() == '/') {
        return dir;
    }
    return dir + "/";
}

std::string joinPath(const std::string& base, const std::string& path) {
    if (path.empty()) {
        return std::string();
    }
    if (path.size() > 1 && path[1] == ':') {
        return path;
    }
    if (!path.empty() && path.front() == '/') {
        return path;
    }
    return ensureTrailingSlash(base) + path;
}

std::string wrapTextByWidth(const sf::Font& font,
                            unsigned int charSize,
                            float maxWidth,
                            const std::string& source) {
    if (source.empty()) {
        return source;
    }

    sf::Text probe;
    probe.setFont(font);
    probe.setCharacterSize(charSize);

    std::ostringstream out;
    std::istringstream lineReader(source);
    std::string rawLine;
    bool firstLine = true;

    while (std::getline(lineReader, rawLine)) {
        std::istringstream wordReader(rawLine);
        std::string word;
        std::string currentLine;

        while (wordReader >> word) {
            const std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
            probe.setString(candidate);

            if (!currentLine.empty() && probe.getLocalBounds().width > maxWidth) {
                if (!firstLine) {
                    out << '\n';
                }
                out << currentLine;
                currentLine = word;
                firstLine = false;
            } else {
                currentLine = candidate;
            }
        }

        if (!currentLine.empty()) {
            if (!firstLine) {
                out << '\n';
            }
            out << currentLine;
            firstLine = false;
        } else if (rawLine.empty()) {
            if (!firstLine) {
                out << '\n';
            }
            firstLine = false;
        }
    }

    return out.str();
}
} // namespace

namespace viewsGUI {
DynamicPopupBox::DynamicPopupBox(sf::Vector2f windowSize,
                                 const sf::Font& headerFont,
                                 const sf::Font& bodyFont)
    : m_isVisible(false),
      m_isMinimized(false),
      m_windowSize(windowSize),
      m_headerFont(headerFont),
      m_bodyFont(bodyFont),
      m_mode(PopupMode::INFO),
      m_pressedActionIndex(-1),
      m_pressedMinimize(false),
      m_pressedShowMenu(false) {
    m_headerText.setFont(m_headerFont);
    m_headerText.setCharacterSize(64);
    m_headerText.setFillColor(sf::Color(53, 45, 36));

    m_cardTitleText.setFont(m_headerFont);
    m_cardTitleText.setCharacterSize(52);
    m_cardTitleText.setFillColor(sf::Color(53, 45, 36));

    m_priceText.setFont(m_bodyFont);
    m_priceText.setCharacterSize(28);
    m_priceText.setFillColor(sf::Color(53, 45, 36));

    m_descriptionText.setFont(m_bodyFont);
    m_descriptionText.setCharacterSize(24);
    m_descriptionText.setFillColor(sf::Color(53, 45, 36));

    m_showMenuText.setFont(m_bodyFont);
    m_showMenuText.setCharacterSize(24);
    m_showMenuText.setFillColor(sf::Color(53, 45, 36));
    m_showMenuText.setString("Show Menu");

    m_propertyRibbon.setFillColor(sf::Color(181, 164, 128));
}

bool DynamicPopupBox::loadAssets(const std::string& uiDir) {
    m_uiDir = ensureTrailingSlash(uiDir);

    bool success = true;

    if (!loadTextureWithFallback(m_popupBaseTexture,
                                 {m_uiDir + "popup_base.png", m_uiDir + "popup_box.png", "Group 40.png"})) {
        buildSolidTexture(m_popupBaseTexture, 900, 843, sf::Color(241, 231, 214));
        std::cerr << "[WARN] DynamicPopupBox: base popup texture tidak ditemukan, gunakan dummy.\n";
        success = false;
    }

    if (!loadTextureWithFallback(m_cardTemplateTexture,
                                 {m_uiDir + "card_template.png", m_uiDir + "asset_card_template.png"})) {
        buildSolidTexture(m_cardTemplateTexture, 520, 680, sf::Color(252, 244, 228));
        std::cerr << "[WARN] DynamicPopupBox: card template texture tidak ditemukan, gunakan dummy.\n";
        success = false;
    }

    if (!loadTextureWithFallback(m_minimizeIconTexture,
                                 {m_uiDir + "btn_minimize.png", m_uiDir + "popup_minimize.png"})) {
        buildSolidTexture(m_minimizeIconTexture, 32, 32, sf::Color(226, 188, 102));
        success = false;
    }

    if (!loadTextureWithFallback(m_showMenuTexture,
                                 {m_uiDir + "btn_show_menu.png", m_uiDir + "popup_show_menu.png"})) {
        buildSolidTexture(m_showMenuTexture, 220, 72, sf::Color(241, 231, 214));
        success = false;
    }

    m_popupBaseSprite.setTexture(m_popupBaseTexture);
    m_cardTemplateSprite.setTexture(m_cardTemplateTexture);
    m_minimizeIconSprite.setTexture(m_minimizeIconTexture);
    m_showMenuSprite.setTexture(m_showMenuTexture);

    layoutExpanded();
    layoutMinimized();
    return success;
}

void DynamicPopupBox::show(const PopupPayload& payload, ActionCallback onAction) {
    m_payload = payload;
    m_mode = payload.mode;
    m_onAction = std::move(onAction);

    if (!payload.popupBaseImagePath.empty()) {
        sf::Texture custom;
        if (loadTextureWithFallback(custom,
                                    {payload.popupBaseImagePath,
                                     joinPath(m_uiDir, payload.popupBaseImagePath),
                                     m_uiDir + "popup_base.png",
                                     "Group 40.png"})) {
            m_popupBaseTexture = std::move(custom);
        }
    }

    if (!payload.cardTemplateImagePath.empty()) {
        sf::Texture custom;
        if (loadTextureWithFallback(custom,
                                    {payload.cardTemplateImagePath,
                                     joinPath(m_uiDir, payload.cardTemplateImagePath),
                                     m_uiDir + "card_template.png",
                                     m_uiDir + "asset_card_template.png"})) {
            m_cardTemplateTexture = std::move(custom);
        }
    }

    if (!payload.minimizeIconImagePath.empty()) {
        sf::Texture custom;
        if (loadTextureWithFallback(custom,
                                    {payload.minimizeIconImagePath,
                                     joinPath(m_uiDir, payload.minimizeIconImagePath),
                                     m_uiDir + "btn_minimize.png"})) {
            m_minimizeIconTexture = std::move(custom);
        }
    }

    if (!payload.showMenuImagePath.empty()) {
        sf::Texture custom;
        if (loadTextureWithFallback(custom,
                                    {payload.showMenuImagePath,
                                     joinPath(m_uiDir, payload.showMenuImagePath),
                                     m_uiDir + "btn_show_menu.png"})) {
            m_showMenuTexture = std::move(custom);
        }
    }

    m_popupBaseSprite.setTexture(m_popupBaseTexture, true);
    m_cardTemplateSprite.setTexture(m_cardTemplateTexture, true);
    m_minimizeIconSprite.setTexture(m_minimizeIconTexture, true);
    m_showMenuSprite.setTexture(m_showMenuTexture, true);

    m_isVisible = true;
    m_isMinimized = false;
    m_pressedActionIndex = -1;
    m_pressedMinimize = false;
    m_pressedShowMenu = false;

    rebuildCardTexts();
    rebuildActionSprites();
    layoutExpanded();
}

void DynamicPopupBox::hide() {
    m_isVisible = false;
    m_isMinimized = false;
    m_pressedActionIndex = -1;
    m_pressedMinimize = false;
    m_pressedShowMenu = false;
    m_actionSprites.clear();
    m_rentTexts.clear();
    m_onAction = nullptr;
}

void DynamicPopupBox::minimize() {
    if (!m_isVisible) {
        return;
    }
    m_isMinimized = true;
    m_pressedActionIndex = -1;
    m_pressedMinimize = false;
    layoutMinimized();
}

void DynamicPopupBox::maximize() {
    if (!m_isVisible) {
        return;
    }
    m_isMinimized = false;
    m_pressedShowMenu = false;
    layoutExpanded();
}

void DynamicPopupBox::update(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return;
    }

    if (m_isMinimized) {
        if (!m_pressedShowMenu) {
            if (containsShowMenuButton(mousePos)) {
                m_showMenuSprite.setColor(sf::Color(255, 245, 220));
            } else {
                m_showMenuSprite.setColor(sf::Color::White);
            }
        }
        return;
    }

    for (auto& action : m_actionSprites) {
        if (!action.enabled || action.pressed) {
            continue;
        }
        action.hovered = action.sprite.getGlobalBounds().contains(mousePos);
    }

    if (!m_pressedMinimize) {
        if (containsMinimizeIcon(mousePos)) {
            m_minimizeIconSprite.setColor(sf::Color(255, 245, 220));
        } else {
            m_minimizeIconSprite.setColor(sf::Color::White);
        }
    }

    updateActionVisuals();
}

bool DynamicPopupBox::handleMousePressed(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return false;
    }

    if (m_isMinimized) {
        if (containsShowMenuButton(mousePos)) {
            m_pressedShowMenu = true;
            m_showMenuSprite.setColor(sf::Color(235, 220, 190));
            return true;
        }
        return false;
    }

    if (containsMinimizeIcon(mousePos)) {
        m_pressedMinimize = true;
        m_minimizeIconSprite.setColor(sf::Color(235, 220, 190));
        return true;
    }

    const int index = actionIndexAt(mousePos);
    if (index >= 0) {
        m_pressedActionIndex = index;
        m_actionSprites[static_cast<size_t>(index)].pressed = true;
        updateActionVisuals();
        return true;
    }

    return containsExpanded(mousePos);
}

bool DynamicPopupBox::handleMouseReleased(sf::Vector2f mousePos) {
    if (!m_isVisible) {
        return false;
    }

    if (m_isMinimized) {
        const bool wasPressed = m_pressedShowMenu;
        m_pressedShowMenu = false;

        if (wasPressed && containsShowMenuButton(mousePos)) {
            maximize();
            return true;
        }

        m_showMenuSprite.setColor(sf::Color::White);
        return false;
    }

    if (m_pressedMinimize) {
        const bool shouldMinimize = containsMinimizeIcon(mousePos);
        m_pressedMinimize = false;
        m_minimizeIconSprite.setColor(sf::Color::White);

        if (shouldMinimize) {
            minimize();
        }
        return true;
    }

    if (m_pressedActionIndex >= 0) {
        const int pressedIndex = m_pressedActionIndex;
        m_pressedActionIndex = -1;

        for (auto& action : m_actionSprites) {
            action.pressed = false;
        }

        updateActionVisuals();

        const bool inside = m_actionSprites[static_cast<size_t>(pressedIndex)].sprite.getGlobalBounds().contains(mousePos);
        const bool enabled = m_actionSprites[static_cast<size_t>(pressedIndex)].enabled;

        if (inside && enabled) {
            invokeAction(m_actionSprites[static_cast<size_t>(pressedIndex)].actionId);
            return true;
        }

        return false;
    }

    return containsExpanded(mousePos);
}

void DynamicPopupBox::render(sf::RenderWindow& window) const {
    if (!m_isVisible) {
        return;
    }

    if (m_isMinimized) {
        window.draw(m_showMenuSprite);
        window.draw(m_showMenuText);
        return;
    }

    window.draw(m_popupBaseSprite);
    window.draw(m_headerText);

    window.draw(m_cardTemplateSprite);
    if (m_payload.showStreetColorRibbon && m_mode == PopupMode::PROPERTY) {
        window.draw(m_propertyRibbon);
    }

    window.draw(m_cardTitleText);

    if (!m_priceText.getString().isEmpty()) {
        window.draw(m_priceText);
    }

    for (const auto& rentText : m_rentTexts) {
        window.draw(rentText);
    }

    if (!m_descriptionText.getString().isEmpty()) {
        window.draw(m_descriptionText);
    }

    for (const auto& action : m_actionSprites) {
        window.draw(action.sprite);
        window.draw(action.label);
    }

    window.draw(m_minimizeIconSprite);
}

void DynamicPopupBox::layoutExpanded() {
    const sf::Vector2f popupPos((m_windowSize.x - PopupLayout::kPopupWidth) * 0.5f,
                                (m_windowSize.y - PopupLayout::kPopupHeight) * 0.5f);

    const sf::Vector2u baseSize = m_popupBaseTexture.getSize();
    if (baseSize.x > 0 && baseSize.y > 0) {
        m_popupBaseSprite.setScale(PopupLayout::kPopupWidth / static_cast<float>(baseSize.x),
                                   PopupLayout::kPopupHeight / static_cast<float>(baseSize.y));
    }
    m_popupBaseSprite.setPosition(popupPos);

    const sf::FloatRect headerBounds = m_headerText.getLocalBounds();
    m_headerText.setOrigin(headerBounds.left + headerBounds.width * 0.5f, 0.0f);
    m_headerText.setPosition(popupPos.x + PopupLayout::kPopupWidth * 0.5f, popupPos.y + PopupLayout::kHeaderTop);

    const sf::Vector2f cardPos(popupPos.x + PopupLayout::kCardOffsetX, popupPos.y + PopupLayout::kCardOffsetY);
    const sf::Vector2f cardSize(PopupLayout::kCardWidth, PopupLayout::kCardHeight);

    const sf::Vector2u cardTexSize = m_cardTemplateTexture.getSize();
    if (cardTexSize.x > 0 && cardTexSize.y > 0) {
        m_cardTemplateSprite.setScale(cardSize.x / static_cast<float>(cardTexSize.x),
                                      cardSize.y / static_cast<float>(cardTexSize.y));
    }
    m_cardTemplateSprite.setPosition(cardPos);

    m_propertyRibbon.setPosition(cardPos.x + PopupLayout::kRibbonMarginX, cardPos.y + PopupLayout::kRibbonTop);
    m_propertyRibbon.setSize(
        sf::Vector2f(cardSize.x - (2.0f * PopupLayout::kRibbonMarginX), PopupLayout::kRibbonHeight));
    m_propertyRibbon.setFillColor(m_payload.propertyColor);

    const sf::FloatRect titleBounds = m_cardTitleText.getLocalBounds();
    m_cardTitleText.setOrigin(titleBounds.left + titleBounds.width * 0.5f, titleBounds.top);
    const bool hasPropertyRows = (m_payload.purchasePrice > 0) || !m_payload.rentPrices.empty();

    if (hasPropertyRows) {
        m_cardTitleText.setPosition(cardPos.x + cardSize.x * 0.5f, cardPos.y + PopupLayout::kPropertyTitleY);
        m_priceText.setPosition(cardPos.x + PopupLayout::kCardTextPaddingX, cardPos.y + PopupLayout::kPropertyPriceY);

        for (size_t i = 0; i < m_rentTexts.size(); ++i) {
            m_rentTexts[i].setPosition(cardPos.x + PopupLayout::kCardTextPaddingX,
                                       cardPos.y + PopupLayout::kPropertyRentStartY +
                                           static_cast<float>(i) * PopupLayout::kPropertyRentStepY);
        }

        m_descriptionText.setPosition(cardPos.x + PopupLayout::kCardTextPaddingX,
                                      cardPos.y + PopupLayout::kPropertyDescriptionY);
    } else {
        m_cardTitleText.setPosition(cardPos.x + cardSize.x * 0.5f, cardPos.y + PopupLayout::kGenericTitleY);
        m_priceText.setPosition(cardPos.x + PopupLayout::kCardTextPaddingX, cardPos.y + PopupLayout::kGenericDescriptionY);
        m_descriptionText.setPosition(cardPos.x + PopupLayout::kCardTextPaddingX,
                                      cardPos.y + PopupLayout::kGenericDescriptionY);
    }

    const sf::Vector2u iconSize = m_minimizeIconTexture.getSize();
    if (iconSize.x > 0 && iconSize.y > 0) {
        m_minimizeIconSprite.setScale(PopupLayout::kMinimizeIconSize / static_cast<float>(iconSize.x),
                                      PopupLayout::kMinimizeIconSize / static_cast<float>(iconSize.y));
    }
    m_minimizeIconSprite.setPosition(popupPos.x + PopupLayout::kPopupWidth - PopupLayout::kMinimizeIconRight,
                                     popupPos.y + PopupLayout::kMinimizeIconTop);

    const float btnX = popupPos.x + PopupLayout::kButtonsX;
    const float btnWidth = PopupLayout::kButtonWidth;
    const float btnHeight = PopupLayout::kButtonHeight;
    const float gapY = PopupLayout::kButtonGapY;
    const float blockHeight = (m_actionSprites.empty())
                                  ? 0.0f
                                  : (static_cast<float>(m_actionSprites.size()) * btnHeight) +
                                        (static_cast<float>(m_actionSprites.size() - 1) * gapY);
    const float btnStartY = popupPos.y + PopupLayout::kButtonsAreaTop +
                            std::max(0.0f, (PopupLayout::kButtonsAreaHeight - blockHeight) * 0.5f);

    for (size_t i = 0; i < m_actionSprites.size(); ++i) {
        SpriteAction& action = m_actionSprites[i];
        const sf::Vector2u texSize = action.texture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            action.sprite.setScale(btnWidth / static_cast<float>(texSize.x),
                                   btnHeight / static_cast<float>(texSize.y));
        }

        const float y = btnStartY + static_cast<float>(i) * (btnHeight + gapY);
        action.sprite.setPosition(btnX, y);

        const sf::FloatRect labelBounds = action.label.getLocalBounds();
        action.label.setOrigin(labelBounds.left + labelBounds.width * 0.5f,
                               labelBounds.top + labelBounds.height * 0.5f);
        action.label.setPosition(btnX + btnWidth * 0.5f, y + btnHeight * 0.5f - 2.0f);
    }

    updateActionVisuals();
}

void DynamicPopupBox::layoutMinimized() {
    const sf::Vector2f pos(m_windowSize.x - PopupLayout::kMinimizedWidth - PopupLayout::kMinimizedRight,
                           m_windowSize.y - PopupLayout::kMinimizedHeight - PopupLayout::kMinimizedBottom);

    const sf::Vector2u texSize = m_showMenuTexture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        m_showMenuSprite.setScale(PopupLayout::kMinimizedWidth / static_cast<float>(texSize.x),
                                  PopupLayout::kMinimizedHeight / static_cast<float>(texSize.y));
    }
    m_showMenuSprite.setPosition(pos);

    const sf::FloatRect textBounds = m_showMenuText.getLocalBounds();
    m_showMenuText.setOrigin(textBounds.left + textBounds.width * 0.5f,
                             textBounds.top + textBounds.height * 0.5f);
    m_showMenuText.setPosition(pos.x + PopupLayout::kMinimizedWidth * 0.5f,
                               pos.y + PopupLayout::kMinimizedHeight * 0.5f - 2.0f);
}

void DynamicPopupBox::rebuildCardTexts() {
    m_headerText.setString(m_payload.headerTitle);
    m_cardTitleText.setString(
        wrapTextByWidth(m_headerFont, m_cardTitleText.getCharacterSize(), PopupLayout::kCardTextMaxWidth, m_payload.cardTitle));

    if (m_payload.purchasePrice > 0) {
        m_priceText.setString("Harga beli: M" + std::to_string(m_payload.purchasePrice));
    } else {
        m_priceText.setString("");
    }

    m_rentTexts.clear();
    for (size_t i = 0; i < m_payload.rentPrices.size(); ++i) {
        sf::Text text;
        text.setFont(m_bodyFont);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color(53, 45, 36));

        std::ostringstream row;
        row << "Sewa " << i << ": M" << m_payload.rentPrices[i];
        text.setString(row.str());

        m_rentTexts.push_back(text);
    }

    m_descriptionText.setString(
        wrapTextByWidth(m_bodyFont, m_descriptionText.getCharacterSize(), PopupLayout::kCardTextMaxWidth, m_payload.description));
}

void DynamicPopupBox::rebuildActionSprites() {
    m_actionSprites.clear();

    for (const PopupActionItem& item : m_payload.actionItems) {
        SpriteAction spriteAction;
        spriteAction.enabled = item.enabled;
        spriteAction.actionId = item.actionId;

        std::vector<std::string> candidates;
        if (!item.texturePath.empty()) {
            candidates.push_back(item.texturePath);
            candidates.push_back(joinPath(m_uiDir, item.texturePath));
        }
        candidates.push_back(m_uiDir + "popup_option_normal.png");

        if (!loadTextureWithFallback(spriteAction.texture, candidates)) {
            buildSolidTexture(spriteAction.texture, 250, 72, sf::Color(239, 223, 190));
        }

        spriteAction.sprite.setTexture(spriteAction.texture);
        spriteAction.label.setFont(m_bodyFont);
        const unsigned int labelSize = (item.label.size() > 24) ? 20U : 26U;
        spriteAction.label.setCharacterSize(labelSize);
        spriteAction.label.setFillColor(sf::Color(53, 45, 36));
        spriteAction.label.setString(
            wrapTextByWidth(m_bodyFont, labelSize, PopupLayout::kButtonWidth - 20.0f, item.label));

        m_actionSprites.push_back(std::move(spriteAction));
    }

    updateActionVisuals();
}

void DynamicPopupBox::updateActionVisuals() {
    for (auto& action : m_actionSprites) {
        if (!action.enabled) {
            action.sprite.setColor(sf::Color(180, 180, 180));
            action.label.setFillColor(sf::Color(120, 120, 120));
            continue;
        }

        if (action.pressed) {
            action.sprite.setColor(sf::Color(230, 208, 170));
            action.label.setFillColor(sf::Color(46, 38, 30));
            continue;
        }

        if (action.hovered) {
            action.sprite.setColor(sf::Color(255, 245, 220));
            action.label.setFillColor(sf::Color(46, 38, 30));
            continue;
        }

        action.sprite.setColor(sf::Color::White);
        action.label.setFillColor(sf::Color(53, 45, 36));
    }
}

void DynamicPopupBox::invokeAction(const std::string& actionId) {
    if (m_onAction) {
        m_onAction(actionId);
    }
}

bool DynamicPopupBox::loadTextureWithFallback(sf::Texture& texture,
                                              const std::vector<std::string>& paths) const {
    for (const std::string& path : paths) {
        if (path.empty()) {
            continue;
        }
        if (texture.loadFromFile(path)) {
            return true;
        }
    }
    return false;
}

void DynamicPopupBox::buildSolidTexture(sf::Texture& texture,
                                        unsigned int width,
                                        unsigned int height,
                                        sf::Color color) {
    sf::Image image;
    image.create(width, height, color);
    texture.loadFromImage(image);
}

bool DynamicPopupBox::containsExpanded(sf::Vector2f point) const {
    return m_popupBaseSprite.getGlobalBounds().contains(point);
}

bool DynamicPopupBox::containsMinimizeIcon(sf::Vector2f point) const {
    return m_minimizeIconSprite.getGlobalBounds().contains(point);
}

bool DynamicPopupBox::containsShowMenuButton(sf::Vector2f point) const {
    return m_showMenuSprite.getGlobalBounds().contains(point);
}

int DynamicPopupBox::actionIndexAt(sf::Vector2f point) const {
    for (size_t i = 0; i < m_actionSprites.size(); ++i) {
        if (!m_actionSprites[i].enabled) {
            continue;
        }
        if (m_actionSprites[i].sprite.getGlobalBounds().contains(point)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
} // namespace viewsGUI
