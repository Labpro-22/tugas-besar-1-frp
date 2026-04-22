#include "../../include/views/CommandParser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

std::string CommandParser::normalizeToken(const std::string& token) const {
    std::string normalized = token;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return normalized;
}

CommandType CommandParser::resolveType(const std::string& token) const {
    if (token == "HELP" || token == "BANTUAN") return CommandType::HELP;
    if (token == "KELUAR" || token == "EXIT") return CommandType::EXIT;
    if (token == "MULAI") return CommandType::START_GAME;
    if (token == "LEMPAR_DADU") return CommandType::ROLL_DICE;
    if (token == "ATUR_DADU") return CommandType::SET_DICE;
    if (token == "CETAK_PAPAN") return CommandType::PRINT_BOARD;
    if (token == "CETAK_AKTA") return CommandType::PRINT_DEED;
    if (token == "CETAK_PROPERTI") return CommandType::PRINT_PROPERTIES;
    if (token == "CETAK_LOG") return CommandType::PRINT_LOG;
    if (token == "GADAI") return CommandType::MORTGAGE;
    if (token == "TEBUS") return CommandType::REDEEM;
    if (token == "BANGUN") return CommandType::BUILD;
    if (token == "BAYAR_DENDA" || token == "PAY_JAIL_FINE") return CommandType::PAY_JAIL_FINE;
    if (token == "GUNAKAN_KEMAMPUAN" || token == "GUNAKAN_KARTU" || token == "KARTU") return CommandType::USE_SKILL;
    if (token == "SIMPAN") return CommandType::SAVE;
    if (token == "MUAT") return CommandType::LOAD;
    if (token == "AKHIRI_GILIRAN" || token == "END_TURN") return CommandType::END_TURN;
    return CommandType::UNKNOWN;
}

Command CommandParser::parse(const std::string& rawInput) const {
    std::istringstream iss(rawInput);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    Command cmd;
    cmd.raw = rawInput;

    if (tokens.empty()) {
        cmd.type = CommandType::UNKNOWN;
        return cmd;
    }

    const std::string head = normalizeToken(tokens.front());
    cmd.type = resolveType(head);

    for (size_t i = 1; i < tokens.size(); ++i) {
        cmd.args.push_back(tokens[i]);
    }

    return cmd;
}
