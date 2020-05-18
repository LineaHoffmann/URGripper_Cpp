#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <array>
#include <memory>
#include <mutex>    // For thread safe read/writes
#include <thread>   // Multithreading

#include "adc0832.h"
#include "l298.h"

enum class MOTOR_CONTROL_ERROR_CODE {
    ALL_OK = 0x00,
    INVALID_POSITION_RANGE = 0x01,
    NEGATIVE_VALUE = 0x03,
    FORCE_ABOVE_MAX = 04,
    NOT_INITIALIZED_CORRECTLY = 0x05
};

// For easier accecs to hardware data
struct GripperState {

    uint8_t Voltage3v3Val;
    uint8_t PositionVal;
    uint8_t CurrentVal;
    uint8_t ForceVal;
};

class MotorController {
public:
    // Function for creating singleton instance of object
    static MotorController& buildController(L298&, ADC0832&, ADC0832&);

    // No destructor required (yet)
    //~MotorController();

    // Move command
    // takes new position in [mm] and max force in [kg]
    // returns error enum value
    enum MOTOR_CONTROL_ERROR_CODE move(double newPosition, double maxForce);

    // Position functions
    double getPosition() const;
    enum MOTOR_CONTROL_ERROR_CODE setPositionOffset(double offset);
    enum MOTOR_CONTROL_ERROR_CODE changePositionRange(double upper, double lower);

    // Force functions
    double getForce() const;

    // State getter
    enum MOTOR_CONTROL_ERROR_CODE getState();

    // ADC information getter
    std::array<uint8_t,4> getADCs();

private:
    // Initializer with shared pointer objects from main controller
    MotorController(L298&, ADC0832&, ADC0832&);
    // Explicit delete of copy constructor and assignment operator
    MotorController(const MotorController&) = delete;
    MotorController& operator=(const MotorController&) = delete;

    // Controller state code
    enum MOTOR_CONTROL_ERROR_CODE state_ = MOTOR_CONTROL_ERROR_CODE::NOT_INITIALIZED_CORRECTLY;
    // Controller state struct
    GripperState state_struct_;

    // Position variables
    double positionFactor_;
    uint8_t positionOffset_;
    std::pair<uint8_t, uint8_t> positionRange_;

    // Force variables
    uint8_t maxForce_;
    double forceFactor_;
    uint8_t forceThreshold_;
    uint8_t forceOffset_;

    // Force functions
    enum MOTOR_CONTROL_ERROR_CODE setMaxForce(double newMaxForce);

    // Jogging PWM ratio
    uint8_t jog_ratio_;

    // Wheatstone
    unsigned int gain{1000}; //gain fra 1. stage
    double offset{0.4844}; // Skal ændres efter korrekt graf
    double slope {37.918}; // skal ændres efter korrekt graf.
    unsigned int force;

    // hardware
    L298 *driver_;
    ADC0832 *adc0_;
    ADC0832 *adc1_;

    void SpawnAdcReader_();
    std::thread adc_thread_;
    std::mutex lock_;
    // Helper functions for ADC reading
    uint8_t adcPosition_() const;
    uint8_t adcCurrent_() const;
    uint8_t adcForce_() const;
    uint8_t adc3v3_() const;
    // Variables for holding last read
    uint8_t rdPos_, rdCurrent_, rdForce_, rd3v3_;
};

#endif // MOTORCONTROLLER_H
