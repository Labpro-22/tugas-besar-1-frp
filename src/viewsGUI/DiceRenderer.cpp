#include "../../include/viewsGUI/DiceRenderer.hpp"

#include <cmath>
#include <iostream>

namespace viewsGUI {
DiceRenderer::DiceRenderer()
    : m_isRolling(false),
      m_isVisible(false),
      m_isFadingOut(false),
      m_target1(1),
      m_target2(1),
      m_rollDuration(1.5f),
      m_timeElapsed(0.0f),
      m_frameTimer(0.0f),
      m_frameInterval(0.05f),
      m_fadeDuration(0.45f),
      m_fadeElapsed(0.0f),
      m_resultHoldDuration(1.0f),
      m_resultHoldElapsed(0.0f),
      m_isHoldingResult(false),
      m_doubleResult(false),
      m_doubleFxTimer(0.0f),
      m_rng(std::random_device{}()) {
    m_faceTextures.resize(6);
}

bool DiceRenderer::loadAssets(const std::string& assetDir) {
    bool success = true;

    const std::string base = (!assetDir.empty() && assetDir.back() == '/') ? assetDir : assetDir + "/";
    for (int i = 0; i < 6; ++i) {
        const std::string filename = base + "dice_" + std::to_string(i + 1) + ".png";
        if (!m_faceTextures[static_cast<size_t>(i)].loadFromFile(filename)) {
            std::cerr << "[ERROR] Gagal memuat dadu: " << filename << "\n";
            success = false;
        }
    }

    if (!success) {
        return false;
    }

    m_dice1.setTexture(m_faceTextures[0]);
    m_dice2.setTexture(m_faceTextures[0]);

    const sf::FloatRect bounds = m_dice1.getLocalBounds();
    m_dice1.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    m_dice2.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

    return true;
}

void DiceRenderer::startRoll(int resultDice1, int resultDice2, sf::Vector2f centerPos) {
    m_target1 = resultDice1;
    m_target2 = resultDice2;

    m_isRolling = true;
    m_isVisible = true;
    m_isFadingOut = false;
    m_isHoldingResult = false;

    m_timeElapsed = 0.0f;
    m_frameTimer = 0.0f;
    m_fadeElapsed = 0.0f;
    m_resultHoldElapsed = 0.0f;

    m_doubleResult = (m_target1 == m_target2);
    m_doubleFxTimer = 0.0f;

    // Keep both dice horizontally aligned with wider spacing so they never stack.
    m_basePos1 = {centerPos.x - 130.0f, centerPos.y};
    m_basePos2 = {centerPos.x + 130.0f, centerPos.y};

    m_dice1.setPosition(m_basePos1);
    m_dice2.setPosition(m_basePos2);
    m_dice1.setRotation(0.0f);
    m_dice2.setRotation(0.0f);
    applyAlpha(255);
}

void DiceRenderer::applyAlpha(unsigned int alpha) {
    m_dice1.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
    m_dice2.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
}

int DiceRenderer::randomFaceIndex() {
    std::uniform_int_distribution<int> dist(0, 5);
    return dist(m_rng);
}

float DiceRenderer::randomRange(float minValue, float maxValue) {
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    return dist(m_rng);
}

void DiceRenderer::update(sf::Time dt) {
    if (!m_isVisible) {
        return;
    }

    const float dtSec = dt.asSeconds();

    if (m_isRolling) {
        m_timeElapsed += dtSec;
        m_frameTimer += dtSec;

        const float progress = std::min(1.0f, m_timeElapsed / m_rollDuration);
        const float easeOut = 1.0f - (progress * progress);
        const float rotationAmplitude = 22.0f * easeOut;
        const float shakeAmplitude = 8.0f * easeOut;

        if (m_frameTimer >= m_frameInterval) {
            m_frameTimer = 0.0f;
            m_dice1.setTexture(m_faceTextures[static_cast<size_t>(randomFaceIndex())]);
            m_dice2.setTexture(m_faceTextures[static_cast<size_t>(randomFaceIndex())]);

            m_dice1.setRotation(randomRange(-rotationAmplitude, rotationAmplitude));
            m_dice2.setRotation(randomRange(-rotationAmplitude, rotationAmplitude));

            const float sharedYOffset = randomRange(-shakeAmplitude * 0.35f, shakeAmplitude * 0.35f);
            m_dice1.setPosition(m_basePos1.x + randomRange(-shakeAmplitude, shakeAmplitude),
                                m_basePos1.y + sharedYOffset);
            m_dice2.setPosition(m_basePos2.x + randomRange(-shakeAmplitude, shakeAmplitude),
                                m_basePos2.y + sharedYOffset);
        }

        if (m_timeElapsed >= m_rollDuration) {
            m_isRolling = false;
            m_isHoldingResult = true;
            m_isFadingOut = false;
            m_fadeElapsed = 0.0f;
            m_resultHoldElapsed = 0.0f;

            m_dice1.setTexture(m_faceTextures[static_cast<size_t>(m_target1 - 1)]);
            m_dice2.setTexture(m_faceTextures[static_cast<size_t>(m_target2 - 1)]);
            m_dice1.setRotation(-14.0f);
            m_dice2.setRotation(9.0f);
            m_dice1.setPosition(m_basePos1);
            m_dice2.setPosition(m_basePos2);

            if (m_doubleResult) {
                m_doubleFxTimer = 0.5f;
            }
        }
    }

    if (m_doubleFxTimer > 0.0f) {
        m_doubleFxTimer = std::max(0.0f, m_doubleFxTimer - dtSec);
    }

    if (m_isHoldingResult) {
        m_resultHoldElapsed += dtSec;
        if (m_resultHoldElapsed >= m_resultHoldDuration) {
            m_isHoldingResult = false;
            m_isFadingOut = true;
            m_fadeElapsed = 0.0f;
        }
    }

    if (m_isFadingOut) {
        m_fadeElapsed += dtSec;
        const float fadeT = std::min(1.0f, m_fadeElapsed / m_fadeDuration);
        const unsigned int alpha = static_cast<unsigned int>(255.0f * (1.0f - fadeT));
        applyAlpha(alpha);

        if (fadeT >= 1.0f) {
            m_isFadingOut = false;
            m_isVisible = false;
        }
    }
}

void DiceRenderer::render(sf::RenderWindow& window) const {
    if (!m_isVisible) {
        return;
    }

    if (m_doubleFxTimer > 0.0f) {
        const float glowPulse = 0.7f + 0.3f * std::sin((0.5f - m_doubleFxTimer) * 30.0f);
        sf::Sprite glow1 = m_dice1;
        sf::Sprite glow2 = m_dice2;

        glow1.setColor(sf::Color(255, 240, 130, 120));
        glow2.setColor(sf::Color(255, 240, 130, 120));
        glow1.setScale(1.0f + glowPulse * 0.14f, 1.0f + glowPulse * 0.14f);
        glow2.setScale(1.0f + glowPulse * 0.14f, 1.0f + glowPulse * 0.14f);

        window.draw(glow1, sf::BlendAdd);
        window.draw(glow2, sf::BlendAdd);
    }

    window.draw(m_dice1);
    window.draw(m_dice2);
}
} // namespace viewsGUI
