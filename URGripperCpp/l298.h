#ifndef L298_H
#define L298_H
#include <cppgpio.hpp>

class L298
{
public:
    L298();
    void off();
    void setRatio(unsigned int r);
    void setDirection(unsigned int d);
private:
    static GPIO::DigitalOut in1,in2;
    static GPIO::PWMOut en;
};
#endif // L298_H
