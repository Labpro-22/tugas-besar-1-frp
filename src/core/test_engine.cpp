#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../include/core/TurnManager.hpp"
#include "../../include/models/Dice.hpp"
#include "../../include/utils/ConfigLoader.hpp"
#include "../../include/utils/GameException.hpp"
#include "../../include/utils/Gamestateserializer.hpp"

using std::cout;
using std::map;
using std::string;
using std::vector;

void run_leaderboard_tests();

namespace {

struct PrimitivePlayer {
    string username;
    int money;
    int positionIndex;
    string status;
};

struct PrimitiveHarness {
    bool initialized = false;
    int maxTurn = 15;
    int initialBalance = 1000;
    int goSalary = 200;
    int jailFine = 50;
    int pphFlat = 150;
    int pphPercentage = 10;
    int pbmFlat = 200;

    TurnManager turnManager;
    Dice dice;

    map<int, int> railroadRentByCount;
    map<int, int> utilityMultiplierByCount;

    vector<PrimitivePlayer> players;
    vector<string> boardCodes;
    vector<SavedPropertyState> properties;
    SavedDeckState deck;
    vector<SavedLogEntry> log;
};

string trim(const string& s) {
    const size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    const size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string upper(const string& s) {
    string out = s;
    for (char& ch : out) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return out;
}

vector<string> split(const string& s) {
    std::istringstream ss(s);
    vector<string> out;
    string token;
    while (ss >> token) out.push_back(token);
    return out;
}

bool isBankruptStatus(const string& s) {
    return upper(s) == "BANKRUPT";
}

vector<bool> buildBankruptFlags(const PrimitiveHarness& h) {
    vector<bool> flags;
    flags.reserve(h.players.size());
    for (const PrimitivePlayer& p : h.players) {
        flags.push_back(isBankruptStatus(p.status));
    }
    return flags;
}

void initBoardCodes(PrimitiveHarness& h) {
    h.boardCodes = {
        "GO", "GRT", "DNU", "TSK", "PPH", "GBR", "BGR", "FES", "DPK", "BKS",
        "PEN", "MGL", "PLN", "SOL", "YOG", "STB", "MAL", "DNU", "SMG", "SBY",
        "BBP", "MKS", "KSP", "BLP", "MND", "TUG", "PLB", "PKB", "PAM", "MED",
        "PPJ", "BDG", "DEN", "FES", "MTR", "GUB", "KSP", "JKT", "PBM", "IKN"
    };
}

void loadPropertyDefaults(PrimitiveHarness& h) {
    ConfigLoader loader;
    const vector<PropertyDef> defs = loader.loadProperties("config/property.txt");

    h.properties.clear();
    h.properties.reserve(defs.size());

    for (const PropertyDef& def : defs) {
        string type = "street";
        if (def.getTypeName() == "RAILROAD") type = "railroad";
        if (def.getTypeName() == "UTILITY") type = "utility";

        h.properties.push_back(SavedPropertyState(
            def.getCode(), type, "BANK", "BANK", 1, 0, "0"));
    }
}

void loadAllConfigs(PrimitiveHarness& h) {
    ConfigLoader loader;

    const MiscConfig misc = loader.loadMiscConfig("config/misc.txt");
    const SpecialConfig special = loader.loadSpecialConfig("config/special.txt");
    const TaxConfig tax = loader.loadTaxConfig("config/tax.txt");

    h.maxTurn = misc.getMaxTurn();
    h.initialBalance = misc.getInitialBalance();
    h.goSalary = special.getGoSalary();
    h.jailFine = special.getJailFine();
    h.pphFlat = tax.getPphFlat();
    h.pphPercentage = tax.getPphPercentage();
    h.pbmFlat = tax.getPbmFlat();

    h.railroadRentByCount = loader.loadRailroadConfig("config/railroad.txt");
    h.utilityMultiplierByCount = loader.loadUtilityConfig("config/utility.txt");
    loadPropertyDefaults(h);
}

void printConfigSummary(const PrimitiveHarness& h) {
    cout << "\n=== CONFIG SUMMARY ===\n";
    cout << "MAX_TURN      : " << h.maxTurn << "\n";
    cout << "SALDO_AWAL    : " << h.initialBalance << "\n";
    cout << "GO_SALARY     : " << h.goSalary << "\n";
    cout << "JAIL_FINE     : " << h.jailFine << "\n";
    cout << "PPH_FLAT      : " << h.pphFlat << "\n";
    cout << "PPH_%         : " << h.pphPercentage << "\n";
    cout << "PBM_FLAT      : " << h.pbmFlat << "\n";
    cout << "PROPERTY_DEF  : " << h.properties.size() << "\n";
    cout << "RAILROAD_DEF  : " << h.railroadRentByCount.size() << "\n";
    cout << "UTILITY_DEF   : " << h.utilityMultiplierByCount.size() << "\n";
    cout << "======================\n\n";
}

void printHelp() {
    cout << "\n=== COMMAND LIST ===\n";
    cout << "HELP                                 : tampilkan bantuan\n";
    cout << "INIT                                 : input jumlah pemain + nama, urutan random\n";
    cout << "STATUS                               : tampilkan turn, pemain aktif, data semua pemain\n";
    cout << "ORDER                                : tampilkan urutan giliran saat ini\n";
    cout << "ROLL                                 : lempar dadu random dan gerakkan pemain aktif\n";
    cout << "ATUR_DADU X Y                        : atur dadu manual lalu gerakkan pemain aktif\n";
    cout << "NEXT                                 : pindah ke pemain berikutnya\n";
    cout << "SET_STATUS <USERNAME> <STATUS>       : ubah status pemain (ACTIVE/JAILED/BANKRUPT)\n";
    cout << "SET_MONEY <USERNAME> <NOMINAL>       : ubah uang pemain\n";
    cout << "SET_POS <USERNAME> <KODE_PETAK>      : ubah posisi pemain ke kode petak\n";
    cout << "CONFIG                               : tampilkan ringkasan config aktif\n";
    cout << "RELOAD_CONFIG                        : reload semua file config/*.txt\n";
    cout << "SAVE [NAMA_FILE.txt]                 : simpan snapshot (default: file_save.txt)\n";
    cout << "LOAD [NAMA_FILE.txt]                 : muat snapshot (default: file_save.txt)\n";
    cout << "SHOW_SAVE [NAMA_FILE.txt]            : tampilkan isi file save\n";
    cout << "EXIT                                 : keluar\n\n";
}

int findPlayerIndex(const PrimitiveHarness& h, const string& username) {
    for (size_t i = 0; i < h.players.size(); ++i) {
        if (h.players[i].username == username) return static_cast<int>(i);
    }
    return -1;
}

int findBoardIndexByCode(const PrimitiveHarness& h, const string& code) {
    for (size_t i = 0; i < h.boardCodes.size(); ++i) {
        if (h.boardCodes[i] == code) return static_cast<int>(i);
    }
    return -1;
}

string activePlayerName(const PrimitiveHarness& h) {
    const int idx = h.turnManager.getCurrentPlayerIndex();
    if (idx < 0 || idx >= static_cast<int>(h.players.size())) {
        throw GameException("Index pemain aktif tidak valid");
    }
    return h.players[idx].username;
}

void initializeHarness(PrimitiveHarness& h) {
    cout << "Jumlah pemain (2-4): ";
    int n = 0;
    std::cin >> n;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (n < 2 || n > 4) {
        throw GameException("Jumlah pemain harus 2 sampai 4");
    }

    h.players.clear();
    h.players.reserve(static_cast<size_t>(n));

    for (int i = 0; i < n; ++i) {
        cout << "Nama pemain " << (i + 1) << ": ";
        string name;
        std::getline(std::cin, name);
        name = trim(name);
        if (name.empty()) {
            throw GameException("Nama pemain tidak boleh kosong");
        }

        PrimitivePlayer p;
        p.username = name;
        p.money = h.initialBalance;
        p.positionIndex = 0;
        p.status = "ACTIVE";
        h.players.push_back(p);
    }

    h.turnManager.initializeOrder(n);
    h.turnManager.resetExtraTurn();
    h.log.clear();
    h.deck.clearCardTypes();
    h.deck.addCardType("MoveCard");
    h.deck.addCardType("DiscountCard");
    h.deck.addCardType("ShieldCard");

    loadPropertyDefaults(h);
    h.initialized = true;

    cout << "[OK] Game harness diinisialisasi. Pemain aktif awal: "
         << activePlayerName(h) << "\n";
}

void printStatus(const PrimitiveHarness& h) {
    if (!h.initialized) {
        cout << "[WARN] Jalankan INIT dulu.\n";
        return;
    }

    cout << "\n=== STATUS ===\n";
    cout << "Turn sekarang : " << h.turnManager.getTurnNumber() << "/" << h.maxTurn << "\n";
    cout << "Pemain aktif  : " << activePlayerName(h) << "\n";
    cout << "Last dice     : " << h.dice.getDie1() << "+" << h.dice.getDie2()
         << "=" << h.dice.getTotal() << "\n";

    for (const PrimitivePlayer& p : h.players) {
        const string code = h.boardCodes[p.positionIndex % static_cast<int>(h.boardCodes.size())];
        cout << "- " << p.username
             << " | Uang: " << p.money
             << " | Pos: " << code
             << " | Status: " << p.status << "\n";
    }
    cout << "\n";
}

void printOrder(const PrimitiveHarness& h) {
    if (!h.initialized) {
        cout << "[WARN] Jalankan INIT dulu.\n";
        return;
    }

    const vector<int>& order = h.turnManager.getTurnOrder();
    cout << "Urutan giliran: ";
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) cout << " -> ";
        cout << h.players[order[i]].username;
    }
    cout << "\n";
}

void moveActivePlayerBy(PrimitiveHarness& h, int steps, int d1, int d2) {
    const int activeIdx = h.turnManager.getCurrentPlayerIndex();
    PrimitivePlayer& p = h.players[activeIdx];

    if (isBankruptStatus(p.status)) {
        cout << "[WARN] Pemain aktif BANKRUPT, gunakan NEXT untuk skip.\n";
        return;
    }

    const int boardSize = static_cast<int>(h.boardCodes.size());
    p.positionIndex = (p.positionIndex + steps) % boardSize;
    const string landed = h.boardCodes[p.positionIndex];

    std::ostringstream detail;
    detail << "Lempar: " << d1 << "+" << d2 << "=" << steps
           << " mendarat di " << landed;
    h.log.push_back(SavedLogEntry(
        h.turnManager.getTurnNumber(),
        p.username,
        "DADU",
        detail.str()));

    cout << "Hasil: " << d1 << "+" << d2 << "=" << steps
         << " | " << p.username << " mendarat di " << landed << "\n";
}

GameSnapshot buildSnapshot(const PrimitiveHarness& h) {
    GameSnapshot s;

    s.setCurrentTurn(h.turnManager.getTurnNumber());
    s.setMaxTurn(h.maxTurn);
    s.setNumPlayers(static_cast<int>(h.players.size()));

    for (const PrimitivePlayer& p : h.players) {
        SavedPlayerState sp(p.username, p.money, h.boardCodes[p.positionIndex], p.status);
        s.addPlayer(sp);
    }

    const vector<int>& order = h.turnManager.getTurnOrder();
    for (int idx : order) {
        s.addTurnOrder(h.players[idx].username);
    }
    s.setActivePlayer(activePlayerName(h));

    for (const SavedPropertyState& prop : h.properties) {
        s.addProperty(prop);
    }
    s.setDeck(h.deck);
    for (const SavedLogEntry& e : h.log) {
        s.addLogEntry(e);
    }

    return s;
}

void applySnapshot(PrimitiveHarness& h, const GameSnapshot& s) {
    if (s.getNumPlayers() < 2 || s.getNumPlayers() > 4) {
        throw GameException("Snapshot jumlah pemain invalid");
    }

    h.players.clear();
    for (const SavedPlayerState& sp : s.getPlayers()) {
        PrimitivePlayer p;
        p.username = sp.getUsername();
        p.money = sp.getMoney();
        p.status = sp.getStatus();

        const int posIdx = findBoardIndexByCode(h, sp.getPositionCode());
        p.positionIndex = (posIdx >= 0) ? posIdx : 0;
        h.players.push_back(p);
    }

    std::unordered_map<string, int> idxByName;
    for (int i = 0; i < static_cast<int>(h.players.size()); ++i) {
        idxByName[h.players[i].username] = i;
    }

    vector<int> restoredOrder;
    restoredOrder.reserve(s.getTurnOrder().size());
    for (const string& name : s.getTurnOrder()) {
        if (idxByName.find(name) == idxByName.end()) {
            throw GameException("Username di turn order tidak ditemukan: " + name);
        }
        restoredOrder.push_back(idxByName[name]);
    }

    int activeOrderIndex = 0;
    for (int i = 0; i < static_cast<int>(s.getTurnOrder().size()); ++i) {
        if (s.getTurnOrder()[i] == s.getActivePlayer()) {
            activeOrderIndex = i;
            break;
        }
    }

    h.turnManager.restoreState(restoredOrder, activeOrderIndex, s.getCurrentTurn(), false);
    h.maxTurn = s.getMaxTurn();

    h.properties = s.getProperties();
    h.deck = s.getDeck();
    h.log = s.getLog();
    h.initialized = true;
}

void saveSnapshotToFile(const GameSnapshot& snapshot,
                        const string& filename,
                        const Gamestateserializer& serializer) {
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
        throw GameException("Nama file save harus berekstensi .txt");
    }

    std::ofstream out(filename);
    if (!out.is_open()) {
        throw GameException("Tidak bisa membuka file untuk menulis: " + filename);
    }
    out << serializer.serialize(snapshot);
    if (out.fail()) {
        throw GameException("Gagal menulis file save: " + filename);
    }
}

GameSnapshot loadSnapshotFromFile(const string& filename,
                                  const Gamestateserializer& serializer) {
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
        throw GameException("Nama file save harus berekstensi .txt");
    }

    std::ifstream in(filename);
    if (!in.is_open()) {
        throw GameException("File save tidak ditemukan: " + filename);
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    return serializer.deserialize(buffer.str());
}

void showSaveFile(const string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        cout << "[ERR] File tidak ditemukan: " << filename << "\n";
        return;
    }

    cout << "\n=== ISI " << filename << " ===\n";
    string line;
    while (std::getline(in, line)) {
        cout << line << "\n";
    }
    cout << "=== END ===\n\n";
}

} // namespace

int main() {
    PrimitiveHarness harness;
    Gamestateserializer serializer;

    try {
        initBoardCodes(harness);
        harness.dice.setManual(1, 1);
        loadAllConfigs(harness);
    } catch (const std::exception& e) {
        cout << "[WARN] Gagal preload config: " << e.what() << "\n";
        cout << "[WARN] Program tetap jalan dengan nilai default.\n";
        try {
            loadPropertyDefaults(harness);
        } catch (...) {
            // Biarkan harness tetap bisa jalan walau tanpa config properti.
        }
    }

    cout << "=== Primitive Test Engine (Serializer Harness) ===\n";
    cout << "Tujuan: uji serializer/save/load lewat command interaktif.\n";
    run_leaderboard_tests();
    printHelp();

    string line;
    while (true) {
        cout << "> ";
        if (!std::getline(std::cin, line)) break;

        line = trim(line);
        if (line.empty()) continue;

        vector<string> args = split(line);
        const string cmd = upper(args[0]);

        try {
            if (cmd == "HELP") {
                printHelp();
            } else if (cmd == "EXIT") {
                cout << "Keluar dari test harness.\n";
                break;
            } else if (cmd == "INIT") {
                initializeHarness(harness);
                printOrder(harness);
                printStatus(harness);
            } else if (cmd == "STATUS") {
                printStatus(harness);
            } else if (cmd == "ORDER") {
                printOrder(harness);
            } else if (cmd == "ROLL") {
                if (!harness.initialized) {
                    cout << "[WARN] Jalankan INIT dulu.\n";
                    continue;
                }
                const auto d = harness.dice.rollRandom();
                moveActivePlayerBy(harness, d.first + d.second, d.first, d.second);
            } else if (cmd == "ATUR_DADU") {
                if (!harness.initialized) {
                    cout << "[WARN] Jalankan INIT dulu.\n";
                    continue;
                }
                if (args.size() != 3) {
                    cout << "Usage: ATUR_DADU X Y\n";
                    continue;
                }
                const int x = std::stoi(args[1]);
                const int y = std::stoi(args[2]);
                const auto d = harness.dice.setManual(x, y);
                moveActivePlayerBy(harness, d.first + d.second, d.first, d.second);
            } else if (cmd == "NEXT") {
                if (!harness.initialized) {
                    cout << "[WARN] Jalankan INIT dulu.\n";
                    continue;
                }
                harness.turnManager.nextPlayer(buildBankruptFlags(harness));
                cout << "Pemain aktif sekarang: " << activePlayerName(harness)
                     << " | Turn: " << harness.turnManager.getTurnNumber() << "\n";
            } else if (cmd == "SET_STATUS") {
                if (args.size() != 3) {
                    cout << "Usage: SET_STATUS <USERNAME> <ACTIVE|JAILED|BANKRUPT>\n";
                    continue;
                }
                const int idx = findPlayerIndex(harness, args[1]);
                if (idx < 0) {
                    cout << "[ERR] Pemain tidak ditemukan\n";
                    continue;
                }
                harness.players[idx].status = upper(args[2]);
                cout << "[OK] Status " << args[1] << " = " << harness.players[idx].status << "\n";
            } else if (cmd == "SET_MONEY") {
                if (args.size() != 3) {
                    cout << "Usage: SET_MONEY <USERNAME> <NOMINAL>\n";
                    continue;
                }
                const int idx = findPlayerIndex(harness, args[1]);
                if (idx < 0) {
                    cout << "[ERR] Pemain tidak ditemukan\n";
                    continue;
                }
                harness.players[idx].money = std::stoi(args[2]);
                cout << "[OK] Uang " << args[1] << " = " << harness.players[idx].money << "\n";
            } else if (cmd == "SET_POS") {
                if (args.size() != 3) {
                    cout << "Usage: SET_POS <USERNAME> <KODE_PETAK>\n";
                    continue;
                }
                const int idx = findPlayerIndex(harness, args[1]);
                if (idx < 0) {
                    cout << "[ERR] Pemain tidak ditemukan\n";
                    continue;
                }
                const int pos = findBoardIndexByCode(harness, upper(args[2]));
                if (pos < 0) {
                    cout << "[ERR] Kode petak tidak ditemukan\n";
                    continue;
                }
                harness.players[idx].positionIndex = pos;
                cout << "[OK] Posisi " << args[1] << " = " << upper(args[2]) << "\n";
            } else if (cmd == "CONFIG") {
                printConfigSummary(harness);
            } else if (cmd == "RELOAD_CONFIG") {
                loadAllConfigs(harness);
                cout << "[OK] Semua config berhasil di-reload dari folder config/.\n";
                printConfigSummary(harness);
            } else if (cmd == "SAVE") {
                if (!harness.initialized) {
                    cout << "[WARN] Jalankan INIT dulu.\n";
                    continue;
                }
                const string filename = (args.size() >= 2) ? args[1] : "file_save.txt";
                saveSnapshotToFile(buildSnapshot(harness), filename, serializer);
                cout << "[OK] Snapshot tersimpan ke: " << filename << "\n";
            } else if (cmd == "LOAD") {
                const string filename = (args.size() >= 2) ? args[1] : "file_save.txt";
                applySnapshot(harness, loadSnapshotFromFile(filename, serializer));
                cout << "[OK] Snapshot termuat dari: " << filename << "\n";
                printOrder(harness);
                printStatus(harness);
            } else if (cmd == "SHOW_SAVE") {
                const string filename = (args.size() >= 2) ? args[1] : "file_save.txt";
                showSaveFile(filename);
            } else {
                cout << "[ERR] Command tidak dikenal. Ketik HELP.\n";
            }
        } catch (const std::exception& e) {
            cout << "[ERR] " << e.what() << "\n";
        }
    }
    return 0;
}
