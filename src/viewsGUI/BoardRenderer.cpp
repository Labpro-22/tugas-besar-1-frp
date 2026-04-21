#include "../../include/viewsGUI/BoardRenderer.hpp"

#include "../../include/models/Board.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/viewsGUI/Theme.hpp"

namespace viewsGUI {
BoardRenderer::BoardRenderer(float boardSize, const sf::Font& font)
    : m_boardSize(boardSize),
      m_cornerSize(90.0f),
      m_tileSize(60.0f),
    m_font(font),
      m_tileTextures(40),
      m_tileSprites(40) {}

bool BoardRenderer::loadAssets(const std::string& assetDirectory) {
    bool success = true;
    const std::string baseDir =
        (!assetDirectory.empty() && assetDirectory.back() == '/') ? assetDirectory
                                                                   : assetDirectory + "/";

    if (!m_centerTexture.loadFromFile(baseDir + "center_board.png")) {
        std::cerr << "[ERROR] Gagal memuat " << baseDir << "center_board.png\n";
        success = false;
    } else {
        m_centerSprite.setTexture(m_centerTexture);
        m_centerSprite.setPosition(m_cornerSize, m_cornerSize);

        const float centerAreaSize = m_boardSize - (2.0f * m_cornerSize);
        const sf::Vector2u centerTexSize = m_centerTexture.getSize();
        if (centerTexSize.x > 0 && centerTexSize.y > 0) {
            m_centerSprite.setScale(centerAreaSize / static_cast<float>(centerTexSize.x),
                                    centerAreaSize / static_cast<float>(centerTexSize.y));
        }
    }

    for (int i = 0; i < 40; ++i) {
        const std::string filename = baseDir + "tile_" + std::to_string(i) + ".png";
        if (!m_tileTextures[static_cast<size_t>(i)].loadFromFile(filename)) {
            std::cerr << "[ERROR] Gagal memuat " << filename << "\n";
            success = false;
            continue;
        }

        m_tileSprites[static_cast<size_t>(i)].setTexture(m_tileTextures[static_cast<size_t>(i)]);

        const sf::Vector2f targetSize = getTileSize(i);
        const sf::Vector2f targetPos = getTilePosition(i);
        const sf::Vector2u texSize = m_tileTextures[static_cast<size_t>(i)].getSize();

        if (texSize.x > 0 && texSize.y > 0) {
            m_tileSprites[static_cast<size_t>(i)].setScale(
                targetSize.x / static_cast<float>(texSize.x),
                targetSize.y / static_cast<float>(texSize.y));
        }
        m_tileSprites[static_cast<size_t>(i)].setPosition(targetPos);
    }

    return success;
}

sf::Vector2f BoardRenderer::getTilePosition(int index) const {
    if (index == 0) return {m_boardSize - m_cornerSize, m_boardSize - m_cornerSize};
    if (index > 0 && index < 10)
        return {m_boardSize - m_cornerSize - (index * m_tileSize), m_boardSize - m_cornerSize};
    if (index == 10) return {0.0f, m_boardSize - m_cornerSize};
    if (index > 10 && index < 20)
        return {0.0f, m_boardSize - m_cornerSize - ((index - 10) * m_tileSize)};
    if (index == 20) return {0.0f, 0.0f};
    if (index > 20 && index < 30)
        return {m_cornerSize + ((index - 21) * m_tileSize), 0.0f};
    if (index == 30) return {m_boardSize - m_cornerSize, 0.0f};
    if (index > 30 && index < 40)
        return {m_boardSize - m_cornerSize, m_cornerSize + ((index - 31) * m_tileSize)};
    return {0.0f, 0.0f};
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

sf::Color BoardRenderer::resolveHeaderColor(int index) const {
    if (index == 1 || index == 3) return Theme::Coklat;
    if (index == 6 || index == 8 || index == 9) return Theme::BiruMuda;
    if (index == 11 || index == 13 || index == 14) return Theme::Pink;
    if (index == 16 || index == 18 || index == 19) return Theme::Oranye;
    if (index == 21 || index == 23 || index == 24) return Theme::Merah;
    if (index == 26 || index == 27 || index == 29) return Theme::Kuning;
    if (index == 31 || index == 32 || index == 34) return Theme::Hijau;
    if (index == 37 || index == 39) return Theme::BiruTua;
    return sf::Color::Transparent;
}

void BoardRenderer::drawColorHeader(sf::RenderWindow& window,
                                    int index,
                                    sf::Color headerColor) const {
    if (headerColor == sf::Color::Transparent || index % 10 == 0) {
        return;
    }

    const sf::Vector2f pos = getTilePosition(index);
    sf::RectangleShape headerShape;
    headerShape.setFillColor(headerColor);
    headerShape.setOutlineThickness(1.0f);
    headerShape.setOutlineColor(sf::Color(0, 0, 0, 100));

    const float headerThickness = 18.0f;

    if (index > 0 && index < 10) {
        headerShape.setSize({m_tileSize, headerThickness});
        headerShape.setPosition(pos.x, pos.y);
    } else if (index > 10 && index < 20) {
        headerShape.setSize({headerThickness, m_tileSize});
        headerShape.setPosition(pos.x + m_cornerSize - headerThickness, pos.y);
    } else if (index > 20 && index < 30) {
        headerShape.setSize({m_tileSize, headerThickness});
        headerShape.setPosition(pos.x, pos.y + m_cornerSize - headerThickness);
    } else if (index > 30 && index < 40) {
        headerShape.setSize({headerThickness, m_tileSize});
        headerShape.setPosition(pos.x, pos.y);
    }

    window.draw(headerShape);
}

void BoardRenderer::drawDynamicPrice(sf::RenderWindow& window,
                                     int index,
                                     const std::string& priceText) const {
    if (index % 10 == 0 || priceText.empty()) {
        return;
    }

    sf::Text text(priceText, m_font, 6);
    text.setFillColor(Theme::TextDark);

    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + (bounds.width / 2.0f), bounds.top + (bounds.height / 2.0f));

    const sf::Vector2f pos = getTilePosition(index);
    const sf::Vector2f size = getTileSize(index);

    if (index > 0 && index < 10) {
        text.setRotation(0.0f);
        text.setPosition(pos.x + (size.x / 2.0f) + 4.0f, pos.y + size.y - 8.0f);
    } else if (index > 10 && index < 20) {
        text.setRotation(90.0f);
        text.setPosition(pos.x + 8.0f, pos.y + (size.y / 2.0f) + 4.0f);
    } else if (index > 20 && index < 30) {
        text.setRotation(180.0f);
        text.setPosition(pos.x + (size.x / 2.0f) - 4.0f, pos.y + 8.0f);
    } else if (index > 30 && index < 40) {
        text.setRotation(270.0f);
        text.setPosition(pos.x + size.x - 8.0f, pos.y + (size.y / 2.0f) - 4.0f);
    }

    window.draw(text);
}

void BoardRenderer::render(sf::RenderWindow& window, const Board& board) const {
    sf::RectangleShape boardBg({m_boardSize, m_boardSize});
    boardBg.setFillColor(sf::Color(245, 250, 245));
    window.draw(boardBg);
    window.draw(m_centerSprite);

    const int tileCount = board.size();
    for (int i = 0; i < 40; ++i) {
        window.draw(m_tileSprites[static_cast<size_t>(i)]);

        const sf::Color color = resolveHeaderColor(i);
        drawColorHeader(window, i, color);

        if (i >= tileCount) {
            continue;
        }

        const Tile& tile = board.getTileByIndex(i);
        if (!tile.isProperty()) {
            continue;
        }

        const auto* propertyTile = dynamic_cast<const PropertyTile*>(&tile);
        if (!propertyTile) {
            continue;
        }

        const int purchasePrice = propertyTile->getProperty().getPurchasePrice();
        if (purchasePrice > 0) {
            drawDynamicPrice(window, i, std::to_string(purchasePrice));
        }
    }
}
} // namespace viewsGUI
