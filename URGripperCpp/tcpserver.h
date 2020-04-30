#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <mutex>
#include <queue>
#include <thread>

#include <boost/asio.hpp>

class TcpServer
{
public:
    static TcpServer& Build();
    //TcpServer();
    ~TcpServer();
    void Start();
    std::string GetData();
    void SetReply(const std::string& msg);

private:
    TcpServer();
    [[noreturn]] void SpawnListener_();

    std::string Read_(boost::asio::ip::tcp::socket &socket);
    void Write_(boost::asio::ip::tcp::socket &socket, std::string &str);

    std::mutex lock_;
    std::queue<std::string> incoming_data_;
    std::string outgoing_data_{"WAIT"};
    std::thread listener_thread_;
    bool listener_stop_{false};
    std::thread gui_thread_;
    bool gui_stop_{false};
};

#endif // TCPSERVER_H
