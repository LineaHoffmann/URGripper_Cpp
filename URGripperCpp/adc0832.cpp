#include <cppgpio.hpp>
#include "adc0832.h"
#include <stdexcept>

ADC0832::ADC0832(unsigned int n, unsigned int uwait) :
    dout_((validate(n) == 0 ? 17 : 9),GPIO::GPIO_PULL::OFF,std::chrono::microseconds(10),std::chrono::microseconds(5)),
    din_((validate(n) == 0 ? 27 : 11)),
    clk_((validate(n) == 0 ? 22 : 5)),
    cs_((validate(n) == 0 ? 10 : 6)),
    uwait_(uwait/2){}

unsigned int ADC0832::readADC(unsigned int channel) {
    validate(channel);
    // Total function takes 2 setup + 8 read + 2*5us
    // Start clock low
    clk_.off();
    // Reset with CS high -> low
    clock(10);
    // Startbit HIGH
    din_.on();
    // Clock 1
    clock(uwait_);
    // SGL bit HIGH -> Two single ended ADCs
    din_.on();
    // Clock 2
    clock(uwait_);
    // Select if channel 0 or 1 on ADC
    if (channel % 2 == 0) din_.on();
    else din_.off();
    for (int i = 0; i < 8; ++i) {
        // Loop for 8 bits
        // Clock once to advance
        clock(uwait_);
        // Shift input variable left
        rd_ <<= 1;
        // Add 1 if DOUT is high
        if (dout_.get_state()) rd_ |= 0x1;
    }
    // Reset CS
    cs_.on();
    // Return read value
    return rd_;
}

unsigned int ADC0832::validate(unsigned int n) {
    // If n is anything other than 0 or 1, it throws runtime error
    if (!(n == 0 || n == 1)) throw std::invalid_argument("Only channels 0 and 1 are available.");
    return n;
}
void ADC0832::clock(unsigned int c) {
    clk_.on();
    std::this_thread::sleep_for(std::chrono::microseconds(c));
    clk_.off();
    std::this_thread::sleep_for(std::chrono::microseconds(c));
}
