#include "../../include/viewsGUI/SpriteButton.hpp"

#include <iostream>

namespace viewsGUI {
SpriteButton::SpriteButton()
    : m_visible(true),
      m_enabled(true),
      m_hovered(false),
      m_pressed(false),
      m_onClick(nullptr) {}

bool SpriteButton::loadTextures(const std::string& normalPath,
                                const std::string& hoverPath,
                                const std::string& pressedPath,
                                const std::string& disabledPath) {
    bool ok = true;

    if (!m_normalTexture.loadFromFile(normalPath)) {
        std::cerr << "[ERROR] Gagal memuat tombol normal: " << normalPath << "\n";
        ok = false;
    }

    if (!m_hoverTexture.loadFromFile(hoverPath)) {
        std::cerr << "[WARN] Gagal memuat tombol hover: " << hoverPath << " (fallback normal)\n";
        m_hoverTexture = m_normalTexture;
    }

    if (!m_pressedTexture.loadFromFile(pressedPath)) {
        std::cerr << "[WARN] Gagal memuat tombol pressed: " << pressedPath << " (fallback hover)\n";
        m_pressedTexture = m_hoverTexture;
    }

    if (!m_disabledTexture.loadFromFile(disabledPath)) {
        std::cerr << "[WARN] Gagal memuat tombol disabled: " << disabledPath << " (fallback normal)\n";
        m_disabledTexture = m_normalTexture;
    }

    m_sprite.setTexture(m_normalTexture);
    applyVisualState();
    return ok;
}

void SpriteButton::setPosition(sf::Vector2f position) {
    m_sprite.setPosition(position);
}

void SpriteButton::setScale(sf::Vector2f scale) {
    m_sprite.setScale(scale);
}

void SpriteButton::setVisible(bool visible) {
    m_visible = visible;
    if (!m_visible) {
        m_pressed = false;
        m_hovered = false;
    }
    applyVisualState();
}

void SpriteButton::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (!m_enabled) {
        m_pressed = false;
    }
    applyVisualState();
}

void SpriteButton::setOnClick(std::function<void()> callback) {
    m_onClick = std::move(callback);
}

bool SpriteButton::contains(sf::Vector2f point) const {
    if (!m_visible) {
        return false;
    }
    return m_sprite.getGlobalBounds().contains(point);
}

void SpriteButton::update(sf::Vector2f mousePos) {
    if (!m_visible || !m_enabled) {
        m_hovered = false;
        applyVisualState();
        return;
    }

    m_hovered = contains(mousePos);
    applyVisualState();
}

bool SpriteButton::handleMousePressed(sf::Vector2f mousePos) {
    if (!m_visible || !m_enabled || !contains(mousePos)) {
        return false;
    }

    m_pressed = true;
    applyVisualState();
    return true;
}

bool SpriteButton::handleMouseReleased(sf::Vector2f mousePos) {
    if (!m_visible) {
        return false;
    }

    const bool wasPressed = m_pressed;
    m_pressed = false;
    applyVisualState();

    if (!m_enabled || !wasPressed || !contains(mousePos)) {
        return false;
    }

    if (m_onClick) {
        m_onClick();
    }
    return true;
}

void SpriteButton::applyVisualState() {
    if (!m_visible) {
        return;
    }

    if (!m_enabled) {
        m_sprite.setTexture(m_disabledTexture);
        return;
    }

    if (m_pressed) {
        m_sprite.setTexture(m_pressedTexture);
        return;
    }

    if (m_hovered) {
        m_sprite.setTexture(m_hoverTexture);
        return;
    }

    m_sprite.setTexture(m_normalTexture);
}

void SpriteButton::render(sf::RenderWindow& window) const {
    if (!m_visible) {
        return;
    }
    window.draw(m_sprite);
}
} // namespace viewsGUI
