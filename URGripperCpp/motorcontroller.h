#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H
#include <array>
#include "adc0832.h"
#include "l298.h"

class MotorController
{
public:
    // Er lidt i tvivl om vi vil give adc og l298 med som parametre
    MotorController();
    MotorController(L298 driver, ADC0832 potAdc, ADC0832 forceAdc);

    //Dette er kommandoen man skal kalde for at få griberen til at åbne eller lukke
    //her gives den position man vil til med, samt hvilken kraft der må bruges på det.
    //så hvis man vil lukke med kraft, gies fuld luk position med og så ønsked kraft
    //og modsat hvis man vil til en position så skal der også gives kraft med
    //så maskinen ikke bliver skadet. (den vil også tjekke op mod en "hardcoded maxForce"
    bool move(unsigned int toPosition, unsigned int force);


    //metode til udregning af position
    unsigned int calcPosition();

    //support metoder
    unsigned int getPotReading(unsigned int channel);

    void setOffsetPosition(unsigned int offset);

    unsigned  getPosition() const;

    void changePotRange(unsigned int lower, unsigned int upper);

    //metode til force udregning
    //til linea og Søren
    unsigned int calcForce();


private:
    unsigned int position;
    unsigned int offsetPosition{0};
    unsigned int maxForce;
    std::array<unsigned int, 2> potRange{20,100};

    //wheatstone
    unsigned int gain{1000}; //gain fra 1. stage
    unsigned int offset;
    unsigned int slope;

    //igen er i tvivl som controlleren skal styre dette, eller og det skal være "hardcoded"
    L298 _driver;
    ADC0832 _potAdc;
    ADC0832 _forceAdc;

};

#endif // MOTORCONTROLLER_H
