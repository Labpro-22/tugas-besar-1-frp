#pragma once

#include <string>
#include <vector>

enum class CommandType {
    UNKNOWN,
    HELP,
    EXIT,
    START_GAME,
    ROLL_DICE,
    SET_DICE,
    PRINT_BOARD,
    PRINT_LOG,
    SAVE,
    LOAD,
    END_TURN
};

struct Command {
    CommandType type = CommandType::UNKNOWN;
    std::string raw;
    std::vector<std::string> args;
};
