#include "../../include/viewsGUI/SfmlGuiManager.hpp"

#include "../../include/core/CardManager.hpp"
#include "../../include/core/Command.hpp"
#include "../../include/core/CommandResult.hpp"
#include "../../include/core/GameEngine.hpp"
#include "../../include/core/PropertyManager.hpp"
#include "../../include/models/Board.hpp"
#include "../../include/models/CardTile.hpp"
#include "../../include/models/Dice.hpp"
#include "../../include/models/FestivalTile.hpp"
#include "../../include/models/GoTile.hpp"
#include "../../include/models/GoToJailTile.hpp"
#include "../../include/models/JailTile.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/PropertyTile.hpp"
#include "../../include/models/RailroadProperty.hpp"
#include "../../include/models/SkillCard.hpp"
#include "../../include/models/StreetProperty.hpp"
#include "../../include/models/TaxTile.hpp"
#include "../../include/models/Tile.hpp"
#include "../../include/models/UtilityProperty.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/viewsGUI/Layout1920.hpp"
#include "../../include/viewsGUI/Theme.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
constexpr const char* kPreTurnSkillUsePrefix = "preturn_skill_use:";
constexpr const char* kPreTurnSkillSkipAction = "preturn_skill_skip";

std::string buildMessageFromResult(const CommandResult& result) {
    std::ostringstream out;
    if (!result.commandName.empty()) {
        out << result.commandName << "\n";
    }

    for (const GameEvent& event : result.events) {
        out << "- " << event.title << ": " << event.message << "\n";
    }

    if (!result.prompts.empty()) {
        out << "- INPUT: " << result.prompts.front().message;
    }

    return out.str();
}

std::string buildErrorMessageFromResult(const CommandResult& result) {
    for (const GameEvent& event : result.events) {
        if (!event.message.empty() &&
            (event.tone == UiTone::ERROR || event.tone == UiTone::WARNING)) {
            return event.message;
        }
    }

    for (const GameEvent& event : result.events) {
        if (!event.message.empty()) {
            return event.message;
        }
    }

    const std::string fallback = buildMessageFromResult(result);
    if (!fallback.empty()) {
        return fallback;
    }
    return "Aksi ditolak oleh backend.";
}

std::string upperCopy(const std::string& value) {
    std::string upper = value;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return upper;
}

std::string sanitizeActionId(std::string value) {
    if (value.size() >= 3 &&
        static_cast<unsigned char>(value[0]) == 0xEF &&
        static_cast<unsigned char>(value[1]) == 0xBB &&
        static_cast<unsigned char>(value[2]) == 0xBF) {
        value.erase(0, 3);
    }

    auto isSpaceLike = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    while (!value.empty() && isSpaceLike(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && isSpaceLike(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }

    return value;
}

int parseSkillTargetPromptCardIndex(const std::string& promptId) {
    const std::string prefix = "skill_target_";
    if (promptId.rfind(prefix, 0) != 0) {
        return -1;
    }

    const size_t lastUnderscore = promptId.rfind('_');
    if (lastUnderscore == std::string::npos || lastUnderscore + 1 >= promptId.size()) {
        return -1;
    }

    const std::string suffix = promptId.substr(lastUnderscore + 1);
    if (suffix.empty() ||
        !std::all_of(suffix.begin(), suffix.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) {
        return -1;
    }

    try {
        return std::stoi(suffix);
    } catch (const std::exception&) {
        return -1;
    }
}

std::string resolveSkillCardTexture(const std::string& cardTypeName) {
    namespace fs = std::filesystem;

    std::string normalized;
    normalized.reserve(cardTypeName.size());
    for (char c : cardTypeName) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    if (normalized.size() >= 4 && normalized.substr(normalized.size() - 4) == "card") {
        normalized = normalized.substr(0, normalized.size() - 4);
    }

    const std::array<std::string, 4> candidates = {
        "assets/images/ui/skill_" + normalized + ".png",
        "assets/images/ui/skill_" + upperCopy(normalized) + ".png",
        "assets/images/ui/card_" + normalized + ".png",
        "assets/images/ui/asset_card_template.png"};

    for (const std::string& path : candidates) {
        std::error_code ec;
        if (fs::exists(path, ec) && !ec) {
            return path;
        }
    }

    return "assets/images/ui/asset_card_template.png";
}

std::string buildSkillLabel(const SkillCard& card, int oneBasedIndex) {
    std::ostringstream label;
    label << oneBasedIndex << ". " << card.getTypeName();

    if (card.getValue() > 0) {
        label << " (Nilai " << card.getValue() << ")";
    }

    return label.str();
}

void appendPreTurnDebugLog(const std::string& line) {
    std::ofstream out("build/gui_preturn_debug.log", std::ios::app);
    if (out.is_open()) {
        out << line << '\n';
    }
}
} // namespace

namespace viewsGUI {
SfmlGuiManager::SfmlGuiManager(GameEngine& engine)
    : m_window(sf::VideoMode(static_cast<unsigned int>(Layout1920::kDesignWidth),
                             static_cast<unsigned int>(Layout1920::kDesignHeight)),
               "Nimonspoli GUI",
               sf::Style::Default),
      m_baseView(sf::FloatRect(0.0f, 0.0f, Layout1920::kDesignWidth, Layout1920::kDesignHeight)),
      m_engine(engine),
      m_currentState(GuiState::IDLE),
      m_lastMessage("Siap memulai game."),
      m_preTurnSkillHandled(false) {
    if (!loadFontWithFallback()) {
        throw std::runtime_error("Gagal memuat font GUI. Pastikan assets/fonts atau font sistem tersedia.");
    }

    m_window.setFramerateLimit(60);
    m_window.setView(m_baseView);
    updateLetterboxView(m_window.getSize().x, m_window.getSize().y);

    m_boardView = std::make_unique<BoardRenderer>(Layout1920::kBoardSize, m_mainFont, Layout1920::kBoardOrigin);
    m_mainUi = std::make_unique<MainUI>(m_titleFont, m_mainFont);
    m_diceRenderer = std::make_unique<DiceRenderer>();
    m_popupBox = std::make_unique<DynamicPopupBox>(
        sf::Vector2f(Layout1920::kDesignWidth, Layout1920::kDesignHeight), m_titleFont, m_mainFont);

    if (!m_boardView->loadAssets("assets/images/board/")) {
        m_lastMessage = "PERINGATAN: Sebagian aset board GUI gagal dimuat.";
    }
    if (!m_mainUi->loadAssets("assets/images/ui/", "assets/images/board/")) {
        m_lastMessage += "\nPERINGATAN: Sebagian aset UI utama gagal dimuat.";
    }
    if (!m_diceRenderer->loadAssets("assets/images/ui/dice/")) {
        m_lastMessage += "\nPERINGATAN: Aset animasi dadu gagal dimuat.";
    }
    if (!m_popupBox->loadAssets("assets/images/ui/")) {
        m_lastMessage += "\nPERINGATAN: Aset popup gagal dimuat.";
    }

    initializeGameAndPieces();
    bindEngineCallbacks();
    refreshFromEngineState();
    setUiInputEnabled(true);
    maybeShowPreTurnSkillPopup();
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
            break;
        }
    }

    if (m_mainFont.getInfo().family.empty()) {
        return false;
    }

    if (!m_titleFont.loadFromFile("assets/fonts/BebasNeue-Regular.ttf")) {
        m_titleFont = m_mainFont;
    }

    return true;
}

void SfmlGuiManager::updateLetterboxView(unsigned int windowWidth, unsigned int windowHeight) {
    if (windowWidth == 0 || windowHeight == 0) {
        return;
    }

    const float windowRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const float viewRatio = m_baseView.getSize().x / m_baseView.getSize().y;

    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio > viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX = (1.0f - sizeX) * 0.5f;
    } else if (windowRatio < viewRatio) {
        sizeY = windowRatio / viewRatio;
        posY = (1.0f - sizeY) * 0.5f;
    }

    m_baseView.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    m_window.setView(m_baseView);
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
        auto renderer = std::make_unique<PieceRenderer>(
            static_cast<int>(i), pieceColors[i % pieceColors.size()], tileCenter);

        const std::string tokenPath = "assets/images/ui/piece/piece_p" + std::to_string(i + 1) + ".png";
        renderer->loadTokenTexture(tokenPath);
        m_players.push_back(std::move(renderer));
    }
}

void SfmlGuiManager::bindEngineCallbacks() {
    m_mainUi->setOnRollDice([this]() {
        if (m_currentState != GuiState::IDLE) {
            return;
        }
        submitRollDice();
    });
}

void SfmlGuiManager::setUiInputEnabled(bool enabled) {
    m_mainUi->setRollEnabled(enabled);
}

void SfmlGuiManager::submitRollDice() {
    try {
        m_mainUi->setRollVisible(false);
        setUiInputEnabled(false);
        m_pendingPrompts.clear();
        m_deferredMovement.reset();

        Command cmd;
        cmd.type = CommandType::ROLL_DICE;
        cmd.raw = "LEMPAR_DADU";

        const CommandResult result = m_engine.processCommand(cmd);
        const bool hasMovement = result.movement.has_value();
        consumeResult(result, !hasMovement);
        enqueuePrompts(result.prompts);

        const Dice& dice = m_engine.getDice();
        const sf::Vector2f boardCenter = m_boardView->getBoardCenter();
        m_diceRenderer->startRoll(dice.getDie1(),
                                  dice.getDie2(),
                                  sf::Vector2f(boardCenter.x + Layout1920::kDiceCenterOffsetFromBoardCenter.x,
                                               boardCenter.y + Layout1920::kDiceCenterOffsetFromBoardCenter.y));

        if (hasMovement) {
            m_deferredMovement = result.movement.value();
        } else {
            m_deferredMovement.reset();
        }

        m_currentState = GuiState::ANIMATING_DICE;
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        refreshFromEngineState();
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        m_currentState = GuiState::IDLE;
    }
}

void SfmlGuiManager::beginMovementAnimation(const MovementPayload& movement) {
    m_deferredMovement = movement;

    if (movement.playerIndex >= 0 && movement.playerIndex < static_cast<int>(m_players.size())) {
        std::vector<sf::Vector2f> path;
        path.reserve(movement.path.size());
        for (int index : movement.path) {
            path.push_back(m_boardView->getTileCenter(index));
        }
        m_players[static_cast<size_t>(movement.playerIndex)]->moveAlongPath(path);
    }

    m_currentState = GuiState::ANIMATING_PIECE;
}

void SfmlGuiManager::enqueuePrompts(const std::vector<PromptRequest>& prompts) {
    for (const PromptRequest& prompt : prompts) {
        m_pendingPrompts.push_back(prompt);
    }
}

void SfmlGuiManager::processNextPrompt() {
    if (m_popupBox->isVisible()) {
        return;
    }

    if (m_pendingPrompts.empty()) {
        resumeFlowAfterPopup();
        return;
    }

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    const PromptRequest prompt = m_pendingPrompts.front();
    m_popupBox->showPrompt(prompt, [this, prompt](const std::string& answerKey) {
        const std::string stableAnswerKey = answerKey;
        m_popupBox->hide();
        if (!m_pendingPrompts.empty()) {
            m_pendingPrompts.pop_front();
        }
        const bool fromSkillTargetPrompt = (prompt.id.rfind("skill_target_", 0) == 0);
        auto showErrorAndRetrySkillTarget = [this, prompt](const std::string& message) {
            showBackendErrorPopup(message, [this, prompt]() {
                m_pendingPrompts.push_front(prompt);
                processNextPrompt();
            });
        };

        try {
            const std::string cleanAnswer = sanitizeActionId(stableAnswerKey);
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] prompt_answer id=" + prompt.id +
                                  " answer=" + cleanAnswer);
            m_engine.setPromptAnswer(prompt.id, cleanAnswer);

            if (!m_engine.hasPendingContinuation()) {
                // Fallback: if continuation unexpectedly missing for skill target prompt,
                // run USE_SKILL directly from prompt id + input target.
                const int fallbackCardIndex = parseSkillTargetPromptCardIndex(prompt.id);
                if (fallbackCardIndex > 0) {
                    appendPreTurnDebugLog(
                        "[DEBUG][PreTurnSkill] fallback_direct_use prompt_id=" + prompt.id +
                        " answer=" + cleanAnswer + " idx=" + std::to_string(fallbackCardIndex));

                    const std::string normalizedAnswer = sanitizeActionId(cleanAnswer);
                    const std::string upperAnswer = upperCopy(normalizedAnswer);
                    if (!normalizedAnswer.empty() && upperAnswer != "CANCEL" && upperAnswer != "CLOSE") {
                        Command followUp;
                        followUp.type = CommandType::USE_SKILL;
                        followUp.raw = "GUNAKAN_KEMAMPUAN " + std::to_string(fallbackCardIndex) + " " + normalizedAnswer;
                        followUp.args = {std::to_string(fallbackCardIndex), normalizedAnswer};
                        const CommandResult directResult = m_engine.processCommand(followUp);
                        const bool hasMovement = directResult.movement.has_value();
                        const Player& afterDirect = m_engine.getCurrentPlayer();
                        appendPreTurnDebugLog(
                            "[DEBUG][PreTurnSkill] fallback_direct_result success=" +
                            std::string(directResult.success ? "true" : "false") +
                            " pos=" + std::to_string(afterDirect.getPosition()) +
                            " cards=" + std::to_string(afterDirect.countCards()) +
                            " used=" + std::string(afterDirect.hasUsedSkillThisTurn() ? "true" : "false"));
                        consumeResult(directResult, !hasMovement);
                        enqueuePrompts(directResult.prompts);

                        if (!directResult.success && !hasMovement && directResult.prompts.empty()) {
                            if (fromSkillTargetPrompt) {
                                showErrorAndRetrySkillTarget(buildErrorMessageFromResult(directResult));
                                return;
                            }
                            showBackendErrorPopup(buildErrorMessageFromResult(directResult));
                            return;
                        }

                        if (hasMovement) {
                            beginMovementAnimation(directResult.movement.value());
                            return;
                        }

                        refreshFromEngineState();
                        processNextPrompt();
                        return;
                    }
                }

                refreshFromEngineState();
                processNextPrompt();
                return;
            }

            const CommandResult resumed = m_engine.resumePendingAction();
            const bool hasMovement = resumed.movement.has_value();
            const Player& afterResume = m_engine.getCurrentPlayer();
            appendPreTurnDebugLog(
                "[DEBUG][PreTurnSkill] resumed_result success=" +
                std::string(resumed.success ? "true" : "false") +
                " pos=" + std::to_string(afterResume.getPosition()) +
                " cards=" + std::to_string(afterResume.countCards()) +
                " used=" + std::string(afterResume.hasUsedSkillThisTurn() ? "true" : "false"));
            consumeResult(resumed, !hasMovement);
            enqueuePrompts(resumed.prompts);

            if (!resumed.success && !hasMovement && resumed.prompts.empty()) {
                if (fromSkillTargetPrompt) {
                    showErrorAndRetrySkillTarget(buildErrorMessageFromResult(resumed));
                    return;
                }
                showBackendErrorPopup(buildErrorMessageFromResult(resumed));
                return;
            }

            if (hasMovement) {
                beginMovementAnimation(resumed.movement.value());
                return;
            }

            refreshFromEngineState();
            processNextPrompt();
        } catch (const std::exception& e) {
            m_lastMessage = std::string("ERROR: ") + e.what();
            m_pendingPrompts.clear();
            m_engine.clearPendingContinuation();
            m_engine.clearPromptAnswers();
            if (fromSkillTargetPrompt) {
                showErrorAndRetrySkillTarget(e.what());
                return;
            }
            showBackendErrorPopup(e.what());
        }
    });
}

std::string SfmlGuiManager::buildCurrentTurnGateKey() const {
    const Player& current = m_engine.getCurrentPlayer();
    return current.getUsername() + "@" + std::to_string(m_engine.getCurrentTurn());
}

void SfmlGuiManager::maybeShowPreTurnSkillPopup() {
    if (m_currentState != GuiState::IDLE || m_popupBox->isVisible()) {
        return;
    }

    if (!m_pendingPrompts.empty() || m_engine.hasPendingContinuation()) {
        return;
    }

    const std::string currentGateKey = buildCurrentTurnGateKey();
    if (currentGateKey != m_preTurnSkillGateKey) {
        m_preTurnSkillGateKey = currentGateKey;
        m_preTurnSkillHandled = false;
    }

    if (m_preTurnSkillHandled) {
        return;
    }

    const Player& current = m_engine.getCurrentPlayer();
    const auto& hand = current.getHandCards();
    if (hand.empty() || current.hasUsedSkillThisTurn() || current.isBankrupt()) {
        m_preTurnSkillHandled = true;
        return;
    }

    m_preTurnSkillHandled = true;

    PopupPayload payload;
    payload.mode = PopupMode::SPECIAL;
    payload.headerTitle = "KARTU KEMAMPUAN";
    payload.cardTitle = "PRA-GILIRAN";
    payload.description =
        "Klik kartu untuk mengirim command GUNAKAN_KEMAMPUAN sebelum lempar dadu.";

    for (size_t i = 0; i < hand.size(); ++i) {
        const std::shared_ptr<SkillCard>& card = hand[i];
        if (!card) {
            continue;
        }

        const int engineCardIndex = static_cast<int>(i) + 1;
        payload.actionItems.push_back(PopupActionItem{
            std::string(kPreTurnSkillUsePrefix) + std::to_string(engineCardIndex),
            buildSkillLabel(*card, engineCardIndex),
            resolveSkillCardTexture(card->getTypeName()),
            true});
    }

    payload.actionItems.push_back(PopupActionItem{
        kPreTurnSkillSkipAction,
        "Lanjut Lempar Dadu",
        "assets/images/ui/btn_cancel.png",
        true});

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(payload, [this](const std::string& actionId) {
        // Keep a stable local copy because hide() clears popup internals.
        const std::string stableActionId = actionId;
        appendPreTurnDebugLog("[DEBUG][PreTurnSkill] popup_action=" + stableActionId);
        m_popupBox->hide();

        const std::string cleanActionId = sanitizeActionId(stableActionId);
        appendPreTurnDebugLog("[DEBUG][PreTurnSkill] clean_action=" + cleanActionId);

        if (cleanActionId == kPreTurnSkillSkipAction || cleanActionId == "close" ||
            cleanActionId == "cancel") {
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] branch=skip_or_close");
            resumeFlowAfterPopup();
            return;
        }

        const std::string prefix = kPreTurnSkillUsePrefix;
        if (cleanActionId.rfind(prefix, 0) != 0) {
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] branch=prefix_mismatch");
            resumeFlowAfterPopup();
            return;
        }
        appendPreTurnDebugLog("[DEBUG][PreTurnSkill] prefix_pos=0");

        const std::string rawIndex = cleanActionId.substr(prefix.size());
        int engineCardIndex = -1;
        try {
            engineCardIndex = std::stoi(rawIndex);
        } catch (const std::exception&) {
            engineCardIndex = -1;
        }
        appendPreTurnDebugLog("[DEBUG][PreTurnSkill] parsed_index=" + std::to_string(engineCardIndex));

        if (engineCardIndex <= 0) {
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] branch=invalid_index");
            m_lastMessage = "ERROR: Index kartu kemampuan tidak valid.";
            refreshFromEngineState();
            m_preTurnSkillHandled = false;
            resumeFlowAfterPopup();
            return;
        }

        try {
            const Player& actingPlayer = m_engine.getCurrentPlayer();
            const int cardCountBefore = actingPlayer.countCards();
            const bool usedSkillBefore = actingPlayer.hasUsedSkillThisTurn();
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] pre_call cards=" + std::to_string(cardCountBefore) +
                                  " used=" + (usedSkillBefore ? "true" : "false"));

            Command cmd;
            cmd.type = CommandType::USE_SKILL;
            cmd.raw = "GUNAKAN_KEMAMPUAN " + std::to_string(engineCardIndex);
            cmd.args = {std::to_string(engineCardIndex)};
            appendPreTurnDebugLog("[DEBUG][PreTurnSkill] sending_cmd=" + cmd.raw);

            const CommandResult result = m_engine.processCommand(cmd);
            std::ostringstream debugMessage;
            if (!result.events.empty()) {
                for (const GameEvent& event : result.events) {
                    if (!debugMessage.str().empty()) {
                        debugMessage << " | ";
                    }
                    debugMessage << event.title << ": " << event.message;
                }
            } else {
                debugMessage << "(tanpa event)";
            }
            const size_t promptCount = result.prompts.size();
            const bool hasPendingContinuation = m_engine.hasPendingContinuation();
            const Player& debugPlayer = m_engine.getCurrentPlayer();
            std::cout << "[DEBUG][PreTurnSkill] cmd='" << cmd.raw
                      << "' commandName='" << result.commandName << "'"
                      << " success=" << (result.success ? "true" : "false")
                      << " cardsBefore=" << cardCountBefore
                      << " cardsAfter=" << debugPlayer.countCards()
                      << " usedBefore=" << (usedSkillBefore ? "true" : "false")
                      << " usedAfter=" << (debugPlayer.hasUsedSkillThisTurn() ? "true" : "false")
                      << " prompts=" << promptCount
                      << " pendingContinuation=" << (hasPendingContinuation ? "true" : "false")
                      << " message=" << debugMessage.str() << "\n";
            std::ostringstream line;
            line << "[DEBUG][PreTurnSkill] cmd='" << cmd.raw
                 << "' commandName='" << result.commandName << "'"
                 << " success=" << (result.success ? "true" : "false")
                 << " cardsBefore=" << cardCountBefore
                 << " cardsAfter=" << debugPlayer.countCards()
                 << " usedBefore=" << (usedSkillBefore ? "true" : "false")
                 << " usedAfter=" << (debugPlayer.hasUsedSkillThisTurn() ? "true" : "false")
                 << " prompts=" << promptCount
                 << " pendingContinuation=" << (hasPendingContinuation ? "true" : "false")
                 << " message=" << debugMessage.str();
            appendPreTurnDebugLog(line.str());

            const bool hasMovement = result.movement.has_value();
            consumeResult(result, !hasMovement);
            enqueuePrompts(result.prompts);

            if (!hasMovement && !result.success && result.prompts.empty() && !hasPendingContinuation) {
                bool isNoValidLassoTarget = false;
                for (const GameEvent& event : result.events) {
                    if (event.message.find("Tidak ada pemain lawan valid di depanmu (jarak 1-12 petak).") !=
                            std::string::npos ||
                        event.title == "Target Tidak Ada") {
                        isNoValidLassoTarget = true;
                        break;
                    }
                }

                if (isNoValidLassoTarget) {
                    PromptRequest infoPrompt;
                    infoPrompt.id = "skill_info_no_target_" + std::to_string(m_engine.getCurrentTurn()) + "_" +
                                    std::to_string(engineCardIndex);
                    infoPrompt.title = "KARTU KEMAMPUAN";
                    infoPrompt.message = "Tidak ada pemain lawan valid di depanmu (jarak 1-12 petak).";
                    infoPrompt.options.push_back(PromptOption{"close", "OK"});
                    m_pendingPrompts.push_front(infoPrompt);
                    appendPreTurnDebugLog(
                        "[DEBUG][PreTurnSkill] enqueue_info_popup=no_valid_lasso_target");
                }
            }

            const Player& updatedPlayer = m_engine.getCurrentPlayer();
            const bool skillConsumed =
                (!usedSkillBefore && updatedPlayer.hasUsedSkillThisTurn() &&
                 updatedPlayer.countCards() < cardCountBefore);
            const bool awaitingFollowUp = (promptCount > 0) || hasPendingContinuation;

            // Keep gate closed while waiting follow-up prompt (e.g., target input),
            // otherwise popup can re-open repeatedly and block Roll Dice.
            if (!skillConsumed && !awaitingFollowUp) {
                m_preTurnSkillHandled = false;
            }

            if (hasMovement) {
                beginMovementAnimation(result.movement.value());
                return;
            }

            refreshFromEngineState();
            processNextPrompt();
        } catch (const std::exception& e) {
            m_lastMessage = std::string("ERROR: ") + e.what();
            appendPreTurnDebugLog(std::string("[DEBUG][PreTurnSkill] exception=") + e.what());
            refreshFromEngineState();
            m_preTurnSkillHandled = false;
            resumeFlowAfterPopup();
        }
    });
}

void SfmlGuiManager::consumeResult(const CommandResult& result, bool syncPiecePositions) {
    m_lastMessage = buildMessageFromResult(result);

    const Player& current = m_engine.getCurrentPlayer();
    m_mainUi->updateData(m_engine.getPlayers(), current, m_lastMessage);

    if (!syncPiecePositions) {
        return;
    }

    const auto& players = m_engine.getPlayers();
    const size_t count = std::min(players.size(), m_players.size());
    for (size_t i = 0; i < count; ++i) {
        m_players[i]->snapTo(m_boardView->getTileCenter(players[i]->getPosition()));
    }
}

void SfmlGuiManager::refreshFromEngineState() {
    const Player& current = m_engine.getCurrentPlayer();
    m_mainUi->updateData(m_engine.getPlayers(), current, m_lastMessage);

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

        if (event.type == sf::Event::Resized) {
            updateLetterboxView(event.size.width, event.size.height);
            continue;
        }

        if (event.type == sf::Event::TextEntered) {
            if (m_popupBox->isVisible() && !m_popupBox->isMinimized() &&
                m_popupBox->handleTextEntered(event.text.unicode)) {
                continue;
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (m_popupBox->isVisible() && !m_popupBox->isMinimized() &&
                m_popupBox->handleKeyPressed(event.key.code)) {
                continue;
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
            const bool popupBlocksUiInput = m_popupBox->isVisible() && !m_popupBox->isMinimized();
            const bool canInteractUi = (m_currentState == GuiState::IDLE) || m_popupBox->isMinimized();

            if (!popupBlocksUiInput && canInteractUi) {
                m_mainUi->handleMouseWheel(event.mouseWheelScroll.delta, mousePos);
            }
            continue;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
            const bool popupBlocksUiInput = m_popupBox->isVisible() && !m_popupBox->isMinimized();
            const bool canInteractUi = (m_currentState == GuiState::IDLE) || m_popupBox->isMinimized();
            bool consumedByPopup = false;

            if (m_popupBox->isVisible()) {
                consumedByPopup = m_popupBox->handleMousePressed(mousePos);
            }

            if (!consumedByPopup && !popupBlocksUiInput && canInteractUi) {
                m_mainUi->handleMousePressed(mousePos);
            }
            continue;
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
            const bool popupBlocksUiInput = m_popupBox->isVisible() && !m_popupBox->isMinimized();
            const bool canInteractUi = (m_currentState == GuiState::IDLE) || m_popupBox->isMinimized();
            bool consumedByPopup = false;

            if (m_popupBox->isVisible()) {
                consumedByPopup = m_popupBox->handleMouseReleased(mousePos);
            }

            if (!consumedByPopup && !popupBlocksUiInput && canInteractUi) {
                m_mainUi->handleMouseReleased(mousePos);
            }
            continue;
        }
    }
}

void SfmlGuiManager::update(sf::Time dt) {
    const sf::Vector2f mousePos =
        m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

    const bool popupExpanded = m_popupBox->isVisible() && !m_popupBox->isMinimized();
    if (popupExpanded) {
        m_popupBox->update(mousePos);
    } else {
        m_mainUi->update(mousePos);
        if (m_popupBox->isVisible()) {
            m_popupBox->update(mousePos);
        }
    }

    bool anyMoving = false;
    for (const auto& player : m_players) {
        player->update(dt);
        if (player->isMoving()) {
            anyMoving = true;
        }
    }

    m_diceRenderer->update(dt);

    if (m_currentState == GuiState::ANIMATING_DICE && !m_diceRenderer->isRolling()) {
        if (m_deferredMovement.has_value()) {
            beginMovementAnimation(m_deferredMovement.value());
            return;
        }

        refreshFromEngineState();
        processNextPrompt();
        return;
    }

    if (m_currentState == GuiState::ANIMATING_PIECE && !anyMoving) {
        refreshFromEngineState();
        if (m_deferredMovement.has_value()) {
            showLandingPopup(m_deferredMovement.value());
            m_deferredMovement.reset();
        } else {
            processNextPrompt();
        }
    }

    if (m_currentState == GuiState::IDLE && !m_popupBox->isVisible()) {
        maybeShowPreTurnSkillPopup();
    }
}

void SfmlGuiManager::render() {
    m_window.clear(sf::Color::Black);
    m_window.setView(m_baseView);

    m_mainUi->renderBackground(m_window);
    m_boardView->render(m_window, m_engine.getBoard());

    for (const auto& player : m_players) {
        player->render(m_window);
    }

    m_diceRenderer->render(m_window);

    m_mainUi->renderOverlay(m_window);
    m_popupBox->render(m_window);

    m_window.display();
}

void SfmlGuiManager::showLandingPopup(const MovementPayload& movement) {
    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(buildLandingPayload(movement), [this](const std::string& actionId) {
        this->handlePopupAction(actionId);
    });
}

void SfmlGuiManager::handlePopupAction(const std::string& actionId) {
    const std::string stableActionId = sanitizeActionId(actionId);

    auto resolvePendingPurchase = [this](const std::string& answer) -> bool {
        for (auto it = m_pendingPrompts.begin(); it != m_pendingPrompts.end(); ++it) {
            if (it->id.rfind("beli_", 0) != 0) {
                continue;
            }

            const std::string promptId = it->id;
            m_pendingPrompts.erase(it);
            m_engine.setPromptAnswer(promptId, answer);

            if (!m_engine.hasPendingContinuation()) {
                return true;
            }

            const CommandResult resumed = m_engine.resumePendingAction();
            const bool hasMovement = resumed.movement.has_value();
            consumeResult(resumed, !hasMovement);
            enqueuePrompts(resumed.prompts);

            if (hasMovement) {
                beginMovementAnimation(resumed.movement.value());
            } else {
                refreshFromEngineState();
                processNextPrompt();
            }
            return true;
        }
        return false;
    };

    try {
        m_popupBox->hide();

        if (stableActionId == "buy_land") {
            if (!resolvePendingPurchase("y")) {
                m_lastMessage = "Tidak ada prompt pembelian aktif untuk diproses.";
                refreshFromEngineState();
                processNextPrompt();
            }
            return;
        }

        if (stableActionId == "build_property" || stableActionId == "add_house") {
            const Player& current = m_engine.getCurrentPlayer();
            const Tile& currentTile = m_engine.getBoard().getTileByIndex(current.getPosition());
            const auto* propertyTile = dynamic_cast<const PropertyTile*>(&currentTile);
            if (!propertyTile) {
                throw GameException("BANGUN hanya bisa dilakukan di petak properti.");
            }

            Command cmd;
            cmd.type = CommandType::BUILD;
            cmd.raw = "BANGUN";
            cmd.args = {propertyTile->getProperty().getCode()};

            const CommandResult result = m_engine.processCommand(cmd);
            const bool hasMovement = result.movement.has_value();
            consumeResult(result, !hasMovement);
            enqueuePrompts(result.prompts);

            if (hasMovement) {
                beginMovementAnimation(result.movement.value());
            } else {
                refreshFromEngineState();
                processNextPrompt();
            }
            return;
        }

        if (stableActionId == "cancel" || stableActionId == "close") {
            if (!resolvePendingPurchase("n")) {
                m_lastMessage = "Popup ditutup oleh pemain.";
                refreshFromEngineState();
                processNextPrompt();
            }
            return;
        }

        m_lastMessage = "Aksi popup dipilih: " + stableActionId;
        refreshFromEngineState();
        processNextPrompt();
    } catch (const GameException& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        refreshFromEngineState();
        processNextPrompt();
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        refreshFromEngineState();
        processNextPrompt();
    }
}

PopupPayload SfmlGuiManager::buildLandingPayload(const MovementPayload& movement) const {
    PopupPayload payload;
    payload.headerTitle = "INFO PETAK";
    payload.cardTitle = "Nimonspoli";
    payload.description = movement.playerName + " mendarat di petak.";
    payload.mode = PopupMode::INFO;

    if (movement.toIndex < 0 || movement.toIndex >= m_engine.getBoard().size()) {
        payload.actionItems = {
            PopupActionItem{"close", "Tutup", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    const Tile& tile = m_engine.getBoard().getTileByIndex(movement.toIndex);
    payload.cardTitle = tile.getName();
    payload.description = "Kode: " + tile.getCode() + "\nPemain: " + movement.playerName;

    if (const auto* propertyTile = dynamic_cast<const PropertyTile*>(&tile)) {
        const Property& property = propertyTile->getProperty();
        payload.mode = PopupMode::PROPERTY;
        payload.headerTitle = "PROPERTI";
        payload.cardTitle = property.getName();
        payload.purchasePrice = property.getPurchasePrice();
        payload.rentPrices = buildDummyRentRows(property);
        payload.description = "Kode: " + property.getCode() + "\nHarga hipotek: M" +
                              std::to_string(property.getMortgageValue());

        if (dynamic_cast<const StreetProperty*>(&property) != nullptr) {
            payload.showStreetColorRibbon = true;
            payload.propertyColor = resolvePropertyColor(property);
        } else {
            payload.showStreetColorRibbon = false;
            payload.propertyColor = sf::Color::Transparent;
        }

        const std::string buyPromptId = "beli_" + property.getCode();
        const bool hasBuyOption = std::any_of(
            m_pendingPrompts.begin(), m_pendingPrompts.end(), [&buyPromptId](const PromptRequest& prompt) {
                return prompt.id == buyPromptId;
            });

        if (hasBuyOption) {
            payload.actionItems.push_back(
                PopupActionItem{"buy_land", "Beli Lahan", "assets/images/ui/btn_beli.png", true});
        }

        if (const auto* street = dynamic_cast<const StreetProperty*>(&property)) {
            const Player& currentPlayer = m_engine.getCurrentPlayer();
            const PropertyManager::BuildOption buildOption =
                m_engine.getPropertyManager().getBuildOption(currentPlayer, *street);

            if (buildOption == PropertyManager::BuildOption::HOUSE) {
                payload.actionItems.push_back(
                    PopupActionItem{"build_property", "+ Rumah", "assets/images/ui/btn_rumah.png", true});
            } else if (buildOption == PropertyManager::BuildOption::HOTEL) {
                payload.actionItems.push_back(
                    PopupActionItem{"build_property", "+ Hotel", "assets/images/ui/btn_rumah.png", true});
            }
        }

        if (hasBuyOption) {
            payload.actionItems.push_back(
                PopupActionItem{"cancel", "Males ah", "assets/images/ui/btn_cancel.png", true});
        }

        if (payload.actionItems.empty()) {
            payload.actionItems.push_back(
                PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true});
        }
        return payload;
    }

    if (const auto* cardTile = dynamic_cast<const CardTile*>(&tile)) {
        if (cardTile->getDrawType() == CardDrawType::CHANCE) {
            payload.mode = PopupMode::CHANCE;
            payload.headerTitle = "KESEMPATAN";
        } else {
            payload.mode = PopupMode::COMMUNITY;
            payload.headerTitle = "DANA UMUM";
        }

        payload.description += "\nAmbil kartu sesuai jenis petak.";
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    if (const auto* taxTile = dynamic_cast<const TaxTile*>(&tile)) {
        payload.mode = PopupMode::TAX;
        payload.headerTitle = "PAJAK";
        payload.description += "\nKewajiban pajak: ";
        if (taxTile->getTaxType() == TaxType::PPH) {
            payload.description += "PPH " + std::to_string(taxTile->getPercentage()) + "%";
        } else {
            payload.description += "PBM Flat M" + std::to_string(taxTile->getFlatAmount());
        }
        payload.actionItems = {PopupActionItem{"close", "Mengerti", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    if (dynamic_cast<const FestivalTile*>(&tile) != nullptr) {
        payload.mode = PopupMode::SPECIAL;
        payload.headerTitle = "FESTIVAL";
        payload.description += "\nPilih properti milikmu pada prompt festival untuk mengaktifkan bonus sewa.";
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    if (dynamic_cast<const GoTile*>(&tile) != nullptr) {
        payload.mode = PopupMode::SPECIAL;
        payload.headerTitle = "GO";
        payload.description += "\nSelamat melewati titik start.";
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    if (dynamic_cast<const GoToJailTile*>(&tile) != nullptr ||
        dynamic_cast<const JailTile*>(&tile) != nullptr) {
        payload.mode = PopupMode::SPECIAL;
        payload.headerTitle = "PENJARA";
        payload.description += "\nStatus penjara dikelola engine.";
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    payload.actionItems = {PopupActionItem{"close", "Tutup", "assets/images/ui/btn_cancel.png", true}};
    return payload;
}

std::vector<int> SfmlGuiManager::buildDummyRentRows(const Property& property) const {
    if (dynamic_cast<const StreetProperty*>(&property) != nullptr) {
        return {50, 100, 150, 300, 450, 650};
    }
    if (dynamic_cast<const RailroadProperty*>(&property) != nullptr) {
        return {250, 500, 1000, 2000};
    }
    if (dynamic_cast<const UtilityProperty*>(&property) != nullptr) {
        return {300, 600};
    }
    return {100, 200, 300};
}

sf::Color SfmlGuiManager::resolvePropertyColor(const Property& property) const {
    if (const auto* street = dynamic_cast<const StreetProperty*>(&property)) {
        const std::string group = upperCopy(street->getColorGroup());
        if (group == "COKLAT" || group == "CK") return Theme::Coklat;
        if (group == "BIRU_MUDA" || group == "BM") return Theme::BiruMuda;
        if (group == "MERAH_MUDA" || group == "PK") return Theme::Pink;
        if (group == "ORANGE" || group == "OR") return Theme::Oranye;
        if (group == "MERAH" || group == "MR") return Theme::Merah;
        if (group == "KUNING" || group == "KN") return Theme::Kuning;
        if (group == "HIJAU" || group == "HJ") return Theme::Hijau;
        if (group == "BIRU_TUA" || group == "BT") return Theme::BiruTua;
    }

    return sf::Color::Transparent;
}

void SfmlGuiManager::showBackendErrorPopup(const std::string& message, std::function<void()> onOk) {
    PopupPayload payload;
    payload.mode = PopupMode::INFO;
    payload.headerTitle = "ERROR";
    payload.cardTitle = "AKSI DITOLAK";
    payload.description = message.empty() ? "Aksi ditolak oleh backend." : message;
    payload.actionItems = {
        PopupActionItem{"close", "OK", "assets/images/ui/btn_cancel.png", true}};

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(payload, [this, onOk = std::move(onOk)](const std::string&) mutable {
        m_popupBox->hide();
        if (onOk) {
            onOk();
            return;
        }
        resumeFlowAfterPopup();
    });
}

void SfmlGuiManager::resumeFlowAfterPopup() {
    refreshFromEngineState();
    m_currentState = GuiState::IDLE;
    m_mainUi->setRollVisible(true);
    setUiInputEnabled(true);
}
} // namespace viewsGUI
