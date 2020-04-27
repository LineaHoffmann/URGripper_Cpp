#include "statecontroller.h"
#include "tcpserver.h"
#include "motorcontroller.h"
#include <iostream>

statecontroller::statecontroller()
{

}

/**
 * @brief statecontroller::readCommand
 * Takes a string from TCP get Data...
 * @param command
 * @return commandArray
 */
void statecontroller::readCommand(const std::string& command)
{
    std::string delimiter = ";";

    size_t position = 0;
    size_t previous = 0;
    std::string temp;
    std::array<std::string,5> commandArray;

    uint i = 0;
    while((position = command.find(delimiter,previous)) != std::string::npos)
    {
        temp = command.substr(previous,position-previous);
        previous = position+delimiter.length();
        commandArray[i++] = temp;
    }
    commandBuffer.beginningState = commandArray[0];
    commandBuffer.byForce = static_cast<bool>(std::stoi(commandArray[1])); //cast from
    commandBuffer.byDistance = static_cast<bool>(std::stoi(commandArray[2]));
    commandBuffer.forceValue = std::stoi(commandArray[3]);
    commandBuffer.distanceValue = std::stoi(commandArray[4]);

    return;
}

bool statecontroller::executeCommand()
{
    if(commandBuffer.beginningState == "ST")
    {
        return true;
    }

    if(commandBuffer.beginningState == "OP")
    {



    }

    if(commandBuffer.beginningState == "CL")
    {

    }
    return false;

}

void statecontroller::reply()
{


}

void statecontroller::AddMotorcontroller(std::shared_ptr<MotorController>)
{

}

