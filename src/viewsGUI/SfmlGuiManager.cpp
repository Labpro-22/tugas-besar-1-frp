#include "../../include/viewsGUI/SfmlGuiManager.hpp"

#include "../../include/core/CardManager.hpp"
#include "../../include/core/Command.hpp"
#include "../../include/core/CommandResult.hpp"
#include "../../include/core/GameEngine.hpp"
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

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

std::string upperCopy(const std::string& value) {
    std::string upper = value;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return upper;
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
      m_lastMessage("Siap memulai game.") {
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

        Command cmd;
        cmd.type = CommandType::ROLL_DICE;
        cmd.raw = "LEMPAR_DADU";

        const CommandResult result = m_engine.processCommand(cmd);
        const bool hasMovement = result.movement.has_value();
        consumeResult(result, !hasMovement);

        if (result.movement.has_value()) {
            m_deferredMovement = result.movement.value();
        } else {
            m_deferredMovement.reset();
        }

        if (result.prompt.has_value()) {
            m_deferredPrompt = result.prompt.value();
        } else {
            m_deferredPrompt.reset();
        }

        const Dice& dice = m_engine.getDice();
        const sf::Vector2f boardCenter = m_boardView->getBoardCenter();
        m_diceRenderer->startRoll(dice.getDie1(),
                                  dice.getDie2(),
                                  sf::Vector2f(boardCenter.x + Layout1920::kDiceCenterOffsetFromBoardCenter.x,
                                               boardCenter.y + Layout1920::kDiceCenterOffsetFromBoardCenter.y));

        if (hasMovement) {
            const MovementPayload& movement = result.movement.value();
            if (movement.playerIndex >= 0 && movement.playerIndex < static_cast<int>(m_players.size())) {
                std::vector<sf::Vector2f> path;
                path.reserve(movement.path.size());
                for (int index : movement.path) {
                    path.push_back(m_boardView->getTileCenter(index));
                }
                m_players[static_cast<size_t>(movement.playerIndex)]->moveAlongPath(path);
            }
        }

        m_currentState = GuiState::ANIMATING;
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        refreshFromEngineState();
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        m_currentState = GuiState::IDLE;
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
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        handlePromptIfAny(result);
    } catch (const std::exception& e) {
        m_lastMessage = std::string("ERROR: ") + e.what();
        m_currentState = GuiState::IDLE;
        m_mainUi->setRollVisible(true);
        setUiInputEnabled(true);
        refreshFromEngineState();
    }
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

void SfmlGuiManager::handlePromptIfAny(const CommandResult& result) {
    if (!result.prompt.has_value()) {
        resumeFlowAfterPopup();
        return;
    }

    handlePromptRequest(result.prompt.value());
}

void SfmlGuiManager::handlePromptRequest(const PromptRequest& prompt) {
    if (prompt.key == "SKILL_DROP") {
        m_currentState = GuiState::WAITING_CONFIRMATION;
        m_mainUi->setRollVisible(false);
        setUiInputEnabled(false);

        m_popupBox->show(buildSkillDropPayload(prompt), [this](const std::string& actionId) {
            this->handlePopupAction(actionId);
        });
        return;
    }

    resumeFlowAfterPopup();
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

        if (event.type == sf::Event::MouseWheelScrolled) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

            if (!m_popupBox->isVisible() && m_currentState == GuiState::IDLE) {
                m_mainUi->handleMouseWheel(event.mouseWheelScroll.delta, mousePos);
            }
            continue;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

            if (m_popupBox->isVisible()) {
                m_popupBox->handleMousePressed(mousePos);
            } else if (m_currentState == GuiState::IDLE) {
                m_mainUi->handleMousePressed(mousePos);
            }
            continue;
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            const sf::Vector2f mousePos =
                m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

            if (!m_popupBox->isVisible() && m_currentState == GuiState::IDLE) {
                m_mainUi->handleMouseReleased(mousePos);
            } else if (m_popupBox->isVisible()) {
                m_popupBox->handleMouseReleased(mousePos);
            }
            continue;
        }
    }
}

void SfmlGuiManager::update(sf::Time dt) {
    const sf::Vector2f mousePos =
        m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_baseView);

    if (m_popupBox->isVisible()) {
        m_popupBox->update(mousePos);
    } else {
        m_mainUi->update(mousePos);
    }

    bool anyMoving = false;
    for (const auto& player : m_players) {
        player->update(dt);
        if (player->isMoving()) {
            anyMoving = true;
        }
    }

    m_diceRenderer->update(dt);

    if (m_currentState == GuiState::ANIMATING && !anyMoving && !m_diceRenderer->isRolling()) {
        refreshFromEngineState();
        if (m_deferredMovement.has_value()) {
            showLandingPopup(m_deferredMovement.value());
            m_deferredMovement.reset();
        } else if (m_deferredPrompt.has_value()) {
            handlePromptRequest(m_deferredPrompt.value());
            m_deferredPrompt.reset();
        } else {
            resumeFlowAfterPopup();
        }
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
    if (startsWith(actionId, "skill_drop:")) {
        int selected = 0;
        try {
            selected = std::stoi(actionId.substr(std::string("skill_drop:").size()));
        } catch (const std::exception&) {
            m_lastMessage = "ERROR: index pilihan kartu tidak valid.";
            m_popupBox->hide();
            resumeFlowAfterPopup();
            refreshFromEngineState();
            return;
        }

        m_popupBox->hide();
        submitResolveSkillDrop(selected);
        return;
    }

    if (actionId == "buy_land") {
        m_lastMessage = "Aksi 'Beli Lahan' dipilih (dummy UI, belum terhubung ke command engine).";
    } else if (actionId == "add_house") {
        m_lastMessage = "Aksi '+ Rumah' dipilih (dummy UI, belum terhubung ke command engine).";
    } else if (actionId == "cancel" || actionId == "close") {
        m_lastMessage = "Popup ditutup oleh pemain.";
    } else {
        m_lastMessage = "Aksi popup dipilih: " + actionId;
    }

    m_popupBox->hide();
    refreshFromEngineState();

    if (m_deferredPrompt.has_value()) {
        const PromptRequest deferred = m_deferredPrompt.value();
        m_deferredPrompt.reset();
        handlePromptRequest(deferred);
        return;
    }

    resumeFlowAfterPopup();
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

        payload.actionItems = {
            PopupActionItem{"buy_land", "Beli Lahan", "assets/images/ui/btn_beli.png", true},
            PopupActionItem{"add_house", "+ Rumah", "assets/images/ui/btn_rumah.png", true},
            PopupActionItem{"cancel", "Males ah", "assets/images/ui/btn_cancel.png", true}};
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
        payload.description += "\nEfek festival ditangani engine.";
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

PopupPayload SfmlGuiManager::buildSkillDropPayload(const PromptRequest& prompt) const {
    PopupPayload payload;
    payload.mode = PopupMode::INFO;
    payload.headerTitle = "PILIH KARTU DIBUANG";
    payload.cardTitle = "KARTU KEMAMPUAN";
    payload.description = prompt.message;

    for (size_t i = 0; i < prompt.options.size(); ++i) {
        payload.actionItems.push_back(PopupActionItem{
            "skill_drop:" + std::to_string(i),
            std::to_string(i) + ". " + prompt.options[i],
            "assets/images/ui/btn_cancel.png",
            true});
    }

    if (payload.actionItems.empty()) {
        payload.actionItems.push_back(PopupActionItem{"close", "Tutup", "assets/images/ui/btn_cancel.png", true});
    }

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

void SfmlGuiManager::resumeFlowAfterPopup() {
    m_currentState = GuiState::IDLE;
    m_mainUi->setRollVisible(true);
    setUiInputEnabled(true);
}
} // namespace viewsGUI
