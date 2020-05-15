#include "l298.h"

// Output pins on GPIO13 and GPIO19
GPIO::DigitalOut L298::in1(13), L298::in2(19);
// PWM output on GPIO26 (soft PWM, 10kHz/20 =~ 500Hz, initial value 0)
GPIO::PWMOut L298::en(26,20,0);

/**
 * @brief L298::buildL298
 * @return L298 singleton reference
 */
L298& L298::buildL298() {
    static L298 driver;
    return driver;
}
L298::L298() {
    // This line should make hardware PWM possible but it does not.
    // Bug, probably because CppGPIO was written mainly for one of the older CPUs
    GPIO::GPIOBase::force_full_mapping();
}
void L298::setRatio(unsigned int r) {
    if (r == 0) {
        // If set to 0, all outputs low
        en.set_ratio(0);
        in1.off();
        in2.off();
        return;
    }
    // Keep the value within limits or ignore it?
    if (r <= 20) en.set_ratio(r);
}
void L298::setDirection(unsigned int d) {
    if (d) {
        // Waits are for eliminating shoot through
        in2.off();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        in1.on();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    } else {
        in1.off();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        in2.on();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}
