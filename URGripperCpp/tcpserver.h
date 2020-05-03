#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <mutex>
#include <queue>
#include <thread>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>

class TcpServer
{
public:
    static TcpServer& Build(uint16_t port_robot, uint16_t port_gui);
    //TcpServer();
    ~TcpServer();
    void Start();
    void Stop();
    std::string GetData();
    void SetReply(const std::string& msg);

private:
    TcpServer(uint16_t port_robot, uint16_t port_gui);
    void SpawnRobotListener_();
    void RobotAcceptHandler_(const boost::system::error_code& error);
    void SpawnGuiListener_();
    void GuiAcceptHandler_(const boost::system::error_code& error);

    std::string Read_(boost::asio::ip::tcp::socket &socket);
    void Write_(boost::asio::ip::tcp::socket &socket, std::string &str);

    std::mutex lock_;
    std::queue<std::string> incoming_data_;
    std::string outgoing_data_{};

    uint16_t port_robot_;
    uint16_t port_gui_;

    std::thread robot_thread_;
    std::thread gui_thread_;
    bool robot_stop_{false};
    bool gui_stop_{false};
};

#endif // TCPSERVER_H
