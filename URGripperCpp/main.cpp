#include <chrono>
#include <memory>       // For C++ smart pointers
#include <ncurses.h>    // For console GUI formatting
#include <vector>

#include <boost/asio.hpp>

#include "adc0832.h"
#include "l298.h"
#include "motorcontroller.h"


/**
 * @brief Circular string buffer class
 * Used for storing log and state window printouts
 */
class CircularBuffer {
public:
    CircularBuffer(uint size = 22) {
        size_ = size;
        data_.resize(size_);
    }
    void addNew (std::string &newData){
        if (data_.size() >= size_) {
            data_.erase(data_.begin());
        }
        data_.push_back(newData);
    }
    std::string get(uint index){
        if (index > size_) return nullptr;
        return data_.at(index);
    }
    uint size() {return size_;}
private:
    uint size_;
    std::vector<std::string> data_;
};

/**
 * @brief Struct for subwindows
 */
struct window {
    WINDOW* win = nullptr;
    int height;
    int width;
    CircularBuffer circBuf;
};

// Creating two subwindows
// One for logging, one for state view
static window wState;
static window wLog;

/**
 * @brief Creates the Console GUI
 */
void startConsole() {
    // Create two subwindows with set height and widths
    wState.height = 24;
    wState.width = 40;
    wLog.height = 24;
    wLog.width = 40;

    // Initialize the ncurses view
    initscr();
    // Stop displaying inputted chars
    noecho();
    // Set cursor invisible
    curs_set(0);
    // Set getch timeout in milliseconds
    timeout(0);
    // Show the window
    refresh();

    // Start Log window area
    wLog.win = newwin(wLog.height,wLog.width,0,0);
    box(wLog.win,0,0);
    mvwprintw(wLog.win,0,13,"Controller Log");
    wrefresh(wLog.win);

    // Start State window area
    wState.win = newwin(wState.height,wState.width,0,wLog.width);
    box(wState.win,0,0);
    mvwprintw(wState.win,0,12,"Controller State");
    wrefresh(wState.win);
}

/**
 * @brief Add new line to log window
 */
static void printLog(std::string s) {
    // If string is too long, cut it short
    if (static_cast<int>(s.size()) > wLog.width-2) {s.erase(s.begin()+wLog.width-2,s.end());}
    // Push new log string to buffer
    wLog.circBuf.addNew(s);
    // Print for entire window size / buffer
    // Create and write line of enough spaces to erase previously displayed line
    // Empty line made here because all lines are equal width
    std::string ts;
    for (int j = 0; j < wLog.width-2; ++j) {
        ts.append(" ");
    }
    for (uint i = 0; i < wLog.circBuf.size(); ++i) {
        mvwprintw(wLog.win,static_cast<int>(i+1),1,ts.c_str());
        // Print new line
        mvwprintw(wLog.win,static_cast<int>(i+1),1,wLog.circBuf.get(i).c_str());
    }
    // Refresh the log window
    wrefresh(wLog.win);
}

/**
 * @brief Draw State window
 */
void drawState(std::shared_ptr<L298> driver, std::shared_ptr<ADC0832> adc0, std::shared_ptr<ADC0832> adc1, std::shared_ptr<MotorController> moCtrl) {
    // Writing ADC labels
    mvwprintw(wState.win,2,2,"ADC Readings");
    mvwprintw(wState.win,3,2,"Force:    ");
    mvwprintw(wState.win,4,2,"Current:  ");
    mvwprintw(wState.win,5,2,"Position: ");
    mvwprintw(wState.win,6,2,"3.3V:     ");

    // Writing error code labels
    mvwprintw(wState.win,8,2,"Error Codes");
    mvwprintw(wState.win,9,2,"Motor Driver:  ");
    mvwprintw(wState.win,10,2,"ADC 0:         ");
    mvwprintw(wState.win,11,2,"ADC 1:         ");
    mvwprintw(wState.win,12,2,"Motor Control: ");
    mvwprintw(wState.win,13,2,"TCP Server:    ");

    // Get data from pointers
    uint8_t tempADC[4];
    tempADC[0] = adc0->readADC(0);
    tempADC[1] = adc0->readADC(1);
    tempADC[2] = adc1->readADC(0);
    tempADC[3] = adc1->readADC(1);
    mvwprintw(wState.win,3,12,"Error Codes");
    mvwprintw(wState.win,4,12,"Error Codes");
    mvwprintw(wState.win,5,12,"Error Codes");
    mvwprintw(wState.win,6,12,"Error Codes");

    MOTOR_CONTROL_ERROR_CODE moCtrlErr = moCtrl->getState();
    mvwprintw(wState.win,8,2,"Error Codes");



    // Refresh screen with updates
    wrefresh(wState.win);
    return;
}



int main() {
    //GPIO::GPIOBase::simulation(true);
    // Time keeping for State Window framerate
    using Framerate = std::chrono::duration<std::chrono::steady_clock::rep,std::ratio<1,60>>;
    auto next = std::chrono::steady_clock::now() + Framerate{1};

    // Initialize the console GUI
    startConsole();
    printLog("Console GUI started");


    // Creating L298 hardware object
    // Driver Enable pin has soft PWM at ~500Hz
    // PWM dutycycle range is [0,20] in integers
    L298& driver = L298::buildL298();
    std::shared_ptr<L298> driverPtr = std::make_shared<L298>(driver);
    printLog("L298 Driver initialized");

    // Creating two ADC0832 hardware objects
    // ADCs requires setting clock cycle period in constructor (in us)
    // ADC default clock is 50us/ =~ about 0.7ms per read maybe? =~ around 20kHz
    // Clocks outside 10kHz-400kHz are not guaranteed in datasheet
    ADC0832 adc0(0);
    ADC0832 adc1(1);
    std::shared_ptr<ADC0832> adc0Ptr = std::make_shared<ADC0832>(adc0);
    std::shared_ptr<ADC0832> adc1Ptr = std::make_shared<ADC0832>(adc1);
    printLog("ADCs initialized");

    // Creating Motor Controller object
    // This takes three shared pointers, 1xL298 + 2xADC0832
    MotorController& motorControl = MotorController::buildController(driverPtr,adc0Ptr,adc1Ptr);
    std::shared_ptr<MotorController> motorControlPtr = std::make_shared<MotorController>(motorControl);
    printLog("Motor Controller initialized");

    // Self testing? Periodic testing?
    //SystemTester& sysTest = SystemTester::buildTester(driverPtr,adc0Ptr,adc1Ptr);
    //std::unique_ptr<SystemTester> sysTestPtr = std::make_unique<SystemTester>(sysTest);
    //std::cout << "System test object initialized." << std::endl;

    // TCP Server
    //MyServer tcpServer;

    // Program loop
    // If user presses x, loop exits
    // Each getch() hangs system for t = timeout
    while (getch() != 'x') {
        // This is the freerunning part of the loop

        // This part handles State window drawing
        while (std::chrono::steady_clock::now() > next) {
            next += Framerate{1};
            drawState(driverPtr,adc0Ptr,adc1Ptr,motorControlPtr);
        }
    }

    printLog("End reached");
    printLog("Press any button to stop");
    // Loop locks until keypress, to help see window before ending
    while(1) {
        if (getch() != -1) break;
    }
    // Clean up and close
    //tcpServer.close();
    delwin(wState.win);
    delwin(wLog.win);
    endwin();
    return 0;
}
