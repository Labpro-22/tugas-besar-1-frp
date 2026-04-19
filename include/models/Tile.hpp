#pragma once

#include <string>
using namespace std;

class Player;
class GameEngine;

class Tile {
protected:
    int index;
    string code;
    string name;

public:
    Tile(int index, const string& code, const string& name);
    virtual ~Tile() = default;
    
    int getIndex() const;
    string getCode() const;
    string getName() const;

    virtual void onLand(Player& player, GameEngine& engine) = 0;
    virtual bool isProperty() const;    
};