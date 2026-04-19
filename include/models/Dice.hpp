#pragma once
#include <utility>
using namespace std;
class Dice{
    private:
        int die1;
        int die2;
        int faces;
    
    public:
        explicit Dice(int faces = 6); //explicit untuk mencegah implicit conversion
        pair<int,int> rollRandom();
        pair<int,int> setManual(int dadu1, int dadu2);
        bool isDouble() const;
        int getTotal() const;
        int getDie1() const;
        int getDie2() const;
};