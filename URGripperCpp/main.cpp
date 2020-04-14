#include <iostream>
#include <chrono>
#include <memory>

//#include <QCoreApplication>

#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"


int main()
{
    std::cout << "Hej" << std::endl;
    std::cout << "===== Begin =====" << std::endl;

    // Creating L298 hardware object
    // Driver Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298& driver = L298::buildL298();
    std::shared_ptr<L298> driverPtr = std::make_shared<L298>(driver);
    std::cout << "H-Bridge succesfully initialized." << std::endl;

    // Creating two ADC0832 hardware objects
    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC default clock is 50us/ =~ about 0.7ms per read maybe? =~ around 20kHz
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0);
    ADC0832 adc1(1);
    std::shared_ptr<ADC0832> adc0Ptr = std::make_shared<ADC0832>(adc0);
    std::shared_ptr<ADC0832> adc1Ptr = std::make_shared<ADC0832>(adc1);
    std::cout << "Both ADCs succesfully initialized." << std::endl;

    // Creating Motor Controller object
    // This takes three shared pointers, 1xL298 + 2xADC0832
    MotorController& motorControl = MotorController::buildController(driverPtr,adc0Ptr,adc1Ptr);
    std::unique_ptr<MotorController> motorControlPtr = std::make_unique<MotorController>(motorControl);
    std::cout << "Motor controller object initialized." << std::endl;

    // Self testing? Periodic testing?
    //SystemTester& sysTest = SystemTester::buildTester(driverPtr,adc0Ptr,adc1Ptr);
    //std::unique_ptr<SystemTester> sysTestPtr = std::make_unique<SystemTester>(sysTest);
    //std::cout << "System test object initialized." << std::endl;

    // TCP Server
    //MyServer tcpServer;


    // Program Loop
    bool escFlag = false;
    while (!escFlag) {
        escFlag = true;
    };

    //tcpServer.close();
    std::cout << "===== End =====" << std::endl;
    return 0;
}
