#ifndef VIEWSGUI_SFML_GUI_MANAGER_HPP
#define VIEWSGUI_SFML_GUI_MANAGER_HPP

#include <SFML/Graphics.hpp>

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../core/CommandResult.hpp"
#include "BoardRenderer.hpp"
#include "DiceRenderer.hpp"
#include "DynamicPopupBox.hpp"
#include "MainUI.hpp"
#include "PieceRenderer.hpp"

class GameEngine;
class Property;

namespace viewsGUI {
enum class GuiState { IDLE, ANIMATING_DICE, ANIMATING_PIECE, WAITING_CONFIRMATION };

class SfmlGuiManager {
public:
    explicit SfmlGuiManager(GameEngine& engine);
    void run();

private:
    sf::RenderWindow m_window;
    sf::View m_baseView;
    GameEngine& m_engine;

    sf::Font m_mainFont;
    sf::Font m_titleFont;
    GuiState m_currentState;

    std::unique_ptr<BoardRenderer> m_boardView;
    std::unique_ptr<MainUI> m_mainUi;
    std::unique_ptr<DiceRenderer> m_diceRenderer;
    std::unique_ptr<DynamicPopupBox> m_popupBox;
    std::vector<std::unique_ptr<PieceRenderer>> m_players;

    std::string m_lastMessage;
    std::deque<PromptRequest> m_pendingPrompts;
    std::optional<MovementPayload> m_deferredMovement;
    std::string m_preTurnSkillGateKey;
    bool m_preTurnSkillHandled;

    void processEvents();
    void update(sf::Time dt);
    void render();

    void bindEngineCallbacks();
    void initializeGameAndPieces();
    void refreshFromEngineState();

    bool loadFontWithFallback();
    void updateLetterboxView(unsigned int windowWidth, unsigned int windowHeight);
    void setUiInputEnabled(bool enabled);

    void submitRollDice();
    void beginMovementAnimation(const MovementPayload& movement);
    void enqueuePrompts(const std::vector<PromptRequest>& prompts);
    void processNextPrompt();
    std::string buildCurrentTurnGateKey() const;
    void maybeShowPreTurnSkillPopup();

    void consumeResult(const CommandResult& result, bool syncPiecePositions);
    void showLandingPopup(const MovementPayload& movement);
    void handlePopupAction(const std::string& actionId);
    PopupPayload buildLandingPayload(const MovementPayload& movement) const;
    std::vector<int> buildDummyRentRows(const Property& property) const;
    sf::Color resolvePropertyColor(const Property& property) const;
    void showBackendErrorPopup(const std::string& message, std::function<void()> onOk = {});
    void resumeFlowAfterPopup();
};
} // namespace viewsGUI

#endif
