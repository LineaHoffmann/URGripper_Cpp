//#include <array>
#include <cmath>

#include "motorcontroller.h"
#include "adc0832.h"
#include "l298.h"
#include <memory>

//#include <iostream>

MotorController& MotorController::buildController(L298& driver, ADC0832& adc0, ADC0832& adc1) {
    static MotorController controller(driver,adc0,adc1);
    return controller;
}
MotorController::MotorController(L298& driver, ADC0832& adc0, ADC0832& adc1){
    // Binding pointers to member variables
    driver_ = &driver;
    adc0_ = &adc0;
    adc1_ = &adc1;
    // Setting different settings
    forceThreshold_ = 15; // Threshold for force detection
    forceOffset_ = 7; // Minimum limit for measurement
    forceFactor_ = 0.01; // uint8_t to kg conversion factor
    maxForce_ = 255; // Max allowable force
    jog_ratio_ = 15; // Jogging move speed [0:20]
    positionFactor_ = 0.4073; // uint8_t to millimeter conversion factor
    positionOffset_ = 0; // For other gripper fingertips
    positionRange_.first = 27; // = 0mm open
    positionRange_.second = 180; // = 65mm open

    // Starting ADC reading thread
    adc_thread_ = std::thread(&MotorController::SpawnAdcReader_,this);

    // General init complete
    state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}

// Move command
enum MOTOR_CONTROL_ERROR_CODE MotorController::move(double newPos, double force) {
    // If inputs are negative, return error
    if (newPos < 0 || force < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    // int position = (pos in mm / conversion factor) + 0mm uint8_t value
    uint tempPos = static_cast<uint>(newPos/positionFactor_) + positionRange_.first;
    // If new position is out off range, return error
    // Low check never happens, as newPos would be <0.
    if (tempPos > positionRange_.second) return MOTOR_CONTROL_ERROR_CODE::INVALID_POSITION_RANGE;
    // int force = (force in kg / conversion factor) + offset uint8_t value
    uint tempForce = static_cast<uint>(force/forceFactor_) + forceOffset_;
        // If force ended as 0 after conversion, force will be 0%
    if (tempForce == forceOffset_) tempForce = forceThreshold_;
        // If new max force is above set limit, set 100%
    if (tempForce > maxForce_) {
        tempForce = maxForce_;
    }
    uint8_t targetPosition = static_cast<uint8_t>(tempPos);
    uint8_t targetForce = static_cast<uint8_t>(tempForce);
    // Direction -- true = in | false = out
    bool direction = (targetPosition < rdPos_) ? true : false;
    driver_->setDirection(direction);

    // Variable for the control loop(s)
    uint8_t increment{0};

    if (direction) {
        // Moving in, force is important
        while (rdForce_ < targetForce) {
            // Read position and compare with target
            //rdPos_ = adcPosition_();
            if (rdPos_ <= targetPosition) break;
            // Increment of PWM ratio wrapped in timed check
            auto previousIncrease = std::chrono::steady_clock::time_point();
            if (jog_ratio_ + increment < 20) {
                if (rdForce_ > forceThreshold_ && rdForce_ < maxForce_) {
                    if (previousIncrease < std::chrono::steady_clock::now() + std::chrono::milliseconds(50)) {
                        previousIncrease = std::chrono::steady_clock::now();
                        increment++;
                    }
                }
            }
            // (Re)set ratio
            driver_->setRatio(jog_ratio_ + increment);
        }
    } else {
        // Moving out, ignore force
        while (1) {
            // Read position and compare with target based on set direction
            //rdPos_ = adcPosition_();
            if (rdPos_ >= targetPosition) break;
            else driver_->setRatio(jog_ratio_);
        }
    }
    // Turn off motor, it's a fairly hard stop
    driver_->setRatio(0);
    // If we made it here, all may have gone well
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Position getter in mm
double MotorController::getPosition() const {
    // (uint8_t measurement - 0mm uint8_t value) * conversion factor
    return static_cast<double>(rdPos_ - positionRange_.first) * positionFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setPositionOffset(double offset) {
    if (offset < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = static_cast<int>(offset / positionFactor_);
    if (temp > positionRange_.second) return state_ = MOTOR_CONTROL_ERROR_CODE::INVALID_POSITION_RANGE;
    positionOffset_ = static_cast<uint8_t>(temp);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::changePositionRange(double upper, double lower) {
    int temp_lower = static_cast<int>(lower / positionFactor_) + positionOffset_;
    int temp_upper = static_cast<int>(upper / positionFactor_) + positionOffset_;
    if (temp_upper <= temp_lower) return MOTOR_CONTROL_ERROR_CODE::INVALID_POSITION_RANGE;
    // Remember to correct for physical upper and lower limits
    // 13-05-2020 - Set to [50,206] by PoSRP/soepe13.
    if (temp_upper < 50 || temp_lower < 50) return state_ = MOTOR_CONTROL_ERROR_CODE::INVALID_POSITION_RANGE;
    if (temp_lower > 206 || temp_upper > 206) return state_ = MOTOR_CONTROL_ERROR_CODE::INVALID_POSITION_RANGE;
    positionRange_.first = static_cast<uint8_t>(temp_lower);
    positionRange_.second = static_cast<uint8_t>(temp_upper);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// Force getter in kg
double MotorController::getForce() const {
    return static_cast<double>(rdForce_ - forceOffset_) * forceFactor_;
}
enum MOTOR_CONTROL_ERROR_CODE MotorController::setMaxForce(double newMaxForce) {
    if (newMaxForce < 0) return state_ = MOTOR_CONTROL_ERROR_CODE::NEGATIVE_VALUE;
    int temp = static_cast<int>(newMaxForce / forceFactor_) + forceOffset_;
    if (temp > 255) return state_ = MOTOR_CONTROL_ERROR_CODE::FORCE_ABOVE_MAX;
    maxForce_ = static_cast<uint8_t>(temp);
    return state_ = MOTOR_CONTROL_ERROR_CODE::ALL_OK;
}
// State getter
enum MOTOR_CONTROL_ERROR_CODE MotorController::getState() {
    std::unique_lock<std::mutex> lock(lock_, std::try_to_lock);
    if (!lock.owns_lock()) lock.try_lock();
    return state_;
}
std::array<uint8_t,4> MotorController::getADCs() {
    std::array<uint8_t,4> ret;
    std::unique_lock<std::mutex> lock(lock_, std::try_to_lock);
    if (!lock.owns_lock()) lock.try_lock();
    ret.at(0) = rdForce_;
    ret.at(1) = rdPos_;
    ret.at(2) = rdCurrent_;
    ret.at(3) = rd3v3_;
    this->lock_.unlock();
    return ret;
}
void MotorController::SpawnAdcReader_() {
    // Updates all ADC readings every 20ms
    auto time_step = std::chrono::milliseconds(20);
    auto next = std::chrono::steady_clock::now() + time_step;
    std::unique_lock<std::mutex> lock(lock_);
    while (1) {
        while (!lock.owns_lock()) lock.try_lock();
        rdForce_ = adcForce_();
        rdPos_ = adcPosition_();
        rdCurrent_ = adcCurrent_();
        rd3v3_ = adc3v3_();
        lock.unlock();
        std::this_thread::sleep_until((next += time_step));
    }
}
// ADC helper functions
uint8_t MotorController::adcPosition_() const {return adc1_->readADC(0);}
uint8_t MotorController::adcForce_() const {return adc0_->readADC(1,4);}
uint8_t MotorController::adcCurrent_() const {return adc0_->readADC(0);}
uint8_t MotorController::adc3v3_() const {return adc1_->readADC(1);}
