#ifndef VIEWSGUI_DYNAMIC_POPUP_BOX_HPP
#define VIEWSGUI_DYNAMIC_POPUP_BOX_HPP

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <vector>

#include "../core/CommandResult.hpp"

namespace viewsGUI {
enum class PopupMode {
    PROPERTY,
    CHANCE,
    COMMUNITY,
    TAX,
    SPECIAL,
    INFO
};

struct PopupActionItem {
    std::string actionId;
    std::string label;
    std::string texturePath;
    bool enabled = true;
};

struct PopupPayload {
    PopupMode mode = PopupMode::INFO;

    std::string headerTitle;
    std::string cardTitle;
    std::string description;

    bool showStreetColorRibbon = false;
    int purchasePrice = 0;
    std::vector<int> rentPrices;
    sf::Color propertyColor = sf::Color(181, 164, 128);

    std::vector<PopupActionItem> actionItems;

    std::string popupBaseImagePath;
    std::string cardTemplateImagePath;
    std::string minimizeIconImagePath;
    std::string showMenuImagePath;
};

class DynamicPopupBox {
public:
    using ActionCallback = std::function<void(const std::string& actionId)>;

    DynamicPopupBox(sf::Vector2f windowSize, const sf::Font& headerFont, const sf::Font& bodyFont);

    bool loadAssets(const std::string& uiDir);

    void show(const PopupPayload& payload, ActionCallback onAction);
    void showPrompt(const PromptRequest& prompt, ActionCallback onAnswer);
    void hide();
    void minimize();
    void maximize();

    bool isVisible() const { return m_isVisible; }
    bool isMinimized() const { return m_isMinimized; }

    void update(sf::Vector2f mousePos);
    bool handleTextEntered(sf::Uint32 unicode);
    bool handleKeyPressed(sf::Keyboard::Key key);
    bool handleMousePressed(sf::Vector2f mousePos);
    bool handleMouseReleased(sf::Vector2f mousePos);
    void render(sf::RenderWindow& window) const;

private:
    struct SpriteAction {
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Text label;
        std::string actionId;
        bool enabled = true;
        bool hovered = false;
        bool pressed = false;
    };

    bool m_isVisible;
    bool m_isMinimized;

    sf::Vector2f m_windowSize;
    sf::Font m_headerFont;
    sf::Font m_bodyFont;

    PopupMode m_mode;
    PopupPayload m_payload;
    ActionCallback m_onAction;

    std::string m_uiDir;

    sf::Texture m_popupBaseTexture;
    sf::Sprite m_popupBaseSprite;
    sf::Texture m_cardTemplateTexture;
    sf::Sprite m_cardTemplateSprite;
    sf::Texture m_minimizeIconTexture;
    sf::Sprite m_minimizeIconSprite;
    sf::Texture m_showMenuTexture;
    sf::Sprite m_showMenuSprite;

    sf::RectangleShape m_propertyRibbon;
    sf::Text m_headerText;
    sf::Text m_cardTitleText;
    sf::Text m_priceText;
    std::vector<sf::Text> m_rentTexts;
    sf::Text m_descriptionText;
    sf::Text m_showMenuText;
    sf::RectangleShape m_bidInputBox;
    sf::Text m_bidInputText;
    sf::Text m_bidInputHint;

    std::vector<SpriteAction> m_actionSprites;

    bool m_promptWantsBidInput;
    bool m_promptWantsTextInput;
    std::string m_bidInputValue;

    int m_pressedActionIndex;
    bool m_pressedMinimize;
    bool m_pressedShowMenu;

    static constexpr float kPopupWidth = 900.0f;
    static constexpr float kPopupHeight = 843.14f;
    static constexpr float kMinimizedWidth = 220.0f;
    static constexpr float kMinimizedHeight = 72.0f;

    void layoutExpanded();
    void layoutMinimized();
    PopupPayload buildFromPrompt(const PromptRequest& prompt);
    void rebuildCardTexts();
    void rebuildActionSprites();
    void updateActionVisuals();
    void invokeAction(const std::string& actionId);

    bool loadTextureWithFallback(sf::Texture& texture, const std::vector<std::string>& paths) const;
    static void buildSolidTexture(sf::Texture& texture,
                                  unsigned int width,
                                  unsigned int height,
                                  sf::Color color);

    bool containsExpanded(sf::Vector2f point) const;
    bool containsMinimizeIcon(sf::Vector2f point) const;
    bool containsShowMenuButton(sf::Vector2f point) const;
    int actionIndexAt(sf::Vector2f point) const;
};
} // namespace viewsGUI

#endif
