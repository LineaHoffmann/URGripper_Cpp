#include "tcpserver.h"

/*
 * Object gets created
 * Start() function gets called -> creates listener thread
 * Thread stores data in incoming_data_
 *
 * For replying to requests, data is loaded into outgoing_data_
 * This data gets transmitted back on next incoming request
 *
 * GetData doesn't return reference as object gets destroyed
*/

/**
 * @brief For building static reference object
*/
TcpServer& TcpServer::Build() {
    static TcpServer server;
    return server;
}
/**
 * @brief Destructor
*/
TcpServer::~TcpServer() {
    // Naive attempt to rejoin detached thread
    // todo: fix this
    listener_stop_ = true;
    gui_stop_ = true;
}
/**
 * @brief Constructor (does nothing)
*/
TcpServer::TcpServer() {}
/**
 * @brief Start detached listener thread
*/
void TcpServer::Start() {
    std::thread listener_thread_(&TcpServer::SpawnListener_,this);
    listener_thread_.detach();
    return;
}
/**
 * @brief Poll the data queue
*/
std::string TcpServer::GetData() {
    if (incoming_data_.empty()) return {};
    std::string str = incoming_data_.front();
    incoming_data_.pop();
    return str;
}
/**
 * @brief Set reply for next incoming connection
*/
void TcpServer::SetReply(const std::string &s) {outgoing_data_ = s;}
/**
 * @brief Listener function for Start()
*/
void TcpServer::SpawnListener_() {
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12321));
    boost::asio::ip::tcp::socket socket(io);
    // todo: this should not be infinite, see thread detaching
    // Ultimately causes an exception when debug ends
    while (1) {
        acceptor.accept(socket);
        std::lock_guard<std::mutex> lock(lock_);
        incoming_data_.push(Read_(socket));
        Write_(socket, outgoing_data_);
        outgoing_data_.erase();
        lock.~lock_guard();
        socket.close();
    }
    //return;
}
/**
 * @brief boost::asio type read from socket
*/
std::string TcpServer::Read_(boost::asio::ip::tcp::socket &socket) {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, "\n");
    std::string data = boost::asio::buffer_cast<const char*>(buffer.data());
    return data;
}
/**
 * @brief boost::asio type write to socket
*/
void TcpServer::Write_(boost::asio::ip::tcp::socket &socket, std::string &str) {
    const std::string msg = str;
    boost::asio::write(socket, boost::asio::buffer(msg));
    return;
}
