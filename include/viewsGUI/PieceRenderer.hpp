#ifndef VIEWSGUI_PIECE_RENDERER_HPP
#define VIEWSGUI_PIECE_RENDERER_HPP

#include <SFML/Graphics.hpp>

#include <queue>
#include <vector>

namespace viewsGUI {
class PieceRenderer {
public:
    PieceRenderer(int playerId, sf::Color playerColor, sf::Vector2f startPos);

    void moveAlongPath(const std::vector<sf::Vector2f>& pathQueue);
    void update(sf::Time dt);
    void render(sf::RenderWindow& window) const;

    void snapTo(sf::Vector2f tileCenter);
    bool isMoving() const { return m_isMoving; }

private:
    sf::CircleShape m_shape;
    sf::Vector2f m_currentPos;
    std::queue<sf::Vector2f> m_pathQueue;
    bool m_isMoving;
    float m_speed;
    sf::Vector2f m_offset;
};
} // namespace viewsGUI

#endif
