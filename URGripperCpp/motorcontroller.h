#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <memory>

#include "adc0832.h"
#include "l298.h"

enum class MOTOR_CONTROL_ERROR_CODE {
    ALL_OK = 0x00,
    OUT_OF_POSITION_RANGE = 0x01,
    FORCE_ABOVE_MAX = 0x02,
    NEGATIVE_VALUE = 0x03,
    OUT_OF_POSITION_OFFSET_RANGE = 0x04,
    NOT_INITIALIZED_CORRECTLY = 0x05
};

class MotorController {
public:
    // Function for creating singleton instance of object
    static MotorController& buildController(std::shared_ptr<L298>,std::shared_ptr<ADC0832>,std::shared_ptr<ADC0832>);

    // No destructor required (yet)
    //virtual ~MotorController();

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
    enum MOTOR_CONTROL_ERROR_CODE setMaxForce(double newMaxForce);

    // State getter
    enum MOTOR_CONTROL_ERROR_CODE getState();

private:
    // Initializer with shared pointer objects from main controller
    // Hide constructor, copy-, and assignment operators in private
    MotorController(std::shared_ptr<L298>,std::shared_ptr<ADC0832>,std::shared_ptr<ADC0832>);
    //MotorController(const MotorController&); // Cannot be made unique pointer if copy restricted??
    MotorController& operator=(const MotorController&);

    // Controller state
    enum MOTOR_CONTROL_ERROR_CODE state_ = MOTOR_CONTROL_ERROR_CODE::NOT_INITIALIZED_CORRECTLY;

    // Position variables
    double positionFactor_;
    uint8_t positionOffset_;
    std::pair<uint8_t, uint8_t> positionRange_;

    // Force variables
    uint8_t maxForce_;
    double forceFactor_;
    uint8_t forceOffset_{5};

    // Wheatstone
    unsigned int gain{1000}; //gain fra 1. stage
    double offset{0.4844}; // Skal ændres efter korrekt graf
    double slope {37.918}; // skal ændres efter korrekt graf.
    unsigned int force;

    // Shared pointers to hardware from main controller
    std::shared_ptr<L298> driver_;
    std::shared_ptr<ADC0832> adc0_;
    std::shared_ptr<ADC0832> adc1_;

    // Helper functions for ADC reading
    uint8_t adcPosition_() const;
    uint8_t adcCurrent_() const;
    uint8_t adcForce_() const;
};

#endif // MOTORCONTROLLER_H
