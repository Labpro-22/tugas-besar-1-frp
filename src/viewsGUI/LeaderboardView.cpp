#include "../../include/viewsGUI/LeaderboardView.hpp"

#include "../../include/models/Player.hpp"

#include <algorithm>
#include <iostream>

namespace viewsGUI {
LeaderboardView::LeaderboardView(const sf::Font& headerFont, const sf::Font& bodyFont)
    : m_headerFont(headerFont), m_bodyFont(bodyFont), m_position(0.0f, 0.0f) {}

bool LeaderboardView::loadAssets(const std::string& panelPath) {
    if (!m_panelTexture.loadFromFile(panelPath)) {
        std::cerr << "[ERROR] Gagal memuat panel leaderboard: " << panelPath << "\n";
        return false;
    }

    m_panelSprite.setTexture(m_panelTexture);
    return true;
}

void LeaderboardView::setPosition(sf::Vector2f position) {
    m_position = position;
    m_panelSprite.setPosition(m_position);
}

void LeaderboardView::updateFromPlayers(const std::vector<Player*>& players) {
    m_rows.clear();
    m_rows.reserve(players.size());

    for (const Player* player : players) {
        if (!player) {
            continue;
        }

        const int cash = player->getMoney();
        const int assets = std::max(0, player->getTotalWealth() - cash);
        const int total = player->getTotalWealth();
        m_rows.push_back(Row{player->getUsername(), cash, assets, total});
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
    assetsText.setPosition(originX + 560.0f, rowY);

    sf::Text totalText(std::to_string(row.total), m_bodyFont, 32);
    totalText.setFillColor(sf::Color(53, 45, 36));
    const sf::FloatRect totalBounds = totalText.getLocalBounds();
    totalText.setOrigin(totalBounds.left + totalBounds.width, totalBounds.top);
    totalText.setPosition(originX + 770.0f, rowY);

    window.draw(playerText);
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
