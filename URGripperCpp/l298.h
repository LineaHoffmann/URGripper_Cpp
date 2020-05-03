#ifndef L298_H
#define L298_H

#include <cppgpio/output.hpp>


class L298
{
public:
    static L298& buildL298();
    void off();
    void setRatio(unsigned int r);
    void setDirection(unsigned int d);
private:
    L298();
    L298& operator=(const L298&);

    static GPIO::DigitalOut in1,in2;
    static GPIO::PWMOut en;
};
#endif // L298_H
