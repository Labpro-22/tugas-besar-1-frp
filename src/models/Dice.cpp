#include "../../include/models/Dice.hpp"
#include "../../include/utils/GameException.hpp"
#include <cstdlib>
#include <ctime>

Dice::Dice(int faces) : die1(1), die2(1), faces(faces){
    srand(static_cast<unsigned int>(time(nullptr)));
}

pair<int,int> Dice::rollRandom(){
    die1 = (rand() % faces) + 1;
    die2 = (rand() % faces) + 1;
    return{die1,die2};
}

pair<int, int> Dice::setManual(int dice1, int dice2){
    if (dice1 < 1 || dice1 > faces) {
        throw InvalidDiceValueException(dice1,faces);
    }
    if(dice2 <1 || dice2 > faces){
        throw InvalidDiceValueException(dice2, faces);
    } 
    die1 = dice1;
    die2 = dice2;
    return {die1,die2};
}

int Dice::getTotal() const{
    return die1 + die2;
}

bool Dice::isDouble() const{
    return die1==die2;
}

int Dice::getDie1() const{
    return die1;
}

int Dice::getDie2() const{
    return die2;
}