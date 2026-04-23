#include "../../include/viewsGUI/PieceRenderer.hpp"

#include <cmath>

namespace viewsGUI {
PieceRenderer::PieceRenderer(int playerId, sf::Color playerColor, sf::Vector2f startPos)
    : m_useTokenSprite(false), m_isMoving(false), m_speed(360.0f) {
    m_shape.setRadius(12.0f);
    m_shape.setOrigin(12.0f, 12.0f);
    m_shape.setFillColor(playerColor);
    m_shape.setOutlineThickness(2.0f);
    m_shape.setOutlineColor(sf::Color::White);

    const float offsetX = (playerId % 2 == 0) ? -10.0f : 10.0f;
    const float offsetY = (playerId > 1) ? 10.0f : -10.0f;
    m_offset = sf::Vector2f(offsetX, offsetY);

    m_currentPos = startPos + m_offset;
    m_shape.setPosition(m_currentPos);
    m_tokenSprite.setPosition(m_currentPos);
}

bool PieceRenderer::loadTokenTexture(const std::string& texturePath) {
    if (!m_tokenTexture.loadFromFile(texturePath)) {
        m_useTokenSprite = false;
        return false;
    }

    const sf::Vector2u textureSize = m_tokenTexture.getSize();
    if (textureSize.x <= 2 || textureSize.y <= 2) {
        // Placeholder 1x1/2x2 should not override fallback piece shape.
        m_useTokenSprite = false;
        return false;
    }

    m_useTokenSprite = true;
    m_tokenSprite.setTexture(m_tokenTexture);

    const sf::FloatRect bounds = m_tokenSprite.getLocalBounds();
    m_tokenSprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    if (bounds.width > 0.0f && bounds.height > 0.0f) {
        m_tokenSprite.setScale(28.0f / bounds.width, 28.0f / bounds.height);
    }
    m_tokenSprite.setPosition(m_currentPos);
    return true;
}

void PieceRenderer::moveAlongPath(const std::vector<sf::Vector2f>& pathQueue) {
    while (!m_pathQueue.empty()) {
        m_pathQueue.pop();
    }

    for (const sf::Vector2f& pos : pathQueue) {
        m_pathQueue.push(pos + m_offset);
    }

    m_isMoving = !m_pathQueue.empty();
}

void PieceRenderer::update(sf::Time dt) {
    if (m_pathQueue.empty()) {
        m_isMoving = false;
        return;
    }

    const sf::Vector2f targetPos = m_pathQueue.front();
    sf::Vector2f direction = targetPos - m_currentPos;
    const float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 4.0f) {
        m_currentPos = targetPos;
        m_pathQueue.pop();
    } else {
        direction /= distance;
        m_currentPos += direction * m_speed * dt.asSeconds();
    }

    m_shape.setPosition(m_currentPos);
    m_tokenSprite.setPosition(m_currentPos);
}

void PieceRenderer::render(sf::RenderWindow& window) const {
    if (m_useTokenSprite) {
        window.draw(m_tokenSprite);
        return;
    }
    window.draw(m_shape);
}

void PieceRenderer::snapTo(sf::Vector2f tileCenter) {
    m_currentPos = tileCenter + m_offset;
    m_shape.setPosition(m_currentPos);
    m_tokenSprite.setPosition(m_currentPos);
}
} // namespace viewsGUI
