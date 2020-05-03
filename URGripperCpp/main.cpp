#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"
#include "consolegui.h"
#include "tcpserver.h"

int main() {
    //GPIO::GPIOBase::simulation(true);
    // Time keeping for State Window framerate
    using Framerate = std::chrono::duration<std::chrono::steady_clock::rep,std::ratio<1,60>>;
    auto next = std::chrono::steady_clock::now() + Framerate{1};

    // Frame counter for State view
    unsigned long long fcount{0};
    // Console GUI start
    ConsoleGUI& gui = ConsoleGUI::Build();

    // This stream isn't perfect
    // gui << "" << ""; will make separate lines (for now?)
    gui << "Hello, Console here.";
    gui << "This stream wraps long lines for you, like a gentleman.";
    gui << "Press 'y' to continue. To stop again, press 'x'.";
    // getch is from ncurses.h
    // It check for keyboard char, doesn't wait
    while(getch() != 'y') {};
    // Creating L298 hardware object
    // Driver Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298& driver = L298::buildL298();
    std::shared_ptr<L298> driverPtr = std::make_shared<L298>(driver);
    gui << "L298 Driver initialized";

    // Creating two ADC0832 hardware objects
    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC default clock is 50us/ =~ about 0.7ms per read maybe? =~ around 20kHz
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0);
    ADC0832 adc1(1);
    std::shared_ptr<ADC0832> adc0Ptr = std::make_shared<ADC0832>(adc0);
    std::shared_ptr<ADC0832> adc1Ptr = std::make_shared<ADC0832>(adc1);
    gui.AddComponent(adc0Ptr);
    gui.AddComponent(adc1Ptr);
    gui << "ADCs initialized";

    // Creating Motor Controller object
    // This takes three shared pointers, 1xL298 + 2xADC0832
    MotorController& motorControl = MotorController::buildController(driverPtr,adc0Ptr,adc1Ptr);
    std::shared_ptr<MotorController> motorControlPtr = std::make_shared<MotorController>(motorControl);
    gui.AddComponent(motorControlPtr);
    gui << "Motor Controller initialized";

    // TCP Server
    TcpServer& server = TcpServer::Build(12321,12322);
    server.Start();
    gui << "TCP Server starting on port 12321";

    // Self testing? Periodic testing?
    //SystemTester& sysTest = SystemTester::buildTester(driverPtr,adc0Ptr,adc1Ptr);
    //std::unique_ptr<SystemTester> sysTestPtr = std::make_unique<SystemTester>(sysTest);
    //std::cout << "System test object initialized." << std::endl;

    // Program loop
    // If user presses x, loop exits
    while (getch() != 'x') {
        // This is the freerunning part of the loop
        std::string msg = server.GetData();
        if (msg != "") {
            msg.insert(0,"Client: ");
            gui << msg;
        }



        // This part handles State window drawing
        while (std::chrono::steady_clock::now() > next) {
            next += Framerate{1};
            gui.DrawState(fcount++);
        }
    }
    gui << "Attempting to close TCP servers...";
    server.Stop();
    gui << "Server closed";
    gui << "End reached";
    return 0;
}
