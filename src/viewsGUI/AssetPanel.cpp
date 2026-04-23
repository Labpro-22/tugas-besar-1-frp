#include "../../include/viewsGUI/AssetPanel.hpp"

#include "../../include/models/Player.hpp"
#include "../../include/models/Property.hpp"
#include "../../include/models/SkillCard.hpp"
#include "../../include/models/StreetProperty.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace viewsGUI {
namespace {
constexpr std::size_t kLogWrapWidth = 35;

std::string wrapTextByWord(const std::string& text, std::size_t maxCharsPerLine) {
    if (maxCharsPerLine == 0 || text.empty()) {
        return text;
    }

    std::istringstream lineStream(text);
    std::string sourceLine;
    std::string wrapped;
    bool firstLine = true;

    auto appendLine = [&](const std::string& line) {
        if (!firstLine) {
            wrapped.push_back('\n');
        }
        wrapped += line;
        firstLine = false;
    };

    while (std::getline(lineStream, sourceLine)) {
        std::istringstream words(sourceLine);
        std::string word;
        std::string currentLine;
        bool hasWord = false;

        while (words >> word) {
            hasWord = true;
            if (currentLine.empty()) {
                currentLine = word;
                continue;
            }

            if (currentLine.size() + 1 + word.size() <= maxCharsPerLine) {
                currentLine += " " + word;
            } else {
                appendLine(currentLine);
                currentLine = word;
            }
        }

        if (!hasWord) {
            appendLine("");
            continue;
        }

        appendLine(currentLine);
    }

    return wrapped;
}
} // namespace

AssetPanel::AssetPanel(const sf::Font& titleFont, const sf::Font& bodyFont)
    : m_titleFont(titleFont),
      m_bodyFont(bodyFont),
      m_position(0.0f, 0.0f),
      m_panelSize(820.0f, 548.0f),
      m_hasCardTemplateSprite(false),
      m_hasScrollbarAssets(false),
      m_mode(Mode::ASSET),
      m_currentPlayerName(""),
      m_scrollOffset(0.0f),
      m_maxScrollOffset(0.0f),
      m_draggingScrollbar(false),
      m_dragGrabOffsetY(0.0f),
      m_pressedItemIndex(-1),
      m_detailPopupVisible(false) {
    m_titleText.setFont(m_titleFont);
    m_titleText.setCharacterSize(68);
    m_titleText.setFillColor(sf::Color(240, 239, 229));
    refreshPanelTitle();
}

bool AssetPanel::loadAssets(const std::string& uiDir, const std::string& boardDir) {
    bool success = true;

    const std::string baseUi = (!uiDir.empty() && uiDir.back() == '/') ? uiDir : uiDir + "/";
    const std::string baseBoard = (!boardDir.empty() && boardDir.back() == '/') ? boardDir : boardDir + "/";

    if (!m_panelAssetTexture.loadFromFile(baseUi + "panel_assets_inventory.png")) {
        std::cerr << "[ERROR] Gagal memuat panel assets/inventory.\n";
        success = false;
    }
    if (!m_panelLogTexture.loadFromFile(baseUi + "panel_log.png")) {
        std::cerr << "[ERROR] Gagal memuat panel log.\n";
        success = false;
    }
    if (!m_cardTemplateTexture.loadFromFile(baseUi + "asset_card_template.png")) {
        std::cerr << "[ERROR] Gagal memuat template card asset.\n";
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

    const sf::Vector2u cardSize = m_cardTemplateTexture.getSize();
    m_hasCardTemplateSprite = (cardSize.x > 8 && cardSize.y > 8);

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

    updateScrollVisual();
}

void AssetPanel::setMode(Mode mode) {
    m_mode = mode;
    m_scrollOffset = 0.0f;
    m_pressedItemIndex = -1;
    refreshPanelTitle();

    clampScroll();
    updateScrollVisual();
}

sf::FloatRect AssetPanel::getContentRect() const {
    return sf::FloatRect(m_position.x + 24.0f,
                         m_position.y + 98.0f,
                         m_panelSize.x - 56.0f,
                         m_panelSize.y - 112.0f);
}

float AssetPanel::getCardWidth() const { return 145.0f; }
float AssetPanel::getCardHeight() const { return 236.0f; }
float AssetPanel::getCardGapX() const { return 34.0f; }
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

    const int columns = 4;
    const int rows = (itemCount + columns - 1) / columns;
    return rows * getCardHeight() + (rows - 1) * getCardGapY();
}

void AssetPanel::clampScroll() {
    const float contentHeight = getContentRect().height;
    m_maxScrollOffset = std::max(0.0f, computeTotalContentHeight() - contentHeight);
    m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, m_maxScrollOffset);
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
    const int columns = 4;

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
    refreshPanelTitle();

    m_assetItems.clear();
    const auto& owned = currentPlayer.getOwnedProperties();
    for (const Property* property : owned) {
        if (!property) {
            continue;
        }

        int buildingCount = 0;
        if (const auto* street = dynamic_cast<const StreetProperty*>(property)) {
            buildingCount = street->getBuildingCount();
        }

        m_assetItems.push_back(AssetItem{property,
                                         property->getCode(),
                                         property->getName(),
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

        m_inventoryItems.push_back(InventoryItem{card->getTypeName(),
                                                 card->getDescription(),
                                                 card->getValue(),
                                                 card->getDuration()});
    }

    m_systemLog = wrapTextByWord(systemLog, kLogWrapWidth);

    clampScroll();
    updateScrollVisual();
}

void AssetPanel::refreshPanelTitle() {
    if (m_mode == Mode::LOG) {
        m_titleText.setString("LOG");
        return;
    }

    const std::string owner = m_currentPlayerName.empty() ? "Player" : m_currentPlayerName;
    if (m_mode == Mode::ASSET) {
        m_titleText.setString(owner + "'s Asset");
        return;
    }

    m_titleText.setString(owner + "'s Inventory");
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
    if (m_mode == Mode::LOG || m_detailPopupVisible) {
        return false;
    }

    if (!getContentRect().contains(mousePos)) {
        return false;
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
        out << "Harga Beli: M " << item.purchasePrice << "\n";
        out << "Mortgage: M " << item.mortgageValue << "\n";
        out << "Bangunan: " << item.buildingCount;
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

        if (m_hasCardTemplateSprite) {
            sf::Sprite card(m_cardTemplateTexture);
            const sf::Vector2u cardTexSize = m_cardTemplateTexture.getSize();
            if (cardTexSize.x > 0 && cardTexSize.y > 0) {
                card.setScale(rect.width / static_cast<float>(cardTexSize.x),
                              rect.height / static_cast<float>(cardTexSize.y));
            }
            card.setPosition(rect.left, rect.top);
            window.draw(card);
        }

        if (m_mode == Mode::ASSET && i < m_assetItems.size()) {
            const AssetItem& item = m_assetItems[i];

            const auto itBanner = m_propertyBannerTextures.find(item.code);
            if (itBanner != m_propertyBannerTextures.end()) {
                sf::Sprite banner(itBanner->second);
                const sf::Vector2u bannerSize = itBanner->second.getSize();
                if (bannerSize.x > 0 && bannerSize.y > 0) {
                    banner.setScale((rect.width - 18.0f) / static_cast<float>(bannerSize.x),
                                    56.0f / static_cast<float>(bannerSize.y));
                }
                banner.setPosition(rect.left + 9.0f, rect.top + 8.0f);
                window.draw(banner);
            }

            sf::Text nameText(item.title, m_bodyFont, 20);
            nameText.setFillColor(sf::Color(53, 45, 36));
            nameText.setPosition(rect.left + 12.0f, rect.top + 70.0f);

            sf::Text detailText("Harga: M" + std::to_string(item.purchasePrice) +
                                    "\nMortgage: M" + std::to_string(item.mortgageValue) +
                                    "\nBangunan: " + std::to_string(item.buildingCount),
                                m_bodyFont,
                                16);
            detailText.setFillColor(sf::Color(53, 45, 36));
            detailText.setPosition(rect.left + 12.0f, rect.top + 106.0f);

            window.draw(nameText);
            window.draw(detailText);
        }

        if (m_mode == Mode::INVENTORY && i < m_inventoryItems.size()) {
            const InventoryItem& item = m_inventoryItems[i];

            sf::Text nameText(item.typeName, m_bodyFont, 20);
            nameText.setFillColor(sf::Color(53, 45, 36));
            nameText.setPosition(rect.left + 12.0f, rect.top + 18.0f);

            sf::Text detailText(item.description +
                                    "\nV:" + std::to_string(item.value) +
                                    " D:" + std::to_string(item.duration),
                                m_bodyFont,
                                16);
            detailText.setFillColor(sf::Color(53, 45, 36));
            detailText.setPosition(rect.left + 12.0f, rect.top + 56.0f);

            window.draw(nameText);
            window.draw(detailText);
        }
    }

    window.setView(previous);

    if (m_hasScrollbarAssets && m_mode != Mode::LOG) {
        window.draw(m_scrollTrackSprite);
        window.draw(m_scrollThumbSprite);
    }
}

void AssetPanel::renderLog(sf::RenderWindow& window) const {
    sf::Text logText(m_systemLog, m_bodyFont, 24);
    logText.setFillColor(sf::Color(53, 45, 36));
    logText.setPosition(m_position.x + 34.0f, m_position.y + 106.0f);
    window.draw(logText);
}

void AssetPanel::renderDetailPopup(sf::RenderWindow& window) const {
    if (!m_detailPopupVisible) {
        return;
    }

    const sf::Vector2f popupSize(400.0f, 300.0f);
    const sf::Vector2f popupPos(m_position.x + (m_panelSize.x - popupSize.x) / 2.0f,
                                m_position.y + (m_panelSize.y - popupSize.y) / 2.0f);

    if (m_hasCardTemplateSprite) {
        sf::Sprite popup(m_cardTemplateTexture);
        const sf::Vector2u texSize = m_cardTemplateTexture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            popup.setScale(popupSize.x / static_cast<float>(texSize.x),
                           popupSize.y / static_cast<float>(texSize.y));
        }
        popup.setPosition(popupPos);
        window.draw(popup);
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
