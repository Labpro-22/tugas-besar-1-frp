#pragma once

#include "../../include/core/CommandResult.hpp"

#include <string>

class UiFormatter {
public:
    std::string format(const CommandResult& result) const;

private:
    std::string formatTone(UiTone tone, const std::string& text) const;
};
