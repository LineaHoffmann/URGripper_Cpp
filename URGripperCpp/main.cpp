#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"
#include "consolegui.h"
#include "tcpserver.h"
#include "statecontroller.h"



int main() {
    // This line should be run if not running on R-Pi
    // Doesn't really seem to work tho?
/*#ifndef __arm__
    GPIO::GPIOBase::simulation(true);
#endif*/

    // Time keeping for State Window framerate
    using Framerate = std::chrono::duration<std::chrono::steady_clock::rep,std::ratio<1,60>>;
    auto nextStateFrame = std::chrono::steady_clock::now() + Framerate{1};
    // Frame counter for State view
    unsigned int fcount{0};

    // Console GUI start
    ConsoleGUI& gui = ConsoleGUI::Build();

    // This "stream" isn't perfect
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
    // ADCs can have clock cycle period set in constructor (in us)
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
    TcpServer& server = TcpServer::Build(12321);
    //std::shared_ptr<TcpServer> serverPtr = std::make_shared<TcpServer>(server);
    //gui.AddComponent(serverPtr);
    //serverPtr->Start();
    server.Start();
    gui << "TCP Server starting on port 12321";

    //Statecontroller
    statecontroller stateControl;
    stateControl.AddMotorcontroller(motorControlPtr);

    while (1) {
        // This is the freerunning program loop
        // Checking for user inputs
        int userInput = getch();
        if (userInput == 'x') {
            // X is meant to quit the program
            // Cannot be handled in switch
            // because we'd have to double-break
            break;
        }
        switch (userInput) {
        // For testing
        case 'w':
            gui << "Moving to 50mm";
            motorControlPtr->move(50,0);
            break;
        case 'a':
            gui << "Moving to 40mm";
            motorControlPtr->move(40,0);
            break;
        case 's':
            gui << "Moving to 25mm and force 20?";
            motorControlPtr->move(25,20);
            break;
        case 'd':
            gui << "Moving to 5mm and force 20?";
            motorControlPtr->move(5,24);
            break;

        }

        // Get message from server object
        std::string msg = server.GetData();
        if (msg != "") {
            // Statecontroller "setup"
            stateControl.readCommand(msg);
            // Print message to console gui
            msg.insert(0,"Client: ");
            gui << msg;
            // Execute the state controller and send the return value
            server.SetReply(stateControl.executeCommand());
        }

        // This part handles State window drawing
        if (std::chrono::steady_clock::now() > nextStateFrame) {
            nextStateFrame += Framerate{1};
            gui.DrawState(fcount++);
        }
    }
    gui << "Attempting to close TCP server...";
    server.Stop();
    gui << "Server closed";
    gui << "End reached";
    return 0;
}
