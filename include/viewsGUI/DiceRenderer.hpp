#ifndef VIEWSGUI_DICE_RENDERER_HPP
#define VIEWSGUI_DICE_RENDERER_HPP

#include <SFML/Graphics.hpp>

#include <random>
#include <string>
#include <vector>

namespace viewsGUI {
class DiceRenderer {
public:
    DiceRenderer();

    bool loadAssets(const std::string& assetDir);

    void startRoll(int resultDice1, int resultDice2, sf::Vector2f centerPos);

    void update(sf::Time dt);
    void render(sf::RenderWindow& window) const;

    bool isRolling() const { return m_isRolling; }
    bool isVisible() const { return m_isVisible; }

private:
    std::vector<sf::Texture> m_faceTextures;
    sf::Sprite m_dice1;
    sf::Sprite m_dice2;

    bool m_isRolling;
    bool m_isVisible;
    bool m_isFadingOut;

    int m_target1;
    int m_target2;
    sf::Vector2f m_basePos1;
    sf::Vector2f m_basePos2;

    float m_rollDuration;
    float m_timeElapsed;
    float m_frameTimer;
    float m_frameInterval;

    float m_fadeDuration;
    float m_fadeElapsed;
    float m_resultHoldDuration;
    float m_resultHoldElapsed;
    bool m_isHoldingResult;

    bool m_doubleResult;
    float m_doubleFxTimer;

    std::mt19937 m_rng;

    void applyAlpha(unsigned int alpha);
    int randomFaceIndex();
    float randomRange(float minValue, float maxValue);
};
} // namespace viewsGUI

#endif
