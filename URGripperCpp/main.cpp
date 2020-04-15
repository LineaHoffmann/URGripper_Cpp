#include <chrono>
#include <memory>       // For C++ smart pointers
#include <ncurses.h>    // For console GUI formatting
#include <vector>
#include <sstream>      // For string stream manipulation
#include <iomanip>      // As above

//#include <boost/asio.hpp>

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
 * @brief init draw State window
 */
void initDrawState() {
    // This only draws the labels
    // Writing ADC labels
    // Labels to be corrected when final
    mvwprintw(wState.win,2,2,"ADC READING");
    mvwprintw(wState.win,2,17,"VALUE");
    mvwprintw(wState.win,3,2,"ADC 0,0:");
    mvwprintw(wState.win,4,2,"ADC 0,1:");
    mvwprintw(wState.win,5,2,"ADC 1,0:");
    mvwprintw(wState.win,6,2,"ADC 1,1:");

    // Writing error code labels
    mvwprintw(wState.win,8,2,"Error Codes");
    mvwprintw(wState.win,10,2,"ADC 0:");
    mvwprintw(wState.win,11,2,"ADC 1:");
    mvwprintw(wState.win,12,2,"Motor:");
    mvwprintw(wState.win,13,2,"Server:");
    mvwprintw(wState.win,14,2,"Power:");

    // Writing misc. labels
    mvwprintw(wState.win,wState.height-2,2,"Frame:");
}

/**
 *  @brief uint8_t to 3 digit wide zero padded std::string
 */
std::string uint8tostr(uint8_t num) {
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

/**
 * @brief Draw State window
 */
void drawState(std::shared_ptr<L298> driver,
               std::shared_ptr<ADC0832> adc0,
               std::shared_ptr<ADC0832> adc1,
               std::shared_ptr<MotorController> moCtrl,
               unsigned long long &fcount) {
// A few defines for better readability
// May change in final version
#define rdForce     adc0->readADC(0);
#define rdPosition  adc0->readADC(1);
#define rdCurrent   adc1->readADC(0);
#define rd3V3       adc1->readADC(1);

    // Get data from ADC pointers
    std::array<uint8_t, 4> tempADC{0};
    tempADC[0] = rdForce;
    tempADC[1] = rdPosition;
    tempADC[2] = rdCurrent;
    tempADC[3] = rd3V3;

    // Convert to char* string and write uint8_t style value
    // Padding with zeroes in front to make 3 digits
    for (uint i = 0; i < tempADC.size(); ++i) {
        mvwprintw(wState.win,3+static_cast<int>(i),17,uint8tostr(tempADC[i]).c_str());
    }

    // Get Motor Controller error code
    MOTOR_CONTROL_ERROR_CODE moCtrlErr = moCtrl->getState();
    // Print accordingly
    if (moCtrlErr == MOTOR_CONTROL_ERROR_CODE::ALL_OK) {
        mvwprintw(wState.win,12,17,"OK");
    } else {
        mvwprintw(wState.win,12,17,"ERR");
    }

    // Get TCP Server error code
    //
    //

    // Get 3.3V rail, measurement at exact 3.3V is 3.05V = 155-156;
    // Limit chosen as +-5% [147 - 164]
    if (tempADC[3] <= 164 && tempADC[3] >= 147) {
        mvwprintw(wState.win,14,17,"OK");
    } else {
        mvwprintw(wState.win,14,17,"ERR");
    }

    // Increment frame counter
    fcount++;
    mvwprintw(wState.win, wState.height - 2, 9, std::to_string(fcount).c_str());
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

    // Frame counter for State view
    unsigned long long fcount{0};
    // Initalizing State window labels
    initDrawState();

    // Program loop
    // If user presses x, loop exits
    // Each getch() hangs system for t = timeout
    while (getch() != 'x') {
        // This is the freerunning part of the loop

        // This part handles State window drawing
        while (std::chrono::steady_clock::now() > next) {
            next += Framerate{1};
            drawState(driverPtr,adc0Ptr,adc1Ptr,motorControlPtr,fcount);
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
