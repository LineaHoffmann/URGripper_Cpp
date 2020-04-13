//#include <array>
#include <cmath>

#include "motorcontroller.h"
#include "adc0832.h"
#include "l298.h"

//#include <iostream>

MotorController& MotorController::buildController(std::shared_ptr<L298> driver, std::shared_ptr<ADC0832> adc0, std::shared_ptr<ADC0832> adc1) {
    static MotorController controller(driver,adc0,adc1);
    return controller;
}
MotorController::MotorController(std::shared_ptr<L298> driver, std::shared_ptr<ADC0832> adc0, std::shared_ptr<ADC0832> adc1){
    driver_ = driver ; adc0_ = adc0; adc1_ = adc1;
}

/*
bool MotorController::move(unsigned int toPosition, unsigned int force)
{
    if (toPosition > 0 && toPosition <=100) {
        if (this->calcForce() <= maxForce) {
            if (this->calcPosition() == 1000) {
                //std::cout << "error in potmeter" << std::endl;
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
            //std::cout << "error in force" << std::endl;
            return 1;
}
    }
    else {
       //std::cout << "error in position" << std::endl;
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
    unsigned int tempReading = adcPosition_();
    if (tempReading < 256 && tempReading >= potRange[0] && tempReading <= potRange[1]) {
        return tempReading;
    } else {
        //std::cout << "pot reading not in accepted range" << std::endl;
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
uint8_t MotorController::getWheatstoneReading(uint8_t)
unsigned int MotorController::getWheatstoneReading(unsigned int channel)
    {
        unsigned int tempReading = adcForce_();
        if (tempReading <= 256 && tempReading >= 0)
        {
            return tempReading;
        }
        else
        {
            //std::cout << "Wheatstone reading not in accepted rang. " << std::endl;
            return 0;
        }
    }
double MotorController::getOffset(){
    return offset;
}
double MotorController::getSlope()
{
    return slope;
}
unsigned int MotorController::calcForce()
{
    unsigned int wheatstoneReading = this ->getWheatstoneReading(2);
    if (wheatstoneReading == 0)
    {
        // Error in reading
        return 0;
    }
    else
    {
        force = static_cast<unsigned int>((slope*wheatstoneReading) - offset);
        return force;
    }

}
*/

// Move command
enum MOTOR_CONTROL_ERROR_CODE MotorController::move(double newPos, double force) {
    // Calculated conversion factors from [mm] to [0-255] ADC reading
    double positionFactor{100};
    double forceFactor{100};

    // If inputs are negative, return error
    if (newPos < 0 || force < 0)
        return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    // If new position is out off range, return error
    if (static_cast<uint32_t>(std::round(newPos * positionFactor)) < positionRange_.first || static_cast<uint32_t>(std::round(newPos * positionFactor)) > positionRange_.second)
        return state_ = MOTOR_CONTROL_ERROR_CODE::OUT_OF_POSITION_RANGE;
    // If new max force is above set limit, return error
    if (static_cast<uint32_t>(std::round(force * forceFactor)) > maxForce_)
        return state_ = MOTOR_CONTROL_ERROR_CODE::FORCE_ABOVE_MAX;

    uint8_t targetPosition = static_cast<uint8_t>(std::round(newPos * positionFactor + positionOffset_));
    uint8_t maxForce = static_cast<uint8_t>(std::round(force * forceFactor) + forceOffset_);
    // Direction -- true = in | false = out
    bool direction = (targetPosition < adcPosition_()) ? true : false;
    driver_->setDirection(direction);

    // Variables common for the control loops
    uint8_t i{0};
    uint8_t rdForce;
    uint8_t rdPos;

    // Loop until touching or position correct
    while (1) {
        // Read new position and force once per loop
        rdForce = adcForce_();
        rdPos = adcPosition_();
        // Direction is used to determine which way we approach the target from
        // If moving in and target becomes >= to current position, stop and break
        // If moving out and target becomes <= to current position, stop and break
        if (direction) {if (targetPosition >= rdPos) driver_->setRatio(0); break;}
        else {if (targetPosition <= rdPos) driver_->setRatio(0); break;}
        // If force is more than measurable threshold, break
        if (rdForce > forceOffset_) break;
        // Move slowly, no force required
        driver_->setRatio(1); // 5% power
    };

    // Check if moved far enough
    // Cheating with i variable for next loop to fall through on first run
    if (direction) {if (targetPosition >= rdPos) i = 20;}
    else {if (targetPosition <= rdPos) i = 20;}

    // Loop until force is correct, if not at or past position or until actual max output
    // If previous loop broke because of position, this should fall through
    // If not, we should be touching an object measureably
    while (!(i == 20) && rdForce < maxForce ) {
        // Small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Read force and position once per loop
        rdForce = adcForce_();
        rdPos = adcPosition_();
        // If moving in and target becomes >= to current position, break
        // If moving out and target becomes <= to current position, break
        if (direction) {if (targetPosition >= rdPos) break;}
        else {if (targetPosition <= rdPos) break;}
        // If read force is less than max, step up
        if (rdForce < maxForce) driver_->setRatio(++i);
    };
    // If we made it here, all may have gone well
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Position controls
double MotorController::getPosition() const {
    return static_cast<double>(adcPosition_() + positionOffset_) * positionFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setPositionOffset(double offset) {
    if (offset < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = offset / positionFactor_;
    if (temp > 256) return state_ = MOTOR_CONTROL_ERROR_CODE::OUT_OF_POSITION_OFFSET_RANGE;
    positionOffset_ = static_cast<uint8_t>(temp);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::changePositionRange(double upper, double lower) {
    if (upper < 0 || lower < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    positionRange_.first = (lower / positionFactor_) + positionOffset_;
    positionRange_.second = (upper / positionFactor_) + positionOffset_;
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Force controls
double MotorController::getForce() const {
    return static_cast<double>(adcForce_() + forceOffset_) * forceFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setMaxForce(double newMaxForce) {
    if (newMaxForce < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = (newMaxForce / forceFactor_) + forceOffset_;
    if (temp > 256) return state_ = MOTOR_CONTROL_ERROR_CODE::FORCE_ABOVE_MAX;
    maxForce_ = static_cast<uint8_t>(temp);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// State getter
enum MOTOR_CONTROL_ERROR_CODE MotorController::getState() {return state_;}

// ADC helper functions
// Tweak to correct physical configuration here
uint8_t MotorController::adcPosition_() const {return adc0_->readADC(0);}
uint8_t MotorController::adcCurrent_() const {return adc0_->readADC(1);}
uint8_t MotorController::adcForce_() const {return adc1_->readADC(0);}
