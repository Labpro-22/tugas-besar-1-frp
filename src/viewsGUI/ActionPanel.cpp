#include "../../include/viewsGUI/ActionPanel.hpp"

#include "../../include/viewsGUI/Theme.hpp"

#include <algorithm>

namespace viewsGUI {
ActionPanel::ActionPanel(sf::Vector2f position, sf::Vector2f size, const sf::Font& font)
    : m_position(position), m_size(size), m_font(font) {
    m_background.setPosition(m_position);
    m_background.setSize(m_size);
    m_background.setFillColor(Theme::Panel);
    initLayout();
}

void ActionPanel::initLayout() {
    m_statusBox.setPosition({m_position.x + 20.0f, m_position.y + 20.0f});
    m_statusBox.setSize({m_size.x - 40.0f, 110.0f});
    m_statusBox.setFillColor(Theme::PanelSection);
    m_statusBox.setOutlineThickness(2.0f);
    m_statusBox.setOutlineColor(Theme::Kuning);

    m_titleText.setFont(m_font);
    m_titleText.setString("GILIRAN SAAT INI");
    m_titleText.setCharacterSize(14);
    m_titleText.setFillColor(sf::Color(200, 200, 200));
    m_titleText.setPosition({m_position.x + 30.0f, m_position.y + 28.0f});

    m_playerInfoText.setFont(m_font);
    m_playerInfoText.setString("Pemain -");
    m_playerInfoText.setCharacterSize(22);
    m_playerInfoText.setFillColor(sf::Color::White);
    m_playerInfoText.setPosition({m_position.x + 30.0f, m_position.y + 52.0f});

    m_turnInfoText.setFont(m_font);
    m_turnInfoText.setString("Turn 0/0");
    m_turnInfoText.setCharacterSize(14);
    m_turnInfoText.setFillColor(sf::Color(220, 220, 220));
    m_turnInfoText.setPosition({m_position.x + 30.0f, m_position.y + 84.0f});

    const float startY = m_position.y + 150.0f;
    const float padding = 20.0f;
    const float btnWidth = (m_size.x - (3.0f * padding)) / 2.0f;
    const float btnHeight = 54.0f;

    m_btnRollDice = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding, startY),
        sf::Vector2f(btnWidth, btnHeight),
        "LEMPAR DADU",
        m_font);

    m_btnBuild = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding * 2.0f + btnWidth, startY),
        sf::Vector2f(btnWidth, btnHeight),
        "BANGUN RUMAH",
        m_font);

    m_btnMortgage = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding, startY + btnHeight + padding),
        sf::Vector2f(btnWidth, btnHeight),
        "GADAI ASET",
        m_font);

    m_btnUseCard = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding * 2.0f + btnWidth, startY + btnHeight + padding),
        sf::Vector2f(btnWidth, btnHeight),
        "PAKAI KARTU",
        m_font);

    m_btnSave = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding, startY + 2.0f * (btnHeight + padding)),
        sf::Vector2f(btnWidth, btnHeight),
        "SIMPAN",
        m_font);

    m_btnLoad = std::make_shared<Button>(
        sf::Vector2f(m_position.x + padding * 2.0f + btnWidth, startY + 2.0f * (btnHeight + padding)),
        sf::Vector2f(btnWidth, btnHeight),
        "MUAT",
        m_font);

    m_buttons = {m_btnRollDice, m_btnBuild, m_btnMortgage, m_btnUseCard, m_btnSave, m_btnLoad};

    m_logBox.setPosition({m_position.x + 20.0f, m_position.y + 360.0f});
    m_logBox.setSize({m_size.x - 40.0f, m_size.y - 380.0f});
    m_logBox.setFillColor(sf::Color(30, 34, 38));
    m_logBox.setOutlineThickness(2.0f);
    m_logBox.setOutlineColor(Theme::BiruTua);

    m_logText.setFont(m_font);
    m_logText.setCharacterSize(14);
    m_logText.setFillColor(sf::Color(225, 225, 225));
    m_logText.setPosition({m_logBox.getPosition().x + 12.0f, m_logBox.getPosition().y + 12.0f});
    m_logText.setString("Siap memulai game.");

    m_btnBuild->setEnabled(false);
    m_btnMortgage->setEnabled(false);
    m_btnUseCard->setEnabled(false);
}

void ActionPanel::updatePlayerInfo(const std::string& playerName, int money, int turn, int maxTurn) {
    m_playerInfoText.setString(playerName + " | Uang: M" + std::to_string(money));

    const int safeMaxTurn = std::max(0, maxTurn);
    if (safeMaxTurn > 0) {
        m_turnInfoText.setString("Turn " + std::to_string(turn) + "/" + std::to_string(safeMaxTurn));
        return;
    }

    m_turnInfoText.setString("Turn " + std::to_string(turn));
}

void ActionPanel::setSystemMessage(const std::string& message) { m_logText.setString(message); }

void ActionPanel::update(sf::Vector2f mousePos) {
    for (const auto& btn : m_buttons) {
        btn->update(mousePos);
    }
}

void ActionPanel::handleMouseClick(sf::Vector2f mousePos) {
    for (const auto& btn : m_buttons) {
        if (btn->handleMouseClick(mousePos)) {
            break;
        }
    }
}

void ActionPanel::render(sf::RenderWindow& window) const {
    window.draw(m_background);
    window.draw(m_statusBox);
    window.draw(m_titleText);
    window.draw(m_playerInfoText);
    window.draw(m_turnInfoText);
    window.draw(m_logBox);
    window.draw(m_logText);

    for (const auto& btn : m_buttons) {
        btn->render(window);
    }
}
} // namespace viewsGUI
