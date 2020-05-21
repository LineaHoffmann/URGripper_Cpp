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
 * @brief For building static singleton
*/
TcpServer& TcpServer::Build(uint16_t port) {
    static TcpServer server(port);
    return server;
}
/**
 * @brief Destructor?
*/
TcpServer::~TcpServer() {}
/**
 * @brief Constructor
*/
TcpServer::TcpServer(uint16_t port) :
    port_{port} {
}

/**
 * @brief
*/
void TcpServer::AddComponent(MotorController& controller) {
    motor_controller_ptr_ = &controller;
}

/**
 * @brief Start listener TCP thread
*/
void TcpServer::Start() {
    thread_ = std::thread(&TcpServer::SpawnListener_,this);
    return;
}
/**
 * @brief Stop listener TCP thread
*/
void TcpServer::Stop() {
    // Join the extra thread
    std::unique_lock<std::mutex> lock(lock_,std::try_to_lock);
    state_ = SERVER_ERROR_CODE::STOPPING;
    tcp_stop_ = true;
    lock.~unique_lock();
    thread_.join();
    state_ = SERVER_ERROR_CODE::STOPPED;
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
void TcpServer::SpawnListener_() {
    // Starting required IO services
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_));
    boost::asio::ip::tcp::socket socket(io);
    // Setup
    acceptor.non_blocking(true);
    boost::system::error_code err_code;
    acceptor.listen();
    // Check tcp_stop_
    std::unique_lock<std::mutex> lock(lock_, std::try_to_lock);
    state_ = SERVER_ERROR_CODE::RUNNING;
    bool stopFlag = tcp_stop_;
    lock.unlock();
    while (!stopFlag) {
        // Accept incomming request
        acceptor.accept(socket,err_code);
        if (err_code.value() == 11) {
            // Request failed bacause of no peer
            // This happens because accept is non-blocking
            // which we need to kill the thread when we want
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        } else if (err_code.value()) {
            // Error has happened during accept, logging as incoming data
            while (!lock.owns_lock()) lock.try_lock();
            incoming_data_.push("TCP Error: " + err_code.message());
            lock.unlock();
            continue;
        }
        // Read and write data
        std::string rdData;
        try {
            // Catching errors like disconnection
            rdData = Read_(socket);
        } catch (std::exception& e) {
            std::string str = "Error: ";
            str.append(e.what());
            incoming_data_.push(str);
        }
        //std::string rdData = Read_(socket);
        while (!lock.owns_lock()) lock.try_lock();
        if (rdData == "GUI") {
            // This is our Windows GUI asking for data
            // todo: handle this here
            // This object will need a full set of "stuff"
            // Construct a single string and use Write_ to send it
            // Delimiter = ';'
            // status, power, gripperdistance, clampingforce,
            // adc3v3, adcPot, adcMotor, adcForce, new log lines
            lock.unlock();
            std::array<uint8_t,4> adc_reads = (*motor_controller_ptr_).getADCs();
            MOTOR_CONTROL_ERROR_CODE mo_err = motor_controller_ptr_->getState();
            std::stringstream rtn_sstream;
            // Motor controller status
            if (mo_err == MOTOR_CONTROL_ERROR_CODE::ALL_OK) {
                rtn_sstream << "OK;";
            } else {
                rtn_sstream << "ERR;";
            }
            // 3.3V power status
            if (adc_reads.at(3) <= 156*1.05 && adc_reads.at(3) >= 155*0.95) {
                rtn_sstream << "OK;";
            } else {
                rtn_sstream << "ERR;";
            }
            // Gripper distance double
            rtn_sstream << motor_controller_ptr_->getPosition() << ";";
            // Clamping force double
            rtn_sstream << motor_controller_ptr_->getForce() << ";";
            // ADCs in 3.3v, pos, current, force order
            rtn_sstream << std::to_string(adc_reads.at(3)) << ";"
                        << std::to_string(adc_reads.at(0)) << ";"
                        << std::to_string(adc_reads.at(2)) << ";"
                        << std::to_string(adc_reads.at(1)) << ";";
            // Log lines?
            // This should only be the new lines, and should be handled elsewhere?
            rtn_sstream << incoming_data_.front();
            Write_(socket,rtn_sstream.str().c_str());
        } else {
            // If it isn't the GUI, we assume it's the robot
            // Write the data from outgoing_data_ and reset it
            Write_(socket, outgoing_data_);
            outgoing_data_ = "WAIT";
            // Log the incoming request to a buffer
            incoming_data_.push(rdData);
        }

        // Get new state of stop flag
        stopFlag = tcp_stop_;
        if (lock.operator bool()) lock.unlock();
        socket.close();
    }
    state_ = SERVER_ERROR_CODE::STOPPING;
    return;
}

/**
*
*/
SERVER_ERROR_CODE TcpServer::GetState() const {
    return state_;
}

/**
 * @brief boost::asio type read from socket
*/
std::string TcpServer::Read_(boost::asio::ip::tcp::socket &socket) {
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, "\0");
    std::string data = boost::asio::buffer_cast<const char*>(buffer.data());
    return data;
}
/**
 * @brief boost::asio type write to socket
*/
void TcpServer::Write_(boost::asio::ip::tcp::socket &socket, const std::string &str) {
    boost::asio::write(socket, boost::asio::buffer(str));
    return;
}
