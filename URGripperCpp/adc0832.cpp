#include <stdexcept>
#include <cmath>

#include "adc0832.h"

ADC0832::ADC0832(uint8_t n, uint32_t uwait) :
    // Select pins based on n
    dout_((validate(n) == 0 ? 17 : 9),GPIO::GPIO_PULL::OFF,std::chrono::microseconds(10),std::chrono::microseconds(5)),
    din_((validate(n) == 0 ? 27 : 11)),
    clk_((validate(n) == 0 ? 22 : 5)),
    cs_((validate(n) == 0 ? 10 : 6)),
    uwait_(uwait/2) {
    // Start all outputs low
    din_.off();clk_.off();cs_.off();
}
uint8_t ADC0832::readADC(uint8_t channel) {
    validate(channel);
    rd_ = 0; // Reset from previous reading    
    // Start clock low
    clk_.off();
    // Reset with CS high -> low
    cs_.on();
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    cs_.off();
    // Startbit HIGH
    din_.on();
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.on(); // Clock rising 1
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.off(); // Clock falling 1
    din_.on(); // 1 = SGL | 0 = DIF
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.on(); // Clock 2 rising edge
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.off(); // Clock 2 falling edge
    if (channel % 2 == 0) din_.on(); // Select if channel 0 or 1
    else din_.off();
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.on(); // Clock 3 rising edge
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
    clk_.off(); //Clock 3 falling edge
    std::this_thread::sleep_for(std::chrono::microseconds(uwait_));

    // Data starts here.
    // First read bit is not important, but useable for error check
    // If read bit is low, good, if it is tristate or high, bad.
//    unsigned int err = dout_.get_state();
//    if (err) return 0;

    for (int i = 0; i < 8; ++i) {
        clk_.on(); // Clock (i + 4) rising edge
        std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
        clk_.off(); // Clock (i + 4) falling edge
        std::this_thread::sleep_for(std::chrono::microseconds(uwait_));
        rd_ <<= 1; // Shift rd
        dout_.triggered(); // get_state doesn't work without calling this too? It's a bug.
        if (dout_.get_state()) {
            rd_ |= 0x1; // Add one to rd_ if input if high
        }
    }
    return rd_;
}
uint8_t ADC0832::readADC(uint8_t channel, uint8_t samples) {
    if (samples <= 0) return 0;
    uint16_t rdavg{0};
    for (uint8_t i = 0; i < samples; ++i) {
        rdavg += readADC(channel);
    }
    return rd_ = static_cast<uint8_t>(rdavg / samples);
}
uint8_t ADC0832::validate(uint8_t n) {
    // If n is anything other than 0 or 1, it throws runtime error
    if (!(n == 0 || n == 1)) throw std::invalid_argument("Only channels 0 and 1 are available.");
    return n;
}
