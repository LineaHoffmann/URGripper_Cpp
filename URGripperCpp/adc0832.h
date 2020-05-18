#ifndef ADC0832_H
#define ADC0832_H

#include <cppgpio/gpio.hpp>
#include <cppgpio/buttons.hpp>
#include <cppgpio/output.hpp>

class ADC0832
{
public:
    ADC0832(uint8_t n, uint32_t uwait = 50);
    // Read functions
    uint8_t readADC(uint8_t channel);
    uint8_t readADC(uint8_t channel, uint8_t samples);

private:
    // We cant allow copies of hardware objects
    ADC0832(const ADC0832&) = delete;
    ADC0832& operator=(const ADC0832&) = delete;

    // 4 I/O pins for the communication
    GPIO::DigitalIn dout_;
    GPIO::DigitalOut din_, clk_, cs_;

    // Validation of ADC choices
    uint8_t validate(uint8_t);

    // Clock cycle
    uint32_t uwait_;

    // Storage of last reading
    uint8_t rd_;
};

#endif // ADC0832_H
