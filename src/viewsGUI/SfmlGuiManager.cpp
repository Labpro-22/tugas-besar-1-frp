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
#include "../../include/models/GameContext.hpp"
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
constexpr float kTimedCardPopupDurationSec = 4.0f;

bool isChanceNearestRailroadPayload(const std::string& payload) {
    return payload == "CHANCE_NEAREST_RAILROAD" || payload == "CHANCE_STASIUN";
}

bool isChanceMoveBackPayload(const std::string& payload) {
    return payload == "CHANCE_MOVE_BACK" || payload == "CHANCE_MUNDUR";
}

bool isChanceGoToJailPayload(const std::string& payload) {
    return payload == "CHANCE_GO_TO_JAIL" || payload == "CHANCE_PENJARA";
}

bool isChanceFestivalPayload(const std::string& payload) {
    return payload == "CHANCE_FESTIVAL";
}

bool isChanceTimedMovementPayload(const std::string& payload) {
    return isChanceNearestRailroadPayload(payload) ||
           isChanceMoveBackPayload(payload) ||
           isChanceGoToJailPayload(payload) ||
           isChanceFestivalPayload(payload);
}

bool isDrawChancePromptId(const std::string& promptId) {
    return promptId.rfind("draw_chance_card_", 0) == 0;
}

std::vector<int> buildFollowUpPathForTimedChance(const std::string& payload,
                                                 int fromIndex,
                                                 int finalIndex,
                                                 int boardSize) {
    std::vector<int> path;
    if (finalIndex < 0) {
        return path;
    }

    if (boardSize <= 0) {
        path.push_back(finalIndex);
        return path;
    }

    if (isChanceMoveBackPayload(payload)) {
        int idx = fromIndex;
        for (int i = 0; i < 3; ++i) {
            idx = (idx - 1 + boardSize) % boardSize;
            path.push_back(idx);
        }
        return path;
    }

    if (isChanceNearestRailroadPayload(payload) || isChanceFestivalPayload(payload)) {
        int idx = fromIndex;
        for (int i = 0; i < boardSize; ++i) {
            idx = (idx + 1) % boardSize;
            path.push_back(idx);
            if (idx == finalIndex) {
                break;
            }
        }
        if (path.empty() || path.back() != finalIndex) {
            path.clear();
            path.push_back(finalIndex);
        }
        return path;
    }

    path.push_back(finalIndex);
    return path;
}

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

std::string trimCopy(std::string value) {
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

std::string normalizeSaveFilename(std::string value, const std::string& fallbackName) {
    value = trimCopy(value);
    if (value.empty()) {
        value = fallbackName;
    }

    std::filesystem::path path(value);
    if (path.extension().empty() ||
        path.extension() != ".txt") {
        path.replace_extension(".txt");
    }
    if (!path.has_parent_path()) {
        path = std::filesystem::path("data") / path;
    }

    return path.generic_string();
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

std::string buildSkillLabel(const SkillCard& card) {
    return card.getTypeName();
}

void appendPreTurnDebugLog(const std::string& line) {
    std::ofstream out("build/gui_preturn_debug.log", std::ios::app);
    if (out.is_open()) {
        out << line << '\n';
    }
}

void centerTextInRect(sf::Text& text, const sf::FloatRect& rect) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + (bounds.width * 0.5f), bounds.top + (bounds.height * 0.5f));
    text.setPosition(rect.left + (rect.width * 0.5f), rect.top + (rect.height * 0.5f));
}

std::string cleanWinnerToken(std::string value) {
    value = trimCopy(value);
    auto isNameChar = [](unsigned char c) {
        return std::isalnum(c) != 0 || c == '_' || c == '-' || c == ' ';
    };

    while (!value.empty() && !isNameChar(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && !isNameChar(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }

    return trimCopy(value);
}

std::vector<std::string> splitWinnerNamesByComma(const std::string& value) {
    std::vector<std::string> names;
    std::stringstream ss(value);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = cleanWinnerToken(token);
        if (!token.empty()) {
            names.push_back(token);
        }
    }
    return names;
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
      m_currentState(GuiState::START_MENU),
      m_startMenuPlayerPopupVisible(false),
      m_lastMessage("Siap memulai game."),
      m_cardTimer(0.0f),
      m_timedCardPopupFromQueue(false),
      m_pendingSaveFilename(""),
      m_uiBasePath("assets/images/ui/"),
      m_preTurnSkillHandled(false),
      m_jailPopupShownForKey(""),
      m_endGamePopupShown(false),
      m_endGameBannerText("") {
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
    m_debugDicePopup = std::make_unique<DebugDicePopup>(
        sf::Vector2f(Layout1920::kDesignWidth, Layout1920::kDesignHeight), m_titleFont);

    if (!m_boardView->loadAssets("assets/images/board/")) {
        m_lastMessage = "PERINGATAN: Sebagian aset board GUI gagal dimuat.";
    }
    m_boardView->loadIconTextures(m_uiBasePath);
    if (!m_mainUi->loadAssets(m_uiBasePath, "assets/images/board/")) {
        m_lastMessage += "\nPERINGATAN: Sebagian aset UI utama gagal dimuat.";
    }
    if (!m_diceRenderer->loadAssets("assets/images/ui/dice/")) {
        m_lastMessage += "\nPERINGATAN: Aset animasi dadu gagal dimuat.";
    }
    if (!m_popupBox->loadAssets(m_uiBasePath)) {
        m_lastMessage += "\nPERINGATAN: Aset popup gagal dimuat.";
    }

    if (!loadStartMenuAssets()) {
        m_lastMessage += "\nPERINGATAN: Aset start screen gagal dimuat.";
    }

    bindEngineCallbacks();
    setUiInputEnabled(false);
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

bool SfmlGuiManager::loadStartMenuAssets() {
    bool success = true;
    const std::string backgroundPath = m_uiBasePath + "start_screen.png";
    const std::string newGamePath = m_uiBasePath + "btn_newgame.png";
    const std::string loadPath = m_uiBasePath + "btn_load.png";

    if (!m_startMenuBackgroundTexture.loadFromFile(backgroundPath)) {
        std::cerr << "[ERROR] Gagal memuat start screen background: " << backgroundPath << "\n";
        success = false;
    }
    m_startMenuBackgroundSprite.setTexture(m_startMenuBackgroundTexture);
    const sf::Vector2u bgSize = m_startMenuBackgroundTexture.getSize();
    if (bgSize.x > 0 && bgSize.y > 0) {
        m_startMenuBackgroundSprite.setScale(
            Layout1920::kDesignWidth / static_cast<float>(bgSize.x),
            Layout1920::kDesignHeight / static_cast<float>(bgSize.y));
    }

    if (!m_startMenuNewGameButton.loadTextures(newGamePath, newGamePath, newGamePath, newGamePath)) {
        success = false;
    }
    if (!m_startMenuLoadButton.loadTextures(loadPath, loadPath, loadPath, loadPath)) {
        success = false;
    }

    auto applyButtonLayout = [](SpriteButton& button,
                                const std::string& texturePath,
                                sf::Vector2f pos,
                                sf::Vector2f size) {
        button.setPosition(pos);
        sf::Texture textureProbe;
        if (!textureProbe.loadFromFile(texturePath)) {
            return;
        }
        const sf::Vector2u texSize = textureProbe.getSize();
        if (texSize.x == 0 || texSize.y == 0) {
            return;
        }
        button.setScale(sf::Vector2f(size.x / static_cast<float>(texSize.x),
                                     size.y / static_cast<float>(texSize.y)));
    };

    const sf::Vector2f buttonSize(452.4f, 142.1f);
    applyButtonLayout(m_startMenuNewGameButton, newGamePath, sf::Vector2f(507.6f, 715.5f), buttonSize);
    applyButtonLayout(m_startMenuLoadButton, loadPath, sf::Vector2f(960.0f, 715.5f), buttonSize);

    m_startMenuNewGameButton.setOnClick([this]() { openStartMenuPlayerPopup(); });
    m_startMenuLoadButton.setOnClick([this]() { loadGameFromMenu(); });
    setupStartMenuPlayerPopup();

    return success;
}

void SfmlGuiManager::setupStartMenuPlayerPopup() {
    m_startMenuPlayerPopupBackdrop.setSize(sf::Vector2f(Layout1920::kDesignWidth, Layout1920::kDesignHeight));
    m_startMenuPlayerPopupBackdrop.setFillColor(sf::Color(0, 0, 0, 125));

    const sf::Vector2f panelSize(540.0f, 300.0f);
    const sf::Vector2f panelPos((Layout1920::kDesignWidth - panelSize.x) * 0.5f,
                                (Layout1920::kDesignHeight - panelSize.y) * 0.5f);
    m_startMenuPlayerPopupPanel.setSize(panelSize);
    m_startMenuPlayerPopupPanel.setPosition(panelPos);
    m_startMenuPlayerPopupPanel.setFillColor(sf::Color(244, 232, 208));
    m_startMenuPlayerPopupPanel.setOutlineThickness(3.0f);
    m_startMenuPlayerPopupPanel.setOutlineColor(sf::Color(90, 64, 36));

    m_startMenuPlayerPopupTitle.setFont(m_titleFont);
    m_startMenuPlayerPopupTitle.setCharacterSize(54);
    m_startMenuPlayerPopupTitle.setFillColor(sf::Color(52, 40, 28));
    m_startMenuPlayerPopupTitle.setString("PILIH JUMLAH PEMAIN");
    centerTextInRect(m_startMenuPlayerPopupTitle,
                     sf::FloatRect(panelPos.x, panelPos.y + 18.0f, panelSize.x, 80.0f));

    static const std::array<int, 3> kPlayerChoices = {2, 3, 4};
    const float buttonWidth = 130.0f;
    const float buttonHeight = 84.0f;
    const float gap = 34.0f;
    const float totalWidth = (buttonWidth * 3.0f) + (gap * 2.0f);
    const float startX = panelPos.x + (panelSize.x - totalWidth) * 0.5f;
    const float y = panelPos.y + 152.0f;

    for (size_t i = 0; i < m_startMenuPlayerButtons.size(); ++i) {
        sf::RectangleShape& button = m_startMenuPlayerButtons[i];
        button.setSize(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(startX + static_cast<float>(i) * (buttonWidth + gap), y);
        button.setFillColor(sf::Color(231, 214, 183));
        button.setOutlineThickness(2.0f);
        button.setOutlineColor(sf::Color(88, 63, 37));

        sf::Text& label = m_startMenuPlayerButtonLabels[i];
        label.setFont(m_titleFont);
        label.setCharacterSize(52);
        label.setFillColor(sf::Color(53, 42, 29));
        label.setString(std::to_string(kPlayerChoices[i]) + "P");
        centerTextInRect(label, button.getGlobalBounds());
    }
}

void SfmlGuiManager::updateStartMenuPlayerPopupHover(sf::Vector2f mousePos) {
    for (size_t i = 0; i < m_startMenuPlayerButtons.size(); ++i) {
        sf::RectangleShape& button = m_startMenuPlayerButtons[i];
        if (button.getGlobalBounds().contains(mousePos)) {
            button.setFillColor(sf::Color(241, 226, 197));
        } else {
            button.setFillColor(sf::Color(231, 214, 183));
        }
    }
}

int SfmlGuiManager::hitStartMenuPlayerButton(sf::Vector2f mousePos) const {
    for (size_t i = 0; i < m_startMenuPlayerButtons.size(); ++i) {
        if (m_startMenuPlayerButtons[i].getGlobalBounds().contains(mousePos)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void SfmlGuiManager::openStartMenuPlayerPopup() {
    m_startMenuPlayerPopupVisible = true;
}

void SfmlGuiManager::closeStartMenuPlayerPopup() {
    m_startMenuPlayerPopupVisible = false;
}

void SfmlGuiManager::renderStartMenu() {
    if (m_startMenuBackgroundTexture.getSize().x > 0 && m_startMenuBackgroundTexture.getSize().y > 0) {
        m_window.draw(m_startMenuBackgroundSprite);
    } else {
        sf::RectangleShape fallback(sf::Vector2f(Layout1920::kDesignWidth, Layout1920::kDesignHeight));
        fallback.setFillColor(sf::Color(32, 24, 17));
        m_window.draw(fallback);
    }

    m_startMenuNewGameButton.render(m_window);
    m_startMenuLoadButton.render(m_window);

    if (m_startMenuPlayerPopupVisible) {
        m_window.draw(m_startMenuPlayerPopupBackdrop);
        m_window.draw(m_startMenuPlayerPopupPanel);
        m_window.draw(m_startMenuPlayerPopupTitle);
        for (size_t i = 0; i < m_startMenuPlayerButtons.size(); ++i) {
            m_window.draw(m_startMenuPlayerButtons[i]);
            m_window.draw(m_startMenuPlayerButtonLabels[i]);
        }
    }
}

void SfmlGuiManager::startNewGameFromMenu(int playerCount) {
    try {
        closeStartMenuPlayerPopup();
        m_pendingPrompts.clear();
        m_pendingCardEventImagePaths.clear();
        m_deferredMovement.reset();
        m_splitMovementForCommand.reset();
        m_landingMovementContext.reset();
        m_timedCardImageAfterLanding.reset();
        m_timedCardMovementAfterPopup.reset();
        m_cardTimer = 0.0f;
        m_timedCardPopupFromQueue = false;
        m_preTurnSkillGateKey.clear();
        m_preTurnSkillHandled = false;
        m_jailPopupShownForKey.clear();
        m_endGamePopupShown = false;
        m_endGameBannerText.clear();
        if (m_popupBox->isVisible()) {
            m_popupBox->hide();
        }
        if (m_debugDicePopup && m_debugDicePopup->isVisible()) {
            m_debugDicePopup->hide();
        }

        std::vector<std::string> names;
        names.reserve(static_cast<size_t>(playerCount));
        for (int i = 1; i <= playerCount; ++i) {
            names.push_back("P" + std::to_string(i));
        }

        const CommandResult startResult = m_engine.startNewGame(playerCount, names);
        initializeGameAndPieces();
        consumeResult(startResult, true);
        enqueuePrompts(startResult.prompts);

        m_currentState = GuiState::IDLE;
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        if (!m_pendingPrompts.empty()) {
            processNextPrompt();
        }
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
    }
}

void SfmlGuiManager::loadGameFromMenu() {
    openLoadFilenamePopupFromMenu();
}

void SfmlGuiManager::openLoadFilenamePopupFromMenu() {
    closeStartMenuPlayerPopup();

    PromptRequest prompt;
    prompt.id = "file_input_load";
    prompt.title = "MUAT PERMAINAN";
    prompt.message = "Masukkan nama file untuk dimuat (default: data/file_save.txt).";
    prompt.required = false;
    prompt.options = {
        PromptOption{"SUBMIT", "Muat"},
        PromptOption{"cancel", "Batal"}};

    m_popupBox->showPrompt(prompt, [this](const std::string& answerKey) {
        const std::string cleanAnswer = sanitizeActionId(answerKey);
        m_popupBox->hide();

        const std::string upper = upperCopy(cleanAnswer);
        if (upper == "CANCEL" || upper == "CLOSE") {
            return;
        }

        const std::string filename = normalizeSaveFilename(cleanAnswer, "data/file_save.txt");
        submitLoadFromMenuRequest(filename);
    });
}

void SfmlGuiManager::submitLoadFromMenuRequest(const std::string& filename) {
    try {
        m_pendingPrompts.clear();
        m_pendingCardEventImagePaths.clear();
        m_deferredMovement.reset();
        m_splitMovementForCommand.reset();
        m_landingMovementContext.reset();
        m_timedCardImageAfterLanding.reset();
        m_timedCardMovementAfterPopup.reset();
        m_cardTimer = 0.0f;
        m_timedCardPopupFromQueue = false;
        m_preTurnSkillGateKey.clear();
        m_preTurnSkillHandled = false;
        m_jailPopupShownForKey.clear();
        m_endGamePopupShown = false;
        m_endGameBannerText.clear();
        if (m_popupBox->isVisible()) {
            m_popupBox->hide();
        }
        if (m_debugDicePopup && m_debugDicePopup->isVisible()) {
            m_debugDicePopup->hide();
        }

        CommandResult loadResult;
        std::string loadError;
        if (!m_engine.tryLoadGame(filename, loadResult, loadError)) {
            showStartMenuMessagePopup("GAGAL MUAT",
                                      loadError.empty() ? "File tidak ditemukan atau rusak."
                                                        : loadError);
            return;
        }

        initializeGameAndPieces();
        consumeResult(loadResult, true);
        enqueuePrompts(loadResult.prompts);

        m_currentState = GuiState::IDLE;
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        if (!m_pendingPrompts.empty()) {
            processNextPrompt();
        }
    } catch (const std::exception& e) {
        showStartMenuMessagePopup("GAGAL MUAT", e.what());
    }
}

void SfmlGuiManager::showStartMenuMessagePopup(const std::string& title, const std::string& message) {
    PopupPayload payload;
    payload.mode = PopupMode::INFO;
    payload.headerTitle = title.empty() ? "NOTIFIKASI" : title;
    payload.cardTitle = payload.headerTitle;
    payload.description = message.empty() ? "Terjadi kesalahan." : message;
    payload.actionItems = {
        PopupActionItem{"close", "OK", "assets/images/ui/btn_cancel.png", true}};

    m_popupBox->show(payload, [this](const std::string&) {
        m_popupBox->hide();
    });
}

void SfmlGuiManager::initializeGameAndPieces() {
    const auto& players = m_engine.getPlayers();
    m_boardView->setTileCountHint(m_engine.getBoard().size());
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

void SfmlGuiManager::openSaveFilenamePopup() {
    PromptRequest prompt;
    prompt.id = "file_input_save";
    prompt.title = "SIMPAN PERMAINAN";
    prompt.message = "Masukkan nama file save (default: data/file_save.txt).";
    prompt.required = false;
    prompt.options = {
        PromptOption{"SUBMIT", "Simpan"},
        PromptOption{"cancel", "Batal"}};

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->showPrompt(prompt, [this](const std::string& answerKey) {
        const std::string cleanAnswer = sanitizeActionId(answerKey);
        m_popupBox->hide();

        const std::string upper = upperCopy(cleanAnswer);
        if (upper == "CANCEL" || upper == "CLOSE") {
            resumeFlowAfterPopup();
            return;
        }

        const std::string filename = normalizeSaveFilename(cleanAnswer, "data/file_save.txt");
        submitSaveRequest(filename, false);
    });
}

void SfmlGuiManager::submitSaveRequest(const std::string& filename, bool overwrite) {
    m_pendingSaveFilename = filename;
    const SaveGameResult saveResult = m_engine.saveGame(filename, overwrite);

    if (saveResult.status == SaveGameStatus::SUCCESS) {
        showSaveNotificationPopup("BERHASIL MENYIMPAN",
                                  saveResult.message.empty() ? "Permainan berhasil disimpan."
                                                             : saveResult.message,
                                  UiTone::SUCCESS);
        return;
    }

    if (saveResult.status == SaveGameStatus::FILE_EXISTS) {
        showSaveOverwriteConfirmation(filename);
        return;
    }

    showSaveNotificationPopup("GAGAL MENYIMPAN",
                              saveResult.message.empty() ? "Gagal menyimpan permainan."
                                                         : saveResult.message,
                              UiTone::ERROR);
}

void SfmlGuiManager::showSaveOverwriteConfirmation(const std::string& filename) {
    PopupPayload payload;
    payload.mode = PopupMode::INFO;
    payload.headerTitle = "KONFIRMASI";
    payload.cardTitle = "SIMPAN";
    payload.description = "File sudah ada. Timpa file lama?\n" + filename;
    payload.actionItems = {
        PopupActionItem{"overwrite_yes", "Ya, Timpa", "assets/images/ui/btn_beli.png", true},
        PopupActionItem{"cancel", "Tidak", "assets/images/ui/btn_cancel.png", true}};

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(payload, [this, filename](const std::string& actionId) {
        const std::string cleanAction = sanitizeActionId(actionId);
        m_popupBox->hide();
        if (cleanAction == "overwrite_yes") {
            submitSaveRequest(filename, true);
            return;
        }
        resumeFlowAfterPopup();
    });
}

void SfmlGuiManager::showSaveNotificationPopup(const std::string& title,
                                               const std::string& message,
                                               UiTone tone) {
    PopupPayload payload;
    payload.mode = PopupMode::INFO;
    payload.headerTitle = title.empty() ? "NOTIFIKASI" : title;
    payload.cardTitle = (tone == UiTone::ERROR) ? "ERROR" : "SUKSES";
    payload.description = message.empty() ? "Operasi selesai." : message;
    payload.actionItems = {
        PopupActionItem{"close", "OK", "assets/images/ui/btn_cancel.png", true}};

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(payload, [this](const std::string&) {
        m_popupBox->hide();
        resumeFlowAfterPopup();
    });
}

void SfmlGuiManager::submitRollDice() {
    Command cmd;
    cmd.type = CommandType::ROLL_DICE;
    cmd.raw = "LEMPAR_DADU";
    executeDiceCommand(cmd);
}

void SfmlGuiManager::submitManualDice(int die1, int die2) {
    Command cmd;
    cmd.type = CommandType::SET_DICE;
    cmd.raw = "ATUR_DADU " + std::to_string(die1) + " " + std::to_string(die2);
    cmd.args = {std::to_string(die1), std::to_string(die2)};
    executeDiceCommand(cmd);
}

void SfmlGuiManager::executeDiceCommand(const Command& cmd) {
    try {
        m_mainUi->setRollVisible(false);
        setUiInputEnabled(false);
        m_pendingPrompts.clear();
        m_pendingCardEventImagePaths.clear();
        m_cardTimer = 0.0f;
        m_timedCardPopupFromQueue = false;
        m_deferredMovement.reset();
        m_landingMovementContext.reset();
        m_timedCardImageAfterLanding.reset();
        m_timedCardMovementAfterPopup.reset();
        m_splitMovementForCommand.reset();

        const CommandResult result = m_engine.processCommand(cmd);
        consumeResult(result, !result.movement.has_value());
        enqueuePrompts(result.prompts);

        const std::optional<MovementPayload> movementToAnimate =
            m_splitMovementForCommand.has_value() ? m_splitMovementForCommand : result.movement;
        const bool hasMovement = movementToAnimate.has_value();

        const Dice& dice = m_engine.getDice();
        const sf::Vector2f boardCenter = m_boardView->getBoardCenter();
        m_diceRenderer->startRoll(dice.getDie1(),
                                  dice.getDie2(),
                                  sf::Vector2f(boardCenter.x + Layout1920::kDiceCenterOffsetFromBoardCenter.x,
                                               boardCenter.y + Layout1920::kDiceCenterOffsetFromBoardCenter.y));

        if (hasMovement) {
            m_deferredMovement = movementToAnimate.value();
        } else {
            m_deferredMovement.reset();
        }
        m_splitMovementForCommand.reset();
        m_landingMovementContext.reset();

        m_currentState = GuiState::ANIMATING_DICE;
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        refreshFromEngineState();
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        m_currentState = GuiState::IDLE;
    }
}

void SfmlGuiManager::openDebugDicePopup() {
    if (!m_debugDicePopup || m_debugDicePopup->isVisible()) {
        return;
    }
    if (m_currentState != GuiState::IDLE || m_popupBox->isVisible()) {
        return;
    }

    setUiInputEnabled(false);
    m_debugDicePopup->show(
        [this](int die1, int die2) {
            if (m_debugDicePopup) {
                m_debugDicePopup->hide();
            }
            setUiInputEnabled(true);
            submitManualDice(die1, die2);
        },
        [this]() {
            if (m_debugDicePopup) {
                m_debugDicePopup->hide();
            }
            setUiInputEnabled(true);
        });
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
    // Jangan proses prompt saat fase animasi masih berjalan.
    if (m_currentState == GuiState::ANIMATING_DICE ||
        m_currentState == GuiState::SHOWING_TIMED_CARD ||
        (m_currentState == GuiState::ANIMATING_PIECE && m_deferredMovement.has_value())) {
        return;
    }

    if (m_popupBox->isVisible()) {
        return;
    }

    if (showPendingCardEventPopup()) {
        return;
    }

    if (m_pendingPrompts.empty()) {
        if (m_engine.hasPendingContinuation()) {
            try {
                const CommandResult resumed = m_engine.resumePendingAction();
                const bool hasMovement = resumed.movement.has_value();
                consumeResult(resumed, !hasMovement);
                enqueuePrompts(resumed.prompts);
                if (!resumed.success && !hasMovement && resumed.prompts.empty()) {
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
                m_engine.clearPendingContinuation();
                m_engine.clearPromptAnswers();
                showBackendErrorPopup(e.what());
            }
            return;
        }
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

void SfmlGuiManager::maybeShowJailPopup() {
    if (m_currentState != GuiState::IDLE || m_popupBox->isVisible()) {
        return;
    }
    if (!m_pendingPrompts.empty() || m_engine.hasPendingContinuation()) {
        return;
    }

    const Player& current = m_engine.getCurrentPlayer();
    if (!current.isJailed() || m_engine.isDiceRolledThisTurn()) {
        return;
    }

    // Gunakan gate key per giliran+pemain supaya popup tidak muncul berkali-kali
    const std::string gateKey = current.getUsername() + "@jail@" +
                                std::to_string(m_engine.getCurrentTurn());
    if (m_jailPopupShownForKey == gateKey) {
        return;
    }
    m_jailPopupShownForKey = gateKey;

    const int jailTurns = current.getJailTurns();
    const int jailFine  = m_engine.getJailFine();

    PopupPayload payload;
    payload.mode        = PopupMode::SPECIAL;
    payload.headerTitle = "DALAM PENJARA";
    payload.cardTitle   = current.getUsername() + " di Penjara";
    const bool hasCard = current.hasJailFreeCard();
    payload.description =
        "Percobaan ke-" + std::to_string(jailTurns + 1) + " dari 3.\n"
        "Pilih tindakan:\n"
        "  • Bayar Denda: bayar M" + std::to_string(jailFine) +
        " lalu lempar dadu (double = bergerak + giliran lagi; non-double = bebas tapi tidak bergerak).\n"
        "  • Roll Dadu: langsung lempar, keluar jika mendapat double." +
        (hasCard ? "\n  • Pakai Kartu: gunakan kartu Bebas dari Penjara, lalu lempar dadu normal." : "");
    payload.actionItems = {
        PopupActionItem{"jail_pay_fine", "Bayar Denda (M" + std::to_string(jailFine) + ")",
                        "assets/images/ui/btn_beli.png", true},
        PopupActionItem{"jail_roll_dice", "Roll Dadu",
                        "assets/images/ui/btn_cancel.png", true}};
    payload.description =
        "Percobaan ke-" + std::to_string(jailTurns + 1) + " dari 3.\n"
        "Pilih tindakan:\n"
        "  - Bayar Denda: bayar M" + std::to_string(jailFine) +
        " lalu lempar dadu (double = bergerak + giliran lagi; non-double = bebas tapi tidak bergerak).\n"
        "  - Roll Dadu: langsung lempar, keluar jika mendapat double, lalu lempar lagi untuk bergerak." +
        (hasCard ? "\n  - Pakai Kartu: gunakan kartu Bebas dari Penjara, lalu lempar dadu normal." : "");
    if (hasCard) {
        payload.actionItems.push_back(
            PopupActionItem{"jail_use_card", "Pakai Kartu Bebas",
                            "assets/images/ui/btn_beli.png", true});
    }

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);

    m_popupBox->show(payload, [this](const std::string& actionId) {
        const std::string cleanId = sanitizeActionId(actionId);
        m_popupBox->hide();

        if (cleanId == "jail_pay_fine") {
            try {
                Command cmd;
                cmd.type = CommandType::PAY_JAIL_FINE;
                cmd.raw  = "BAYAR_DENDA";
                const CommandResult result = m_engine.processCommand(cmd);
                consumeResult(result, true);
                enqueuePrompts(result.prompts);
                // Setelah bayar denda pemain bisa roll; kembalikan tombol roll
                refreshFromEngineState();
                m_currentState = GuiState::IDLE;
                m_mainUi->setRollVisible(true);
                setUiInputEnabled(true);
                if (!m_pendingPrompts.empty()) {
                    processNextPrompt();
                }
            } catch (const std::exception& e) {
                m_lastMessage = std::string("ERROR: ") + e.what();
                refreshFromEngineState();
                resumeFlowAfterPopup();
            }
            return;
        }

        if (cleanId == "jail_use_card") {
            try {
                Command cmd;
                cmd.type = CommandType::USE_JAIL_CARD;
                cmd.raw  = "PAKAI_KARTU_BEBAS";
                const CommandResult result = m_engine.processCommand(cmd);
                consumeResult(result, true);
                enqueuePrompts(result.prompts);
                refreshFromEngineState();
                m_currentState = GuiState::IDLE;
                m_mainUi->setRollVisible(true);
                setUiInputEnabled(true);
                if (!m_pendingPrompts.empty()) {
                    processNextPrompt();
                }
            } catch (const std::exception& e) {
                m_lastMessage = std::string("ERROR: ") + e.what();
                refreshFromEngineState();
                resumeFlowAfterPopup();
            }
            return;
        }

        // jail_roll_dice atau close → biarkan pemain menekan tombol roll sendiri
        resumeFlowAfterPopup();
    });
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
    payload.cardTitle = "PILIH KARTU KEMAMPUAN";
    payload.description =
        "Pilih kartu kemampuan yang ingin Anda gunakan atau lanjut ke lempar dadu maka kartu Anda akan tersimpan.";

    for (size_t i = 0; i < hand.size(); ++i) {
        const std::shared_ptr<SkillCard>& card = hand[i];
        if (!card) {
            continue;
        }

        const int engineCardIndex = static_cast<int>(i) + 1;
        payload.actionItems.push_back(PopupActionItem{
            std::string(kPreTurnSkillUsePrefix) + std::to_string(engineCardIndex),
            buildSkillLabel(*card),
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

void SfmlGuiManager::cacheTimedChanceTransition(const CommandResult& result) {
    if (result.commandName != "LEMPAR_DADU" &&
        result.commandName != "ATUR_DADU" &&
        result.commandName != "GUNAKAN_KEMAMPUAN") {
        return;
    }

    std::string timedPayload;
    for (const GameEvent& event : result.events) {
        if (event.type == GameEventType::CARD && isChanceTimedMovementPayload(event.eventPayload)) {
            timedPayload = event.eventPayload;
            break;
        }
    }

    if (timedPayload.empty() || !result.movement.has_value()) {
        return;
    }

    const MovementPayload& fullMovement = result.movement.value();
    if (fullMovement.path.size() < 2) {
        return;
    }

    const std::string imagePath = mapCardEventPayloadToImage(timedPayload);
    if (imagePath.empty()) {
        return;
    }

    int chancePathIndex = -1;
    const int boardSize = m_engine.getBoard().size();
    for (size_t i = 0; i < fullMovement.path.size(); ++i) {
        const int tileIndex = fullMovement.path[i];
        if (tileIndex < 0 || tileIndex >= boardSize) {
            continue;
        }
        const Tile& tile = m_engine.getBoard().getTileByIndex(tileIndex);
        const auto* cardTile = dynamic_cast<const CardTile*>(&tile);
        if (cardTile && cardTile->getDrawType() == CardDrawType::CHANCE) {
            chancePathIndex = static_cast<int>(i);
            break;
        }
    }
    if (chancePathIndex < 0 || chancePathIndex >= static_cast<int>(fullMovement.path.size()) - 1) {
        return;
    }

    const int chanceLandingIndex = fullMovement.path[static_cast<size_t>(chancePathIndex)];
    const int finalIndex = fullMovement.path.back();
    if (chanceLandingIndex == finalIndex) {
        return;
    }

    MovementPayload firstPhase = fullMovement;
    firstPhase.path.assign(
        fullMovement.path.begin(),
        fullMovement.path.begin() + static_cast<std::ptrdiff_t>(chancePathIndex + 1));
    if (firstPhase.path.empty()) {
        return;
    }
    firstPhase.toIndex = firstPhase.path.back();

    MovementPayload followUpPhase;
    followUpPhase.playerIndex = fullMovement.playerIndex;
    followUpPhase.playerName = fullMovement.playerName;
    followUpPhase.fromIndex = chanceLandingIndex;
    followUpPhase.toIndex = finalIndex;
    followUpPhase.path = buildFollowUpPathForTimedChance(
        timedPayload, chanceLandingIndex, finalIndex, boardSize);
    if (followUpPhase.path.empty()) {
        followUpPhase.path.push_back(finalIndex);
    }
    followUpPhase.toIndex = followUpPhase.path.back();

    m_splitMovementForCommand = firstPhase;
    m_timedCardImageAfterLanding = imagePath;
    m_timedCardMovementAfterPopup = followUpPhase;
}

void SfmlGuiManager::consumeResult(const CommandResult& result, bool syncPiecePositions) {
    cacheTimedChanceTransition(result);
    updateEndGameStateFromResult(result);
    m_lastMessage = buildMessageFromResult(result);
    enqueueCardEventPopups(result);
    std::string logPanelText = m_engine.getTransactionLogReport();
    if (logPanelText.empty()) {
        logPanelText = "=== Log Transaksi ===\n";
    }

    const Player& current = m_engine.getCurrentPlayer();
    m_mainUi->updateData(m_engine.getLeaderboard(),
                         current,
                         logPanelText,
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

void SfmlGuiManager::refreshFromEngineState() {
    std::string logPanelText = m_engine.getTransactionLogReport();
    if (logPanelText.empty()) {
        logPanelText = "=== Log Transaksi ===\n";
    }

    const Player& current = m_engine.getCurrentPlayer();
    m_mainUi->updateData(m_engine.getLeaderboard(),
                         current,
                         logPanelText,
                         m_engine.getCurrentTurn(),
                         m_engine.getMaxTurn());

    const auto& players = m_engine.getPlayers();
    const size_t count = std::min(players.size(), m_players.size());
    for (size_t i = 0; i < count; ++i) {
        m_players[i]->snapTo(m_boardView->getTileCenter(players[i]->getPosition()));
    }

    // Update board renderer with current player pointer → index mapping for building icons
    std::vector<const void*> playerPtrs;
    playerPtrs.reserve(players.size());
    for (const Player* p : players) {
        playerPtrs.push_back(static_cast<const void*>(p));
    }
    m_boardView->updatePlayerInfo(playerPtrs);
}

void SfmlGuiManager::updateAllPanels() {
    refreshFromEngineState();
}

std::string SfmlGuiManager::mapCardEventPayloadToImage(const std::string& payload) const {
    if (payload == "CARD_BIRTHDAY") {
        return m_uiBasePath + "dana_umum_m100.png";
    }
    if (payload == "CARD_DOCTOR") {
        return m_uiBasePath + "dana_umum_m700.png";
    }
    if (payload == "CARD_ELECTION") {
        return m_uiBasePath + "dana_umum_m200x(n-1).png";
    }
    if (payload == "CARD_ARISAN") {
        return m_uiBasePath + "dana_umum_m300.png";
    }
    if (payload == "CARD_BEGAL") {
        return m_uiBasePath + "dana_umum_m200.png";
    }
    if (payload == "CHANCE_NEAREST_RAILROAD" || payload == "CHANCE_STASIUN") {
        return m_uiBasePath + "kesempatan_stasiun.png";
    }
    if (payload == "CHANCE_MOVE_BACK" || payload == "CHANCE_MUNDUR") {
        return m_uiBasePath + "kesempatan_mundur.png";
    }
    if (payload == "CHANCE_GO_TO_JAIL" || payload == "CHANCE_PENJARA") {
        return m_uiBasePath + "kesempatan_penjara.png";
    }
    if (payload == "CHANCE_FESTIVAL") {
        return m_uiBasePath + "kesempatan_festival.png";
    }
    // Canonical backend payload: CHANCE_BEBAS_PENJARA.
    // Alias lama tetap diterima demi kompatibilitas data/event lama.
    if (payload == "CHANCE_BEBAS_PENJARA" ||
        payload == "CHANCE_GET_OUT_OF_JAIL" ||
        payload == "CHANCE_KELUAR") {
        return m_uiBasePath + "kesempatan_keluar.png";
    }
    return "";
}

void SfmlGuiManager::enqueueCardEventPopups(const CommandResult& result) {
    const bool hasDeferredDrawChancePrompt = std::any_of(
        result.prompts.begin(), result.prompts.end(), [](const PromptRequest& prompt) {
            return isDrawChancePromptId(prompt.id);
        });
    const bool hasSplitTimedChanceMovement = m_splitMovementForCommand.has_value();

    for (const GameEvent& event : result.events) {
        if (event.type != GameEventType::CARD || event.eventPayload.empty()) {
            continue;
        }
        if (isChanceTimedMovementPayload(event.eventPayload) &&
            (hasSplitTimedChanceMovement || !hasDeferredDrawChancePrompt)) {
            continue;
        }

        const std::string imagePath = mapCardEventPayloadToImage(event.eventPayload);
        if (!imagePath.empty()) {
            m_pendingCardEventImagePaths.push_back(imagePath);
        }
    }
}

bool SfmlGuiManager::showPendingCardEventPopup() {
    if (m_pendingCardEventImagePaths.empty()) {
        return false;
    }

    const std::string imagePath = m_pendingCardEventImagePaths.front();

    PopupPayload payload;
    payload.mode = PopupMode::FULL_IMAGE_DISMISSABLE;
    payload.cardTemplateImagePath = imagePath;

    m_currentState = GuiState::SHOWING_TIMED_CARD;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);
    m_timedCardPopupFromQueue = true;
    m_cardTimer = 0.0f;
    m_timedCardMovementAfterPopup.reset();

    m_popupBox->show(payload, [this](const std::string&) {});
    return true;
}

void SfmlGuiManager::dismissTimedCardEventPopup() {
    if (m_currentState != GuiState::SHOWING_TIMED_CARD) {
        return;
    }

    m_cardTimer = 0.0f;

    if (m_popupBox->isVisible()) {
        m_popupBox->hide();
    }

    if (m_timedCardPopupFromQueue && !m_pendingCardEventImagePaths.empty()) {
        m_pendingCardEventImagePaths.pop_front();
    }
    m_timedCardPopupFromQueue = false;

    if (m_timedCardMovementAfterPopup.has_value()) {
        const MovementPayload followUpMovement = m_timedCardMovementAfterPopup.value();
        m_timedCardMovementAfterPopup.reset();
        m_currentState = GuiState::IDLE;
        beginMovementAnimation(followUpMovement);
        return;
    }

    m_currentState = GuiState::IDLE;
    updateAllPanels();
    processNextPrompt();
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
        const bool debugPopupVisible = m_debugDicePopup && m_debugDicePopup->isVisible();
        const bool showingTimedCard = (m_currentState == GuiState::SHOWING_TIMED_CARD);

        if (event.type == sf::Event::Closed) {
            m_window.close();
            continue;
        }

        if (event.type == sf::Event::Resized) {
            updateLetterboxView(event.size.width, event.size.height);
            continue;
        }

        if (m_currentState == GuiState::START_MENU) {
            if (m_popupBox->isVisible()) {
                if (event.type == sf::Event::TextEntered) {
                    m_popupBox->handleTextEntered(event.text.unicode);
                    continue;
                }

                if (event.type == sf::Event::KeyPressed) {
                    m_popupBox->handleKeyPressed(event.key.code);
                    continue;
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    const sf::Vector2f mousePos =
                        m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
                    m_popupBox->handleMousePressed(mousePos);
                    continue;
                }

                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    const sf::Vector2f mousePos =
                        m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
                    m_popupBox->handleMouseReleased(mousePos);
                    continue;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                const sf::Vector2f mousePos =
                    m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
                if (m_startMenuPlayerPopupVisible) {
                    const int hit = hitStartMenuPlayerButton(mousePos);
                    if (hit >= 0) {
                        m_startMenuPlayerButtons[static_cast<size_t>(hit)].setFillColor(sf::Color(215, 189, 145));
                    }
                } else {
                    m_startMenuNewGameButton.handleMousePressed(mousePos);
                    m_startMenuLoadButton.handleMousePressed(mousePos);
                }
            } else if (event.type == sf::Event::MouseButtonReleased &&
                       event.mouseButton.button == sf::Mouse::Left) {
                const sf::Vector2f mousePos =
                    m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
                if (m_startMenuPlayerPopupVisible) {
                    const int hit = hitStartMenuPlayerButton(mousePos);
                    if (hit == 0) {
                        startNewGameFromMenu(2);
                    } else if (hit == 1) {
                        startNewGameFromMenu(3);
                    } else if (hit == 2) {
                        startNewGameFromMenu(4);
                    } else if (!m_startMenuPlayerPopupPanel.getGlobalBounds().contains(mousePos)) {
                        closeStartMenuPlayerPopup();
                    }
                    updateStartMenuPlayerPopupHover(mousePos);
                } else {
                    if (m_startMenuNewGameButton.handleMouseReleased(mousePos)) {
                        continue;
                    }
                    m_startMenuLoadButton.handleMouseReleased(mousePos);
                }
            }
            continue;
        }

        if (event.type == sf::Event::TextEntered) {
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                continue;
            }
            if (m_popupBox->isVisible() && !m_popupBox->isMinimized() &&
                m_popupBox->handleTextEntered(event.text.unicode)) {
                continue;
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                continue;
            }
            if (m_popupBox->isVisible() && !m_popupBox->isMinimized() &&
                m_popupBox->handleKeyPressed(event.key.code)) {
                continue;
            }

            if (event.key.control && event.key.code == sf::Keyboard::S) {
                if ((m_currentState != GuiState::IDLE && m_currentState != GuiState::END_GAME) ||
                    m_popupBox->isVisible()) {
                    continue;
                }
                if (!m_engine.isGameOver() && !m_engine.canSaveAtTurnStart()) {
                    showBackendErrorPopup("SIMPAN hanya boleh dilakukan di awal giliran sebelum aksi apapun.");
                    continue;
                }
                openSaveFilenamePopup();
                continue;
            }
        }

        if (event.type == sf::Event::MouseWheelScrolled) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                continue;
            }
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
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                m_debugDicePopup->handleMousePressed(mousePos, sf::Mouse::Left);
                continue;
            }
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
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                m_debugDicePopup->handleMouseReleased(mousePos, sf::Mouse::Left);
                continue;
            }
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

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                m_debugDicePopup->handleMousePressed(mousePos, sf::Mouse::Right);
                continue;
            }

            if (m_currentState == GuiState::IDLE && !m_popupBox->isVisible() &&
                m_mainUi->isRollDiceButtonHit(mousePos)) {
                openDebugDicePopup();
            }
            continue;
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
            if (showingTimedCard) {
                continue;
            }
            if (debugPopupVisible) {
                const sf::Vector2f mousePos =
                    m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);
                m_debugDicePopup->handleMouseReleased(mousePos, sf::Mouse::Right);
                continue;
            }
        }
    }
}

void SfmlGuiManager::update(sf::Time dt) {
    const sf::Vector2f mousePos =
        m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

    if (m_currentState == GuiState::START_MENU) {
        if (m_popupBox->isVisible()) {
            m_popupBox->update(mousePos);
        } else if (m_startMenuPlayerPopupVisible) {
            updateStartMenuPlayerPopupHover(mousePos);
        } else {
            m_startMenuNewGameButton.update(mousePos);
            m_startMenuLoadButton.update(mousePos);
        }
        return;
    }

    maybeShowEndGamePopup();

    const bool debugPopupVisible = m_debugDicePopup && m_debugDicePopup->isVisible();
    const bool popupExpanded = m_popupBox->isVisible() && !m_popupBox->isMinimized();
    if (debugPopupVisible) {
        m_debugDicePopup->update(mousePos);
    } else if (popupExpanded) {
        m_popupBox->update(mousePos);
    } else {
        m_mainUi->update(mousePos);
        if (m_popupBox->isVisible()) {
            m_popupBox->update(mousePos);
        }
    }

    if (m_currentState == GuiState::SHOWING_TIMED_CARD) {
        m_cardTimer += dt.asSeconds();
        if (m_cardTimer >= kTimedCardPopupDurationSec) {
            dismissTimedCardEventPopup();
        }
        return;
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

        m_currentState = GuiState::IDLE;
        refreshFromEngineState();
        processNextPrompt();
        return;
    }

    if (m_currentState == GuiState::ANIMATING_PIECE && !anyMoving) {
        if (m_deferredMovement.has_value()) {
            // Don't snap pieces when a timed card follow-up animation is pending.
            // refreshFromEngineState() would snap the piece to the final destination
            // (e.g., station) before the card popup + follow-up animation run,
            // causing the piece to visually jump back-and-forth.
            if (!m_timedCardImageAfterLanding.has_value()) {
                refreshFromEngineState();
            }
            MovementPayload landing = m_deferredMovement.value();
            m_deferredMovement.reset();
            showLandingPopup(landing);
        } else {
            m_currentState = GuiState::IDLE;
            refreshFromEngineState();
            processNextPrompt();
        }
    }

    if (m_currentState == GuiState::IDLE && !m_popupBox->isVisible() &&
        (!m_debugDicePopup || !m_debugDicePopup->isVisible())) {
        maybeShowJailPopup();
        if (m_currentState == GuiState::IDLE && !m_popupBox->isVisible()) {
            maybeShowPreTurnSkillPopup();
        }
    }
}

void SfmlGuiManager::render() {
    m_window.clear(sf::Color::Black);
    m_window.setView(m_baseView);

    if (m_currentState == GuiState::START_MENU) {
        renderStartMenu();
        m_popupBox->render(m_window);
        m_window.display();
        return;
    }

    m_mainUi->renderBackground(m_window);
    m_boardView->render(m_window, m_engine.getBoard());

    for (const auto& player : m_players) {
        player->render(m_window);
    }

    m_diceRenderer->render(m_window);

    m_mainUi->renderOverlay(m_window);
    m_popupBox->render(m_window);
    if (m_debugDicePopup) {
        m_debugDicePopup->render(m_window);
    }

    m_window.display();
}

void SfmlGuiManager::showLandingPopup(const MovementPayload& movement) {
    m_landingMovementContext.reset();

    if (movement.toIndex >= 0 && movement.toIndex < m_engine.getBoard().size()) {
        const Tile& tile = m_engine.getBoard().getTileByIndex(movement.toIndex);
        if (const auto* cardTile = dynamic_cast<const CardTile*>(&tile)) {
            if (cardTile->getDrawType() == CardDrawType::CHANCE && m_timedCardImageAfterLanding.has_value()) {
                PopupPayload timedPayload;
                timedPayload.mode = PopupMode::FULL_IMAGE_DISMISSABLE;
                timedPayload.cardTemplateImagePath = m_timedCardImageAfterLanding.value();
                m_timedCardImageAfterLanding.reset();
                m_timedCardPopupFromQueue = false;
                m_cardTimer = 0.0f;
                m_currentState = GuiState::SHOWING_TIMED_CARD;
                m_mainUi->setRollVisible(false);
                setUiInputEnabled(false);
                m_popupBox->show(timedPayload, [this](const std::string&) {});
                return;
            }

            if (cardTile->getDrawType() == CardDrawType::COMMUNITY ||
                cardTile->getDrawType() == CardDrawType::CHANCE) {
                processNextPrompt();
                return;
            }
        }
    }

    m_currentState = GuiState::WAITING_CONFIRMATION;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);
    m_landingMovementContext = movement;

    m_popupBox->show(buildLandingPayload(movement), [this](const std::string& actionId) {
        this->handlePopupAction(actionId);
    });
}

void SfmlGuiManager::handlePopupAction(const std::string& actionId) {
    const std::string stableActionId = sanitizeActionId(actionId);
    const std::optional<MovementPayload> landingContext = m_landingMovementContext;

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
        m_endGamePopupShown = false;
        m_landingMovementContext.reset();

        if (stableActionId == "endgame_new_game") {
            m_endGameBannerText.clear();
            m_currentState = GuiState::START_MENU;
            m_mainUi->setRollVisible(false);
            setUiInputEnabled(false);
            openStartMenuPlayerPopup();
            return;
        }

        if (stableActionId == "endgame_load_game") {
            m_endGameBannerText.clear();
            m_currentState = GuiState::START_MENU;
            m_mainUi->setRollVisible(false);
            setUiInputEnabled(false);
            openLoadFilenamePopupFromMenu();
            return;
        }

        if (stableActionId == "endgame_save_game") {
            openSaveFilenamePopup();
            return;
        }

        if (stableActionId == "buy_land") {
            if (!resolvePendingPurchase("y")) {
                m_lastMessage = "Tidak ada prompt pembelian aktif untuk diproses.";
                refreshFromEngineState();
                processNextPrompt();
            }
            return;
        }

        if (stableActionId == "build_property" || stableActionId == "add_house") {
            const MovementPayload* landing = nullptr;
            if (landingContext.has_value()) {
                landing = &landingContext.value();
            }

            Player* actingPlayer = nullptr;
            int landingIndex = -1;
            if (landing != nullptr) {
                landingIndex = landing->toIndex;
                for (Player* candidate : m_engine.getPlayers()) {
                    if (!candidate) {
                        continue;
                    }
                    if (candidate->getUsername() == landing->playerName) {
                        actingPlayer = candidate;
                        break;
                    }
                }
            }

            if (actingPlayer == nullptr) {
                actingPlayer = &m_engine.getCurrentPlayer();
            }
            if (landingIndex < 0) {
                landingIndex = actingPlayer->getPosition();
            }

            Tile& landingTile = m_engine.getBoard().getTileByIndex(landingIndex);
            auto* propertyTile = dynamic_cast<PropertyTile*>(&landingTile);
            if (!propertyTile) {
                throw GameException("BANGUN hanya bisa dilakukan di petak properti.");
            }
            auto* street = dynamic_cast<StreetProperty*>(&propertyTile->getProperty());
            if (!street) {
                throw GameException("BANGUN hanya bisa dilakukan di properti STREET.");
            }

            CommandResult result;
            result.commandName = "BANGUN";
            result.success = m_engine.getPropertyManager().buildOnProperty(
                *actingPlayer, *street);
            m_engine.flushEvents(result);
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

    const Player* movingPlayer = nullptr;
    for (const Player* candidate : m_engine.getPlayers()) {
        if (!candidate) {
            continue;
        }
        if (candidate->getUsername() == movement.playerName) {
            movingPlayer = candidate;
            break;
        }
    }

    if (const auto* propertyTile = dynamic_cast<const PropertyTile*>(&tile)) {
        const Property& property = propertyTile->getProperty();
        payload.mode = PopupMode::PROPERTY;
        payload.headerTitle = "PROPERTI";
        payload.cardTitle = property.getName();
        payload.purchasePrice = property.getPurchasePrice();
        payload.rentPrices = buildDummyRentRows(property);
        payload.description = "Kode: " + property.getCode() + "\nNilai gadai: M" +
                              std::to_string(property.getMortgageValue());

        if (property.getOwner() != nullptr) {
            payload.description += "\nPemilik: " + property.getOwner()->getUsername();
            if (movingPlayer != nullptr && property.getOwner() != movingPlayer) {
                if (!property.isMortgaged()) {
                    const GameContext ctx(
                        m_engine.getPlayers(),
                        &m_engine.getBoard(),
                        m_engine.getDice().getTotal());
                    const int rentAmount = property.calculateRent(ctx);
                    payload.description += "\nAnda membayar biaya sewa sebesar M" +
                        std::to_string(rentAmount);
                } else {
                    payload.description += "\nStatus: MORTGAGED (tidak ada sewa)";
                }
            } else if (movingPlayer != nullptr && property.getOwner() == movingPlayer &&
                       property.getType() != PropertyType::STREET) {
                payload.description += "\nAnda sekarang memiliki properti ini.";
            }
        } else {
            payload.description += "\nStatus: BANK";
        }

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
            const Player* buildActor = movingPlayer != nullptr
                ? movingPlayer
                : &m_engine.getCurrentPlayer();
            const PropertyManager::BuildOption buildOption =
                m_engine.getPropertyManager().getBuildOption(*buildActor, *street);

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

    if (dynamic_cast<const GoToJailTile*>(&tile) != nullptr) {
        payload.mode = PopupMode::SPECIAL;
        payload.headerTitle = "PENJARA";
        payload.description += "\n" + movement.playerName +
                               " masuk penjara. Giliran berakhir.";
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    if (dynamic_cast<const JailTile*>(&tile) != nullptr) {
        payload.mode = PopupMode::SPECIAL;
        payload.headerTitle = "PENJARA";
        if (movingPlayer != nullptr && movingPlayer->isJailed()) {
            payload.description += "\n" + movement.playerName +
                                   " masuk penjara dan ditahan.";
        } else {
            payload.description += "\n" + movement.playerName +
                                   " hanya berkunjung ke penjara.";
        }
        payload.actionItems = {PopupActionItem{"close", "Lanjut", "assets/images/ui/btn_cancel.png", true}};
        return payload;
    }

    payload.actionItems = {PopupActionItem{"close", "Tutup", "assets/images/ui/btn_cancel.png", true}};
    return payload;
}

std::vector<int> SfmlGuiManager::buildDummyRentRows(const Property& property) const {
    if (const auto* street = dynamic_cast<const StreetProperty*>(&property)) {
        return street->getRentLevels();
    }
    if (const auto* railroad = dynamic_cast<const RailroadProperty*>(&property)) {
        std::vector<int> rents;
        rents.reserve(railroad->getRentByCount().size());
        for (const auto& row : railroad->getRentByCount()) {
            rents.push_back(row.second);
        }
        return rents;
    }
    if (const auto* utility = dynamic_cast<const UtilityProperty*>(&property)) {
        std::vector<int> rows;
        rows.reserve(utility->getMultiplierByCount().size());
        for (const auto& row : utility->getMultiplierByCount()) {
            rows.push_back(row.second);
        }
        return rows;
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

void SfmlGuiManager::updateEndGameStateFromResult(const CommandResult& result) {
    for (const GameEvent& event : result.events) {
        const std::string parsedBanner = parseWinnerBannerFromMessage(event);
        if (!parsedBanner.empty()) {
            m_endGameBannerText = parsedBanner;
        }
    }

    if (!m_engine.isGameOver()) {
        m_endGamePopupShown = false;
        m_endGameBannerText.clear();
    }
}

std::string SfmlGuiManager::parseWinnerBannerFromMessage(const GameEvent& event) const {
    if (event.type != GameEventType::GAME_OVER) {
        return "";
    }

    const std::string upperTitle = upperCopy(event.title);
    if (upperTitle.find("PEMENANG") == std::string::npos) {
        return "";
    }

    const std::string upperMessage = upperCopy(event.message);
    if (upperMessage.find("TIDAK ADA PEMENANG") != std::string::npos) {
        return "DRAW";
    }

    if (upperTitle.find("SERI") != std::string::npos) {
        const std::vector<std::string> names = splitWinnerNamesByComma(event.message);
        if (names.empty()) {
            return "DRAW";
        }
        std::ostringstream drawBanner;
        drawBanner << "DRAW (";
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) {
                drawBanner << ",";
            }
            drawBanner << names[i];
        }
        drawBanner << ")";
        return drawBanner.str();
    }

    const size_t menangPos = upperMessage.find("MENANG");
    if (menangPos != std::string::npos) {
        const std::string winnerName = cleanWinnerToken(event.message.substr(0, menangPos));
        if (!winnerName.empty()) {
            return winnerName + " WINS!";
        }
    }

    const std::vector<std::string> names = splitWinnerNamesByComma(event.message);
    if (names.size() > 1) {
        std::ostringstream drawBanner;
        drawBanner << "DRAW (";
        for (size_t i = 0; i < names.size(); ++i) {
            if (i > 0) {
                drawBanner << ",";
            }
            drawBanner << names[i];
        }
        drawBanner << ")";
        return drawBanner.str();
    }
    if (names.size() == 1) {
        return names.front() + " WINS!";
    }

    return "";
}

std::string SfmlGuiManager::buildFallbackEndGameBannerText() const {
    const std::vector<Player*> activePlayers = m_engine.getActivePlayers();
    if (activePlayers.empty()) {
        return "DRAW";
    }
    if (activePlayers.size() == 1 && activePlayers.front() != nullptr) {
        return activePlayers.front()->getUsername() + " WINS!";
    }

    std::ostringstream drawBanner;
    drawBanner << "DRAW (";
    bool first = true;
    for (Player* player : activePlayers) {
        if (player == nullptr) {
            continue;
        }
        if (!first) {
            drawBanner << ",";
        }
        drawBanner << player->getUsername();
        first = false;
    }
    drawBanner << ")";
    return first ? "DRAW" : drawBanner.str();
}

void SfmlGuiManager::showEndGamePopup() {
    PopupPayload payload;
    payload.mode = PopupMode::ENDGAME;
    payload.headerTitle = "GAME OVER";
    payload.cardTitle = m_endGameBannerText.empty() ? buildFallbackEndGameBannerText()
                                                    : m_endGameBannerText;
    payload.description = payload.cardTitle;
    payload.actionItems = {
        PopupActionItem{"endgame_new_game", "NEW GAME", "assets/images/ui/popup_option_normal.png", true},
        PopupActionItem{"endgame_load_game", "LOAD GAME", "assets/images/ui/popup_option_normal.png", true},
        PopupActionItem{"endgame_save_game", "SAVE GAME", "assets/images/ui/popup_option_normal.png", true}};

    m_currentState = GuiState::END_GAME;
    m_mainUi->setRollVisible(false);
    setUiInputEnabled(false);
    m_endGamePopupShown = true;

    m_popupBox->show(payload, [this](const std::string& actionId) {
        handlePopupAction(actionId);
    });
}

void SfmlGuiManager::maybeShowEndGamePopup() {
    if (!m_engine.isGameOver()) {
        m_endGamePopupShown = false;
        return;
    }

    if (m_currentState == GuiState::START_MENU ||
        m_currentState == GuiState::ANIMATING_DICE ||
        m_currentState == GuiState::ANIMATING_PIECE ||
        m_currentState == GuiState::SHOWING_TIMED_CARD) {
        return;
    }

    if (m_popupBox->isVisible() || m_endGamePopupShown) {
        return;
    }

    if (m_endGameBannerText.empty()) {
        m_endGameBannerText = buildFallbackEndGameBannerText();
    }
    showEndGamePopup();
}

void SfmlGuiManager::resumeFlowAfterPopup() {
    m_landingMovementContext.reset();
    refreshFromEngineState();
    if (m_engine.isGameOver()) {
        m_currentState = GuiState::END_GAME;
        m_mainUi->setRollVisible(false);
        setUiInputEnabled(false);
        m_endGamePopupShown = false;
        maybeShowEndGamePopup();
        return;
    }

    m_currentState = GuiState::IDLE;
    m_mainUi->setRollVisible(true);
    setUiInputEnabled(true);
}
} // namespace viewsGUI
