#include "../../include/utils/Gamestateserializer.hpp"
#include "../../include/utils/GameException.hpp"
#include <sstream>

namespace {
string trim(const string& s) {
    const size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    const size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool isJailedStatus(const string& status) {
    // Support JAILED and JAILED_<N> (permitted in QnA for save format extension)
    if (status == "JAILED") return true;
    return status.rfind("JAILED_", 0) == 0;
}

bool isBlankLine(const string& line) {
    return trim(line).empty();
}
}

//SavedCardState
SavedCardState::SavedCardState(const string& type, const string& value, const string& duration) : type(type), value(value), duration(duration) {}

string SavedCardState::getType() const { return type; }
string SavedCardState::getValue() const { return value; }
string SavedCardState::getDuration() const { return duration; }
void SavedCardState::setType(const string& t) { type = t; }
void SavedCardState::setValue(const string& v) { value = v; }
void SavedCardState::setDuration(const string& d) { duration = d; }

//SavePlayerState
SavedPlayerState::SavedPlayerState() : money(0) {}
SavedPlayerState::SavedPlayerState(const string& username, int money, const string& positionCode, const string& status) : username(username), money(money), positionCode(positionCode), status(status) {}
string SavedPlayerState::getUsername() const { return username; }
int SavedPlayerState::getMoney() const { return money; }
string SavedPlayerState::getPositionCode() const { return positionCode; }
string SavedPlayerState::getStatus() const { return status; }
const vector<SavedCardState>& SavedPlayerState::getCards() const { return cards; }
void SavedPlayerState::setUsername(const string& u) { username = u; }
void SavedPlayerState::setMoney(int m) { money = m; }
void SavedPlayerState::setPositionCode(const string& c) { positionCode = c; }
void SavedPlayerState::setStatus(const string& s) { status = s; }
void SavedPlayerState::addCard(const SavedCardState& card) { cards.push_back(card); }
void SavedPlayerState::clearCards() { cards.clear(); }

// SavedPropertyState
SavedPropertyState::SavedPropertyState() : festivalMult(1), festivalDur(0) {}

SavedPropertyState::SavedPropertyState(const string& code, const string& type, const string& owner, const string& status, int festivalMult, int festivalDur, const string& buildings) : code(code), type(type), owner(owner), status(status), festivalMult(festivalMult), festivalDur(festivalDur), buildings(buildings) {}

string SavedPropertyState::getCode() const { return code; }
string SavedPropertyState::getType() const { return type; }
string SavedPropertyState::getOwner() const { return owner; }
string SavedPropertyState::getStatus() const { return status; }
int SavedPropertyState::getFestivalMult() const { return festivalMult; }
int SavedPropertyState::getFestivalDur() const { return festivalDur; }
string SavedPropertyState::getBuildings() const { return buildings; }

void SavedPropertyState::setCode(const string& c) { code = c; }
void SavedPropertyState::setType(const string& t) { type = t; }
void SavedPropertyState::setOwner(const string& o) { owner = o; }
void SavedPropertyState::setStatus(const string& s) { status = s; }
void SavedPropertyState::setFestivalMult(int m) { festivalMult = m; }
void SavedPropertyState::setFestivalDur(int d) { festivalDur = d; }
void SavedPropertyState::setBuildings(const string& b) { buildings = b; }

// SavedLogEntry
SavedLogEntry::SavedLogEntry() : turn(0) {}

SavedLogEntry::SavedLogEntry(int turn, const string& username, const string& actionType, const string& detail)
: turn(turn), username(username), actionType(actionType), detail(detail) {}

int SavedLogEntry::getTurn() const { return turn; }
string SavedLogEntry::getUsername() const { return username; }
string SavedLogEntry::getActionType() const { return actionType; }
string SavedLogEntry::getDetail() const { return detail; }

void SavedLogEntry::setTurn(int t) { turn = t; }
void SavedLogEntry::setUsername(const string& u) { username = u; }
void SavedLogEntry::setActionType(const string& a) { actionType = a; }
void SavedLogEntry::setDetail(const string& d) { detail = d; }

// SavedDeckState
const vector<string>& SavedDeckState::getCardTypes() const { return cardTypes; }
void SavedDeckState::addCardType(const string& type) { cardTypes.push_back(type); }
void SavedDeckState::clearCardTypes() { cardTypes.clear(); }

// GameSnapshot
GameSnapshot::GameSnapshot() : currentTurn(0), maxTurn(0), numPlayers(0) {}

GameSnapshot::GameSnapshot(int currentTurn, int maxTurn, int numPlayers) : currentTurn(currentTurn), maxTurn(maxTurn), numPlayers(numPlayers) {}

int GameSnapshot::getCurrentTurn() const { return currentTurn; }
int GameSnapshot::getMaxTurn() const { return maxTurn; }
int GameSnapshot::getNumPlayers() const { return numPlayers; }
string GameSnapshot::getActivePlayer() const { return activePlayer; }

const vector<SavedPlayerState>& GameSnapshot::getPlayers() const { return players; }
const vector<string>& GameSnapshot::getTurnOrder() const { return turnOrder; }
const vector<SavedPropertyState>& GameSnapshot::getProperties() const { return properties; }
const SavedDeckState& GameSnapshot::getDeck() const { return deck; }
const vector<SavedLogEntry>& GameSnapshot::getLog() const { return log; }

void GameSnapshot::setCurrentTurn(int t) { currentTurn = t; }
void GameSnapshot::setMaxTurn(int t) { maxTurn = t; }
void GameSnapshot::setNumPlayers(int n) { numPlayers = n; }
void GameSnapshot::setActivePlayer(const string& u) { activePlayer = u; }
void GameSnapshot::setDeck(const SavedDeckState& d) { deck = d; }
void GameSnapshot::addPlayer(const SavedPlayerState& p) { players.push_back(p); }
void GameSnapshot::addTurnOrder(const string& username) { turnOrder.push_back(username); }
void GameSnapshot::addProperty(const SavedPropertyState& p) { properties.push_back(p); }
void GameSnapshot::addLogEntry(const SavedLogEntry& entry) { log.push_back(entry); }

/*
Gamestateserializer — SERIALIZE

Format output (sesuai spesifikasi):

<TURN> <MAX_TURN>
<JUMLAH_PEMAIN>
<USERNAME> <UANG> <POSISI> <STATUS>      <- satu baris per pemain
<JUMLAH_KARTU>
<JENIS> [<NILAI>] [<DURASI>]             <- satu baris per kartu
...
<URUTAN_GILIRAN (semua dalam satu baris)>
<GILIRAN_AKTIF>
<JUMLAH_PROPERTI>
<KODE> <JENIS> <PEMILIK> <STATUS> <FMULT> <FDUR> <N_BANGUNAN>
...
<JUMLAH_KARTU_DECK>
<JENIS_KARTU>
...
<JUMLAH_ENTRI_LOG>
<TURN> <USERNAME> <JENIS_AKSI> <DETAIL>
...
*/
string Gamestateserializer::serialize(const GameSnapshot& snapshot) const {
    ostringstream out;

    out << serializeHeader(snapshot.getCurrentTurn(),
                           snapshot.getMaxTurn(),
                           snapshot.getNumPlayers())
        << "\n";

    for (size_t i = 0; i < snapshot.getPlayers().size(); ++i) {
        out << serializePlayer(snapshot.getPlayers()[i]);
        out << "\n";
    }

    out << serializeTurnOrder(snapshot.getTurnOrder(),
                              snapshot.getActivePlayer())
        << "\n";

    out << serializeProperties(snapshot.getProperties()) << "\n";
    out << serializeDeck(snapshot.getDeck()) << "\n";
    out << serializeLog(snapshot.getLog());

    return out.str();
}

string Gamestateserializer::serializeHeader(int currentTurn, int maxTurn, int numPlayers) const { 
    return to_string(currentTurn) + " " + to_string(maxTurn) + "\n" + to_string(numPlayers); 
}

string Gamestateserializer::serializeCard(const SavedCardState& card) const {
    string line = card.getType();
    if (!card.getValue().empty()) {
        line += " " + card.getValue();
        if (!card.getDuration().empty()) {
            line += " " + card.getDuration();
        }
    }
    return line;
}

string Gamestateserializer::serializePlayer(const SavedPlayerState& p) const {
    ostringstream out;
    out << p.getUsername()     << " "
        << p.getMoney()        << " "
        << p.getPositionCode() << " "
        << p.getStatus()       << "\n"
        << p.getCards().size();
    for (const SavedCardState& card : p.getCards()) {
        out << "\n" << serializeCard(card);
    }
    return out.str();
}

string Gamestateserializer::serializeTurnOrder(const vector<string>& order, const string& activePlayer) const {
    ostringstream out;
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) out << " ";
        out << order[i];
    }
    out << "\n" << activePlayer;
    return out.str();
}

string Gamestateserializer::serializeProperties(
        const vector<SavedPropertyState>& props) const {
    ostringstream out;
    out << props.size();
    for (const SavedPropertyState& prop : props) {
        out << "\n"
            << prop.getCode()         << " "
            << prop.getType()         << " "
            << prop.getOwner()        << " "
            << prop.getStatus()       << " "
            << prop.getFestivalMult() << " "
            << prop.getFestivalDur()  << " "
            << prop.getBuildings();
    }
    return out.str();
}

string Gamestateserializer::serializeDeck(const SavedDeckState& deck) const {
    ostringstream out;
    out << deck.getCardTypes().size();
    for (const string& type : deck.getCardTypes()) {
        out << "\n" << type;
    }
    return out.str();
}

string Gamestateserializer::serializeLog(
        const vector<SavedLogEntry>& log) const {
    ostringstream out;
    out << log.size();
    for (const SavedLogEntry& entry : log) {
        out << "\n"
            << entry.getTurn()       << " "
            << entry.getUsername()   << " "
            << entry.getActionType() << " "
            << entry.getDetail();
    }
    return out.str();
}

// Gamestateserializer — DESERIALIZE

GameSnapshot Gamestateserializer::deserialize(const string& content) const {
    istringstream ss(content);
    GameSnapshot snap;

    int currentTurn = 0, maxTurn = 0;
    ss >> currentTurn >> maxTurn;
    if (ss.fail()) {
        throw SaveLoadException("Format baris 1 tidak valid (<TURN> <MAX_TURN>)");
    }
    snap.setCurrentTurn(currentTurn);
    snap.setMaxTurn(maxTurn);

    int numPlayers = 0;
    ss >> numPlayers;
    if (ss.fail() || numPlayers < 2 || numPlayers > 4) {
        throw SaveLoadException("Invalid number of players: " +
                                to_string(numPlayers) + " (must be 2-4)");
    }
    snap.setNumPlayers(numPlayers);
    ss.ignore();

    for (int i = 0; i < numPlayers; ++i) {
        snap.addPlayer(parsePlayer(ss));
    }

    string orderLine;
    getline(ss, orderLine);
    while (isBlankLine(orderLine) && ss.good()) {
        getline(ss, orderLine);
    }
    istringstream orderStream(orderLine);
    string name;
    while (orderStream >> name) {
        snap.addTurnOrder(name);
    }
    if (snap.getTurnOrder().size() != static_cast<size_t>(numPlayers)) {
        throw SaveLoadException("Turn order count mismatch with number of players");
    }

    string activeLine;
    getline(ss, activeLine);
    while (isBlankLine(activeLine) && ss.good()) {
        getline(ss, activeLine);
    }
    const string activePlayer = trim(activeLine);
    if (activePlayer.empty()) {
        throw SaveLoadException("Active turn not found");
    }
    snap.setActivePlayer(activePlayer);

    bool activeInOrder = false;
    for (const string& orderedName : snap.getTurnOrder()) {
        if (orderedName == activePlayer) {
            activeInOrder = true;
            break;
        }
    }
    if (!activeInOrder) {
        throw SaveLoadException("Active player is not in turn order");
    }

    int numProperties = 0;
    ss >> numProperties;
    if (ss.fail() || numProperties < 0) {
        throw SaveLoadException("Invalid number of properties");
    }
    ss.ignore();
    for (int i = 0; i < numProperties; ++i) {
        snap.addProperty(parseProperty(ss));
    }

    snap.setDeck(parseDeck(ss));

    int numEntries = 0;
    ss >> numEntries;
    if (ss.fail() || numEntries < 0) {
        throw SaveLoadException("Invalid number of log entries");
    }
    ss.ignore();
    for (int i = 0; i < numEntries; ++i) {
        snap.addLogEntry(parseLogEntry(ss));
    }

    return snap;
}

SavedPlayerState Gamestateserializer::parsePlayer(istringstream& ss) const {
    SavedPlayerState p;

    string playerLine;
    getline(ss, playerLine);
    while (isBlankLine(playerLine) && ss.good()) {
        getline(ss, playerLine);
    }
    istringstream ls(playerLine);

    string username, positionCode, status;
    int money = 0;
    ls >> username >> money >> positionCode >> status;
    if (ls.fail()) {
        throw SaveLoadException("Invalid player row: " + playerLine);
    }
    if (status != "ACTIVE" && status != "BANKRUPT" && !isJailedStatus(status)) {
        throw SaveLoadException("Invalid player status: " + status);
    }
    p.setUsername(username);
    p.setMoney(money);
    p.setPositionCode(positionCode);
    p.setStatus(status);

    int numCards = 0;
    ss >> numCards;
    if (ss.fail() || numCards < 0 || numCards > 3) {
        throw SaveLoadException("Invalid number of cards for player " + username);
    }
    ss.ignore();

    for (int i = 0; i < numCards; ++i) {
        string cardLine;
        getline(ss, cardLine);
        while (isBlankLine(cardLine) && ss.good()) {
            getline(ss, cardLine);
        }
        istringstream cs(cardLine);

        SavedCardState card;
        string type, value, duration;
        cs >> type;
        if (type.empty()) {
            throw SaveLoadException("Card type not found for player " + username);
        }
        card.setType(type);

        if (cs >> value)    card.setValue(value);
        if (cs >> duration) card.setDuration(duration);

        p.addCard(card);
    }

    return p;
}

SavedPropertyState Gamestateserializer::parseProperty(istringstream& ss) const {
    string propLine;
    getline(ss, propLine);
    while (isBlankLine(propLine) && ss.good()) {
        getline(ss, propLine);
    }
    istringstream ls(propLine);

    string code, type, owner, status, buildings;
    int festivalMult = 0, festivalDur = 0;
    ls >> code >> type >> owner >> status >> festivalMult >> festivalDur >> buildings;
    if (ls.fail()) {
        throw SaveLoadException("Invalid property state format: " + propLine);
    }
    if (type != "street" && type != "railroad" && type != "utility") {
        throw SaveLoadException("Invalid property type: " + type);
    }
    if (status != "BANK" && status != "OWNED" && status != "MORTGAGED") {
        throw SaveLoadException("Invalid property status for " + code + ": " + status);
    }
    // Accept 0 for backward compatibility; serializer should emit 1 when inactive.
    if (festivalMult != 0 && festivalMult != 1 && festivalMult != 2 &&
        festivalMult != 4 && festivalMult != 8) {
        throw SaveLoadException("Invalid FMULT for property " + code);
    }
    if (festivalDur < 0 || festivalDur > 3) {
        throw SaveLoadException("Invalid FDUR for property " + code);
    }
    if (buildings != "0" && buildings != "1" && buildings != "2" &&
        buildings != "3" && buildings != "4" && buildings != "H") {
        throw SaveLoadException("Invalid N_BANGUNAN for property " + code);
    }

    if (type != "street" && buildings != "0") {
        throw SaveLoadException("Non-street property must have N_BANGUNAN=0 for " + code);
    }

    return SavedPropertyState(code, type, owner, status,
                              festivalMult == 0 ? 1 : festivalMult,
                              festivalDur, buildings);
}

SavedDeckState Gamestateserializer::parseDeck(istringstream& ss) const {
    SavedDeckState deck;
    int numCards = 0;
    ss >> numCards;
    if (ss.fail() || numCards < 0) {
        throw SaveLoadException("Invalid number of deck cards");
    }
    ss.ignore();
    for (int i = 0; i < numCards; ++i) {
        string line;
        getline(ss, line);
        size_t s = line.find_first_not_of(" \t\r\n");
        size_t e = line.find_last_not_of(" \t\r\n");
        if (s == string::npos) {
            throw SaveLoadException("Card type not found for deck entry " +
                                    to_string(i + 1));
        }
        deck.addCardType(line.substr(s, e - s + 1));
    }
    return deck;
}

SavedLogEntry Gamestateserializer::parseLogEntry(istringstream& ss) const {
    string line;
    if (!getline(ss, line)) {
        throw SaveLoadException("Incomplete log row");
    }
    while (isBlankLine(line) && ss.good()) {
        if (!getline(ss, line)) {
            throw SaveLoadException("Incomplete log row");
        }
    }
    istringstream ls(line);
    int turn = 0;
    string username, actionType;
    ls >> turn >> username >> actionType;
    if (ls.fail()) {
        throw SaveLoadException("Invalid log entry format: " + line);
    }
    string detail;
    getline(ls, detail);
    if (!detail.empty() && detail.front() == ' ') detail.erase(0, 1);

    return SavedLogEntry(turn, username, actionType, detail);
}

string Gamestateserializer::nextToken(istringstream& ss, const string& fieldName) const {
    string token;
    if (!(ss >> token)) {
        throw SaveLoadException("Field '" + fieldName +
                                "' not found in save file");
    }
    return token;
}