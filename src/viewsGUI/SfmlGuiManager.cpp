#include "../../include/viewsGUI/SfmlGuiManager.hpp"

#include "../../include/core/CardManager.hpp"
#include "../../include/core/Command.hpp"
#include "../../include/core/CommandResult.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/GameException.hpp"

#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>

namespace {
std::string buildMessageFromResult(const CommandResult& result) {
    std::ostringstream out;
    if (!result.commandName.empty()) {
        out << result.commandName << "\n";
    }

    for (const GameEvent& event : result.events) {
        out << "- " << event.title << ": " << event.message << "\n";
    }

    if (result.prompt.has_value()) {
        out << "- INPUT: " << result.prompt->message;
    }

    return out.str();
}
} // namespace

namespace viewsGUI {
SfmlGuiManager::SfmlGuiManager(GameEngine& engine)
    : m_window(sf::VideoMode(1280, 720), "Nimonspoli GUI"),
      m_engine(engine),
      m_currentState(GuiState::IDLE),
            m_lastMessage("Siap memulai game.") {
    if (!loadFontWithFallback()) {
        throw std::runtime_error("Gagal memuat font GUI. Pastikan assets/fonts atau font sistem tersedia.");
    }

    m_boardView = std::make_unique<BoardRenderer>(720.0f, m_mainFont);
    m_actionPanel = std::make_unique<ActionPanel>(sf::Vector2f(720.0f, 0.0f), sf::Vector2f(560.0f, 720.0f), m_mainFont);
    m_popupBox = std::make_unique<PopupBox>(sf::Vector2f(1280.0f, 720.0f), m_mainFont);

    if (!m_boardView->loadAssets("assets/images/board/")) {
        m_lastMessage = "PERINGATAN: Sebagian aset board GUI gagal dimuat.";
    }

    initializeGameAndPieces();
    bindEngineCallbacks();
    refreshFromEngineState();
}

bool SfmlGuiManager::loadFontWithFallback() {
    const std::array<std::string, 5> candidates = {
        "assets/fonts/Montserrat-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/opentype/noto/NotoSans-Regular.ttf"};

    for (const std::string& path : candidates) {
        if (m_mainFont.loadFromFile(path)) {
            return true;
        }
    }

    return false;
}

void SfmlGuiManager::initializeGameAndPieces() {
    const CommandResult startResult = m_engine.startNewGame(4, {"P1", "P2", "P3", "P4"});
    consumeResult(startResult, true);

    const auto& players = m_engine.getPlayers();
    static const std::array<sf::Color, 6> pieceColors = {
        sf::Color(216, 83, 79),
        sf::Color(84, 143, 224),
        sf::Color(93, 170, 104),
        sf::Color(240, 190, 80),
        sf::Color(148, 92, 181),
        sf::Color(100, 100, 100)};

    m_players.clear();
    m_players.reserve(players.size());

    for (size_t i = 0; i < players.size(); ++i) {
        const int pos = players[i]->getPosition();
        const sf::Vector2f tileCenter = m_boardView->getTileCenter(pos);
        m_players.push_back(std::make_unique<PieceRenderer>(
            static_cast<int>(i), pieceColors[i % pieceColors.size()], tileCenter));
    }
}

void SfmlGuiManager::bindEngineCallbacks() {
    m_actionPanel->getRollDiceBtn()->setOnClick([this]() {
        if (m_currentState != GuiState::IDLE) {
            return;
        }
        submitRollDice();
    });

    m_actionPanel->getSaveBtn()->setOnClick([this]() {
        if (m_currentState != GuiState::IDLE) {
            return;
        }
        submitSave();
    });

    m_actionPanel->getLoadBtn()->setOnClick([this]() {
        if (m_currentState != GuiState::IDLE) {
            return;
        }
        submitLoad();
    });
}

void SfmlGuiManager::setActionButtonsEnabled(bool enabled) {
    m_actionPanel->getRollDiceBtn()->setEnabled(enabled);
    m_actionPanel->getSaveBtn()->setEnabled(enabled);
    m_actionPanel->getLoadBtn()->setEnabled(enabled);
}

void SfmlGuiManager::submitRollDice() {
    try {
        Command cmd;
        cmd.type = CommandType::ROLL_DICE;
        cmd.raw = "LEMPAR_DADU";

        const CommandResult result = m_engine.processCommand(cmd);
        const bool hasMovement = result.movement.has_value();
        consumeResult(result, !hasMovement);

        if (result.prompt.has_value()) {
            m_deferredPrompt = result.prompt.value();
        } else {
            m_deferredPrompt.reset();
        }

        if (hasMovement) {
            const MovementPayload& movement = result.movement.value();
            if (movement.playerIndex >= 0 && movement.playerIndex < static_cast<int>(m_players.size())) {
                std::vector<sf::Vector2f> path;
                path.reserve(movement.path.size());
                for (int index : movement.path) {
                    path.push_back(m_boardView->getTileCenter(index));
                }
                m_players[static_cast<size_t>(movement.playerIndex)]->moveAlongPath(path);
                m_currentState = GuiState::ANIMATING;
                setActionButtonsEnabled(false);
                return;
            }
        }

        if (result.prompt.has_value()) {
            handlePromptRequest(result.prompt.value());
        } else {
            m_currentState = GuiState::IDLE;
            setActionButtonsEnabled(true);
        }
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        m_actionPanel->setSystemMessage(m_lastMessage);
    }
}

void SfmlGuiManager::submitSave() {
    try {
        Command cmd;
        cmd.type = CommandType::SAVE;
        cmd.raw = "SIMPAN";
        const CommandResult result = m_engine.processCommand(cmd);
        consumeResult(result, true);
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        m_actionPanel->setSystemMessage(m_lastMessage);
    }
}

void SfmlGuiManager::submitLoad() {
    try {
        Command cmd;
        cmd.type = CommandType::LOAD;
        cmd.raw = "MUAT";
        const CommandResult result = m_engine.processCommand(cmd);
        consumeResult(result, true);
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        m_actionPanel->setSystemMessage(m_lastMessage);
    }
}

void SfmlGuiManager::submitResolveSkillDrop(int discardIndex) {
    try {
        Command cmd;
        cmd.type = CommandType::RESOLVE_SKILL_DROP;
        cmd.raw = "PILIH_BUANG_KARTU";
        cmd.args = {std::to_string(discardIndex)};

        const CommandResult result = m_engine.processCommand(cmd);
        consumeResult(result, true);
        m_currentState = GuiState::IDLE;
        setActionButtonsEnabled(true);
        handlePromptIfAny(result);
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        m_actionPanel->setSystemMessage(m_lastMessage);
        m_currentState = GuiState::IDLE;
        setActionButtonsEnabled(true);
    }
}

void SfmlGuiManager::consumeResult(const CommandResult& result, bool syncPiecePositions) {
    m_lastMessage = buildMessageFromResult(result);
    m_actionPanel->setSystemMessage(m_lastMessage);
    const Player& current = m_engine.getCurrentPlayer();
    m_actionPanel->updatePlayerInfo(current.getUsername(),
                                    current.getMoney(),
                                    m_engine.getCurrentTurn(),
                                    m_engine.getMaxTurn());

    if (!syncPiecePositions) {
        return;
    }

    const auto& players = m_engine.getPlayers();
    const size_t count = std::min(players.size(), m_players.size());
    for (size_t i = 0; i < count; ++i) {
        m_players[i]->snapTo(m_boardView->getTileCenter(players[i]->getPosition()));
    }
}

void SfmlGuiManager::handlePromptIfAny(const CommandResult& result) {
    if (!result.prompt.has_value()) {
        m_currentState = GuiState::IDLE;
        setActionButtonsEnabled(true);
        return;
    }

    handlePromptRequest(result.prompt.value());
}

void SfmlGuiManager::handlePromptRequest(const PromptRequest& prompt) {
    if (prompt.key == "SKILL_DROP") {
        m_currentState = GuiState::WAITING_CONFIRMATION;
        setActionButtonsEnabled(false);

        m_popupBox->showOptions(
            "PILIH KARTU DIBUANG",
            prompt.message,
            prompt.options,
            [this](int selected) { this->submitResolveSkillDrop(selected); });
        return;
    }

    m_currentState = GuiState::IDLE;
    setActionButtonsEnabled(true);
}

void SfmlGuiManager::refreshFromEngineState() {
    const Player& current = m_engine.getCurrentPlayer();
    m_actionPanel->updatePlayerInfo(current.getUsername(),
                                    current.getMoney(),
                                    m_engine.getCurrentTurn(),
                                    m_engine.getMaxTurn());

    const auto& players = m_engine.getPlayers();
    const size_t count = std::min(players.size(), m_players.size());
    for (size_t i = 0; i < count; ++i) {
        m_players[i]->snapTo(m_boardView->getTileCenter(players[i]->getPosition()));
    }
}

void SfmlGuiManager::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        const sf::Time dt = clock.restart();
        processEvents();

        // Prevent touching a closed SFML window in update/render on shutdown.
        if (!m_window.isOpen()) {
            break;
        }

        update(dt);
        render();
    }
}

void SfmlGuiManager::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            continue;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

            if (m_popupBox->isVisible()) {
                m_popupBox->handleMouseClick(mousePos);
            } else if (m_currentState == GuiState::IDLE) {
                m_actionPanel->handleMouseClick(mousePos);
            }
        }
    }
}

void SfmlGuiManager::update(sf::Time dt) {
    const sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

    if (m_popupBox->isVisible()) {
        m_popupBox->update(mousePos);
    } else {
        m_actionPanel->update(mousePos);
    }

    bool anyMoving = false;
    for (const auto& player : m_players) {
        player->update(dt);
        if (player->isMoving()) {
            anyMoving = true;
        }
    }

    if (m_currentState == GuiState::ANIMATING && !anyMoving) {
        refreshFromEngineState();
        if (m_deferredPrompt.has_value()) {
            handlePromptRequest(m_deferredPrompt.value());
            m_deferredPrompt.reset();
        } else {
            m_currentState = GuiState::IDLE;
            setActionButtonsEnabled(true);
        }
    }
}

void SfmlGuiManager::render() {
    m_window.clear();
    m_boardView->render(m_window, m_engine.getBoard());
    m_actionPanel->render(m_window);

    for (const auto& player : m_players) {
        player->render(m_window);
    }

    m_popupBox->render(m_window);
    m_window.display();
}
} // namespace viewsGUI
