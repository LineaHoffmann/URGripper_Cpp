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
    static TcpServer& Build(uint16_t port);
    //TcpServer();
    ~TcpServer();
    void Start();
    void Stop();
    std::string GetData();
    void SetReply(const std::string& msg);

private:
    TcpServer(uint16_t port);
    void SpawnListener_();
    void AcceptHandler_(const boost::system::error_code& error);

    std::string Read_(boost::asio::ip::tcp::socket &socket);
    void Write_(boost::asio::ip::tcp::socket &socket, const std::string &str);

    std::mutex lock_;
    std::queue<std::string> incoming_data_;
    std::string outgoing_data_{"WAIT"};

    uint16_t port_;
    std::shared_ptr<std::thread> thread_;
    bool tcp_stop_{false};
};

#endif // TCPSERVER_H
