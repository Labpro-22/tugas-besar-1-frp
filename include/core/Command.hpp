#pragma once

#include <string>
#include <vector>

enum class CommandType {
    UNKNOWN,
    HELP,
    EXIT,
    START_GAME,
    ROLL_DICE,
    RESOLVE_SKILL_DROP,
    SET_DICE,
    PRINT_BOARD,
    PRINT_DEED,
    PRINT_PROPERTIES,
    PRINT_LOG,
    MORTGAGE,
    REDEEM,
    BUILD,
    PAY_JAIL_FINE,
    USE_SKILL,
    SAVE,
    LOAD,
    END_TURN
};

struct Command {
    CommandType type = CommandType::UNKNOWN;
    std::string raw;
    std::vector<std::string> args;
};
