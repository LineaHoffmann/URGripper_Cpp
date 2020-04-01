#include <iostream>
#include "adc0832.h"
#include "l298.h"

int main()
{
    std::cout << "===== Begin =====" << std::endl;

    // L298 should not be created more than once,
    // it will cause problems in the CppGPIO library
    // Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298 hb;
    std::cout << "H-Bridge succesfully initialized." << std::endl;

    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC Read is around 1600 us with waiting = 10
    // Sample time is 10 clock cycles + reset + c++ lines (I think)
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0,100);
    ADC0832 adc1(1,1000*1000);
    std::cout << "Both ADCs succesfully initialized." << std::endl;
    unsigned int rd;
    int n = 1000;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (n > 0) {
        rd = adc0.readADC(0);
        --n;
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time per read is: " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()/1000.0
              << " [us], at 1000 readings" << std::endl;

    std::cout << "===== End =====" << std::endl;
    return 0;
}
