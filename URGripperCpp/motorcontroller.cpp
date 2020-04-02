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

}

unsigned int MotorController::calcPosition()
{
    position = ((this->getPotReading(1) - 0) * (100 - 0) / (256 - 0)) + 0;
    return position;
}

unsigned int MotorController::getPotReading(unsigned int channel)
{
    unsigned int tempReading = _potAdc.readADC(channel);
    if (tempReading <= 256 && tempReading >= potRange[0] && tempReading <= potRange[1]) {
        return tempReading;
    } else {
        std::cout << "pot reading not in accepted range" << std::endl;
        return 0;
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
