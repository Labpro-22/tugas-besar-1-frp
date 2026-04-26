#include "../../include/viewsGUI/LeaderboardView.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace viewsGUI {
namespace {
constexpr int kMaxPlayerTokens = 4;
constexpr float kTokenSize = 28.0f;
constexpr unsigned int kLeaderboardFontSize = 32;
constexpr std::size_t kMaxPlayerNameChars = 7;
constexpr std::size_t kMaxNumericDigits = 4;

// Leaderboard text layout tuning block.
// Atur posisi kolom/row leaderboard di sini.
struct LeaderboardTextLayout {
    static constexpr float kHeaderY = 126.0f;
    static constexpr float kRowStartY = 166.0f;
    static constexpr float kRowStepY = 65.0f;

    static constexpr float kRankX = 116.0f;
    static constexpr float kPlayerX = 188.0f;
    static constexpr float kCashX = 470.0f;
    static constexpr float kAssetX = 620.0f;
    static constexpr float kPropertyX = 740.0f;
};

const std::array<sf::Color, 6> kBoardPieceColors = {
    sf::Color(216, 83, 79),
    sf::Color(84, 143, 224),
    sf::Color(93, 170, 104),
    sf::Color(240, 190, 80),
    sf::Color(148, 92, 181),
    sf::Color(100, 100, 100)};

float textWidth(const sf::Text& text) {
    return text.getLocalBounds().width;
}

sf::Text makeMeasureText(const std::string& content, const sf::Font& font) {
    return sf::Text(content, font, kLeaderboardFontSize);
}

std::size_t countDigits(const std::string& value) {
    std::size_t digits = 0;
    for (unsigned char ch : value) {
        if (std::isdigit(ch)) {
            ++digits;
        }
    }
    return digits;
}

void shrinkTextToFitWidth(sf::Text& text, float maxWidth) {
    text.setScale(1.0f, 1.0f);
    if (maxWidth <= 0.0f) {
        return;
    }

    const float width = textWidth(text);
    if (width <= 0.0f || width <= maxWidth) {
        return;
    }

    const float scale = maxWidth / width;
    text.setScale(scale, scale);
}
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

void LeaderboardView::updateFromLeaderboard(const std::vector<Leaderboard>& rows) {
    m_rows.clear();
    m_rows.reserve(rows.size());

    for (const Leaderboard& row : rows) {
        m_rows.push_back(Row{
            row.getRank(),
            row.getPlayerName(),
            row.getCash(),
            row.getAsset(),
            row.getPropertyCount(),
            row.getTokenIndex()});
    }
}

void LeaderboardView::drawHeaderText(sf::RenderWindow& window) const {
    (void)window;
}

void LeaderboardView::drawRowText(sf::RenderWindow& window, const Row& row, int rowIndex) const {
    const float originX = m_position.x;
    const float rowY = m_position.y + LeaderboardTextLayout::kRowStartY +
                       static_cast<float>(rowIndex) * LeaderboardTextLayout::kRowStepY;
    const float maxNumericWidth = textWidth(makeMeasureText("9999", m_bodyFont));

    const std::string rankValue = std::to_string(row.rank);
    sf::Text rankText(rankValue, m_bodyFont, kLeaderboardFontSize);
    rankText.setFillColor(sf::Color(53, 45, 36));
    if (countDigits(rankValue) > kMaxNumericDigits) {
        shrinkTextToFitWidth(rankText, maxNumericWidth);
    }
    const sf::FloatRect rankBounds = rankText.getLocalBounds();
    rankText.setOrigin(rankBounds.left + rankBounds.width, rankBounds.top);
    rankText.setPosition(originX + LeaderboardTextLayout::kRankX, rowY);

    sf::Text playerText(row.player, m_bodyFont, kLeaderboardFontSize);
    playerText.setFillColor(sf::Color(53, 45, 36));
    if (row.player.size() > kMaxPlayerNameChars) {
        const std::string widthSample = row.player.substr(0, kMaxPlayerNameChars);
        const float maxPlayerWidth = textWidth(makeMeasureText(widthSample, m_bodyFont));
        shrinkTextToFitWidth(playerText, maxPlayerWidth);
    }
    playerText.setPosition(originX + LeaderboardTextLayout::kPlayerX, rowY - 10.0f);

    const std::string cashValue = std::to_string(row.cash);
    sf::Text cashText(cashValue, m_bodyFont, kLeaderboardFontSize);
    cashText.setFillColor(sf::Color(53, 45, 36));
    if (countDigits(cashValue) > kMaxNumericDigits) {
        shrinkTextToFitWidth(cashText, maxNumericWidth);
    }
    const sf::FloatRect cashBounds = cashText.getLocalBounds();
    cashText.setOrigin(cashBounds.left + cashBounds.width, cashBounds.top);
    cashText.setPosition(originX + LeaderboardTextLayout::kCashX, rowY);

    const std::string assetValue = std::to_string(row.asset);
    sf::Text assetText(assetValue, m_bodyFont, kLeaderboardFontSize);
    assetText.setFillColor(sf::Color(53, 45, 36));
    if (countDigits(assetValue) > kMaxNumericDigits) {
        shrinkTextToFitWidth(assetText, maxNumericWidth);
    }
    const sf::FloatRect assetBounds = assetText.getLocalBounds();
    assetText.setOrigin(assetBounds.left + assetBounds.width, assetBounds.top);
    assetText.setPosition(originX + LeaderboardTextLayout::kAssetX, rowY);

    const std::string propertyValue = std::to_string(row.propertyCount);
    sf::Text propertyText(propertyValue, m_bodyFont, kLeaderboardFontSize);
    propertyText.setFillColor(sf::Color(53, 45, 36));
    if (countDigits(propertyValue) > kMaxNumericDigits) {
        shrinkTextToFitWidth(propertyText, maxNumericWidth);
    }
    const sf::FloatRect propertyBounds = propertyText.getLocalBounds();
    propertyText.setOrigin(propertyBounds.left + propertyBounds.width, propertyBounds.top);
    propertyText.setPosition(originX + LeaderboardTextLayout::kPropertyX, rowY);

    window.draw(rankText);
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
    window.draw(assetText);
    window.draw(propertyText);
}

void LeaderboardView::render(sf::RenderWindow& window) const {
    window.draw(m_panelSprite);
    drawHeaderText(window);

    const int visibleRows = std::min(static_cast<int>(m_rows.size()), 4);
    for (int i = 0; i < visibleRows; ++i) {
        drawRowText(window, m_rows[static_cast<size_t>(i)], i);
    }
}
} // namespace viewsGUI
