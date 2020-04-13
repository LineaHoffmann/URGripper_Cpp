#include <iostream>
#include <chrono>
//#include <memory>

#include "adc0832.h"
#include "l298.h"
//#include "motorcontroller.h"
//#include "myserver.h"


int main()
{
    std::cout << "Hej" << std::endl;
    std::cout << "===== Begin =====" << std::endl;

    // L298 should not be created more than once,
    // it will cause problems in the CppGPIO library
    // Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298 driver;
//    std::shared_ptr<L298> driverPtr = std::make_shared<L298>(driver);
    std::cout << "H-Bridge succesfully initialized." << std::endl;

    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC Read is around 1600 us with waiting = 10
    // Sample time is 10 clock cycles + reset + c++ lines (I think)
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0,100);
    ADC0832 adc1(1,100);
//    std::shared_ptr<ADC0832> adc0Ptr = std::make_shared<ADC0832>(adc0);
//    std::shared_ptr<ADC0832> adc1Ptr = std::make_shared<ADC0832>(adc1);
    std::cout << "Both ADCs succesfully initialized." << std::endl;

    // Motor Controller
//    MotorController& motorControl = MotorController::buildController(driverPtr,adc0Ptr,adc1Ptr);
//    std::unique_ptr<MotorController> motorControlPtr = std::make_unique<MotorController>(motorControl);
    std::cout << "Motor controller object seemingly initialized." << std::endl;

    // TCP Server Threading
//    MyServer tcpServer;
//    tcpServer.startServer();

    // Program loop
//    bool escFlag = false;
//    while (!escFlag) {
//        if (tcpServer.hasPendingConnections()) {
//
//        }
//    }


    std::cout << "===== End =====" << std::endl;
    return 0;
}
