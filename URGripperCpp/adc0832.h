#ifndef ADC0832_H
#define ADC0832_H

#include <cppgpio.hpp>

class ADC0832
{
public:
    ADC0832(unsigned int n, unsigned int uwait);
    unsigned int readADC(unsigned int channel);
private:
    GPIO::DigitalIn dout_;
    GPIO::DigitalOut din_, clk_, cs_;
    unsigned int validate(unsigned int);
    unsigned int uwait_;
    unsigned int rd_{0};
    void clock(unsigned int);
};

#endif // ADC0832_H
