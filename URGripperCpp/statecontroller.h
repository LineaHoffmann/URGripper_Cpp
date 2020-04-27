#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H
#include <motorcontroller.h>
#include <string>
#include <memory>
#include <array>

/**
 * @brief The statecontroller class
 * Når vi åbner altid kun sende distance, stadige sende force bare sæt den ligmed 0
 * Når vi lukker kan sende begge, blot sende hvad der modtages.
 */

class statecontroller
{
    struct CommandBuffer
    {
        std::string beginningState;
        bool byForce;
        bool byDistance;
        int forceValue;
        int distanceValue;

    };

public:
    statecontroller();

    //return 1 succes, 0 fail
    void readCommand(const std::string& command);
    bool executeCommand();
    void reply();

    //shared pointer
//    void AddMsg(std::shared_ptr<TcpServer> ptr);
    void AddMotorcontroller(std::shared_ptr<MotorController>);


private:
     enum command{open = 1, close = 2, status = 3};
     CommandBuffer commandBuffer;
     std::shared_ptr<MotorController> motorController_ptr_;


    //shared pointer
   // std::shared_ptr<TcpServer> tcp_server_ptr_;





};

#endif // STATECONTROLLER_H
