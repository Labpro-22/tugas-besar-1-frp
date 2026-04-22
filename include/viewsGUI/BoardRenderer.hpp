#ifndef VIEWSGUI_BOARD_RENDERER_HPP
#define VIEWSGUI_BOARD_RENDERER_HPP

#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Board;
class Tile;
class Property;
class TaxTile;

namespace viewsGUI {
class BoardRenderer {
public:
    BoardRenderer(float boardSize, const sf::Font& font);

    bool loadAssets(const std::string& assetDirectory);

    void render(sf::RenderWindow& window, const Board& board) const;

    sf::Vector2f getTilePosition(int index) const;
    sf::Vector2f getTileSize(int index) const;
    sf::Vector2f getTileCenter(int index) const;

private:
    struct TileRenderInfo {
        sf::Vector2f logicalSize;
        sf::Vector2f worldPosition;
        sf::Vector2f worldSize;
        float rotationDeg;
        bool isCorner;
    };

    float m_boardSize;
    float m_cornerSize;
    float m_tileSize;
    sf::Font m_defaultFont;
    sf::Font m_bebasFont;
    bool m_hasBebasFont;
    std::string m_assetBaseDir;
    sf::Texture m_centerTexture;
    sf::Sprite m_centerSprite;
    std::unordered_map<std::string, sf::Texture> m_textureByKey;
    mutable sf::RenderTexture m_portraitCanvas;
    mutable sf::RenderTexture m_cornerCanvas;
    bool m_renderCanvasesReady;

    bool loadAllTileTextures();
    bool loadTexturesFromDirectory(const std::string& directoryPath,
                                   const std::string& keyPrefix);
    bool initializeRenderCanvases();
    bool loadTexture(const std::string& key, const std::string& path);
    const sf::Texture* getTexture(const std::string& key) const;
    const sf::Texture* getStreetTextureWithFallback(const std::string& code) const;
    const sf::Texture* resolveBaseTexture(const Tile& tile) const;

    TileRenderInfo buildTileRenderInfo(int index) const;
    bool isCornerIndex(int index) const;
    float getTileRotation(int index) const;

    sf::Color resolveStreetColor(const std::string& colorGroup) const;
    std::string normalizeDisplayText(const std::string& raw) const;
    std::string toRailroadTwoLines(const std::string& raw) const;
    unsigned int calculateAutoFontSize(const std::string& text,
                                       float maxWidth,
                                       unsigned int maxSize,
                                       unsigned int minSize) const;

    void drawTile(sf::RenderWindow& window, const Tile& tile, int index) const;
    void drawTileContent(sf::RenderTexture& tileCanvas,
                         const Tile& tile,
                         const sf::Vector2f& logicalSize) const;
    void drawStreetTileContent(sf::RenderTexture& tileCanvas,
                               const Property& property,
                               const sf::Vector2f& logicalSize) const;
    void drawRailroadTileContent(sf::RenderTexture& tileCanvas,
                                 const Property& property,
                                 const sf::Vector2f& logicalSize) const;
    void drawUtilityTileContent(sf::RenderTexture& tileCanvas,
                                const Property& property,
                                const sf::Vector2f& logicalSize) const;
    void drawTaxTileContent(sf::RenderTexture& tileCanvas,
                            const TaxTile& taxTile,
                            const sf::Vector2f& logicalSize) const;
    void drawCenteredText(sf::RenderTarget& target,
                          const std::string& text,
                          float centerX,
                          float centerY,
                          float maxWidth,
                          unsigned int maxSize,
                          unsigned int minSize,
                          const sf::Color& color,
                          float rotation = 0.0f) const;
    void drawTileBorder(sf::RenderWindow& window, int index) const;
    void drawCenterBorder(sf::RenderWindow& window) const;
};
} // namespace viewsGUI

#endif
