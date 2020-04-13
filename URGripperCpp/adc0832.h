#ifndef ADC0832_H
#define ADC0832_H

#include <cppgpio.hpp>

class ADC0832
{
public:
    ADC0832(uint8_t n, uint32_t uwait = 50);
    uint8_t readADC(uint8_t channel);
    uint8_t readADC(uint8_t channel, uint8_t samples);

private:
    GPIO::DigitalIn dout_;
    GPIO::DigitalOut din_, clk_, cs_;
    uint8_t validate(uint8_t);
    uint32_t uwait_;
    uint8_t rd_;
};

#endif // ADC0832_H
