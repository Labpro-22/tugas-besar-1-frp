#pragma once

#include <string>
#include <vector>
#include <map>
using namespace std;

class PropertyDef {
public:
    PropertyDef();
    void setId(int id);
    void setCode(const string& code);
    void setName(const string& name);
    void setTypeName(const string& typeName);  
    void setColorGroup(const string& colorGroup);
    void setPurchasePrice(int price);
    void setMortgageValue(int value);
    void setHouseCost(int cost);
    void setHotelCost(int cost);
    void setRentLevels(const vector<int>& levels); 

    int getId() const;
    string getCode() const;
    string getName() const;
    string getTypeName() const;
    string getColorGroup() const;
    int getPurchasePrice() const;
    int getMortgageValue() const;
    int getHouseCost() const;
    int getHotelCost() const;
    const vector<int>& getRentLevels() const;


private:
    int id;
    string code;
    string name;
    string typeName;
    string colorGroup;
    int  purchasePrice;
    int mortgageValue;
    int houseCost;
    int hotelCost;
    vector<int> rentLevels;
};

class TaxConfig {
public:
    TaxConfig();
    TaxConfig(int pphFlat, int pphPercentage, int pbmFlat);

    void setPhFlat(int v);
    void setPphPercentage(int v);
    void setPbmFlat(int v);

    int getPphFlat() const;
    int getPphPercentage()  const;
    int getPbmFlat() const;

private:
    int pphFlat;
    int pphPercentage;
    int pbmFlat;
};

class SpecialConfig {
public:
    SpecialConfig();
    SpecialConfig(int goSalary, int jailFine);

    void setGoSalary(int v);
    void setJailFine(int v);

    int getGoSalary() const;
    int getJailFine() const;

private:
    int goSalary;
    int jailFine;
};


// max turn and saldo awal
class MiscConfig {
public:
    MiscConfig();
    MiscConfig(int maxTurn, int initialBalance);

    void setMaxTurn(int v);
    void setInitialBalance(int v);

    int getMaxTurn() const;
    int getInitialBalance() const;

private:
    int maxTurn;
    int initialBalance;
};

class ActionTileDef {
public:
    ActionTileDef();

    void setId(int id);
    void setCode(const string& code);
    void setName(const string& name);
    void setTileKind(const string& tileKind);
    void setColorGroup(const string& colorGroup);

    int getId() const;
    string getCode() const;
    string getName() const;
    string getTileKind() const;
    string getColorGroup() const;

private:
    int id;
    string code;
    string name;
    string tileKind;
    string colorGroup;
};

//  baca file conf .txt dan ubah jadi class terstruktur
class ConfigLoader {
public:
    vector<PropertyDef> loadProperties(const string& path) const;
    vector<ActionTileDef> loadActionTiles(const string& path) const;
    map<int,int> loadRailroadConfig(const string& path) const;
    map<int,int> loadUtilityConfig(const string& path) const;

    TaxConfig loadTaxConfig(const string& path) const;
    SpecialConfig  loadSpecialConfig(const string& path) const;
    MiscConfig loadMiscConfig(const string& path) const;

private:
    // pecah baris teks jd token 
    vector<string> splitLine(const string& line) const;
    void validateProperties(const vector<PropertyDef>& defs) const;
    void validateActionTiles(const vector<ActionTileDef>& defs) const;
    // trim whitespace di kiri dan kanan string
    string  trim(const string& s) const;
};
