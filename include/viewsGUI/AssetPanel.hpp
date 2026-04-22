#ifndef VIEWSGUI_ASSET_PANEL_HPP
#define VIEWSGUI_ASSET_PANEL_HPP

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Player;
class Property;
class SkillCard;

namespace viewsGUI {
class AssetPanel {
public:
    enum class Mode {
        ASSET,
        INVENTORY,
        LOG
    };

    AssetPanel(const sf::Font& titleFont, const sf::Font& bodyFont);

    bool loadAssets(const std::string& uiDir, const std::string& boardDir);
    void setPosition(sf::Vector2f position);

    void setMode(Mode mode);
    Mode getMode() const { return m_mode; }

    void updateData(const Player& currentPlayer, const std::string& systemLog);

    void update(sf::Vector2f mousePos);
    bool handleMouseWheel(float delta, sf::Vector2f mousePos);
    bool handleMousePressed(sf::Vector2f mousePos);
    bool handleMouseReleased(sf::Vector2f mousePos);

    bool hasOpenDetail() const { return m_detailPopupVisible; }

    void render(sf::RenderWindow& window) const;

private:
    struct AssetItem {
        const Property* property;
        std::string code;
        std::string title;
        int purchasePrice;
        int mortgageValue;
        int buildingCount;
    };

    struct InventoryItem {
        std::string typeName;
        std::string description;
        int value;
        int duration;
    };

    sf::Font m_titleFont;
    sf::Font m_bodyFont;

    sf::Texture m_panelAssetTexture;
    sf::Texture m_panelLogTexture;
    sf::Texture m_cardTemplateTexture;
    sf::Texture m_scrollTrackTexture;
    sf::Texture m_scrollThumbTexture;

    sf::Sprite m_panelAssetSprite;
    sf::Sprite m_panelLogSprite;
    sf::Sprite m_scrollTrackSprite;
    sf::Sprite m_scrollThumbSprite;

    sf::Text m_titleText;

    std::unordered_map<std::string, sf::Texture> m_propertyBannerTextures;

    sf::Vector2f m_position;
    sf::Vector2f m_panelSize;
    bool m_hasCardTemplateSprite;
    bool m_hasScrollbarAssets;

    Mode m_mode;
    std::vector<AssetItem> m_assetItems;
    std::vector<InventoryItem> m_inventoryItems;
    std::string m_systemLog;

    float m_scrollOffset;
    float m_maxScrollOffset;
    bool m_draggingScrollbar;
    float m_dragGrabOffsetY;
    int m_pressedItemIndex;

    bool m_detailPopupVisible;
    std::string m_detailTitle;
    std::string m_detailBody;

    static constexpr float kDesignWidth = 1920.0f;
    static constexpr float kDesignHeight = 1080.0f;

    sf::FloatRect getContentRect() const;
    float getCardWidth() const;
    float getCardHeight() const;
    float getCardGapX() const;
    float getCardGapY() const;

    int getCurrentItemCount() const;
    float computeTotalContentHeight() const;
    void clampScroll();
    void updateScrollVisual();

    std::vector<sf::FloatRect> buildItemBounds() const;
    int itemIndexAt(sf::Vector2f mousePos) const;

    void openDetailForItem(int index);

    void loadPropertyBannerDirectory(const std::string& directoryPath);

    void renderAssetOrInventory(sf::RenderWindow& window) const;
    void renderLog(sf::RenderWindow& window) const;
    void renderDetailPopup(sf::RenderWindow& window) const;
};
} // namespace viewsGUI

#endif
