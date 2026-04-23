#pragma once

#include <optional>
#include <string>
#include <vector>

enum class UiTone {
    INFO,
    SUCCESS,
    WARNING,
    ERROR
};

enum class GameEventType {
    SYSTEM,
    TURN,
    DICE,
    MOVEMENT,
    LANDING,
    PROPERTY,
    MONEY,
    TAX,
    CARD,
    AUCTION,
    BANKRUPTCY,
    SAVE_LOAD,
    LOG,
    GAME_OVER
};

struct GameEvent {
    GameEventType type = GameEventType::SYSTEM;
    UiTone tone = UiTone::INFO;
    std::string title;
    std::string message;
    std::string eventPayload;
};

struct PromptOption {
    std::string key;
    std::string label;
};

struct PromptRequest {
    std::string id;
    std::string title;
    std::string message;
    std::vector<PromptOption> options;
    bool required = true;
};

struct MovementPayload {
    int playerIndex = -1;
    std::string playerName;
    int fromIndex = -1;
    int toIndex = -1;
    std::vector<int> path;
};

struct CommandResult {
    bool success = true;
    std::string commandName;
    std::vector<GameEvent> events;
    std::vector<PromptRequest> prompts;
    std::optional<MovementPayload> movement;

    void addEvent(GameEventType type,
                  UiTone tone,
                  const std::string& title,
                  const std::string& message,
                  const std::string& eventPayload = "") {
        events.push_back(GameEvent{type, tone, title, message, eventPayload});
    }

    void append(const CommandResult& other) {
        events.insert(events.end(), other.events.begin(), other.events.end());
        prompts.insert(prompts.end(), other.prompts.begin(), other.prompts.end());
        if (!other.success) {
            success = false;
        }
        if (other.movement.has_value()) {
            movement = other.movement;
        }
    }
};
