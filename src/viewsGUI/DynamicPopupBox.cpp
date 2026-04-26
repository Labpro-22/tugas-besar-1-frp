#include "../../include/viewsGUI/DynamicPopupBox.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
constexpr const char* kPromptAnswerPrefix = "prompt_answer:";
constexpr const char* kPromptCustomBidKey = "BID_CUSTOM";
constexpr const char* kPromptCustomTargetKey = "TARGET_CUSTOM";

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool equalsIgnoreCase(const std::string& lhs, const std::string& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i) {
        if (std::toupper(static_cast<unsigned char>(lhs[i])) !=
            std::toupper(static_cast<unsigned char>(rhs[i]))) {
            return false;
        }
    }
    return true;
}

int parseAuctionMinBidFromPromptId(const std::string& promptId) {
    if (!startsWith(promptId, "lelang_")) {
        return 0;
    }

    const size_t lastUnderscore = promptId.rfind('_');
    if (lastUnderscore == std::string::npos || lastUnderscore == 0) {
        return 0;
    }

    const size_t prevUnderscore = promptId.rfind('_', lastUnderscore - 1);
    if (prevUnderscore == std::string::npos || prevUnderscore + 1 >= lastUnderscore) {
        return 0;
    }

    try {
        const int highestBid = std::stoi(promptId.substr(prevUnderscore + 1, lastUnderscore - prevUnderscore - 1));
        return (highestBid < 0) ? 0 : (highestBid + 1);
    } catch (const std::exception&) {
        return 0;
    }
}

// Layout tuning point untuk DynamicPopupBox (1920x1080).
// Ubah angka-angka di sini untuk geser posisi/ukuran komponen popup.
struct PopupLayout {
    static constexpr float kPopupWidth = 900.0f;
    static constexpr float kPopupHeight = 843.14f;

    static constexpr float kHeaderTop = 80.0f;

    static constexpr float kCardOffsetX = 42.0f;
    static constexpr float kCardOffsetY = 260.0f;
    static constexpr float kCardWidth = 542.0f;
    static constexpr float kCardHeight = 520.0f;
    static constexpr float kCardTextPaddingX = 50.0f;
    static constexpr float kCardTextMaxWidth = kCardWidth - (2.0f * kCardTextPaddingX);

    static constexpr float kRibbonMarginX = 21.0f;
    static constexpr float kRibbonTop = 15.0f;
    static constexpr float kRibbonHeight = 50.0f;

    static constexpr float kPropertyTitleY = 72.0f;
    static constexpr float kPropertyPriceY = 150.0f;
    static constexpr float kPropertyRentStartY = 198.0f;
    static constexpr float kPropertyRentStepY = 35.0f;
    static constexpr float kPropertyDescriptionY = 408.0f;

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

void appendPopupDebugLog(const std::string& line) {
    std::ofstream out("build/gui_preturn_debug.log", std::ios::app);
    if (out.is_open()) {
        out << line << '\n';
    }
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
      m_promptWantsBidInput(false),
      m_promptWantsTextInput(false),
      m_promptTextInputForceUppercase(false),
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

    m_bidInputBox.setSize(sf::Vector2f(240.0f, 54.0f));
    m_bidInputBox.setFillColor(sf::Color(252, 244, 228));
    m_bidInputBox.setOutlineThickness(2.0f);
    m_bidInputBox.setOutlineColor(sf::Color(181, 164, 128));

    m_bidInputText.setFont(m_bodyFont);
    m_bidInputText.setCharacterSize(24);
    m_bidInputText.setFillColor(sf::Color(53, 45, 36));

    m_bidInputHint.setFont(m_bodyFont);
    m_bidInputHint.setCharacterSize(18);
    m_bidInputHint.setFillColor(sf::Color(120, 110, 96));
    m_bidInputHint.setString("Ketik nominal BID...");

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
    m_fullImageSprite.setTexture(m_cardTemplateTexture);

    layoutExpanded();
    layoutMinimized();
    return success;
}

void DynamicPopupBox::show(const PopupPayload& payload, ActionCallback onAction) {
    m_payload = payload;
    m_mode = payload.mode;
    m_onAction = std::move(onAction);

    m_isVisible = true;
    m_isMinimized = false;
    m_pressedActionIndex = -1;
    m_pressedMinimize = false;
    m_pressedShowMenu = false;

    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
        m_promptWantsBidInput = false;
        m_promptWantsTextInput = false;
        m_promptTextInputForceUppercase = false;
        m_bidInputValue.clear();
        m_bidInputText.setString("");
        m_actionSprites.clear();
        m_rentTexts.clear();

        const std::string fullImagePath =
            payload.cardTemplateImagePath.empty() ? payload.popupBaseImagePath : payload.cardTemplateImagePath;
        sf::Texture custom;
        if (!fullImagePath.empty() &&
            loadTextureWithFallback(custom, {fullImagePath, joinPath(m_uiDir, fullImagePath)})) {
            m_fullImageTexture = std::move(custom);
        } else {
            buildSolidTexture(m_fullImageTexture, 900, 640, sf::Color(241, 231, 214));
            std::cerr << "[WARN] DynamicPopupBox: full-image popup texture tidak ditemukan, gunakan dummy.\n";
        }

        m_fullImageSprite.setTexture(m_fullImageTexture, true);
        layoutFullImage();
        return;
    }

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

    rebuildCardTexts();
    rebuildActionSprites();
    layoutExpanded();
}

void DynamicPopupBox::showPrompt(const PromptRequest& prompt, ActionCallback onAnswer) {
    m_bidInputValue.clear();
    m_bidInputText.setString("");

    PopupPayload payload = buildFromPrompt(prompt);
    show(payload, [this, onAnswer = std::move(onAnswer)](const std::string& actionId) {
        if (actionId.rfind(kPromptAnswerPrefix, 0) == 0) {
            const std::string answer = actionId.substr(std::char_traits<char>::length(kPromptAnswerPrefix));
            if (answer == kPromptCustomBidKey) {
                if (m_bidInputValue.empty()) {
                    onAnswer("BID_MIN");
                } else {
                    onAnswer("BID " + m_bidInputValue);
                }
                return;
            }

            if (answer == kPromptCustomTargetKey) {
                onAnswer(std::string(m_bidInputValue));
                return;
            }

            onAnswer(answer);
        }
    });
}

PopupPayload DynamicPopupBox::buildFromPrompt(const PromptRequest& prompt) {
    PopupPayload payload;
    const bool isFestivalPrompt = startsWith(prompt.id, "festival_");
    const bool isBankruptcyPrompt = startsWith(prompt.id, "likuidasi_");
    const bool hasBidOption = std::any_of(prompt.options.begin(), prompt.options.end(), [](const PromptOption& option) {
        return equalsIgnoreCase(option.key, "BID_MIN") || startsWith(option.key, "BID ");
    });
    const bool isAuctionPrompt = startsWith(prompt.id, "lelang_") || hasBidOption;
    const bool isSkillTargetPrompt = startsWith(prompt.id, "skill_target_");
    const bool isFileInputPrompt = startsWith(prompt.id, "file_input_");
    const bool hasTargetSubmitOption = std::any_of(
        prompt.options.begin(), prompt.options.end(), [](const PromptOption& option) {
            return equalsIgnoreCase(option.key, "SUBMIT");
        });

    m_promptWantsBidInput = isAuctionPrompt;
    m_promptWantsTextInput = (isSkillTargetPrompt && hasTargetSubmitOption) || isFileInputPrompt;
    m_promptTextInputForceUppercase = isSkillTargetPrompt;

    payload.mode = isFestivalPrompt || isAuctionPrompt || isSkillTargetPrompt || isBankruptcyPrompt
        ? PopupMode::SPECIAL
        : PopupMode::INFO;
    payload.headerTitle = prompt.title.empty() ? "PILIHAN" : prompt.title;
    payload.cardTitle = payload.headerTitle;
    payload.description = prompt.message;

    if (isFestivalPrompt) {
        payload.headerTitle = "FESTIVAL";
        payload.cardTitle = "PILIH PROPERTI";
        payload.description = prompt.message + "\nKlik salah satu properti untuk mengaktifkan efek festival.";
    }

    if (isAuctionPrompt) {
        payload.headerTitle = "LELANG";
        payload.cardTitle = "AUCTION BID";
        payload.description = prompt.message + "\nGunakan PASS/BID MIN atau ketik nominal lalu klik BID ANGKA.";
        m_bidInputHint.setString("Ketik nominal BID...");
    }

    if (isSkillTargetPrompt) {
        payload.headerTitle = prompt.title.empty() ? "TARGET KARTU" : prompt.title;
        payload.cardTitle = "INPUT TARGET";
        m_bidInputHint.setString("Ketik ID/KODE target...");
    }

    if (isFileInputPrompt) {
        payload.headerTitle = prompt.title.empty() ? "INPUT FILE" : prompt.title;
        payload.cardTitle = "NAMA FILE";
        m_bidInputHint.setString("Contoh: game_save.nmp");
    }

    if (isBankruptcyPrompt) {
        payload.headerTitle = "BANGKRUT";
        payload.cardTitle = "BANGKRUT";
    }

    bool hasCustomBidAction = false;
    bool hasCustomTargetAction = false;
    const int minBidFromId = parseAuctionMinBidFromPromptId(prompt.id);

    for (const PromptOption& option : prompt.options) {
        const std::string key = option.key;
        std::string label = option.label.empty() ? key : option.label;
        std::string texturePath = "assets/images/ui/btn_cancel.png";

        if (isBankruptcyPrompt) {
            std::replace(label.begin(), label.end(), '_', ' ');
        }

        if (isFestivalPrompt) {
            texturePath = "assets/images/ui/btn_beli.png";
        }

        if (isAuctionPrompt) {
            if (key == "PASS") {
                texturePath = "assets/images/ui/btn_cancel.png";
                label = "PASS";
            } else {
                texturePath = "assets/images/ui/btn_beli.png";
            }

            if (key == "BID_MIN") {
                label = "BID MIN (M" + std::to_string(minBidFromId) + ")";
            }
        }

        if (m_promptWantsTextInput) {
            if (equalsIgnoreCase(key, "SUBMIT")) {
                payload.actionItems.push_back(PopupActionItem{
                    std::string(kPromptAnswerPrefix) + kPromptCustomTargetKey,
                    label.empty() ? "OK" : label,
                    "assets/images/ui/btn_beli.png",
                    true});
                hasCustomTargetAction = true;
                continue;
            }

            if (equalsIgnoreCase(key, "cancel") || equalsIgnoreCase(key, "close")) {
                texturePath = "assets/images/ui/btn_cancel.png";
            } else {
                texturePath = "assets/images/ui/btn_beli.png";
            }
        }

        payload.actionItems.push_back(PopupActionItem{
            std::string(kPromptAnswerPrefix) + key,
            label,
            texturePath,
            true});

        if (isAuctionPrompt && key == "BID_MIN" && !hasCustomBidAction) {
            payload.actionItems.push_back(PopupActionItem{
                std::string(kPromptAnswerPrefix) + kPromptCustomBidKey,
                "BID ANGKA",
                "assets/images/ui/btn_beli.png",
                true});
            hasCustomBidAction = true;
        }
    }

    if (m_promptWantsTextInput && !hasCustomTargetAction) {
        payload.actionItems.push_back(PopupActionItem{
            std::string(kPromptAnswerPrefix) + kPromptCustomTargetKey,
            "OK",
            "assets/images/ui/btn_beli.png",
            true});
    }

    if (payload.actionItems.empty()) {
        payload.actionItems.push_back(
            PopupActionItem{std::string(kPromptAnswerPrefix), "OK", "assets/images/ui/btn_cancel.png", true});
    }

    return payload;
}

void DynamicPopupBox::hide() {
    m_isVisible = false;
    m_isMinimized = false;
    m_promptWantsBidInput = false;
    m_promptWantsTextInput = false;
    m_promptTextInputForceUppercase = false;
    m_bidInputValue.clear();
    m_bidInputText.setString("");
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
    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
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

    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
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

    if (m_promptWantsBidInput || m_promptWantsTextInput) {
        if (m_bidInputBox.getGlobalBounds().contains(mousePos)) {
            m_bidInputBox.setOutlineColor(sf::Color(226, 188, 102));
        } else {
            m_bidInputBox.setOutlineColor(sf::Color(181, 164, 128));
        }
    }
}

bool DynamicPopupBox::handleTextEntered(char32_t unicode) {
    if (!m_isVisible || m_isMinimized || (!m_promptWantsBidInput && !m_promptWantsTextInput)) {
        return false;
    }

    if (unicode == 8) {
        if (!m_bidInputValue.empty()) {
            m_bidInputValue.pop_back();
        }
    } else if (m_promptWantsBidInput) {
        if (unicode >= '0' && unicode <= '9' && m_bidInputValue.size() < 9) {
            m_bidInputValue.push_back(static_cast<char>(unicode));
        } else {
            return false;
        }
    } else if (m_promptWantsTextInput) {
        const char ch = static_cast<char>(unicode);
        bool allowed = false;
        if (m_promptTextInputForceUppercase) {
            allowed = std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-';
        } else {
            allowed = std::isalnum(static_cast<unsigned char>(ch)) ||
                      ch == '_' || ch == '-' || ch == '.' || ch == '/' || ch == '\\';
        }
        if (!allowed) {
            return false;
        }
        if (m_bidInputValue.size() < 64) {
            if (m_promptTextInputForceUppercase) {
                m_bidInputValue.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            } else {
                m_bidInputValue.push_back(ch);
            }
        }
    } else {
        return false;
    }

    if (m_bidInputValue.empty()) {
        m_bidInputText.setString("");
    } else {
        if (m_promptWantsBidInput) {
            m_bidInputText.setString("M" + m_bidInputValue);
        } else {
            m_bidInputText.setString(m_bidInputValue);
        }
    }

    return true;
}

bool DynamicPopupBox::handleKeyPressed(sf::Keyboard::Key key) {
    if (!m_isVisible || m_isMinimized || (!m_promptWantsBidInput && !m_promptWantsTextInput)) {
        return false;
    }

    if (key == sf::Keyboard::BackSpace) {
        if (!m_bidInputValue.empty()) {
            m_bidInputValue.pop_back();
            if (m_bidInputValue.empty()) {
                m_bidInputText.setString("");
            } else {
                if (m_promptWantsBidInput) {
                    m_bidInputText.setString("M" + m_bidInputValue);
                } else {
                    m_bidInputText.setString(m_bidInputValue);
                }
            }
        }
        return true;
    }

    if (key == sf::Keyboard::Enter || key == sf::Keyboard::Return) {
        if (!m_bidInputValue.empty()) {
            if (m_promptWantsBidInput) {
                invokeAction(std::string(kPromptAnswerPrefix) + kPromptCustomBidKey);
            } else if (m_promptWantsTextInput) {
                invokeAction(std::string(kPromptAnswerPrefix) + kPromptCustomTargetKey);
            }
        }
        return true;
    }

    return false;
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

    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
        return true;
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

    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
        return true;
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

        if (pressedIndex < 0 || pressedIndex >= static_cast<int>(m_actionSprites.size())) {
            return false;
        }

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

    if (m_mode == PopupMode::FULL_IMAGE_DISMISSABLE) {
        window.draw(m_fullImageSprite);
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

    if (m_promptWantsBidInput || m_promptWantsTextInput) {
        window.draw(m_bidInputBox);
        if (!m_bidInputValue.empty()) {
            window.draw(m_bidInputText);
        } else {
            window.draw(m_bidInputHint);
        }
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
    const float buttonsAreaTop = cardPos.y;
    const float buttonsAreaHeight = cardSize.y;
    const float btnWidth = PopupLayout::kButtonWidth;
    float btnHeight = PopupLayout::kButtonHeight;
    float gapY = PopupLayout::kButtonGapY;
    const float areaHeight = buttonsAreaHeight;
    const float actionCount = static_cast<float>(m_actionSprites.size());
    if (actionCount > 0.0f) {
        const float defaultBlock = (actionCount * btnHeight) + ((actionCount - 1.0f) * gapY);
        if (defaultBlock > areaHeight) {
            const float minGap = 1.0f;
            const float preferredGap = 6.0f;
            gapY = std::min(preferredGap, std::max(minGap, areaHeight / (actionCount * 8.0f)));
            btnHeight = (areaHeight - ((actionCount - 1.0f) * gapY)) / actionCount;

            // If still too small, force exact fit without gap to avoid any overflow.
            if (btnHeight < 16.0f) {
                gapY = 0.0f;
                btnHeight = std::max(8.0f, areaHeight / actionCount);
            } else if (actionCount > 1.0f) {
                gapY = std::max(0.0f, (areaHeight - (actionCount * btnHeight)) / (actionCount - 1.0f));
            }
        }
    }
    const float blockHeight = (m_actionSprites.empty())
                                  ? 0.0f
                                  : (static_cast<float>(m_actionSprites.size()) * btnHeight) +
                                        (static_cast<float>(m_actionSprites.size() - 1) * gapY);
    const float btnStartY = buttonsAreaTop +
                            std::max(0.0f, (areaHeight - blockHeight) * 0.5f);

    for (size_t i = 0; i < m_actionSprites.size(); ++i) {
        SpriteAction& action = m_actionSprites[i];
        const sf::Vector2u texSize = action.texture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            action.sprite.setScale(btnWidth / static_cast<float>(texSize.x),
                                   btnHeight / static_cast<float>(texSize.y));
        }

        const unsigned int targetLabelSize = static_cast<unsigned int>(
            std::max(9.0f, std::min(26.0f, btnHeight * 0.38f)));
        if (action.label.getCharacterSize() != targetLabelSize) {
            action.label.setCharacterSize(targetLabelSize);
        }

        const float y = btnStartY + static_cast<float>(i) * (btnHeight + gapY);
        action.sprite.setPosition(btnX, y);

        const sf::FloatRect labelBounds = action.label.getLocalBounds();
        action.label.setOrigin(labelBounds.left + labelBounds.width * 0.5f,
                               labelBounds.top + labelBounds.height * 0.5f);
        action.label.setPosition(btnX + btnWidth * 0.5f, y + btnHeight * 0.5f - 2.0f);
    }

    m_bidInputBox.setPosition(btnX, buttonsAreaTop + buttonsAreaHeight + 8.0f);
    m_bidInputText.setPosition(btnX + 14.0f,
                               buttonsAreaTop + buttonsAreaHeight + 20.0f);
    m_bidInputHint.setPosition(btnX + 14.0f,
                               buttonsAreaTop + buttonsAreaHeight + 23.0f);

    updateActionVisuals();
}

void DynamicPopupBox::layoutFullImage() {
    const sf::Vector2u texSize = m_fullImageTexture.getSize();
    if (texSize.x == 0 || texSize.y == 0) {
        return;
    }

    const float maxWidth = m_windowSize.x * 0.92f;
    const float maxHeight = m_windowSize.y * 0.92f;
    const float sx = maxWidth / static_cast<float>(texSize.x);
    const float sy = maxHeight / static_cast<float>(texSize.y);
    const float scale = std::min(sx, sy) * 0.5f;

    m_fullImageSprite.setOrigin(static_cast<float>(texSize.x) * 0.5f,
                                static_cast<float>(texSize.y) * 0.5f);
    m_fullImageSprite.setScale(scale, scale);
    m_fullImageSprite.setPosition(m_windowSize.x * 0.5f, m_windowSize.y * 0.5f);
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

    // Use smaller font in property mode so description fits below rent rows
    const bool hasPropertyRows = (m_payload.purchasePrice > 0) || !m_payload.rentPrices.empty();
    m_descriptionText.setCharacterSize(hasPropertyRows ? 20u : 24u);
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
        // Keep popup buttons visually uniform (white style) to avoid mixed/black button artifacts.
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
            action.sprite.setColor(sf::Color(220, 220, 220));
            action.label.setFillColor(sf::Color(120, 120, 120));
            continue;
        }

        action.sprite.setColor(sf::Color::White);
        action.label.setFillColor((action.pressed || action.hovered)
            ? sf::Color(46, 38, 30)
            : sf::Color(53, 45, 36));
    }
}

void DynamicPopupBox::invokeAction(const std::string& actionId) {
    // Copy callback first. Handler may call hide(), which resets m_onAction.
    appendPopupDebugLog("[DEBUG][PopupInvoke] action=" + actionId);
    ActionCallback callback = m_onAction;
    if (callback) {
        // Keep action id stable even if callback mutates popup internals (e.g. hide()).
        const std::string stableActionId = actionId;
        callback(stableActionId);
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
