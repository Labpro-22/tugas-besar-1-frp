#pragma once

#include <string>
#include <sstream>
#include <vector>
using namespace std;

class GameEngine;
class Player;
class Property;
class LogEntry;



// Merepresentasikan satu kartu kemampuan yang tersimpan dalam file save.
// Format: <JENIS> <NILAI> <DURASI>
class SavedCardState {
private:
    string type;
    string value;
    string duration;

public:
    SavedCardState() = default;
    SavedCardState(const string& type,
                   const string& value,
                   const string& duration);

    string getType()     const;
    string getValue()    const;
    string getDuration() const;

    void setType(const string& t);
    void setValue(const string& v);
    void setDuration(const string& d);
};


// Merepresentasikan state lengkap satu pemain yang tersimpan dalam file save.
// Format: <USERNAME> <UANG> <POSISI_PETAK> <STATUS> <JUMLAH_KARTU> [kartu...]

class SavedPlayerState {
private:
    string                  username;
    int                          money;
    string                  positionCode;
    string                  status;
    vector<SavedCardState>  cards;

public:
    SavedPlayerState();
    SavedPlayerState(const string& username, int money,
                     const string& positionCode,
                     const string& status);

    string                        getUsername()     const;
    int                                getMoney()        const;
    string                        getPositionCode() const;
    string                        getStatus()       const;
    const vector<SavedCardState>& getCards()        const;

    void setUsername(const string& u);
    void setMoney(int m);
    void setPositionCode(const string& code);
    void setStatus(const string& s);
    void addCard(const SavedCardState& card);
    void clearCards();
};


// Merepresentasikan state satu properti yang tersimpan dalam file save.
// Format: <KODE> <JENIS> <PEMILIK> <STATUS> <FMULT> <FDUR> <N_BANGUNAN>

class SavedPropertyState {
private:
    string code;
    string type;
    string owner;
    string status;
    int         festivalMult;
    int         festivalDur;
    string buildings;

public:
    SavedPropertyState();
    SavedPropertyState(const string& code, const string& type,
                       const string& owner, const string& status,
                       int festivalMult, int festivalDur,
                       const string& buildings);

    string getCode()         const;
    string getType()         const;
    string getOwner()        const;
    string getStatus()       const;
    int         getFestivalMult() const;
    int         getFestivalDur()  const;
    string getBuildings()    const;

    void setCode(const string& c);
    void setType(const string& t);
    void setOwner(const string& o);
    void setStatus(const string& s);
    void setFestivalMult(int m);
    void setFestivalDur(int d);
    void setBuildings(const string& b);
};


// Merepresentasikan satu entri log transaksi dalam file save.
// Format: <TURN> <USERNAME> <JENIS_AKSI> <DETAIL>

class SavedLogEntry {
private:
    int         turn;
    string username;
    string actionType;
    string detail;

public:
    SavedLogEntry();
    SavedLogEntry(int turn, const string& username,
                  const string& actionType, const string& detail);

    int         getTurn()       const;
    string getUsername()   const;
    string getActionType() const;
    string getDetail()     const;

    void setTurn(int t);
    void setUsername(const string& u);
    void setActionType(const string& a);
    void setDetail(const string& d);
};


// SavedDeckState
// Merepresentasikan urutan kartu kemampuan yang tersisa di deck.
// Format: <JUMLAH> <JENIS_KARTU_1> ...

class SavedDeckState {
private:
    vector<string> cardTypes;

public:
    SavedDeckState() = default;

    const vector<string>& getCardTypes() const;
    void addCardType(const string& type);
    void clearCardTypes();
};


// Menampung seluruh state permainan hasil deserialisasi dari file save.
// Digunakan oleh SaveLoadManager untuk meneruskan data ke GameEngine.

class GameSnapshot {
private:
    int                              currentTurn;
    int                              maxTurn;
    int                              numPlayers;
    vector<SavedPlayerState>    players;
    vector<string>         turnOrder;
    string                      activePlayer;
    vector<SavedPropertyState>  properties;
    SavedDeckState                   deck;
    vector<SavedLogEntry>       log;

public:
    GameSnapshot();
    GameSnapshot(int currentTurn, int maxTurn, int numPlayers);

    int                                    getCurrentTurn()  const;
    int                                    getMaxTurn()      const;
    int                                    getNumPlayers()   const;
    const vector<SavedPlayerState>&   getPlayers()      const;
    const vector<string>&        getTurnOrder()    const;
    string                            getActivePlayer() const;
    const vector<SavedPropertyState>& getProperties()   const;
    const SavedDeckState&                  getDeck()         const;
    const vector<SavedLogEntry>&      getLog()          const;

    void setCurrentTurn(int t);
    void setMaxTurn(int t);
    void setNumPlayers(int n);
    void addPlayer(const SavedPlayerState& p);
    void addTurnOrder(const string& username);
    void setActivePlayer(const string& username);
    void addProperty(const SavedPropertyState& prop);
    void setDeck(const SavedDeckState& d);
    void addLogEntry(const SavedLogEntry& entry);
};


// Bertanggung jawab mengubah state game menjadi teks (serialize)
// dan sebaliknya (deserialize). Tidak membuka/menutup file.

class GameStateSerializer {
public:
    string  serialize(const GameSnapshot& snapshot) const;
    string  serializeHeader(int currentTurn, int maxTurn,
                                 int numPlayers) const;
    string  serializePlayer(const SavedPlayerState& p) const;
    string  serializeTurnOrder(const vector<string>& order,
                                    const string& activePlayer) const;
    string  serializeProperties(
                     const vector<SavedPropertyState>& props) const;
    string  serializeDeck(const SavedDeckState& deck) const;
    string  serializeLog(const vector<SavedLogEntry>& log) const;
    GameSnapshot deserialize(const string& content) const;

private:
    string        serializeCard(const SavedCardState& card) const;
    SavedPlayerState   parsePlayer(istringstream& ss) const;
    SavedPropertyState parseProperty(istringstream& ss) const;
    SavedLogEntry      parseLogEntry(istringstream& ss) const;
    SavedDeckState     parseDeck(istringstream& ss) const;
    string        nextToken(istringstream& ss,
                                 const string& fieldName) const;
};