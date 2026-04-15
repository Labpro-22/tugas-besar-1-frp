#pragma once
#include <string>
using namespace std;

class Player;
class GameContext;

enum class PropertyType {
    STREET,
    RAILROAD,
    UTILITY
};

enum class OwnershipStatus {
    BANK,       
    OWNED,      
    MORTGAGED   
};

enum class BuildingLevel {
    NONE    = 0,
    HOUSE_1 = 1,
    HOUSE_2 = 2,
    HOUSE_3 = 3,
    HOUSE_4 = 4,
    HOTEL   = 5
};

class Property {
protected:
    string code_;
    string name_;
    PropertyType type_;
    Player* owner_;
    OwnershipStatus status_;
    int purchasePrice_;
    int mortgageValue_;
    int festivalMultiplier_;
    int festivalDuration; // 0 = ga aktif, 1-3 = aktif
public:
    Property(const std::string& code, const std::string& name,
             PropertyType type, int purchasePrice, int mortgageValue);
 
    virtual ~Property() = default;
    virtual int  calculateRent(const GameContext& ctx) const = 0;
    virtual bool canBuild() const; // bisa dibangun rumah/hotel
    virtual int  getSellValue() const; // nilai jual ke bank
 
    bool isOwned()     const;
    bool isMortgaged() const;
    bool isBank()      const;
 
    void setOwner(Player* p);           // nullptr → status bank
    void setStatus(OwnershipStatus s);
    Player* getOwner() const;

    int  getFestivalMultiplier() const;
    void setFestivalMultiplier(int m);
    int  getFestivalDuration() const;
    void setFestivalDuration(int d);
 
    std::string getCode() const;
    std::string getName() const;
    PropertyType getType() const;
    OwnershipStatus getStatus() const;
    int getPurchasePrice() const;
    int getMortgageValue() const;
};