#include <iostream>
#include <chrono>
#include <memory>

#include <QCoreApplication>

#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"
#include "myserver.h"


int main(int argc, char *argv[])
{
    std::cout << "Hej" << std::endl;
    std::cout << "===== Begin =====" << std::endl;

    // L298 should not be created more than once,
    // it will cause problems in the CppGPIO library
    // Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298 driver;
    std::shared_ptr<L298> driverPtr = std::make_shared<L298>(driver);
    std::cout << "H-Bridge succesfully initialized." << std::endl;

    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC default clock is 50us/ =~ about 0.7ms per read maybe? =~ around 20kHz
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0);
    ADC0832 adc1(1);
    std::shared_ptr<ADC0832> adc0Ptr = std::make_shared<ADC0832>(adc0);
    std::shared_ptr<ADC0832> adc1Ptr = std::make_shared<ADC0832>(adc1);
    std::cout << "Both ADCs succesfully initialized." << std::endl;

    // Motor Controller
    MotorController& motorControl = MotorController::buildController(driverPtr,adc0Ptr,adc1Ptr);
    std::unique_ptr<MotorController> motorControlPtr = std::make_unique<MotorController>(motorControl);
    std::cout << "Motor controller object seemingly initialized." << std::endl;
    // Self testing?

    // TCP Server Threading
    MyServer tcpServer;
    tcpServer.startServer();
    std::cout << "TCP Server started on port " << tcpServer.serverPort()
              << std::endl;

    // Program loop


    bool escFlag = false;
    std::string cmdStr{};
    while (!escFlag) {
        if (tcpServer.hasNewData()) {
            cmdStr = tcpServer.getNextData();
        }
        if (tcpServer.hasPendingConnections()) std::cout << "Pending connection" << std::endl;
        if (!cmdStr.empty()) {
            // Do something
            // Send response to UR Cap
            cmdStr.clear();
        }

//        motorControlPtr->getForce();
//        motorControlPtr->getPosition();

    };

    tcpServer.close();
    std::cout << "===== End =====" << std::endl;
    return 0;
}
