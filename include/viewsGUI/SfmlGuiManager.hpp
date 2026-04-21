#ifndef VIEWSGUI_SFML_GUI_MANAGER_HPP
#define VIEWSGUI_SFML_GUI_MANAGER_HPP

#include <SFML/Graphics.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../core/CommandResult.hpp"
#include "ActionPanel.hpp"
#include "BoardRenderer.hpp"
#include "PieceRenderer.hpp"
#include "PopupBox.hpp"

class GameEngine;

namespace viewsGUI {
enum class GuiState { IDLE, ANIMATING, WAITING_CONFIRMATION };

class SfmlGuiManager {
public:
    explicit SfmlGuiManager(GameEngine& engine);
    void run();

private:
    sf::RenderWindow m_window;
    GameEngine& m_engine;

    sf::Font m_mainFont;
    GuiState m_currentState;

    std::unique_ptr<BoardRenderer> m_boardView;
    std::unique_ptr<ActionPanel> m_actionPanel;
    std::unique_ptr<PopupBox> m_popupBox;
    std::vector<std::unique_ptr<PieceRenderer>> m_players;

    std::string m_lastMessage;
    std::optional<PromptRequest> m_deferredPrompt;

    void processEvents();
    void update(sf::Time dt);
    void render();

    void bindEngineCallbacks();
    void initializeGameAndPieces();
    void refreshFromEngineState();

    bool loadFontWithFallback();
    void setActionButtonsEnabled(bool enabled);

    void submitRollDice();
    void submitSave();
    void submitLoad();
    void submitResolveSkillDrop(int discardIndex);

    void consumeResult(const CommandResult& result, bool syncPiecePositions);
    void handlePromptIfAny(const CommandResult& result);
    void handlePromptRequest(const PromptRequest& prompt);
};
} // namespace viewsGUI

#endif
