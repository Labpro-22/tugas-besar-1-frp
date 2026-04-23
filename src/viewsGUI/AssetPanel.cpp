#include "../../include/viewsGUI/AssetPanel.hpp"

#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/SkillCard.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/viewsGUI/Theme.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace viewsGUI {
namespace {
constexpr unsigned int kLogCharSize = 24U;
constexpr float kLogLineSpacing = 1.15f;

std::string toOwnershipStatusText(const Property& property) {
    if (property.isMortgaged()) {
        return "MORTGAGED [M]";
    }
    if (property.isOwned()) {
        return "OWNED";
    }
    return "BANK";
}

std::string camelCaseToSnakeCase(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);

    for (size_t i = 0; i < value.size(); ++i) {
        const unsigned char uc = static_cast<unsigned char>(value[i]);
        const bool isUpper = std::isupper(uc) != 0;
        if (isUpper && i > 0) {
            const unsigned char prev = static_cast<unsigned char>(value[i - 1]);
            const bool prevIsLowerOrDigit = (std::islower(prev) != 0) || (std::isdigit(prev) != 0);
            const bool nextIsLower =
                (i + 1 < value.size()) && (std::islower(static_cast<unsigned char>(value[i + 1])) != 0);
            if (prevIsLowerOrDigit || nextIsLower) {
                out.push_back('_');
            }
        }
        out.push_back(static_cast<char>(std::tolower(uc)));
    }

    return out;
}

sf::Color resolveStreetColor(const std::string& colorGroup) {
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

void fitTextToMaxWidth(sf::Text& text, float maxWidth, unsigned int minCharacterSize = 9U) {
    if (maxWidth <= 0.0f) {
        return;
    }

    text.setScale(1.0f, 1.0f);
    unsigned int charSize = text.getCharacterSize();
    if (charSize == 0U) {
        return;
    }

    while (charSize > minCharacterSize && text.getLocalBounds().width > maxWidth) {
        --charSize;
        text.setCharacterSize(charSize);
    }

    const float widthAfterResize = text.getLocalBounds().width;
    if (widthAfterResize > maxWidth && widthAfterResize > 0.0f) {
        const float uniformScale = maxWidth / widthAfterResize;
        text.setScale(uniformScale, uniformScale);
    }
}

const sf::Texture* resolvePropertyTexture(const Property* property,
                                          const std::string& code,
                                          const std::unordered_map<std::string, sf::Texture>& textures) {
    if (!property) {
        return nullptr;
    }

    if (property->getType() == PropertyType::RAILROAD) {
        auto itRail = textures.find("RAILROAD");
        if (itRail != textures.end()) {
            return &itRail->second;
        }
    }

    auto itExact = textures.find(code);
    if (itExact != textures.end()) {
        return &itExact->second;
    }

    return nullptr;
}

std::string toBuildingText(const Property* property) {
    const auto* street = dynamic_cast<const StreetProperty*>(property);
    if (!street) {
        return "";
    }

    if (street->getBuildingLevel() == BuildingLevel::HOTEL) {
        return "Hotel";
    }

    const int buildingCount = street->getBuildingCount();
    if (buildingCount <= 0) {
        return "";
    }
    return std::to_string(buildingCount) + " rumah";
}

std::string toRailroadTwoLines(std::string rawName) {
    std::replace(rawName.begin(), rawName.end(), '_', ' ');
    const std::string prefix = "STASIUN ";
    if (rawName.rfind(prefix, 0) == 0 && rawName.size() > prefix.size()) {
        return "STASIUN\n" + rawName.substr(prefix.size());
    }
    return rawName;
}

std::string toStreetDisplayName(std::string rawName) {
    std::replace(rawName.begin(), rawName.end(), '_', ' ');
    return rawName;
}
} // namespace

AssetPanel::AssetPanel(const sf::Font& titleFont, const sf::Font& bodyFont)
    : m_titleFont(titleFont),
      m_bodyFont(bodyFont),
      m_position(0.0f, 0.0f),
      m_panelSize(820.0f, 548.0f),
      m_hasCardTemplateSprite(false),
      m_detailTemplateLoadAttempted(false),
      m_hasScrollbarAssets(false),
      m_uiAssetBaseDir(""),
      m_mode(Mode::ASSET),
      m_wrappedLogText(""),
      m_currentPlayerName(""),
      m_assetTotalValue(0),
      m_scrollOffset(0.0f),
      m_maxScrollOffset(0.0f),
      m_draggingScrollbar(false),
      m_dragGrabOffsetY(0.0f),
      m_pressedItemIndex(-1),
      m_logScrollOffset(0.0f),
      m_logMaxScrollOffset(0.0f),
      m_detailPopupVisible(false) {
    m_titleText.setFont(m_titleFont);
    m_titleText.setCharacterSize(68);
    m_titleText.setFillColor(sf::Color(240, 239, 229));
    refreshPanelTitle();
    refreshWrappedLog();
}

bool AssetPanel::loadAssets(const std::string& uiDir, const std::string& boardDir) {
    bool success = true;

    const std::string baseUi = (!uiDir.empty() && uiDir.back() == '/') ? uiDir : uiDir + "/";
    const std::string baseBoard = (!boardDir.empty() && boardDir.back() == '/') ? boardDir : boardDir + "/";
    m_uiAssetBaseDir = baseUi;
    m_hasCardTemplateSprite = false;
    m_detailTemplateLoadAttempted = false;
    m_inventoryThumbnailTextures.clear();
    m_missingInventoryThumbnailKeys.clear();

    if (!m_panelAssetTexture.loadFromFile(baseUi + "panel_assets_inventory.png")) {
        std::cerr << "[ERROR] Gagal memuat panel assets/inventory.\n";
        success = false;
    }
    if (!m_panelLogTexture.loadFromFile(baseUi + "panel_log.png")) {
        std::cerr << "[ERROR] Gagal memuat panel log.\n";
        success = false;
    }
    if (!m_scrollTrackTexture.loadFromFile(baseUi + "asset_scroll_track.png")) {
        std::cerr << "[WARN] Gagal memuat track scrollbar.\n";
    }
    if (!m_scrollThumbTexture.loadFromFile(baseUi + "asset_scroll_thumb.png")) {
        std::cerr << "[WARN] Gagal memuat thumb scrollbar.\n";
    }

    m_panelAssetSprite.setTexture(m_panelAssetTexture);
    m_panelLogSprite.setTexture(m_panelLogTexture);
    m_scrollTrackSprite.setTexture(m_scrollTrackTexture);
    m_scrollThumbSprite.setTexture(m_scrollThumbTexture);

    const sf::Vector2u trackSize = m_scrollTrackTexture.getSize();
    const sf::Vector2u thumbSize = m_scrollThumbTexture.getSize();
    m_hasScrollbarAssets = (trackSize.x > 4 && trackSize.y > 8 && thumbSize.x > 4 && thumbSize.y > 8);

    loadPropertyBannerDirectory(baseBoard + "Property/Lahan");
    loadPropertyBannerDirectory(baseBoard + "Property/Utilitas");

    sf::Texture railroadTexture;
    if (railroadTexture.loadFromFile(baseBoard + "Property/RAILROAD.png")) {
        m_propertyBannerTextures["RAILROAD"] = std::move(railroadTexture);
    }

    setPosition(m_position);
    setMode(Mode::ASSET);
    return success;
}

void AssetPanel::loadPropertyBannerDirectory(const std::string& directoryPath) {
    namespace fs = std::filesystem;

    std::error_code ec;
    if (!fs::exists(directoryPath, ec)) {
        return;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }

        const fs::path filePath = entry.path();
        const std::string ext = filePath.extension().string();
        if (ext != ".png" && ext != ".PNG") {
            continue;
        }

        sf::Texture texture;
        if (!texture.loadFromFile(filePath.string())) {
            continue;
        }

        m_propertyBannerTextures[filePath.stem().string()] = std::move(texture);
    }
}

void AssetPanel::setPosition(sf::Vector2f position) {
    m_position = position;
    m_panelAssetSprite.setPosition(position);
    m_panelLogSprite.setPosition(position);

    if (m_panelAssetTexture.getSize().x > 0 && m_panelAssetTexture.getSize().y > 0) {
        m_panelSize = sf::Vector2f(static_cast<float>(m_panelAssetTexture.getSize().x),
                                   static_cast<float>(m_panelAssetTexture.getSize().y));
    }

    m_titleText.setPosition(m_position.x + 35.0f, m_position.y + 3.0f);

    const sf::FloatRect content = getContentRect();
    m_scrollTrackSprite.setPosition(content.left + content.width + 8.0f, content.top);

    if (m_hasScrollbarAssets && m_scrollTrackTexture.getSize().x > 0 && m_scrollTrackTexture.getSize().y > 0) {
        const float sx = 16.0f / static_cast<float>(m_scrollTrackTexture.getSize().x);
        const float sy = content.height / static_cast<float>(m_scrollTrackTexture.getSize().y);
        m_scrollTrackSprite.setScale(sx, sy);
    }

    refreshWrappedLog();
    updateScrollVisual();
}

void AssetPanel::setMode(Mode mode) {
    m_mode = mode;
    m_scrollOffset = 0.0f;
    m_logScrollOffset = 0.0f;
    m_pressedItemIndex = -1;
    refreshPanelTitle();

    clampScroll();
    clampLogScroll();
    updateScrollVisual();
}

sf::FloatRect AssetPanel::getContentRect() const {
    return sf::FloatRect(m_position.x + 24.0f,
                         m_position.y + 98.0f,
                         m_panelSize.x - 56.0f,
                         m_panelSize.y - 112.0f);
}

float AssetPanel::getCardWidth() const {
    if (m_mode == Mode::INVENTORY) {
        const float gapX = getCardGapX();
        const float contentWidth = getContentRect().width;
        return std::max(120.0f, (contentWidth - gapX) / 2.0f);
    }
    return 145.0f;
}
float AssetPanel::getCardHeight() const { return (m_mode == Mode::INVENTORY) ? 216.91f : 236.0f; }
float AssetPanel::getCardGapX() const { return (m_mode == Mode::INVENTORY) ? 24.0f : 34.0f; }
float AssetPanel::getCardGapY() const { return 20.0f; }

int AssetPanel::getCurrentItemCount() const {
    if (m_mode == Mode::ASSET) {
        return static_cast<int>(m_assetItems.size());
    }
    if (m_mode == Mode::INVENTORY) {
        return static_cast<int>(m_inventoryItems.size());
    }
    return 0;
}

float AssetPanel::computeTotalContentHeight() const {
    const int itemCount = getCurrentItemCount();
    if (itemCount <= 0) {
        return 0.0f;
    }

    const int columns = (m_mode == Mode::INVENTORY) ? 2 : 4;
    const int rows = (itemCount + columns - 1) / columns;
    return rows * getCardHeight() + (rows - 1) * getCardGapY();
}

void AssetPanel::clampScroll() {
    const float contentHeight = getContentRect().height;
    m_maxScrollOffset = std::max(0.0f, computeTotalContentHeight() - contentHeight);
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, m_maxScrollOffset);
}

std::string AssetPanel::wrapLogText(const std::string& rawText, float maxWidth) const {
    if (rawText.empty() || maxWidth <= 0.0f) {
        return rawText;
    }

    sf::Text measure("", m_bodyFont, kLogCharSize);
    measure.setLineSpacing(kLogLineSpacing);

    auto textWidth = [&measure](const std::string& value) {
        if (value.empty()) {
            return 0.0f;
        }
        measure.setString(value);
        return measure.getLocalBounds().width;
    };

    std::istringstream source(rawText);
    std::string sourceLine;
    std::string wrapped;
    bool firstLine = true;

    auto appendWrappedLine = [&wrapped, &firstLine](const std::string& value) {
        if (!firstLine) {
            wrapped.push_back('\n');
        }
        wrapped += value;
        firstLine = false;
    };

    while (std::getline(source, sourceLine)) {
        if (sourceLine.empty()) {
            appendWrappedLine("");
            continue;
        }

        std::istringstream words(sourceLine);
        std::string word;
        std::string currentLine;

        auto appendLongWord = [&](const std::string& longWord) {
            std::string chunk;
            for (char c : longWord) {
                const std::string candidate = chunk + c;
                if (!chunk.empty() && textWidth(candidate) > maxWidth) {
                    appendWrappedLine(chunk);
                    chunk.assign(1, c);
                } else {
                    chunk = candidate;
                }
            }
            return chunk;
        };

        while (words >> word) {
            if (currentLine.empty()) {
                if (textWidth(word) <= maxWidth) {
                    currentLine = word;
                } else {
                    currentLine = appendLongWord(word);
                }
                continue;
            }

            const std::string candidate = currentLine + " " + word;
            if (textWidth(candidate) <= maxWidth) {
                currentLine = candidate;
                continue;
            }

            appendWrappedLine(currentLine);
            if (textWidth(word) <= maxWidth) {
                currentLine = word;
            } else {
                currentLine = appendLongWord(word);
            }
        }

        appendWrappedLine(currentLine);
    }

    return wrapped;
}

void AssetPanel::clampLogScroll() {
    const sf::FloatRect content = getContentRect();
    const float lineHeight = m_bodyFont.getLineSpacing(kLogCharSize) * kLogLineSpacing;

    int lineCount = 1;
    if (!m_wrappedLogText.empty()) {
        lineCount = 1 + static_cast<int>(std::count(m_wrappedLogText.begin(), m_wrappedLogText.end(), '\n'));
    }
    const float totalLogHeight = static_cast<float>(lineCount) * lineHeight + 8.0f;
    m_logMaxScrollOffset = std::max(0.0f, totalLogHeight - content.height);
    m_logScrollOffset = std::clamp(m_logScrollOffset, 0.0f, m_logMaxScrollOffset);
}

void AssetPanel::refreshWrappedLog() {
    const sf::FloatRect content = getContentRect();
    const float maxTextWidth = std::max(1.0f, content.width - 50.0f);
    m_wrappedLogText = wrapLogText(m_systemLog, maxTextWidth);
    clampLogScroll();
}

void AssetPanel::updateScrollVisual() {
    if (!m_hasScrollbarAssets) {
        return;
    }

    const sf::FloatRect content = getContentRect();
    const sf::FloatRect trackBounds = m_scrollTrackSprite.getGlobalBounds();

    if (m_scrollThumbTexture.getSize().x == 0 || m_scrollThumbTexture.getSize().y == 0) {
        return;
    }

    const float trackHeight = (trackBounds.height > 0.0f) ? trackBounds.height : content.height;
    const float visibleRatio = (m_maxScrollOffset <= 0.0f)
                                   ? 1.0f
                                   : std::clamp(content.height / (content.height + m_maxScrollOffset), 0.15f, 1.0f);

    const float thumbHeight = trackHeight * visibleRatio;

    const float sx = 16.0f / static_cast<float>(m_scrollThumbTexture.getSize().x);
    const float sy = thumbHeight / static_cast<float>(m_scrollThumbTexture.getSize().y);
    m_scrollThumbSprite.setScale(sx, sy);

    const float travel = std::max(0.0f, trackHeight - thumbHeight);
    const float ratio = (m_maxScrollOffset <= 0.0f) ? 0.0f : (m_scrollOffset / m_maxScrollOffset);
    const float thumbY = m_scrollTrackSprite.getPosition().y + travel * ratio;
    m_scrollThumbSprite.setPosition(m_scrollTrackSprite.getPosition().x, thumbY);
}

std::vector<sf::FloatRect> AssetPanel::buildItemBounds() const {
    std::vector<sf::FloatRect> bounds;

    const int itemCount = getCurrentItemCount();
    if (itemCount <= 0) {
        return bounds;
    }

    const sf::FloatRect content = getContentRect();
    const int columns = (m_mode == Mode::INVENTORY) ? 2 : 4;

    bounds.reserve(static_cast<size_t>(itemCount));
    for (int i = 0; i < itemCount; ++i) {
        const int col = i % columns;
        const int row = i / columns;

        const float x = content.left + static_cast<float>(col) * (getCardWidth() + getCardGapX());
        const float y = content.top + static_cast<float>(row) * (getCardHeight() + getCardGapY()) - m_scrollOffset;

        bounds.emplace_back(x, y, getCardWidth(), getCardHeight());
    }

    return bounds;
}

int AssetPanel::itemIndexAt(sf::Vector2f mousePos) const {
    if (m_mode == Mode::LOG) {
        return -1;
    }

    const sf::FloatRect content = getContentRect();
    if (!content.contains(mousePos)) {
        return -1;
    }

    const std::vector<sf::FloatRect> bounds = buildItemBounds();
    for (size_t i = 0; i < bounds.size(); ++i) {
        if (bounds[i].intersects(content) && bounds[i].contains(mousePos)) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void AssetPanel::updateData(const Player& currentPlayer, const std::string& systemLog) {
    m_currentPlayerName = currentPlayer.getUsername();
    m_assetTotalValue = 0;
    refreshPanelTitle();

    m_assetItems.clear();
    const auto& owned = currentPlayer.getOwnedProperties();
    for (const Property* property : owned) {
        if (!property) {
            continue;
        }

        m_assetTotalValue += property->getPurchasePrice();

        int buildingCount = 0;
        std::string displayName = property->getName();
        if (const auto* street = dynamic_cast<const StreetProperty*>(property)) {
            buildingCount = street->getBuildingCount();
            m_assetTotalValue += street->getBuildingSellValue() * 2;
            displayName = toStreetDisplayName(displayName);
        }

        m_assetItems.push_back(AssetItem{property,
                                         property->getCode(),
                                         displayName,
                                         property->getPurchasePrice(),
                                         property->getMortgageValue(),
                                         buildingCount});
    }

    m_inventoryItems.clear();
    const auto& cards = currentPlayer.getHandCards();
    for (const std::shared_ptr<SkillCard>& card : cards) {
        if (!card) {
            continue;
        }
        const std::string key = camelCaseToSnakeCase(card->getTypeName());
        ensureInventoryThumbnailLoaded(key);
        m_inventoryItems.push_back(InventoryItem{card->getTypeName(),
                                                 card->getDescription(),
                                                 card->getValue(),
                                                 card->getDuration(),
                                                 key});
    }
    if (currentPlayer.hasJailFreeCard()) {
        const std::string jailFreeKey = "kesempatan_keluar";
        ensureInventoryThumbnailLoaded(jailFreeKey);
        m_inventoryItems.push_back(InventoryItem{"GetOutOfJailCard",
                                                 "Kartu kesempatan khusus untuk bebas dari penjara.",
                                                 0,
                                                 0,
                                                 jailFreeKey});
    }

    m_systemLog = systemLog;
    refreshWrappedLog();
    refreshPanelTitle();

    clampScroll();
    clampLogScroll();
    updateScrollVisual();
}

void AssetPanel::refreshPanelTitle() {
    if (m_mode == Mode::LOG) {
        m_titleText.setString("LOG");
        return;
    }

    const std::string owner = m_currentPlayerName.empty() ? "Player" : m_currentPlayerName;
    if (m_mode == Mode::ASSET) {
        m_titleText.setString(owner + "'s Asset    Total: M" + std::to_string(std::max(0, m_assetTotalValue)));
        return;
    }

    m_titleText.setString(owner + "'s Inventory");
}

void AssetPanel::ensureInventoryThumbnailLoaded(const std::string& key) {
    if (key.empty()) {
        return;
    }
    if (m_inventoryThumbnailTextures.find(key) != m_inventoryThumbnailTextures.end()) {
        return;
    }
    if (m_missingInventoryThumbnailKeys.find(key) != m_missingInventoryThumbnailKeys.end()) {
        return;
    }

    sf::Texture texture;
    const std::string path = m_uiAssetBaseDir + key + ".png";
    if (!texture.loadFromFile(path)) {
        m_missingInventoryThumbnailKeys.insert(key);
        return;
    }

    m_inventoryThumbnailTextures.emplace(key, std::move(texture));
}

void AssetPanel::update(sf::Vector2f mousePos) {
    if (!m_draggingScrollbar) {
        return;
    }

    const sf::FloatRect trackBounds = m_scrollTrackSprite.getGlobalBounds();
    const sf::FloatRect thumbBounds = m_scrollThumbSprite.getGlobalBounds();

    const float minY = trackBounds.top;
    const float maxY = trackBounds.top + std::max(0.0f, trackBounds.height - thumbBounds.height);
    const float targetY = std::clamp(mousePos.y - m_dragGrabOffsetY, minY, maxY);

    const float ratio = (maxY <= minY) ? 0.0f : (targetY - minY) / (maxY - minY);
    m_scrollOffset = ratio * m_maxScrollOffset;
    updateScrollVisual();
}

bool AssetPanel::handleMouseWheel(float delta, sf::Vector2f mousePos) {
    if (m_detailPopupVisible) {
        return false;
    }

    if (!getContentRect().contains(mousePos)) {
        return false;
    }

    if (m_mode == Mode::LOG) {
        m_logScrollOffset -= delta * 54.0f;
        clampLogScroll();
        return true;
    }

    m_scrollOffset -= delta * 54.0f;
    clampScroll();
    updateScrollVisual();
    return true;
}

bool AssetPanel::handleMousePressed(sf::Vector2f mousePos) {
    if (m_detailPopupVisible) {
        m_detailPopupVisible = false;
        return true;
    }

    if (m_hasScrollbarAssets && m_mode != Mode::LOG &&
        m_scrollThumbSprite.getGlobalBounds().contains(mousePos)) {
        m_draggingScrollbar = true;
        m_dragGrabOffsetY = mousePos.y - m_scrollThumbSprite.getPosition().y;
        return true;
    }

    m_pressedItemIndex = itemIndexAt(mousePos);
    return m_pressedItemIndex >= 0;
}

bool AssetPanel::handleMouseReleased(sf::Vector2f mousePos) {
    if (m_draggingScrollbar) {
        m_draggingScrollbar = false;
        return true;
    }

    const int releasedIndex = itemIndexAt(mousePos);
    if (m_pressedItemIndex >= 0 && m_pressedItemIndex == releasedIndex) {
        openDetailForItem(releasedIndex);
        m_pressedItemIndex = -1;
        return true;
    }

    m_pressedItemIndex = -1;
    return false;
}

void AssetPanel::openDetailForItem(int index) {
    std::ostringstream out;

    if (m_mode == Mode::ASSET) {
        if (index < 0 || index >= static_cast<int>(m_assetItems.size())) {
            return;
        }

        const AssetItem& item = m_assetItems[static_cast<size_t>(index)];
        m_detailTitle = item.title;
        out << "Kode: " << item.code << "\n";
        out << "Bangunan: " << toBuildingText(item.property) << "\n";
        out << "Harga: M" << item.purchasePrice << "\n";
        out << "Status: " << toOwnershipStatusText(*item.property);
    } else if (m_mode == Mode::INVENTORY) {
        if (index < 0 || index >= static_cast<int>(m_inventoryItems.size())) {
            return;
        }

        const InventoryItem& item = m_inventoryItems[static_cast<size_t>(index)];
        m_detailTitle = item.typeName;
        out << item.description << "\n";
        out << "Value: " << item.value << "\n";
        out << "Duration: " << item.duration;
    } else {
        return;
    }

    m_detailBody = out.str();
    m_detailPopupVisible = true;
}

void AssetPanel::renderAssetOrInventory(sf::RenderWindow& window) const {
    const sf::FloatRect content = getContentRect();

    const sf::View previous = window.getView();
    sf::View clipped;
    clipped.setCenter(content.left + content.width / 2.0f, content.top + content.height / 2.0f);
    clipped.setSize(content.width, content.height);
    clipped.setViewport(sf::FloatRect(content.left / kDesignWidth,
                                      content.top / kDesignHeight,
                                      content.width / kDesignWidth,
                                      content.height / kDesignHeight));
    window.setView(clipped);

    const std::vector<sf::FloatRect> itemBounds = buildItemBounds();

    if (itemBounds.empty()) {
        const std::string emptyText = (m_mode == Mode::ASSET) ? "Belum ada aset." : "Belum ada kartu inventory.";
        sf::Text text(emptyText, m_bodyFont, 28);
        text.setFillColor(sf::Color(53, 45, 36, 220));
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        text.setPosition(content.left + content.width / 2.0f, content.top + content.height / 2.0f);
        window.draw(text);
    }

    for (size_t i = 0; i < itemBounds.size(); ++i) {
        const sf::FloatRect& rect = itemBounds[i];
        if (!rect.intersects(content)) {
            continue;
        }

        if (m_mode == Mode::ASSET && i < m_assetItems.size()) {
            const AssetItem& item = m_assetItems[i];

            const sf::FloatRect previewRect(rect.left + 8.0f, rect.top + 8.0f, rect.width - 16.0f, rect.height - 72.0f);
            const sf::Texture* baseTexture = resolvePropertyTexture(item.property, item.code, m_propertyBannerTextures);
            if (baseTexture && baseTexture->getSize().x > 0 && baseTexture->getSize().y > 0) {
                sf::Sprite base(*baseTexture);
                const sf::Vector2u texSize = baseTexture->getSize();
                base.setScale(previewRect.width / static_cast<float>(texSize.x),
                              previewRect.height / static_cast<float>(texSize.y));
                base.setPosition(previewRect.left, previewRect.top);
                window.draw(base);
            } else {
                sf::RectangleShape fallback(sf::Vector2f(previewRect.width, previewRect.height));
                fallback.setPosition(previewRect.left, previewRect.top);
                fallback.setFillColor(sf::Color(240, 228, 198));
                window.draw(fallback);
            }

            bool nameAlreadyDrawn = false;
            if (const auto* street = dynamic_cast<const StreetProperty*>(item.property)) {
                const sf::Color ribbonColor = resolveStreetColor(street->getColorGroup());
                if (ribbonColor != sf::Color::Transparent) {
                    sf::RectangleShape ribbon(sf::Vector2f(previewRect.width, 16.0f));
                    ribbon.setPosition(previewRect.left, previewRect.top);
                    ribbon.setFillColor(ribbonColor);
                    ribbon.setOutlineThickness(1.0f);
                    ribbon.setOutlineColor(sf::Color(0, 0, 0, 95));
                    window.draw(ribbon);

                    sf::Text ribbonName(item.title, m_bodyFont, 13);
                    ribbonName.setFillColor(sf::Color(53, 45, 36));
                    fitTextToMaxWidth(ribbonName, previewRect.width - 8.0f);
                    const sf::FloatRect ribbonNameBounds = ribbonName.getLocalBounds();
                    ribbonName.setOrigin(ribbonNameBounds.left + (ribbonNameBounds.width / 2.0f),
                                         ribbonNameBounds.top + (ribbonNameBounds.height / 2.0f));
                    ribbonName.setPosition(previewRect.left + (previewRect.width / 2.0f),
                                           previewRect.top + 30.0f);
                    window.draw(ribbonName);
                    nameAlreadyDrawn = true;
                }
            }

            if (!nameAlreadyDrawn && item.property &&
                item.property->getType() == PropertyType::RAILROAD) {
                const std::string railroadName = toRailroadTwoLines(item.title);
                const size_t splitPos = railroadName.find('\n');
                if (splitPos != std::string::npos) {
                    const std::string firstLine = railroadName.substr(0, splitPos);
                    const std::string secondLine = railroadName.substr(splitPos + 1);
                    const float centerX = previewRect.left + (previewRect.width / 2.0f);

                    sf::Text first(firstLine, m_bodyFont, 13);
                    first.setFillColor(sf::Color(53, 45, 36));
                    const sf::FloatRect firstBounds = first.getLocalBounds();
                    first.setOrigin(firstBounds.left + (firstBounds.width / 2.0f),
                                    firstBounds.top + (firstBounds.height / 2.0f));
                    first.setPosition(centerX, previewRect.top + 24.0f);
                    window.draw(first);

                    sf::Text second(secondLine, m_bodyFont, 13);
                    second.setFillColor(sf::Color(53, 45, 36));
                    const sf::FloatRect secondBounds = second.getLocalBounds();
                    second.setOrigin(secondBounds.left + (secondBounds.width / 2.0f),
                                     secondBounds.top + (secondBounds.height / 2.0f));
                    second.setPosition(centerX, previewRect.top + 38.0f);
                    window.draw(second);
                } else {
                    sf::Text railroadSingle(railroadName, m_bodyFont, 13);
                    railroadSingle.setFillColor(sf::Color(53, 45, 36));
                    const sf::FloatRect singleBounds = railroadSingle.getLocalBounds();
                    railroadSingle.setOrigin(singleBounds.left + (singleBounds.width / 2.0f),
                                             singleBounds.top + (singleBounds.height / 2.0f));
                    railroadSingle.setPosition(previewRect.left + (previewRect.width / 2.0f),
                                               previewRect.top + 30.0f);
                    window.draw(railroadSingle);
                }
                nameAlreadyDrawn = true;
            }

            const bool hideUtilityName = (item.code == "PLN" || item.code == "PAM");
            if (!nameAlreadyDrawn && !hideUtilityName) {
                sf::Text nameText(item.title, m_bodyFont, 18);
                nameText.setFillColor(sf::Color(53, 45, 36));
                const sf::FloatRect nameBounds = nameText.getLocalBounds();
                nameText.setOrigin(nameBounds.left + (nameBounds.width / 2.0f), nameBounds.top);
                nameText.setPosition(rect.left + (rect.width / 2.0f), rect.top + rect.height - 58.0f);
                window.draw(nameText);
            }

            sf::Text priceText(std::to_string(item.purchasePrice), m_bodyFont, 16);
            priceText.setFillColor(sf::Color(53, 45, 36));
            const sf::FloatRect priceBounds = priceText.getLocalBounds();
            priceText.setOrigin(priceBounds.left + (priceBounds.width / 2.0f), priceBounds.top);
            priceText.setPosition(rect.left + (rect.width / 2.0f) + 10.0f, rect.top + rect.height - 85.0f);

            window.draw(priceText);
        }

        if (m_mode == Mode::INVENTORY && i < m_inventoryItems.size()) {
            const InventoryItem& item = m_inventoryItems[i];
            const sf::FloatRect previewRect(rect.left + 8.0f, rect.top + 8.0f, 324.42, 216.91);

            const auto itTexture = m_inventoryThumbnailTextures.find(item.thumbnailKey);
            if (itTexture != m_inventoryThumbnailTextures.end() &&
                itTexture->second.getSize().x > 0 && itTexture->second.getSize().y > 0) {
                sf::Sprite sprite(itTexture->second);
                const sf::Vector2u texSize = itTexture->second.getSize();
                sprite.setScale(previewRect.width / static_cast<float>(texSize.x),
                                previewRect.height / static_cast<float>(texSize.y));
                sprite.setPosition(previewRect.left, previewRect.top);
                window.draw(sprite);
            } else {
                sf::RectangleShape fallback(sf::Vector2f(previewRect.width, previewRect.height));
                fallback.setPosition(previewRect.left, previewRect.top);
                fallback.setFillColor(sf::Color(233, 221, 194));
                fallback.setOutlineThickness(1.0f);
                fallback.setOutlineColor(sf::Color(150, 130, 95));
                window.draw(fallback);

                sf::Text nameText(item.typeName, m_bodyFont, 16);
                nameText.setFillColor(sf::Color(53, 45, 36));
                const sf::FloatRect nameBounds = nameText.getLocalBounds();
                nameText.setOrigin(nameBounds.left + (nameBounds.width / 2.0f),
                                   nameBounds.top + (nameBounds.height / 2.0f));
                nameText.setPosition(previewRect.left + (previewRect.width / 2.0f),
                                     previewRect.top + (previewRect.height / 2.0f));
                window.draw(nameText);
            }
        }
    }

    window.setView(previous);

    if (m_hasScrollbarAssets && m_mode != Mode::LOG) {
        window.draw(m_scrollTrackSprite);
        window.draw(m_scrollThumbSprite);
    }
}

void AssetPanel::renderLog(sf::RenderWindow& window) const {
    const sf::FloatRect content = getContentRect();
    const sf::View previous = window.getView();

    sf::View clipped;
    clipped.setCenter(content.left + content.width * 0.5f,
                      content.top + content.height * 0.5f + m_logScrollOffset);
    clipped.setSize(content.width, content.height);
    clipped.setViewport(sf::FloatRect(content.left / kDesignWidth,
                                      content.top / kDesignHeight,
                                      content.width / kDesignWidth,
                                      content.height / kDesignHeight));
    window.setView(clipped);

    sf::Text logText(m_wrappedLogText, m_bodyFont, kLogCharSize);
    logText.setLineSpacing(kLogLineSpacing);
    logText.setFillColor(sf::Color(53, 45, 36));
    logText.setPosition(content.left + 10.0f, content.top + 6.0f);
    window.draw(logText);

    window.setView(previous);
}

void AssetPanel::renderDetailPopup(sf::RenderWindow& window) const {
    if (!m_detailPopupVisible) {
        return;
    }

    const sf::Vector2f popupSize(400.0f, 300.0f);
    const sf::Vector2f popupPos(m_position.x + (m_panelSize.x - popupSize.x) / 2.0f,
                                m_position.y + (m_panelSize.y - popupSize.y) / 2.0f);

    if (!m_detailTemplateLoadAttempted) {
        m_detailTemplateLoadAttempted = true;
        if (m_cardTemplateTexture.loadFromFile(m_uiAssetBaseDir + "asset_card_template.png")) {
            const sf::Vector2u texSize = m_cardTemplateTexture.getSize();
            m_hasCardTemplateSprite = (texSize.x > 8 && texSize.y > 8);
        } else {
            m_hasCardTemplateSprite = false;
            std::cerr << "[WARN] Gagal memuat detail template: "
                      << (m_uiAssetBaseDir + "asset_card_template.png") << "\n";
        }
    }

    if (m_hasCardTemplateSprite) {
        sf::Sprite popup(m_cardTemplateTexture);
        const sf::Vector2u texSize = m_cardTemplateTexture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            popup.setScale(popupSize.x / static_cast<float>(texSize.x),
                           popupSize.y / static_cast<float>(texSize.y));
        }
        popup.setPosition(popupPos);
        window.draw(popup);
    } else {
        sf::RectangleShape popupRect(popupSize);
        popupRect.setPosition(popupPos);
        popupRect.setFillColor(sf::Color(236, 221, 192));
        popupRect.setOutlineThickness(2.0f);
        popupRect.setOutlineColor(sf::Color(110, 86, 58));
        window.draw(popupRect);
    }

    sf::Text title(m_detailTitle, m_titleFont, 42);
    title.setFillColor(sf::Color(53, 45, 36));
    title.setPosition(popupPos.x + 20.0f, popupPos.y + 18.0f);

    sf::Text body(m_detailBody + "\n\n(klik untuk menutup)", m_bodyFont, 24);
    body.setFillColor(sf::Color(53, 45, 36));
    body.setPosition(popupPos.x + 20.0f, popupPos.y + 84.0f);

    window.draw(title);
    window.draw(body);
}

void AssetPanel::render(sf::RenderWindow& window) const {
    if (m_mode == Mode::LOG) {
        window.draw(m_panelLogSprite);
    } else {
        window.draw(m_panelAssetSprite);
    }

    window.draw(m_titleText);

    if (m_mode == Mode::LOG) {
        renderLog(window);
    } else {
        renderAssetOrInventory(window);
    }

    renderDetailPopup(window);
}
} // namespace viewsGUI
