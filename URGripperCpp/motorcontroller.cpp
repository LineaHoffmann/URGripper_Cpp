#include "motorcontroller.h"
#include <array>
#include "adc0832.h"
#include "l298.h"
#include <iostream>




MotorController::MotorController() : _driver(L298()), _potAdc(ADC0832(0,100)), _forceAdc(ADC0832(1,1000*1000))
{
    std::cout << "H-Bridge and Adc succesfully initialized." << std::endl;
}


MotorController::MotorController(L298 driver, ADC0832 potAdc, ADC0832 forceAdc) : _driver(driver), _potAdc(potAdc), _forceAdc(forceAdc)
{

}

bool MotorController::move(unsigned int toPosition, unsigned int force)
{
    if (toPosition > 0 && toPosition <=100) {
        if (this->calcForce() <= maxForce) {
            if (this->calcPosition() == 1000) {
                std::cout << "error in potmeter" << std::endl;
                return 1;
            }

            while (position != toPosition){

                if (position < toPosition) {
                    if (this->calcForce() > force) {
                        return 1;
                    }
                    //kode til motor her:






                } else if (position > toPosition) {
                    if (this->calcForce() > force) {
                        return 1;
                    }
                    //kode til motor her:




                }

            }

        } else {
            std::cout << "error in force" << std::endl;
            return 1;
}
    }
    else {
       std::cout << "error in position" << std::endl;
       return 1;
    }
    return 0;
}

unsigned int MotorController::calcPosition()
{
    unsigned int potReading = this->getPotReading(1);
    if (potReading == 1000) {
        //error in reading
        return 1000;
    } else {
    //output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start)
    position = 0 + ((100 - 0) / (potRange[1] - potRange[0])) * (potReading - 0);
    return position;
    }
}

unsigned int MotorController::getPotReading(unsigned int channel)
{
    unsigned int tempReading = _potAdc.readADC(channel);
    if (tempReading < 256 && tempReading >= potRange[0] && tempReading <= potRange[1]) {
        return tempReading;
    } else {
        std::cout << "pot reading not in accepted range" << std::endl;
        return 1000;
    }
}


void MotorController::setOffsetPosition(unsigned int offset)
{
    offsetPosition = offset;
}

unsigned MotorController::getPosition() const
{
    return position;
}

void MotorController::changePotRange(unsigned int lower, unsigned int upper)
{
    potRange[0] = lower;
    potRange[1] = upper;
}







unsigned int MotorController::calcForce()
{

}
