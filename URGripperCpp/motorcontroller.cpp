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
    // Binding pointers to member variables
    driver_ = driver ; adc0_ = adc0; adc1_ = adc1;
    // Setting different settings
    positionOffset_ = 584 / 45;
    positionFactor_ = 65 / 153;
    positionRange_.first = 101;
    positionRange_.second = 253;
    // Init complete
    state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}

/*
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
*/

// Move command
enum MOTOR_CONTROL_ERROR_CODE MotorController::move(double newPos, double force) {
    // If inputs are negative, return error
    if (newPos < 0 || force < 0)
        return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    // If new position is out off range, return error
    if (static_cast<uint32_t>(std::round(newPos * positionFactor_)) < positionRange_.first || static_cast<uint32_t>(std::round(newPos * positionFactor_)) > positionRange_.second)
        return state_ = MOTOR_CONTROL_ERROR_CODE::OUT_OF_POSITION_RANGE;
    // If new max force is above set limit, return error
    if (static_cast<uint32_t>(std::round(force * forceFactor_)) > maxForce_)
        return state_ = MOTOR_CONTROL_ERROR_CODE::FORCE_ABOVE_MAX;

    uint8_t targetPosition = static_cast<uint8_t>(std::round(newPos * positionFactor_ + positionOffset_));
    setMaxForce(force);
    //uint8_t maxForce = static_cast<uint8_t>(std::round(force * forceFactor_) + forceOffset_);
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
        if (direction) {if (targetPosition >= rdPos) {driver_->setRatio(0); break;}}
        else {if (targetPosition <= rdPos) {driver_->setRatio(0); break;}}
        // If force is more than measurable threshold, break
        if (rdForce > forceOffset_) break;
        // Move slowly, no force required
        driver_->setRatio(move_ratio_); // 5% power
    };

    // Check if moved far enough
    // Cheating with i variable for next loop to fall through on first run
    if (direction) {if (targetPosition >= rdPos) i = 20;}
    else {if (targetPosition <= rdPos) i = 20;}

    // Loop until force is correct, if not at or past position or until actual max output
    // If previous loop broke because of position, this should fall through
    // If not, we should be touching an object measureably
    while (!(i == 20) && rdForce < maxForce_) {
        // Read force and position once per loop
        rdForce = adcForce_();
        rdPos = adcPosition_();
        // If moving in and target becomes >= to current position, break
        // If moving out and target becomes <= to current position, break
        if (direction) {if (targetPosition >= rdPos) {driver_->setRatio(0); break;}}
        else {if (targetPosition <= rdPos) {driver_->setRatio(0); break;}}
        // If read force is less than max, step up
        if (rdForce < maxForce_) driver_->setRatio(++i);
        // Small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
    // Turn off motor, gearing will hold it
    driver_->setRatio(0);
    // If we made it here, all may have gone well
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Position controls
double MotorController::getPosition() const {
    return static_cast<double>(adcPosition_() + positionOffset_) * positionFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setPositionOffset(double offset) {
    if (offset < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = static_cast<int>(offset / positionFactor_);
    if (temp > 255) return state_ = MOTOR_CONTROL_ERROR_CODE::OUT_OF_POSITION_OFFSET_RANGE;
    positionOffset_ = static_cast<uint8_t>(temp);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::changePositionRange(double upper, double lower) {
    // Remember to correct for physical lower limit
    if (upper < 0 || lower < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp1, temp2;
    temp1 = static_cast<int>(lower / positionFactor_) + positionOffset_;
    temp2 = static_cast<int>(upper / positionFactor_) + positionOffset_;
    // Remember to correct for physical upper limit
    if (temp1 > 255 || temp2 > 255) return state_ = MOTOR_CONTROL_ERROR_CODE::OUT_OF_POSITION_RANGE;
    positionRange_.first = static_cast<uint8_t>(temp1);
    positionRange_.second = static_cast<uint8_t>(temp2);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Force controls
double MotorController::getForce() const {
    return static_cast<double>(adcForce_() + forceOffset_) * forceFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setMaxForce(double newMaxForce) {
    if (newMaxForce < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = static_cast<int>(newMaxForce / forceFactor_) + forceOffset_;
    if (temp > 255) return state_ = MOTOR_CONTROL_ERROR_CODE::FORCE_ABOVE_MAX;
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
