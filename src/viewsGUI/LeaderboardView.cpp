#include "../../include/viewsGUI/LeaderboardView.hpp"

#include "../../include/models/Player.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

namespace viewsGUI {
namespace {
constexpr int kMaxPlayerTokens = 4;
constexpr float kTokenSize = 28.0f;

const std::array<sf::Color, 6> kBoardPieceColors = {
    sf::Color(216, 83, 79),
    sf::Color(84, 143, 224),
    sf::Color(93, 170, 104),
    sf::Color(240, 190, 80),
    sf::Color(148, 92, 181),
    sf::Color(100, 100, 100)};
}

LeaderboardView::LeaderboardView(const sf::Font& headerFont, const sf::Font& bodyFont)
    : m_headerFont(headerFont), m_bodyFont(bodyFont), m_position(0.0f, 0.0f) {}

bool LeaderboardView::loadAssets(const std::string& panelPath) {
    if (!m_panelTexture.loadFromFile(panelPath)) {
        std::cerr << "[ERROR] Gagal memuat panel leaderboard: " << panelPath << "\n";
        return false;
    }

    m_panelSprite.setTexture(m_panelTexture);

    m_tokenTextures.clear();
    m_tokenTextures.resize(kMaxPlayerTokens);

    const std::filesystem::path panelFile(panelPath);
    const std::filesystem::path tokenDir = panelFile.parent_path() / "piece";
    for (int i = 0; i < kMaxPlayerTokens; ++i) {
        const std::filesystem::path tokenPath = tokenDir / ("piece_p" + std::to_string(i + 1) + ".png");
        if (!m_tokenTextures[static_cast<size_t>(i)].loadFromFile(tokenPath.string())) {
            std::cerr << "[WARN] Gagal memuat token leaderboard: " << tokenPath.string() << "\n";
            continue;
        }

        const sf::Vector2u size = m_tokenTextures[static_cast<size_t>(i)].getSize();
        if (size.x <= 2 || size.y <= 2) {
            std::cerr << "[WARN] Token leaderboard placeholder terdeteksi ("
                      << size.x << "x" << size.y << "): " << tokenPath.string()
                      << ". Gunakan fallback warna pemain.\n";
        }
    }

    return true;
}

void LeaderboardView::setPosition(sf::Vector2f position) {
    m_position = position;
    m_panelSprite.setPosition(m_position);
}

void LeaderboardView::updateFromPlayers(const std::vector<Player*>& players) {
    m_rows.clear();
    m_rows.reserve(players.size());

    for (size_t i = 0; i < players.size(); ++i) {
        const Player* player = players[i];
        if (!player) {
            continue;
        }

        const int cash = player->getMoney();
        const int assets = player->getAssetValue();
        const int total = cash + assets;
        m_rows.push_back(Row{player->getUsername(), cash, assets, total, static_cast<int>(i)});
    }

    std::sort(m_rows.begin(), m_rows.end(), [](const Row& a, const Row& b) {
        if (a.total != b.total) {
            return a.total > b.total;
        }
        if (a.assets != b.assets) {
            return a.assets > b.assets;
        }
        return a.cash > b.cash;
    });
}

void LeaderboardView::drawRowText(sf::RenderWindow& window, const Row& row, int rowIndex) const {
    const float originX = m_position.x;
    const float rowY = m_position.y + 166.0f + static_cast<float>(rowIndex) * 65.0f;

    sf::Text playerText(row.player, m_bodyFont, 32);
    playerText.setFillColor(sf::Color(53, 45, 36));
    playerText.setPosition(originX + 190.0f, rowY - 10.0f);

    sf::Text cashText(std::to_string(row.cash), m_bodyFont, 32);
    cashText.setFillColor(sf::Color(53, 45, 36));
    const sf::FloatRect cashBounds = cashText.getLocalBounds();
    cashText.setOrigin(cashBounds.left + cashBounds.width, cashBounds.top);
    cashText.setPosition(originX + 470.0f, rowY);

    sf::Text assetsText(std::to_string(row.assets), m_bodyFont, 32);
    assetsText.setFillColor(sf::Color(53, 45, 36));
    const sf::FloatRect assetsBounds = assetsText.getLocalBounds();
    assetsText.setOrigin(assetsBounds.left + assetsBounds.width, assetsBounds.top);
    assetsText.setPosition(originX + 620.0f, rowY);

    sf::Text totalText(std::to_string(row.total), m_bodyFont, 32);
    totalText.setFillColor(sf::Color(53, 45, 36));
    const sf::FloatRect totalBounds = totalText.getLocalBounds();
    totalText.setOrigin(totalBounds.left + totalBounds.width, totalBounds.top);
    totalText.setPosition(originX + 770.0f, rowY);

    window.draw(playerText);

    if (row.tokenIndex >= 0 && row.tokenIndex < static_cast<int>(m_tokenTextures.size())) {
        const sf::Texture& tokenTexture = m_tokenTextures[static_cast<size_t>(row.tokenIndex)];
        const sf::FloatRect nameBounds = playerText.getGlobalBounds();
        const float tokenCenterX = std::min(nameBounds.left + nameBounds.width + 18.0f, originX + 444.0f);
        const float tokenCenterY = rowY + 8.0f;

        const sf::Vector2u tokenSize = tokenTexture.getSize();
        const bool hasUsableTokenTexture = (tokenSize.x > 2 && tokenSize.y > 2);
        if (hasUsableTokenTexture) {
            sf::Sprite tokenSprite(tokenTexture);
            const sf::FloatRect bounds = tokenSprite.getLocalBounds();
            tokenSprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            if (bounds.width > 0.0f && bounds.height > 0.0f) {
                tokenSprite.setScale(kTokenSize / bounds.width, kTokenSize / bounds.height);
            }
            tokenSprite.setPosition(tokenCenterX, tokenCenterY);
            window.draw(tokenSprite);
        } else {
            sf::CircleShape fallbackToken(12.0f);
            fallbackToken.setOrigin(12.0f, 12.0f);
            fallbackToken.setFillColor(kBoardPieceColors[static_cast<size_t>(row.tokenIndex) % kBoardPieceColors.size()]);
            fallbackToken.setOutlineThickness(2.0f);
            fallbackToken.setOutlineColor(sf::Color::White);
            fallbackToken.setPosition(tokenCenterX, tokenCenterY);
            window.draw(fallbackToken);
        }
    }

    window.draw(cashText);
    window.draw(assetsText);
    window.draw(totalText);
}

void LeaderboardView::render(sf::RenderWindow& window) const {
    window.draw(m_panelSprite);

    const int visibleRows = std::min(static_cast<int>(m_rows.size()), 4);
    for (int i = 0; i < visibleRows; ++i) {
        drawRowText(window, m_rows[static_cast<size_t>(i)], i);
    }
}
} // namespace viewsGUI
