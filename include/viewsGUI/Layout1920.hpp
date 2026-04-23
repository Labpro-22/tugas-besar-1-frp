#ifndef VIEWSGUI_LAYOUT_1920_HPP
#define VIEWSGUI_LAYOUT_1920_HPP

#include <SFML/Graphics.hpp>

namespace viewsGUI::Layout1920 {
constexpr float kDesignWidth = 1920.0f;
constexpr float kDesignHeight = 1080.0f;

constexpr float kBoardSize = 960.0f;
const sf::Vector2f kBoardOrigin(72.0f, 16.0f);

const sf::Vector2f kLeaderboardPos(1058.0f, 16.0f);
const sf::Vector2f kPanelPos(1070.0f, 450.0f);

const sf::Vector2f kTurnBadgePos(420.0f, 600.0f);
const sf::Vector2f kRollBtnPos(420.0f, 655.0f);
const sf::Vector2f kTurnStatusPos(570.0f, 660.0f);
constexpr unsigned int kTurnStatusCharSize = 40;

const sf::Vector2f kTabAssetPos(84.0f, 988.0f);
const sf::Vector2f kTabLogPos(400.0f, 988.0f);
const sf::Vector2f kTabInventoryPos(700.0f, 988.0f);

const sf::Vector2f kDiceCenterOffsetFromBoardCenter(30.0f, -85.0f);
} // namespace viewsGUI::Layout1920

#endif
