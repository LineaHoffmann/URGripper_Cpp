#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"
#include "consolegui.h"
#include "tcpserver.h"
#include "statecontroller.h"



int main() {
    // This line should be run if not running on R-Pi
    // Doesn't really seem to work tho?
#ifndef __arm__
    GPIO::GPIOBase::simulation(true);
#endif

    // Time keeping for State Window framerate
    using Framerate = std::chrono::duration<std::chrono::steady_clock::rep,std::ratio<1,60>>;
    auto nextStateFrame = std::chrono::steady_clock::now() + Framerate{1};
    // Frame counter for State view
    unsigned int fcount{0};

    // Console GUI start
    ConsoleGUI& gui = ConsoleGUI::Build();

    // This "stream" isn't perfect
    // gui << "" << ""; will make separate lines (for now?)
    gui << "Hello, this is the logging console for the UR gripper.";
    gui << "Press 'y' to continue. To stop, press 'x'.";
    // getch is from ncurses.h
    // It check for keyboard char, doesn't wait
    while(getch() != 'y') {};
    // Creating L298 hardware object
    // Driver Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298& driver = L298::buildL298();
    gui << "L298 Driver initialized";

    // Creating two ADC0832 hardware objects
    // ADCs can have clock cycle period set in constructor (in us)
    // ADC default clock is 50us/ =~ about 0.7ms per read maybe? =~ around 20kHz
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0);
    ADC0832 adc1(1);
//    gui.AddComponent(adc0);
//    gui.AddComponent(adc1);
    gui << "ADCs initialized";

    // Creating Motor Controller object
    // This takes three shared pointers, 1xL298 + 2xADC0832
    MotorController& motorControl = MotorController::buildController(driver,adc0,adc1);
    gui.AddComponent(motorControl);
    gui << "Motor Controller initialized";

    // TCP Server
    TcpServer& server = TcpServer::Build(12321);
    gui.AddComponent(server);
    server.AddComponent(motorControl);
    server.Start();
    gui << "TCP Server starting on port 12321";

    //Statecontroller
    statecontroller stateControl;
    stateControl.AddMotorcontroller(motorControl);

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
            motorControl.move(50,0);
            break;
        case 'a':
            gui << "Moving to 40mm";
            motorControl.move(40,0);
            break;
        case 's':
            gui << "Moving to 25mm and force 20?";
            motorControl.move(25,20);
            break;
        case 'd':
            gui << "Moving to 5mm and force 20?";
            motorControl.move(5,24);
            break;
        case '1':
            gui << "Moving to 20mm, 0";
            motorControl.move(20,0);
            break;
        case '2':
            gui << "Moving to 20mm, 2";
            motorControl.move(20,2);
            break;
        case '3':
            gui << "Moving to 20mm, 4";
            motorControl.move(20,4);
            break;
        case '4':
            gui << "Moving to 20mm, 6";
            motorControl.move(20,6);
            break;
        }

        // Get message from server object
        std::string msg = server.GetData();
        if (msg != "") {
            // Statecontroller "setup"
            stateControl.readCommand(msg);
            if (msg != "ST;") {
                // Print message to console gui
                msg.insert(0,"Client: ");
                gui << msg;
            }

            // Execute the state controller and send the return value
            std::string str = stateControl.executeCommand();
            gui << str;
            server.SetReply(str);
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
