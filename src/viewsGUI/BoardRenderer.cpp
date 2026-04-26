#include "../../include/viewsGUI/BoardRenderer.hpp"

#include "../../include/models/Board.hpp"
#include "../../include/models/CardTile.hpp"
#include "../../include/models/FestivalTile.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/TaxTile.hpp"
#include "../../include/viewsGUI/Theme.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>

namespace {
constexpr float kCenterInset = 2.0f;
constexpr float kTileBorderThickness = 1.0f;
const sf::Color kTileBorderColor(52, 42, 26, 145);
constexpr float kCenterBorderThickness = 2.0f;
const sf::Color kCenterBorderColor(52, 42, 26, 190);

constexpr float kStreetHeaderHeight = 18.0f;

// Typography tuning block: adjust text position/sizing from one place.
namespace Typography {
constexpr float kHorizontalTextPadding = 8.0f;
constexpr float kBottomPricePadding = 7.0f;

constexpr float kStreetNameY = 26.0f;
constexpr unsigned int kStreetNameMaxSize = 14;
constexpr unsigned int kStreetNameMinSize = 8;
constexpr unsigned int kStreetPriceMaxSize = 10;
constexpr unsigned int kStreetPriceMinSize = 6;

constexpr float kRailroadNameY = 15.0f;
constexpr float kRailroadSecondLineOffsetY = 16.0f;
constexpr unsigned int kRailroadNameMaxSize = 12;
constexpr unsigned int kRailroadNameMinSize = 5;
constexpr unsigned int kRailroadPriceMaxSize = 10;
constexpr unsigned int kRailroadPriceMinSize = 6;

constexpr unsigned int kUtilityPriceMaxSize = 10;
constexpr unsigned int kUtilityPriceMinSize = 6;

constexpr unsigned int kTaxPriceMaxSize = 10;
constexpr unsigned int kTaxPriceMinSize = 6;
} // namespace Typography
} // namespace

namespace viewsGUI {
BoardRenderer::BoardRenderer(float boardSize, const sf::Font& font, sf::Vector2f origin)
    : m_boardSize(boardSize),
      m_cornerSize(boardSize * (90.0f / 720.0f)),
      m_tileSize((boardSize - (2.0f * (boardSize * (90.0f / 720.0f)))) / 9.0f),
      m_origin(origin),
      m_defaultFont(font),
      m_hasBebasFont(false),
      m_renderCanvasesReady(false) {
    m_hasBebasFont = m_bebasFont.loadFromFile("assets/fonts/BebasNeue-Regular.ttf");
    if (!m_hasBebasFont) {
        std::cerr << "[WARN] Font BebasNeue tidak ditemukan. Fallback ke font default GUI.\n";
    }
}

bool BoardRenderer::loadAssets(const std::string& assetDirectory) {
    bool success = true;
    m_assetBaseDir =
        (!assetDirectory.empty() && assetDirectory.back() == '/') ? assetDirectory
                                                                   : assetDirectory + "/";

    if (!m_centerTexture.loadFromFile(m_assetBaseDir + "center_board.png")) {
        std::cerr << "[ERROR] Gagal memuat " << m_assetBaseDir << "center_board.png\n";
        success = false;
    } else {
        m_centerSprite.setTexture(m_centerTexture);
        m_centerSprite.setPosition(m_origin.x + m_cornerSize + kCenterInset,
                                   m_origin.y + m_cornerSize + kCenterInset);

        const float centerAreaSize = m_boardSize - (2.0f * m_cornerSize);
        const float targetCenterSize = centerAreaSize - (2.0f * kCenterInset);
        const sf::Vector2u centerSize = m_centerTexture.getSize();
        if (centerSize.x > 0 && centerSize.y > 0) {
            m_centerSprite.setScale(targetCenterSize / static_cast<float>(centerSize.x),
                                    targetCenterSize / static_cast<float>(centerSize.y));
        }
    }

    if (!loadAllTileTextures()) {
        success = false;
    }
    if (!initializeRenderCanvases()) {
        success = false;
    }

    return success;
}

bool BoardRenderer::loadTexture(const std::string& key, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "[WARN] Gagal memuat aset tile: " << path << "\n";
        return false;
    }

    m_textureByKey[key] = std::move(texture);
    return true;
}

bool BoardRenderer::loadTexturesFromDirectory(const std::string& directoryPath,
                                              const std::string& keyPrefix) {
    namespace fs = std::filesystem;
    bool success = true;

    std::error_code ec;
    if (!fs::exists(directoryPath, ec)) {
        std::cerr << "[WARN] Folder aset tidak ditemukan: " << directoryPath << "\n";
        return false;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath, ec)) {
        if (ec) {
            std::cerr << "[WARN] Gagal membaca folder aset: " << directoryPath << "\n";
            return false;
        }
        if (!entry.is_regular_file()) {
            continue;
        }

        const fs::path filePath = entry.path();
        if (filePath.extension() != ".png" && filePath.extension() != ".PNG") {
            continue;
        }

        const std::string code = filePath.stem().string();
        if (code.empty()) {
            continue;
        }

        if (!loadTexture(keyPrefix + code, filePath.string())) {
            success = false;
        }
    }

    return success;
}

bool BoardRenderer::loadIconTextures(const std::string& uiDirectory) {
    const std::string uiDir = (!uiDirectory.empty() && uiDirectory.back() == '/')
                                  ? uiDirectory
                                  : uiDirectory + "/";
    const std::vector<std::string> colorNames = {"blue", "red", "yellow", "green"};
    bool success = true;
    for (const auto& c : colorNames) {
        if (!loadTexture("house:" + c, uiDir + c + "house_icon.png")) {
            success = false;
        }
        if (!loadTexture("hotel:" + c, uiDir + c + "hotel_icon.png")) {
            success = false;
        }
    }
    return success;
}

void BoardRenderer::updatePlayerInfo(const std::vector<const void*>& playerPtrs) {
    m_playerIndexMap.clear();
    for (int i = 0; i < static_cast<int>(playerPtrs.size()); ++i) {
        if (playerPtrs[i]) {
            m_playerIndexMap[playerPtrs[i]] = i;
        }
    }
}

std::string BoardRenderer::getPlayerColorName(int playerIndex) const {
    // Keep this order aligned with piece color order in PieceRenderer/MainUI:
    // P1 red, P2 blue, P3 green, P4 yellow.
    static const std::vector<std::string> kColors = {"red", "blue", "green", "yellow"};
    if (playerIndex < 0 || playerIndex >= static_cast<int>(kColors.size())) {
        return "red";
    }
    return kColors[static_cast<size_t>(playerIndex)];
}

void BoardRenderer::drawBuildingIcons(sf::RenderTexture& tileCanvas,
                                      const std::string& textureKey,
                                      int count,
                                      const sf::Vector2f& logicalSize) const {
    const sf::Texture* iconTex = getTexture(textureKey);
    if (!iconTex || count <= 0) {
        return;
    }

    constexpr float kIconH  = 11.0f;
    constexpr float kIconGap = 2.0f;
    const sf::Vector2u texSize = iconTex->getSize();
    if (texSize.x == 0 || texSize.y == 0) {
        return;
    }
    const float iconW = kIconH * static_cast<float>(texSize.x) / static_cast<float>(texSize.y);
    const float totalW = static_cast<float>(count) * iconW +
                         static_cast<float>(count - 1) * kIconGap;
    float startX = (logicalSize.x - totalW) / 2.0f;
    constexpr float kIconY = 56.0f;

    for (int i = 0; i < count; ++i) {
        sf::Sprite sprite(*iconTex);
        sprite.setScale(iconW / static_cast<float>(texSize.x),
                        kIconH / static_cast<float>(texSize.y));
        sprite.setPosition(startX + static_cast<float>(i) * (iconW + kIconGap), kIconY);
        tileCanvas.draw(sprite);
    }
}

bool BoardRenderer::loadAllTileTextures() {
    bool success = true;

    if (!loadTexturesFromDirectory(m_assetBaseDir + "Property/Lahan", "street:")) {
        success = false;
    }
    if (getTexture("street:DPK") == nullptr) {
        std::cerr << "[WARN] Fallback texture street:DPK tidak ditemukan.\n";
        success = false;
    }

    if (!loadTexturesFromDirectory(m_assetBaseDir + "Property/Utilitas", "utility:")) {
        success = false;
    }

    if (!loadTexture("railroad", m_assetBaseDir + "Property/RAILROAD.png")) {
        success = false;
    }

    if (!loadTexture("tax:PPH", m_assetBaseDir + "Action/Pajak/PPH.png")) {
        success = false;
    }
    if (!loadTexture("tax:PBM", m_assetBaseDir + "Action/Pajak/PPNBM.png")) {
        success = false;
    }

    if (!loadTexture("card:DNU", m_assetBaseDir + "Action/Kartu/DANA_UMUM.png")) {
        success = false;
    }
    if (!loadTexture("card:KSP", m_assetBaseDir + "Action/Kartu/KESEMPATAN.png")) {
        success = false;
    }
    if (!loadTexture("festival", m_assetBaseDir + "Action/FESTIVAL.png")) {
        success = false;
    }

    if (!loadTexture("special:GO", m_assetBaseDir + "Action/Spesial/GO.png")) {
        success = false;
    }
    if (!loadTexture("special:PEN", m_assetBaseDir + "Action/Spesial/PEN.png")) {
        success = false;
    }
    if (!loadTexture("special:BBP", m_assetBaseDir + "Action/Spesial/BBP.png")) {
        success = false;
    }
    if (!loadTexture("special:PPJ", m_assetBaseDir + "Action/Spesial/PPJ.png")) {
        success = false;
    }

    return success;
}

bool BoardRenderer::initializeRenderCanvases() {
    const bool portraitOk = m_portraitCanvas.create(static_cast<unsigned int>(std::round(m_tileSize)),
                                                    static_cast<unsigned int>(std::round(m_cornerSize)));
    const bool cornerOk = m_cornerCanvas.create(static_cast<unsigned int>(std::round(m_cornerSize)),
                                                static_cast<unsigned int>(std::round(m_cornerSize)));
    m_renderCanvasesReady = portraitOk && cornerOk;
    if (!m_renderCanvasesReady) {
        std::cerr << "[ERROR] Gagal membuat render canvas tile GUI.\n";
    }
    return m_renderCanvasesReady;
}

const sf::Texture* BoardRenderer::getTexture(const std::string& key) const {
    const auto it = m_textureByKey.find(key);
    if (it == m_textureByKey.end()) {
        return nullptr;
    }
    return &it->second;
}

const sf::Texture* BoardRenderer::getStreetTextureWithFallback(const std::string& code) const {
    const sf::Texture* exact = getTexture("street:" + code);
    if (exact) {
        return exact;
    }
    return getTexture("street:DPK");
}

const sf::Texture* BoardRenderer::resolveBaseTexture(const Tile& tile) const {
    const std::string code = tile.getCode();

    if (const auto* propertyTile = dynamic_cast<const PropertyTile*>(&tile)) {
        const Property& property = propertyTile->getProperty();
        if (property.getType() == PropertyType::STREET) {
            return getStreetTextureWithFallback(property.getCode());
        }
        if (property.getType() == PropertyType::RAILROAD) {
            return getTexture("railroad");
        }
        if (property.getType() == PropertyType::UTILITY) {
            return getTexture("utility:" + property.getCode());
        }
    }

    if (dynamic_cast<const TaxTile*>(&tile)) {
        return getTexture(code == "PBM" ? "tax:PBM" : "tax:PPH");
    }

    if (dynamic_cast<const CardTile*>(&tile)) {
        return getTexture(code == "KSP" ? "card:KSP" : "card:DNU");
    }

    if (dynamic_cast<const FestivalTile*>(&tile)) {
        return getTexture("festival");
    }

    if (code == "GO" || code == "PEN" || code == "BBP" || code == "PPJ") {
        return getTexture("special:" + code);
    }

    return nullptr;
}

sf::Vector2f BoardRenderer::getTilePosition(int index) const {
    if (index == 0) return {m_origin.x + m_boardSize - m_cornerSize, m_origin.y + m_boardSize - m_cornerSize};
    if (index > 0 && index < 10)
        return {m_origin.x + m_boardSize - m_cornerSize - (index * m_tileSize), m_origin.y + m_boardSize - m_cornerSize};
    if (index == 10) return {m_origin.x + 0.0f, m_origin.y + m_boardSize - m_cornerSize};
    if (index > 10 && index < 20)
        return {m_origin.x + 0.0f, m_origin.y + m_boardSize - m_cornerSize - ((index - 10) * m_tileSize)};
    if (index == 20) return {m_origin.x + 0.0f, m_origin.y + 0.0f};
    if (index > 20 && index < 30)
        return {m_origin.x + m_cornerSize + ((index - 21) * m_tileSize), m_origin.y + 0.0f};
    if (index == 30) return {m_origin.x + m_boardSize - m_cornerSize, m_origin.y + 0.0f};
    if (index > 30 && index < 40)
        return {m_origin.x + m_boardSize - m_cornerSize, m_origin.y + m_cornerSize + ((index - 31) * m_tileSize)};
    return {m_origin.x + 0.0f, m_origin.y + 0.0f};
}

sf::Vector2f BoardRenderer::getTileSize(int index) const {
    if (index % 10 == 0) return {m_cornerSize, m_cornerSize};
    if ((index > 0 && index < 10) || (index > 20 && index < 30)) return {m_tileSize, m_cornerSize};
    return {m_cornerSize, m_tileSize};
}

sf::Vector2f BoardRenderer::getTileCenter(int index) const {
    const sf::Vector2f pos = getTilePosition(index);
    const sf::Vector2f size = getTileSize(index);
    return {pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f)};
}

sf::Vector2f BoardRenderer::getBoardCenter() const {
    return {m_origin.x + (m_boardSize / 2.0f), m_origin.y + (m_boardSize / 2.0f)};
}

bool BoardRenderer::isCornerIndex(int index) const {
    return index % 10 == 0;
}

float BoardRenderer::getTileRotation(int index) const {
    if (isCornerIndex(index)) {
        return 0.0f;
    }
    if (index > 0 && index < 10) {
        return 0.0f;
    }
    if (index > 10 && index < 20) {
        return 90.0f;
    }
    if (index > 20 && index < 30) {
        return 180.0f;
    }
    if (index > 30 && index < 40) {
        return -90.0f;
    }
    return 0.0f;
}

BoardRenderer::TileRenderInfo BoardRenderer::buildTileRenderInfo(int index) const {
    TileRenderInfo info;
    info.worldPosition = getTilePosition(index);
    info.worldSize = getTileSize(index);
    info.rotationDeg = getTileRotation(index);
    info.isCorner = isCornerIndex(index);
    info.logicalSize = info.isCorner ? info.worldSize : sf::Vector2f(m_tileSize, m_cornerSize);
    return info;
}

sf::Color BoardRenderer::resolveStreetColor(const std::string& colorGroup) const {
    if (colorGroup == "COKLAT" || colorGroup == "CK") return Theme::Coklat;
    if (colorGroup == "BIRU_MUDA" || colorGroup == "BM") return Theme::BiruMuda;
    if (colorGroup == "MERAH_MUDA" || colorGroup == "PK") return Theme::Pink;
    if (colorGroup == "ORANGE" || colorGroup == "OR") return Theme::Oranye;
    if (colorGroup == "MERAH" || colorGroup == "MR") return Theme::Merah;
    if (colorGroup == "KUNING" || colorGroup == "KN") return Theme::Kuning;
    if (colorGroup == "HIJAU" || colorGroup == "HJ") return Theme::Hijau;
    if (colorGroup == "BIRU_TUA" || colorGroup == "BT") return Theme::BiruTua;
    return sf::Color::Transparent;
}

std::string BoardRenderer::normalizeDisplayText(const std::string& raw) const {
    std::string out = raw;
    std::replace(out.begin(), out.end(), '_', ' ');
    return out;
}

std::string BoardRenderer::toRailroadTwoLines(const std::string& raw) const {
    const std::string normalized = normalizeDisplayText(raw);
    const std::string prefix = "STASIUN ";
    if (normalized.rfind(prefix, 0) == 0 && normalized.size() > prefix.size()) {
        return "STASIUN\n" + normalized.substr(prefix.size());
    }
    return normalized;
}

unsigned int BoardRenderer::calculateAutoFontSize(const std::string& text,
                                                  float maxWidth,
                                                  unsigned int maxSize,
                                                  unsigned int minSize) const {
    const sf::Font& font = m_hasBebasFont ? m_bebasFont : m_defaultFont;
    for (unsigned int size = maxSize; size >= minSize; --size) {
        sf::Text preview(text, font, size);
        const float width = preview.getLocalBounds().width;
        if (width <= maxWidth || size == minSize) {
            return size;
        }
    }
    return minSize;
}

void BoardRenderer::drawCenteredText(sf::RenderTarget& target,
                                     const std::string& text,
                                     float centerX,
                                     float centerY,
                                     float maxWidth,
                                     unsigned int maxSize,
                                     unsigned int minSize,
                                     const sf::Color& color,
                                     float rotation) const {
    const sf::Font& font = m_hasBebasFont ? m_bebasFont : m_defaultFont;
    const unsigned int characterSize = calculateAutoFontSize(text, maxWidth, maxSize, minSize);

    sf::Text drawText(text, font, characterSize);
    drawText.setFillColor(color);

    const sf::FloatRect bounds = drawText.getLocalBounds();
    drawText.setOrigin(bounds.left + (bounds.width / 2.0f), bounds.top + (bounds.height / 2.0f));
    drawText.setPosition(centerX, centerY);
    drawText.setRotation(rotation);

    target.draw(drawText);
}

void BoardRenderer::drawStreetTileContent(sf::RenderTexture& tileCanvas,
                                          const Property& property,
                                          const sf::Vector2f& logicalSize) const {
    const auto* street = dynamic_cast<const StreetProperty*>(&property);
    std::string colorGroup;
    if (street) {
        colorGroup = street->getColorGroup();
    }

    const sf::Color headerColor = resolveStreetColor(colorGroup);
    if (headerColor != sf::Color::Transparent) {
        sf::RectangleShape header({logicalSize.x, kStreetHeaderHeight});
        header.setPosition(0.0f, 0.0f);
        header.setFillColor(headerColor);
        header.setOutlineThickness(1.0f);
        header.setOutlineColor(sf::Color(0, 0, 0, 95));
        tileCanvas.draw(header);
    }

    drawCenteredText(tileCanvas,
                     normalizeDisplayText(property.getName()),
                     logicalSize.x / 2.0f,
                     Typography::kStreetNameY,
                     logicalSize.x - Typography::kHorizontalTextPadding,
                     Typography::kStreetNameMaxSize,
                     Typography::kStreetNameMinSize,
                     Theme::TextDark);

    drawCenteredText(tileCanvas,
                     std::to_string(property.getPurchasePrice()),
                     logicalSize.x / 2.0f,
                     logicalSize.y - Typography::kBottomPricePadding,
                     logicalSize.x - Typography::kHorizontalTextPadding,
                     Typography::kStreetPriceMaxSize,
                     Typography::kStreetPriceMinSize,
                     Theme::TextDark);

    // Draw house/hotel icons if buildings exist
    if (street && street->getBuildingLevel() != BuildingLevel::NONE) {
        const void* ownerPtr = static_cast<const void*>(property.getOwner());
        auto it = m_playerIndexMap.find(ownerPtr);
        const int playerIdx = (it != m_playerIndexMap.end()) ? it->second : 0;
        const std::string colorName = getPlayerColorName(playerIdx);

        if (street->getBuildingLevel() == BuildingLevel::HOTEL) {
            drawBuildingIcons(tileCanvas, "hotel:" + colorName, 1, logicalSize);
        } else {
            const int houseCount = street->getBuildingCount();
            drawBuildingIcons(tileCanvas, "house:" + colorName, houseCount, logicalSize);
        }
    }
}

void BoardRenderer::drawRailroadTileContent(sf::RenderTexture& tileCanvas,
                                            const Property& property,
                                            const sf::Vector2f& logicalSize) const {
    const std::string railroadName = toRailroadTwoLines(property.getName());
    const size_t splitPos = railroadName.find('\n');

    if (splitPos != std::string::npos) {
        const std::string firstLine = railroadName.substr(0, splitPos);
        const std::string secondLine = railroadName.substr(splitPos + 1);

        drawCenteredText(tileCanvas,
                         firstLine,
                         logicalSize.x / 2.0f,
                         Typography::kRailroadNameY,
                         logicalSize.x - Typography::kHorizontalTextPadding,
                         Typography::kRailroadNameMaxSize,
                         Typography::kRailroadNameMinSize,
                         Theme::TextDark);

        drawCenteredText(tileCanvas,
                         secondLine,
                         logicalSize.x / 2.0f,
                         Typography::kRailroadNameY + Typography::kRailroadSecondLineOffsetY,
                         logicalSize.x - Typography::kHorizontalTextPadding,
                         Typography::kRailroadNameMaxSize,
                         Typography::kRailroadNameMinSize,
                         Theme::TextDark);
    } else {
        drawCenteredText(tileCanvas,
                         railroadName,
                         logicalSize.x / 2.0f,
                         Typography::kRailroadNameY,
                         logicalSize.x - Typography::kHorizontalTextPadding,
                         Typography::kRailroadNameMaxSize,
                         Typography::kRailroadNameMinSize,
                         Theme::TextDark);
    }

    drawCenteredText(tileCanvas,
                     std::to_string(property.getPurchasePrice()),
                     logicalSize.x / 2.0f,
                     logicalSize.y - Typography::kBottomPricePadding,
                     logicalSize.x - Typography::kHorizontalTextPadding,
                     Typography::kRailroadPriceMaxSize,
                     Typography::kRailroadPriceMinSize,
                     Theme::TextDark);

    // Ownership marker for railroad: 1 house icon in owner color.
    if (property.getOwner() != nullptr) {
        const void* ownerPtr = static_cast<const void*>(property.getOwner());
        auto it = m_playerIndexMap.find(ownerPtr);
        const int playerIdx = (it != m_playerIndexMap.end()) ? it->second : 0;
        const std::string colorName = getPlayerColorName(playerIdx);
        drawBuildingIcons(tileCanvas, "house:" + colorName, 1, logicalSize);
    }
}

void BoardRenderer::drawUtilityTileContent(sf::RenderTexture& tileCanvas,
                                           const Property& property,
                                           const sf::Vector2f& logicalSize) const {
    drawCenteredText(tileCanvas,
                     std::to_string(property.getPurchasePrice()),
                     logicalSize.x / 2.0f,
                     logicalSize.y - Typography::kBottomPricePadding,
                     logicalSize.x - Typography::kHorizontalTextPadding,
                     Typography::kUtilityPriceMaxSize,
                     Typography::kUtilityPriceMinSize,
                     Theme::TextDark);

    // Ownership marker for utility: 1 house icon in owner color.
    if (property.getOwner() != nullptr) {
        const void* ownerPtr = static_cast<const void*>(property.getOwner());
        auto it = m_playerIndexMap.find(ownerPtr);
        const int playerIdx = (it != m_playerIndexMap.end()) ? it->second : 0;
        const std::string colorName = getPlayerColorName(playerIdx);
        drawBuildingIcons(tileCanvas, "house:" + colorName, 1, logicalSize);
    }
}

void BoardRenderer::drawTaxTileContent(sf::RenderTexture& tileCanvas,
                                       const TaxTile& taxTile,
                                       const sf::Vector2f& logicalSize) const {
    drawCenteredText(tileCanvas,
                     std::to_string(taxTile.getFlatAmount()),
                     logicalSize.x / 2.0f,
                     logicalSize.y - Typography::kBottomPricePadding,
                     logicalSize.x - Typography::kHorizontalTextPadding,
                     Typography::kTaxPriceMaxSize,
                     Typography::kTaxPriceMinSize,
                     Theme::TextDark);
}

void BoardRenderer::drawTileContent(sf::RenderTexture& tileCanvas,
                                    const Tile& tile,
                                    const sf::Vector2f& logicalSize) const {
    const sf::Texture* baseTexture = resolveBaseTexture(tile);
    if (baseTexture) {
        sf::Sprite sprite(*baseTexture);
        const sf::Vector2u texSize = baseTexture->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            sprite.setScale(logicalSize.x / static_cast<float>(texSize.x),
                            logicalSize.y / static_cast<float>(texSize.y));
        }
        tileCanvas.draw(sprite);
    } else {
        sf::RectangleShape placeholder(logicalSize);
        placeholder.setFillColor(sf::Color(240, 228, 198));
        tileCanvas.draw(placeholder);
    }

    if (const auto* propertyTile = dynamic_cast<const PropertyTile*>(&tile)) {
        const Property& property = propertyTile->getProperty();
        if (property.getType() == PropertyType::STREET) {
            drawStreetTileContent(tileCanvas, property, logicalSize);
        } else if (property.getType() == PropertyType::RAILROAD) {
            drawRailroadTileContent(tileCanvas, property, logicalSize);
        } else if (property.getType() == PropertyType::UTILITY) {
            drawUtilityTileContent(tileCanvas, property, logicalSize);
        }
        return;
    }

    if (const auto* taxTile = dynamic_cast<const TaxTile*>(&tile)) {
        drawTaxTileContent(tileCanvas, *taxTile, logicalSize);
        return;
    }
}

void BoardRenderer::drawTileBorder(sf::RenderWindow& window, int index) const {
    const sf::Vector2f tilePos = getTilePosition(index);
    const sf::Vector2f tileSize = getTileSize(index);

    sf::RectangleShape tileBorder({tileSize.x - 1.0f, tileSize.y - 1.0f});
    tileBorder.setPosition(tilePos.x + 0.5f, tilePos.y + 0.5f);
    tileBorder.setFillColor(sf::Color::Transparent);
    tileBorder.setOutlineThickness(kTileBorderThickness);
    tileBorder.setOutlineColor(kTileBorderColor);
    window.draw(tileBorder);
}

void BoardRenderer::drawCenterBorder(sf::RenderWindow& window) const {
    const float centerSize = m_boardSize - (2.0f * m_cornerSize);
    sf::RectangleShape centerBorder({centerSize - 1.0f, centerSize - 1.0f});
    centerBorder.setPosition(m_origin.x + m_cornerSize + 0.5f, m_origin.y + m_cornerSize + 0.5f);
    centerBorder.setFillColor(sf::Color::Transparent);
    centerBorder.setOutlineThickness(kCenterBorderThickness);
    centerBorder.setOutlineColor(kCenterBorderColor);
    window.draw(centerBorder);
}

void BoardRenderer::drawTile(sf::RenderWindow& window, const Tile& tile, int index) const {
    const TileRenderInfo info = buildTileRenderInfo(index);
    if (!m_renderCanvasesReady) {
        drawTileBorder(window, index);
        return;
    }

    sf::RenderTexture& tileCanvas = info.isCorner ? m_cornerCanvas : m_portraitCanvas;
    tileCanvas.clear(sf::Color::Transparent);
    drawTileContent(tileCanvas, tile, info.logicalSize);
    tileCanvas.display();

    sf::Sprite tileSprite(tileCanvas.getTexture());
    tileSprite.setOrigin(info.logicalSize.x / 2.0f, info.logicalSize.y / 2.0f);
    tileSprite.setPosition(info.worldPosition.x + (info.worldSize.x / 2.0f),
                           info.worldPosition.y + (info.worldSize.y / 2.0f));
    tileSprite.setRotation(info.rotationDeg);

    window.draw(tileSprite);
    drawTileBorder(window, index);
}

void BoardRenderer::render(sf::RenderWindow& window, const Board& board) const {
    sf::RectangleShape boardBg({m_boardSize, m_boardSize});
    boardBg.setPosition(m_origin);
    boardBg.setFillColor(sf::Color(245, 250, 245));
    window.draw(boardBg);
    window.draw(m_centerSprite);

    const int tileCount = std::min(40, board.size());
    for (int index = 0; index < tileCount; ++index) {
        drawTile(window, board.getTileByIndex(index), index);
    }

    for (int index = tileCount; index < 40; ++index) {
        drawTileBorder(window, index);
    }

    drawCenterBorder(window);
}
} // namespace viewsGUI
