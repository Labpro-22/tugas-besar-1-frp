#pragma once

#include "../../include/core/Command.hpp"

#include <string>

class CommandParser {
public:
    Command parse(const std::string& rawInput) const;

private:
    std::string normalizeToken(const std::string& token) const;
    CommandType resolveType(const std::string& token) const;
};
