#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <mutex>    // For thread safe read/writes
#include <queue>    // For FIFO buffer storage
#include <thread>   // Multithreading
#include <sstream>  // String streams

// The C++ Boost Asio library includes networking such as TCP
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>


#include "motorcontroller.h"

enum class SERVER_ERROR_CODE {
    RUNNING = 0x00,
    CLIENT_DISCONNECTED = 0x01,
    STOPPED = 0x03,
    UNINITIALIZED = 04,
    STOPPING = 0x05
};

class TcpServer
{
public:
    static TcpServer& Build(uint16_t port);
    //TcpServer();
    ~TcpServer();
    void Start();
    void Stop();
    std::string GetData();
    void SetReply(const std::string& msg);
    SERVER_ERROR_CODE GetState() const;

    void AddComponent(MotorController&);

private:
    TcpServer(uint16_t port);
    // Explicit delete of copy constructor and assignment operator
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void SpawnListener_();
    void AcceptHandler_(const boost::system::error_code& error);

    std::string Read_(boost::asio::ip::tcp::socket &socket);
    void Write_(boost::asio::ip::tcp::socket &socket, const std::string &str);

    enum SERVER_ERROR_CODE state_ = SERVER_ERROR_CODE::UNINITIALIZED;
    std::mutex lock_;
    std::queue<std::string> incoming_data_;
    std::string outgoing_data_{"WAIT"};

    uint16_t port_;
    std::thread thread_;
    bool tcp_stop_{false};

    // To send data to the Windows GUI, we rely on getting
    // the gripper state from the main controller
    MotorController *motor_controller_ptr_;
    // Buffer for adding to Windows gui logs
    std::queue<std::string> windows_gui_buffer_;
};
#endif // TCPSERVER_H
