#include "l298.h"

// Output pins on GPIO13 and GPIO19
GPIO::DigitalOut L298::in1(13), L298::in2(19);
// PWM output on GPIO26 (soft PWM, 10kHz/20 =~ 500Hz, initial value 0)
GPIO::PWMOut L298::en(26,20,0);

L298& L298::buildL298() {
    static L298 driver;
    return driver;
}
L298::L298() {
    GPIO::GPIOBase::force_full_mapping();
}
void L298::setRatio(unsigned int r) {
    if (r == 0) {
        en.set_ratio(0);
        in1.off();
        in2.off();
        return;
    }
    if (r <= 20) en.set_ratio(r);
}
void L298::setDirection(unsigned int d) {
    if (d) {
        // Waits for securely eliminating shoot through
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
