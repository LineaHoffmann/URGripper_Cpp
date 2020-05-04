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
TcpServer& TcpServer::Build(uint16_t port_robot, uint16_t port_gui) {
    static TcpServer server(port_robot, port_gui);
    return server;
}
/**
 * @brief Destructor?
*/
TcpServer::~TcpServer() {}
/**
 * @brief Constructor
*/
TcpServer::TcpServer(uint16_t port_robot,uint16_t port_gui) :
    port_robot_{port_robot},
    port_gui_{port_gui} {}
/**
 * @brief Start listener and gui TCP threads
*/
void TcpServer::Start() {
    robot_thread_ = std::thread(&TcpServer::SpawnRobotListener_,this);
    gui_thread_ = std::thread(&TcpServer::SpawnGuiListener_,this);
    return;
}
/**
 * @brief Stop listener and gui TCP threads
*/
void TcpServer::Stop() {
    // Join the two extra threads
    std::unique_lock<std::mutex> lock(lock_, std::try_to_lock);
    while(!lock.owns_lock()) lock.try_lock();
    gui_stop_ = true;
    robot_stop_ = true;
    lock.unlock();
    gui_thread_.join();
    robot_thread_.join();
    return;
}
/**
 * @brief Poll the data queue
*/
std::string TcpServer::GetData() {
    std::unique_lock<std::mutex> lock(lock_);
    while(!lock.owns_lock()) lock.try_lock();
    if (incoming_data_.empty()) return {};
    std::string str = incoming_data_.front();
    incoming_data_.pop();
    return str;
}
/**
 * @brief Set reply for next incoming connection
*/
void TcpServer::SetReply(const std::string &s) {
    std::unique_lock<std::mutex> lock(lock_);
    while(!lock.owns_lock()) lock.try_lock();
    outgoing_data_ = s;
}
/**
 * @brief Listener function for Start()
 *
 * If client disconnects unexpectedly after accept, it crashes
 * todo: maybe catch this
*/
void TcpServer::SpawnRobotListener_() {
    // Starting required IO services
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_robot_));
    boost::asio::ip::tcp::socket socket(io);
    // Setup
    acceptor.non_blocking(true);
    boost::system::error_code err_code;
    acceptor.listen();
    // Check robot_stop_
    std::unique_lock<std::mutex> lock(lock_, std::try_to_lock);
    bool stopFlag = robot_stop_;
    lock.unlock();
    while (!stopFlag) {
        // Accept incomming request
        acceptor.accept(socket,err_code);
        if (err_code.value() == 11) {
            // Request failed bacause of no peer
            // This happens because accept is non-blocking
            // which we need to shut the thread when we want
            while (!lock.owns_lock()) lock.try_lock();
            stopFlag = robot_stop_;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        if (err_code) {
            // Error has happened during accept, logging as incoming data
            while (!lock.owns_lock()) lock.try_lock();
            incoming_data_.push("TCP Error: " + err_code.message());
            stopFlag = robot_stop_;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        // Read and write data
        while (!lock.owns_lock()) lock.try_lock();
        incoming_data_.push(Read_(socket));
        Write_(socket, outgoing_data_);
        outgoing_data_ = "WAIT";
        stopFlag = robot_stop_;
        lock.unlock();
        socket.close();
    }
    return;
}
/**
 * @brief todo: make this
 */
void TcpServer::SpawnGuiListener_() {
    return;
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
