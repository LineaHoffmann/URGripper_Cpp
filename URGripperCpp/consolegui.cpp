#include "consolegui.h"
#include <algorithm> // For some search and destroy in std::string

/**
 * @brief Static function for building static singleton object
 */
ConsoleGUI& ConsoleGUI::Build() {
    static ConsoleGUI gui;
    return gui;
}
/**
 * @brief GUI Destructor
 */
ConsoleGUI::~ConsoleGUI() {
    // The destructor needs to clean up after ncurses
    delwin(state_window_.win);
    delwin(log_window_.win);
    endwin();
}
/**
 * @brief GUI Constructor
 */
ConsoleGUI::ConsoleGUI() {
    // My default xterm window size matches this
    state_window_.height = 24;
    state_window_.width = 40;
    log_window_.height = 24;
    log_window_.width = 40;

    initscr();      // Screen initializer
    noecho();       // No echo of user inputs
    cbreak();       // Place break after each input ??
    curs_set(0);    // Cursor be gone

    timeout(0);     // No waiting
    refresh();      // First refresh

    // Left side window to display the log output
    log_window_.win = newwin(log_window_.height,log_window_.width,0,0);
    box(log_window_.win,0,0);
    mvwprintw(log_window_.win,0,13,"Controller Log");
    wrefresh(log_window_.win);

    // Right side window to display the current state of affairs
    state_window_.win = newwin(state_window_.height,state_window_.width,0,state_window_.width);
    box(state_window_.win,0,0);
    mvwprintw(state_window_.win,0,12,"Controller State");
    wrefresh(state_window_.win);

    // This draws the labels for the State window
    // Writing ADC labels
    // todo: Labels to be corrected when final
    mvwprintw(state_window_.win,2,2,"ADC READING");
    mvwprintw(state_window_.win,2,17,"VALUE");
    mvwprintw(state_window_.win,3,2,"M.curr:");
    mvwprintw(state_window_.win,4,2,"FFB:");
    mvwprintw(state_window_.win,5,2,"POS:");
    mvwprintw(state_window_.win,6,2,"3.3V:");

    // Writing error code labels
    mvwprintw(state_window_.win,8,2,"Error Codes");
    mvwprintw(state_window_.win,9,2,"Motor:");
    mvwprintw(state_window_.win,10,2,"Server:");
    mvwprintw(state_window_.win,11,2,"Power:");

    // Position and force labels
    mvwprintw(state_window_.win,14,2,"Pos [mm]:");
    mvwprintw(state_window_.win,15,2,"Force [kg]:");

    // Writing frame counter label
    mvwprintw(state_window_.win,state_window_.height-2,2,"Frame:");

}
/**
 * @brief Adding MotorController shared pointer
 */
void ConsoleGUI::AddComponent(std::shared_ptr<MotorController> ptr) {
    if (motor_controller_ptr_ == nullptr) motor_controller_ptr_ = ptr;
    return;
}
/**
 * @brief Adding ADC0832 shared pointer
 */
void ConsoleGUI::AddComponent(std::shared_ptr<ADC0832> ptr) {
    if (adc_0_ptr_ == nullptr) {adc_0_ptr_ = ptr; return;}
    if (adc_1_ptr_ == nullptr) {adc_1_ptr_ = ptr;} return;
}
/**
 * @brief Adding TCP server shared pointer
 */
void ConsoleGUI::AddComponent(std::shared_ptr<TcpServer> ptr) {
    if (server_ptr_ == nullptr) {server_ptr_ = ptr;} return;
}
/**
 * @brief Redraws the data in the state window
 */
void ConsoleGUI::DrawState(unsigned long long frame) {
    // A few defines for better readability
    // May change in final version
#define rd_current   adc_0_ptr_->readADC(0);
#define rd_force     adc_0_ptr_->readADC(1);
#define rd_position  adc_1_ptr_->readADC(0);
#define rd_3v3       adc_1_ptr_->readADC(1);

    // Get data from ADC pointers
    std::array<uint8_t, 4> temp_adc{0};
    temp_adc[0] = rd_current;
    temp_adc[1] = rd_force;
    temp_adc[2] = rd_position;
    temp_adc[3] = rd_3v3;

    // Convert to char* type string and write uint8_t style value
    // Padding with zeroes in front to make 3 digits
    for (uint i = 0; i < temp_adc.size(); ++i) {
        mvwprintw(state_window_.win,3+static_cast<int>(i),17,Uint8ToStr(temp_adc[i]).c_str());
    }

    // Get Motor Controller error code
    MOTOR_CONTROL_ERROR_CODE moCtrlErr = motor_controller_ptr_->getState();
    // Print accordingly
    if (moCtrlErr == MOTOR_CONTROL_ERROR_CODE::ALL_OK) {
        mvwprintw(state_window_.win,9,17,"OK ");
    } else {
        mvwprintw(state_window_.win,9,17,"ERR");
    }

    // todo: Get TCP Server error code
    // It doesn't make one, and we have no access to it
    // Space is reserved here tho
    mvwprintw(state_window_.win,10,17,"IDK");

    // Get 3.3V rail, measurement at exact 3.3V is 3.05V = 155-156;
    // Limit chosen as +-5% [147 - 164]
    if (temp_adc[3] <= 164 && temp_adc[3] >= 147) {
        mvwprintw(state_window_.win,11,17,"OK ");
    } else {
        mvwprintw(state_window_.win,11,17,"ERR");
    }

    // Print calculated position in mm
    double pos = motor_controller_ptr_->getPosition();
    mvwprintw(state_window_.win, 14, 17, std::to_string(pos).c_str());
    // Print calculated force in kg
    double force = motor_controller_ptr_->getForce();
    mvwprintw(state_window_.win, 15, 17, std::to_string(force).c_str());

    // Print frame counter
    mvwprintw(state_window_.win, state_window_.height - 2, 9, std::to_string(frame).c_str());
    // Refresh screen with updates
    wrefresh(state_window_.win);
    return;
}
/**
 * @brief Converts unint8_t to 3-digit 0-padded string
 */
std::string ConsoleGUI::Uint8ToStr(const uint8_t num) {
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}
/**
 * @brief Helper function for writing strings to the log window
*/
void ConsoleGUI::LogHelper(std::string str) {
    // todo: newline or return chars break the logger
    // For now we just remove them.
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    int width = ConsoleGUI::log_window_.width-2;
    // If string is too long, cut beginning and push to buffer
    if (str.size() > static_cast<uint>(width)) {
        while (static_cast<int>(str.size()) > width) {
            std::string tstr = str.substr(0,static_cast<uint>(width));
            str.erase(0,static_cast<uint>(width));
            ConsoleGUI::log_window_.buf.addNew(tstr);
        }
    }
    // Push new (or end of long) log string to buffer
    ConsoleGUI::log_window_.buf.addNew(str);
    // Print for entire window size / buffer
    // Create and write line of enough spaces to erase previously displayed line
    // Empty line made here because all lines are equal width
    std::string ts{};
    for (int j = 0; j < width; ++j) {
        ts += " ";
    }
    for (uint i = 0; i < ConsoleGUI::log_window_.buf.size(); ++i) {
        // Print empty line
        mvwprintw(ConsoleGUI::log_window_.win,static_cast<int>(i+1),1,ts.c_str());
        // Print actual line
        mvwprintw(ConsoleGUI::log_window_.win,static_cast<int>(i+1),1,ConsoleGUI::log_window_.buf.get(i).c_str());
    }
    // Refresh the log window
    wrefresh(ConsoleGUI::log_window_.win);
    return;
}
