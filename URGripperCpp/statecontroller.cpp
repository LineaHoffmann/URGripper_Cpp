#include "statecontroller.h"
#include "tcpserver.h"
#include "motorcontroller.h"
#include "consolegui.h"
#include <iostream>


statecontroller::statecontroller()
{
}

/**
 * @brief statecontroller::readCommand
 * Takes a string from TcpServer getData(), finds delimiter and seperates the command,
 * then puts into commandArray and returns the array.
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
    commandBuffer.beginningState = commandArray[0]; //hvis den er andet end OP el. CL. Så skal resten af commandBuffer Nulls.
    if (commandArray[0] == "OP" || commandArray[0] == "CL")
    {
        commandBuffer.byForce = static_cast<bool>(std::stoi(commandArray[1])); //cast from
        commandBuffer.byDistance = static_cast<bool>(std::stoi(commandArray[2]));
        commandBuffer.forceValue = std::stoi(commandArray[3]);
        commandBuffer.distanceValue = std::stoi(commandArray[4]);
    } else
    {
        commandBuffer.byForce = 0;
        commandBuffer.byDistance = 0;
        commandBuffer.forceValue = 0;
        commandBuffer.distanceValue = 0;
    }

    return;
}

std::string statecontroller::executeCommand()
{
    //ST - Status, should we do something?
    // This is handled by other code? NOTE
    if(commandBuffer.beginningState == "ST")
    {
        return "WAIT";
    }

    // OP - Open
    // KOmunikations standard. Skal overholdes ellers går alt i stykker.
    if(commandBuffer.beginningState == "OP")
    {
        motorController_ptr_ ->move(commandBuffer.distanceValue, commandBuffer.forceValue);
        if(motorController_ptr_->getState() == MOTOR_CONTROL_ERROR_CODE::ALL_OK)
        {

            return "OK";
        }
    }


    if(commandBuffer.beginningState == "CL")
    {
        if (!commandBuffer.byForce && !commandBuffer.byDistance)
        {
            // Not much happens here.
        }

        if (!commandBuffer.byForce && commandBuffer.byDistance)
        {
            motorController_ptr_->move(commandBuffer.distanceValue,0.0);
            return "OK";
        }

        if (commandBuffer.byForce && !commandBuffer.byDistance)
        {
            motorController_ptr_->move(0.0,commandBuffer.forceValue); //Er ligeglad med position. Håber på at ramme noget.(tryg)
            return "OK";
        }

        if (commandBuffer.byForce && commandBuffer.byDistance)
        {
           motorController_ptr_->move(commandBuffer.distanceValue, commandBuffer.forceValue);
           return "OK";
        }
    }
    return "HALT";
}


void statecontroller::AddMotorcontroller(MotorController& motorController)
{
    motorController_ptr_ = &motorController;
}

