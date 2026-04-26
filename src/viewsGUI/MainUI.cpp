#include "../../include/viewsGUI/MainUI.hpp"

#include "../../include/viewsGUI/Layout1920.hpp"
#include "../../include/models/Player.hpp"

#include <iostream>

namespace viewsGUI {
MainUI::MainUI(const sf::Font& titleFont, const sf::Font& bodyFont)
    : m_titleFont(titleFont),
      m_bodyFont(bodyFont),
      m_leaderboardView(titleFont, bodyFont),
      m_assetPanel(titleFont, bodyFont),
      m_rollVisible(true) {}

bool MainUI::loadAssets(const std::string& uiDir, const std::string& boardDir) {
    bool success = true;

    const std::string baseUi = (!uiDir.empty() && uiDir.back() == '/') ? uiDir : uiDir + "/";

    if (!m_backgroundTexture.loadFromFile(baseUi + "bg_wood_1920x1080.png")) {
        std::cerr << "[ERROR] Gagal memuat background utama.\n";
        success = false;
    }

    m_backgroundSprite.setTexture(m_backgroundTexture);

    if (m_backgroundTexture.getSize().x > 0 && m_backgroundTexture.getSize().y > 0) {
        const float sx = Layout1920::kDesignWidth / static_cast<float>(m_backgroundTexture.getSize().x);
        const float sy = Layout1920::kDesignHeight / static_cast<float>(m_backgroundTexture.getSize().y);
        m_backgroundSprite.setScale(sx, sy);
    }

    if (!m_rollDiceButton.loadTextures(baseUi + "btn_roll_dice_normal.png",
                                       baseUi + "btn_roll_dice_hover.png",
                                       baseUi + "btn_roll_dice_pressed.png",
                                       baseUi + "btn_roll_dice_disabled.png")) {
        success = false;
    }
    m_rollDiceButton.setPosition(Layout1920::kRollBtnPos);

    m_turnStatusText.setFont(m_titleFont);
    m_turnStatusText.setCharacterSize(Layout1920::kTurnStatusCharSize);
    m_turnStatusText.setFillColor(sf::Color(246, 236, 210));
    m_turnStatusText.setOutlineColor(sf::Color(49, 38, 24));
    m_turnStatusText.setOutlineThickness(2.0f);
    m_turnStatusText.setString("P1's Turn");
    {
        const sf::FloatRect bounds = m_turnStatusText.getLocalBounds();
        m_turnStatusText.setOrigin(bounds.left + (bounds.width * 0.5f), bounds.top);
        m_turnStatusText.setPosition(Layout1920::kTurnStatusPos);
    }

    if (!m_tabAsset.loadTextures(baseUi + "tab_asset_normal.png",
                                 baseUi + "tab_asset_normal.png",
                                 baseUi + "tab_asset_active.png",
                                 baseUi + "tab_asset_active.png")) {
        success = false;
    }
    if (!m_tabLog.loadTextures(baseUi + "tab_log_normal.png",
                               baseUi + "tab_log_normal.png",
                               baseUi + "tab_log_active.png",
                               baseUi + "tab_log_active.png")) {
        success = false;
    }
    if (!m_tabInventory.loadTextures(baseUi + "tab_inventory_normal.png",
                                     baseUi + "tab_inventory_normal.png",
                                     baseUi + "tab_inventory_active.png",
                                     baseUi + "tab_inventory_active.png")) {
        success = false;
    }

    m_tabAsset.setPosition(Layout1920::kTabAssetPos);
    m_tabLog.setPosition(Layout1920::kTabLogPos);
    m_tabInventory.setPosition(Layout1920::kTabInventoryPos);

    m_tabAsset.setOnClick([this]() { this->setPanelMode(AssetPanel::Mode::ASSET); });
    m_tabLog.setOnClick([this]() { this->setPanelMode(AssetPanel::Mode::LOG); });
    m_tabInventory.setOnClick([this]() { this->setPanelMode(AssetPanel::Mode::INVENTORY); });

    if (!m_leaderboardView.loadAssets(baseUi + "panel_leaderboard.png")) {
        success = false;
    }
    m_leaderboardView.setPosition(Layout1920::kLeaderboardPos);

    if (!m_assetPanel.loadAssets(baseUi, boardDir)) {
        success = false;
    }
    m_assetPanel.setPosition(Layout1920::kPanelPos);

    setPanelMode(AssetPanel::Mode::ASSET);
    setRollVisible(true);

    return success;
}

void MainUI::setOnRollDice(std::function<void()> callback) {
    m_rollDiceButton.setOnClick(std::move(callback));
}

void MainUI::setRollEnabled(bool enabled) {
    m_rollDiceButton.setEnabled(enabled);
}

void MainUI::setRollVisible(bool visible) {
    m_rollVisible = visible;
    m_rollDiceButton.setVisible(visible);
}

void MainUI::setPanelMode(AssetPanel::Mode mode) {
    m_assetPanel.setMode(mode);
    syncTabActiveState();
}

AssetPanel::Mode MainUI::getPanelMode() const {
    return m_assetPanel.getMode();
}

void MainUI::syncTabActiveState() {
    const AssetPanel::Mode mode = m_assetPanel.getMode();

    m_tabAsset.setEnabled(mode != AssetPanel::Mode::ASSET);
    m_tabLog.setEnabled(mode != AssetPanel::Mode::LOG);
    m_tabInventory.setEnabled(mode != AssetPanel::Mode::INVENTORY);
}

void MainUI::updateData(const std::vector<Leaderboard>& leaderboardRows,
                        const Player& currentPlayer,
                        const std::string& systemLog,
                        int currentTurn,
                        int maxTurn) {
    m_leaderboardView.updateFromLeaderboard(leaderboardRows);
    m_assetPanel.updateData(currentPlayer, systemLog);
    std::string turnLabel = std::to_string(currentTurn) + "/";
    if (maxTurn > 0) {
        turnLabel += std::to_string(maxTurn);
    } else {
        turnLabel += "-";
    }
    m_turnStatusText.setString(currentPlayer.getUsername() + "'s turn " + turnLabel);
    const sf::FloatRect bounds = m_turnStatusText.getLocalBounds();
    m_turnStatusText.setOrigin(bounds.left + (bounds.width * 0.5f), bounds.top);
    m_turnStatusText.setPosition(Layout1920::kTurnStatusPos);
}

void MainUI::update(sf::Vector2f mousePos) {
    m_assetPanel.update(mousePos);

    m_tabAsset.update(mousePos);
    m_tabLog.update(mousePos);
    m_tabInventory.update(mousePos);

    if (m_rollVisible) {
        m_rollDiceButton.update(mousePos);
    }
}

bool MainUI::handleMouseWheel(float delta, sf::Vector2f mousePos) {
    return m_assetPanel.handleMouseWheel(delta, mousePos);
}

bool MainUI::handleMousePressed(sf::Vector2f mousePos) {
    if (m_assetPanel.handleMousePressed(mousePos)) {
        return true;
    }

    if (m_rollVisible && m_rollDiceButton.handleMousePressed(mousePos)) {
        return true;
    }

    if (m_tabAsset.handleMousePressed(mousePos)) {
        return true;
    }
    if (m_tabLog.handleMousePressed(mousePos)) {
        return true;
    }
    if (m_tabInventory.handleMousePressed(mousePos)) {
        return true;
    }

    return false;
}

bool MainUI::handleMouseReleased(sf::Vector2f mousePos) {
    if (m_assetPanel.handleMouseReleased(mousePos)) {
        return true;
    }

    bool consumed = false;

    if (m_rollVisible && m_rollDiceButton.handleMouseReleased(mousePos)) {
        consumed = true;
    }

    if (m_tabAsset.handleMouseReleased(mousePos)) {
        consumed = true;
    }
    if (m_tabLog.handleMouseReleased(mousePos)) {
        consumed = true;
    }
    if (m_tabInventory.handleMouseReleased(mousePos)) {
        consumed = true;
    }

    return consumed;
}

bool MainUI::isRollDiceButtonHit(sf::Vector2f mousePos) const {
    return m_rollVisible && m_rollDiceButton.isVisible() && m_rollDiceButton.isEnabled() &&
           m_rollDiceButton.contains(mousePos);
}

void MainUI::renderBackground(sf::RenderWindow& window) const {
    window.draw(m_backgroundSprite);
}

void MainUI::renderOverlay(sf::RenderWindow& window) const {
    m_leaderboardView.render(window);
    m_assetPanel.render(window);

    m_tabAsset.render(window);
    m_tabLog.render(window);
    m_tabInventory.render(window);

    if (m_rollVisible) {
        m_rollDiceButton.render(window);
    }
    window.draw(m_turnStatusText);
}
} // namespace viewsGUI
