#ifndef CONSOLEGUI_H
#define CONSOLEGUI_H

#include <chrono>
#include <memory>
#include <ncurses.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include "motorcontroller.h"
#include "adc0832.h"
#include "tcpserver.h"

class ConsoleGUI
{
    /**
     * @brief Circular string buffer class
     * Used for storing log and state window printouts
     */
    struct CircularBuffer {
    public:
        CircularBuffer(uint size = 22) {
            size_ = size;
            data_.resize(size_);
        }
        void addNew(const std::string &newData){
            data_.erase(data_.begin());
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
     * @brief Struct for window
     */
    struct Window {
        WINDOW* win = nullptr;
        int height;
        int width;
        ConsoleGUI::CircularBuffer buf;
    };

public:
    // For building the console object
    static ConsoleGUI& Build();
    // Explicit delete of copy constructor and assignment operator
    ConsoleGUI(const ConsoleGUI&) = delete;
    ConsoleGUI& operator=(const ConsoleGUI&) = delete;
    // Destructor for cleaning after ncurses
    virtual ~ConsoleGUI();
    // Adders for functionality blocks
    void AddComponent(MotorController&);
    //void AddComponent(ADC0832&);
    void AddComponent(TcpServer&);
    // State window updater function
    void DrawState(unsigned long long);
    // For sending strings to the log window
    friend ConsoleGUI& operator<<(ConsoleGUI& gui, const std::string str) {gui.LogHelper(str); return gui;}

private:
    // Private constructor
    ConsoleGUI();
    // Helper funtion for writing to log window
    void LogHelper(std::string);
    // uint8_t to string conversion
    std::string Uint8ToStr(const uint8_t);
    // Windows made in construction
    Window state_window_;
    Window log_window_;
    // Storage for the pointers given during controller startup
    MotorController *motor_controller_ptr_;
    //ADC0832 *adc_0_ptr_;
    //ADC0832 *adc_1_ptr_;
    TcpServer *server_ptr_;
};
#endif // CONSOLEGUI_H
