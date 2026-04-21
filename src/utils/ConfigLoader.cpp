#include "../../include/utils/ConfigLoader.hpp"
#include "../../include/utils/GameException.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

PropertyDef::PropertyDef() : id(0), purchasePrice(0), mortgageValue(0), houseCost(0), hotelCost(0) {}

void PropertyDef::setId(int id) { this->id = id; }
void PropertyDef::setCode(const string& code) { this->code = code; }
void PropertyDef::setName(const string& name) { this->name = name;}
void PropertyDef::setTypeName(const string& t) { this->typeName = t;}
void PropertyDef::setColorGroup(const string& cg) { this->colorGroup = cg;}
void PropertyDef::setPurchasePrice(int price) { this->purchasePrice = price; }
void PropertyDef::setMortgageValue(int value)  { this->mortgageValue = value; }
void PropertyDef::setHouseCost(int cost) { this->houseCost = cost;}
void PropertyDef::setHotelCost(int cost) { this->hotelCost = cost;}
void PropertyDef::setRentLevels(const vector<int>& lv) { this->rentLevels = lv;}

int  PropertyDef::getId() const { return id; }
string PropertyDef::getCode() const { return code; }
string PropertyDef::getName() const { return name; }
string PropertyDef::getTypeName() const { return typeName; }
string PropertyDef::getColorGroup() const { return colorGroup; }
int  PropertyDef::getPurchasePrice() const { return purchasePrice; }
int  PropertyDef::getMortgageValue() const { return mortgageValue; }
int  PropertyDef::getHouseCost() const { return houseCost; }
int  PropertyDef::getHotelCost() const { return hotelCost; }
const vector<int>& PropertyDef::getRentLevels() const { return rentLevels; }

TaxConfig::TaxConfig() : pphFlat(0), pphPercentage(0), pbmFlat(0) {}
TaxConfig::TaxConfig(int pphFlat, int pphPercentage, int pbmFlat) : pphFlat(pphFlat), pphPercentage(pphPercentage), pbmFlat(pbmFlat) {}

void TaxConfig::setPhFlat(int v) { pphFlat = v;}
void TaxConfig::setPphPercentage(int v) { pphPercentage = v;  }
void TaxConfig::setPbmFlat(int v) { pbmFlat = v; }
int  TaxConfig::getPphFlat() const { return pphFlat; }
int  TaxConfig::getPphPercentage() const { return pphPercentage; }
int  TaxConfig::getPbmFlat() const { return pbmFlat; }

SpecialConfig::SpecialConfig() : goSalary(0), jailFine(0) {}
SpecialConfig::SpecialConfig(int goSalary, int jailFine)  : goSalary(goSalary), jailFine(jailFine) {}

void SpecialConfig::setGoSalary(int v) { goSalary = v; }
void SpecialConfig::setJailFine(int v) { jailFine = v; }
int  SpecialConfig::getGoSalary() const { return goSalary; }
int  SpecialConfig::getJailFine() const { return jailFine; }

MiscConfig::MiscConfig() : maxTurn(0), initialBalance(0) {}
MiscConfig::MiscConfig(int maxTurn, int initialBalance) : maxTurn(maxTurn), initialBalance(initialBalance) {}

void MiscConfig::setMaxTurn(int v) { maxTurn = v; }
void MiscConfig::setInitialBalance(int v) { initialBalance = v; }
int  MiscConfig::getMaxTurn() const { return maxTurn; }
int  MiscConfig::getInitialBalance() const { return initialBalance; }

string ConfigLoader::trim(const string& s) const {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

vector<string> ConfigLoader::splitLine(const string& line) const {
    vector<string> tokens;
    istringstream iss(line);
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void ConfigLoader::validateProperties(const vector<PropertyDef>& defs) const {
    for (const PropertyDef& def : defs) {
        if (def.getCode().empty()) {
            throw ConfigParseException("Property with id=" +
                to_string(def.getId()) + " has empty code");
        }
        if (def.getTypeName() == "STREET" && def.getRentLevels().size() != 6) {
            throw ConfigParseException("STREET property '" + def.getCode() +
                "' must have 6 rent levels");
        }
        if (def.getPurchasePrice() < 0) {
            throw ConfigParseException("Property '" + def.getCode() +
                "' has negative purchasePrice");
        }
    }
}


vector<PropertyDef> ConfigLoader::loadProperties(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    vector<PropertyDef> defs;
    string line;
    int lineNum = 0;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        vector<string> tokens = splitLine(line);
        if (tokens.empty()) continue;

        // Lewati baris header: token pertama bukan angka (contoh: "ID")
        bool firstIsNumber = true;
        for (char c : tokens[0]) {
            if (!isdigit(c)) { firstIsNumber = false; break; }
        }
        if (!firstIsNumber) continue;

        if (tokens.size() < 6) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected at least 6 tokens, got " + to_string(tokens.size()));
        }

        PropertyDef def;
        try {
            def.setId(stoi(tokens[0]));
            def.setCode(tokens[1]);
            def.setName(tokens[2]);

            string jenis = tokens[3];
            def.setTypeName(jenis);
            def.setColorGroup(tokens[4]);

            if (jenis == "STREET") {
                if (tokens.size() < 15) {
                    throw ConfigParseException(
                        path + " line " + to_string(lineNum) +
                        ": STREET expects 15 tokens, got "
                        + to_string(tokens.size()));
                }
                def.setPurchasePrice(stoi(tokens[5]));
                def.setMortgageValue(stoi(tokens[6]));
                def.setHouseCost(stoi(tokens[7]));
                def.setHotelCost(stoi(tokens[8]));

                vector<int> rentLevels;
                for (int i = 9; i <= 14; i++) {
                    rentLevels.push_back(stoi(tokens[i]));
                }
                def.setRentLevels(rentLevels);

            } else if (jenis == "RAILROAD") {
                if (tokens.size() < 6) {
                    throw ConfigParseException(
                        path + " line " + to_string(lineNum) +
                        ": RAILROAD expects 6 tokens");
                }
                def.setPurchasePrice(stoi(tokens[5]));
                def.setMortgageValue(stoi(tokens[5]));
                def.setHouseCost(0);
                def.setHotelCost(0);

            } else if (jenis == "UTILITY") {
                if (tokens.size() < 6) {
                    throw ConfigParseException(
                        path + " line " + to_string(lineNum) +
                        ": UTILITY expects 6 tokens");
                }
                def.setPurchasePrice(stoi(tokens[5]));
                def.setMortgageValue(stoi(tokens[5]));
                def.setHouseCost(0);
                def.setHotelCost(0);

            } else {
                throw ConfigParseException(
                    path + " line " + to_string(lineNum) +
                    ": unknown property type '" + jenis + "'");
            }

        } catch (const ConfigParseException&) {
            throw;
        } catch (const invalid_argument& e) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": invalid number — " + string(e.what()));
        } catch (const out_of_range& e) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": number out of range — " + string(e.what()));
        }

        defs.push_back(def);
    }

    validateProperties(defs);
    return defs;
}

map<int,int> ConfigLoader::loadRailroadConfig(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    map<int,int> result;
    string line;
    int lineNum = 0;
    bool headerSkipped = false;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (!headerSkipped) { headerSkipped = true; continue; }

        vector<string> tokens = splitLine(line);
        if (tokens.size() < 2) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected JUMLAH_RAILROAD BIAYA_SEWA");
        }
        try {
            result[stoi(tokens[0])] = stoi(tokens[1]);
        } catch (const exception& e) {
            throw ConfigParseException(path + " line " +
                to_string(lineNum) + ": " + string(e.what()));
        }
    }
    return result;
}

map<int,int> ConfigLoader::loadUtilityConfig(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    map<int,int> result;
    string line;
    int lineNum = 0;
    bool headerSkipped = false;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (!headerSkipped) { headerSkipped = true; continue; }

        vector<string> tokens = splitLine(line);
        if (tokens.size() < 2) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected JUMLAH_UTILITY FAKTOR_PENGALI");
        }
        try {
            result[stoi(tokens[0])] = stoi(tokens[1]);
        } catch (const exception& e) {
            throw ConfigParseException(path + " line " +
                to_string(lineNum) + ": " + string(e.what()));
        }
    }
    return result;
}

TaxConfig ConfigLoader::loadTaxConfig(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    TaxConfig cfg;
    string line;
    int lineNum = 0;
    bool headerSkipped = false;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        // Lewati header
        if (!headerSkipped) { headerSkipped = true; continue; }

        // Baris data: PPH_FLAT PPH_PERSENTASE PBM_FLAT
        vector<string> tokens = splitLine(line);
        if (tokens.size() < 3) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected 3 values: PPH_FLAT PPH_PERSENTASE PBM_FLAT, got "
                + to_string(tokens.size()));
        }
        try {
            cfg.setPhFlat(stoi(tokens[0]));
            cfg.setPphPercentage(stoi(tokens[1]));
            cfg.setPbmFlat(stoi(tokens[2]));
        } catch (const exception& e) {
            throw ConfigParseException(path + " line " +
                to_string(lineNum) + ": " + string(e.what()));
        }
        break; // hanya satu baris data
    }
    return cfg;
}

SpecialConfig ConfigLoader::loadSpecialConfig(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    SpecialConfig cfg;
    string line;
    int lineNum = 0;
    bool headerSkipped = false;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (!headerSkipped) { headerSkipped = true; continue; }

        // Baris data: GO_SALARY JAIL_FINE
        vector<string> tokens = splitLine(line);
        if (tokens.size() < 2) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected 2 values: GO_SALARY JAIL_FINE");
        }
        try {
            cfg.setGoSalary(stoi(tokens[0]));
            cfg.setJailFine(stoi(tokens[1]));
        } catch (const exception& e) {
            throw ConfigParseException(path + " line " +
                to_string(lineNum) + ": " + string(e.what()));
        }
        break;
    }
    return cfg;
}

MiscConfig ConfigLoader::loadMiscConfig(const string& path) const {
    ifstream file(path);
    if (!file.is_open()) throw ConfigFileNotFoundException(path);

    MiscConfig cfg;
    string line;
    int lineNum = 0;
    bool headerSkipped = false;

    while (getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (!headerSkipped) { headerSkipped = true; continue; }

        vector<string> tokens = splitLine(line);
        if (tokens.size() < 2) {
            throw ConfigParseException(
                path + " line " + to_string(lineNum) +
                ": expected 2 values: MAX_TURN SALDO_AWAL");
        }
        try {
            cfg.setMaxTurn(stoi(tokens[0]));
            cfg.setInitialBalance(stoi(tokens[1]));
        } catch (const exception& e) {
            throw ConfigParseException(path + " line " +
                to_string(lineNum) + ": " + string(e.what()));
        }
        break;
    }
    return cfg;
}