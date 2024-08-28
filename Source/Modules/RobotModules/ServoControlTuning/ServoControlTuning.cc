//
//	ServoControlTuning.cc		This file is a part of the IKAROS project
//
//   Most of the functions are based on the EpiServos.cc
//    See http://www.ikaros-project.org/ for more information.
//

// Dynamixel settings
#define PROTOCOL_VERSION 2.0 // See which protocol version is used in the Dynamixel
#define BAUDRATE1M 1000000   // XL-320 is limited to 1Mbit
#define BAUDRATE3M 3000000   // MX servos


#define ADDR_P 84
#define ADDR_I 82
#define ADDR_D 80
#define ADDR_TORQUE_ENABLE 64
#define ADDR_GOAL_POSITION 116
#define ADDR_GOAL_CURRENT 102
#define ADDR_PROFILE_ACCELERATION 108
#define ADDR_PROFILE_VELOCITY 112
#define ADDR_PRESENT_POSITION 132
#define ADDR_PRESENT_CURRENT 126
#define ADDR_PRESENT_TEMPERATURE 146

#define ADDR_MIN_POSITION_LIMIT 48
#define ADDR_MAX_POSITION_LIMIT 52

#define INDIRECTADDRESS_FOR_WRITE      168                  
#define INDIRECTADDRESS_FOR_READ       578                  
#define INDIRECTDATA_FOR_WRITE         634
#define INDIRECTDATA_FOR_READ          639
#define LEN_INDIRECTDATA_FOR_WRITE     21 // Torque Enable(1) + P(2) + I(2) + D(2)  + Goal current(2)+ Profile Acceleration (4) + Profile Velocity (4) +  Goal Position(4) 
#define LEN_INDIRECTDATA_FOR_READ      27 // Present Temperature(1)+ P(2) + I(2) + D(2)  + Goal current(2)+ Present Current (2) Profile Acceleration (4) + Profile Velocity (4) +  Goal Position(4) + Present Position(4) 


// ID of each dynamixel chain.
#define HEAD_ID_MIN 2
#define HEAD_ID_MAX 5

#define ARM_ID_MIN 2
#define ARM_ID_MAX 7

#define BODY_ID_MIN 2
#define BODY_ID_MAX 2

#define PUPIL_ID_MIN 2
#define PUPIL_ID_MAX 3

#define EPI_TORSO_NR_SERVOS 6
#define EPI_NR_SERVOS 19

#define TIMER_POWER_ON 2           // Timer ramping up sec
#define TIMER_POWER_OFF 5          // Timer ramping down sec
#define TIMER_POWER_OFF_EXTENDED 3 // Timer until torque enable off sec

#define HEAD_INDEX_IO 0
#define PUPIL_INDEX_IO 4
#define LEFT_ARM_INDEX_IO 6
#define RIGHT_ARM_INDEX_IO 12
#define BODY_INDEX_IO 18

#define MAX_TEMPERATURE 65


// TODO:
// Add fast sync write feature


#include <stdio.h>
#include <vector> // Data from dynamixel sdk
#include <future> // Threads

#include "ikaros.h"

// This must be after ikaros.h
#include "dynamixel_sdk.h" // Uses Dynamixel SDK library

using namespace ikaros;
typedef struct
{
    std::string serialPortPupil;
    std::string serialPortHead;
    std::string serialPortBody;
    std::string serialPortLeftArm;
    std::string serialPortRightArm;
    std::string type;

} Robot_parameters;

class ServoControlTuning : public Module
{
    // Paramteters
    int robotType = 0;
   

    // Ikaros IO
    matrix goalCurrent;
    bool torqueEnable = true;

  

    matrix presentPosition;
    matrix presentCurrent;
    matrix minLimitPosiiton;
    matrix maxLimitPosition;
    matrix dynamixel4bytesParameters;
    matrix dynamixel2bytesParameters;

   

    bool EpiTorsoMode = false;
    bool EpiMode = false;

    int AngleMinLimitPupil[2];
    int AngleMaxLimitPupil[2]; 
    parameter servoToTuneName;
    int servoToTune;
    int servoIndex;
    parameter numberTransitions;

    

    matrix servoParameters;
    dictionary servoIDs;
    matrix servoTransitions;
    int transitionIndex = 0;
 
    

    /*
    dictionary controlParameters;

    controlParameters["Name"] = ["Torque Enable", "Goal Position", "Goal Current" "P_gain", "I_gain", "D_gain", "Profile Acceleration", "Profile Velocity", "Present Position", "Present Current"];
    controlParameters["address"] = [64, 116, 102, 84, 82, 80, 108, 112, 132, 126];
    controlParameters["Indirectaddress"] = [168, 170, 174, 176, 178, 180, 182, 186, 578, 582];
    controlParameters["Bytes"] = [1, 4, 2, 2, 2, 2, 4, 4, 4, 2];

    */

    dynamixel::PortHandler *portHandlerHead;
    dynamixel::PacketHandler *packetHandlerHead;
    dynamixel::GroupSyncRead *groupSyncReadHead;
    dynamixel::GroupSyncWrite *groupSyncWriteHead;

    dynamixel::PortHandler *portHandlerPupil;
    dynamixel::PacketHandler *packetHandlerPupil;
    dynamixel::GroupSyncRead *groupSyncReadPupil;
    dynamixel::GroupSyncWrite *groupSyncWritePupil;

    dynamixel::PortHandler *portHandlerLeftArm;
    dynamixel::PacketHandler *packetHandlerLeftArm;
    dynamixel::GroupSyncRead *groupSyncReadLeftArm;
    dynamixel::GroupSyncWrite *groupSyncWriteLeftArm;

    dynamixel::PortHandler *portHandlerRightArm;
    dynamixel::PacketHandler *packetHandlerRightArm;
    dynamixel::GroupSyncRead *groupSyncReadRightArm;
    dynamixel::GroupSyncWrite *groupSyncWriteRightArm;

    dynamixel::PortHandler *portHandlerBody;
    dynamixel::PacketHandler *packetHandlerBody;
    dynamixel::GroupSyncRead *groupSyncReadBody;
    dynamixel::GroupSyncWrite *groupSyncWriteBody;

    std::string robotName;
    std::map<std::string, Robot_parameters> robot;


    bool CommunicationPupil(int position)
    {
        // Change this function.
        // No need to have torque enable.
        // Only goal position and use sync write.
        int dxl_comm_result = COMM_TX_FAIL; // Communication result
        bool dxl_addparam_result = false;   // addParam result
        bool dxl_getdata_result = false;    // GetParam result
        uint8_t dxl_error = 0;              // Dynamixel error

        // Send to pupil. No feedback

        uint16_t param_default = position; // Not using degrees.
        // Goal postiion feature/bug. If torque enable = 0 and goal position is sent. Torque enable will be 1.
        for (int i = PUPIL_ID_MIN; i <= PUPIL_ID_MAX; i++)
        {
             if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, i, 30, param_default, &dxl_error))
        {
            Notify(msg_warning, std::string("[ID:%03d] write2ByteTxRx failed") + std::to_string(i)); 
            portHandlerPupil->clearPort();
            return false;
        }
         
        }
       
    
        
    
        // XL 320 has no current position mode. Ignores goal current input
        // No feedback from pupils. Also no temperature check. Bad idea?
    
        return (true);
    }

    bool Communication(int ID, int IOIndex, matrix parameterValues, dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler, dynamixel::GroupSyncRead *groupSyncRead, dynamixel::GroupSyncWrite *groupSyncWrite)
    {
        if (portHandler == NULL) // If no port handler return true. Only return false if communication went wrong.
            return true;

        int index = 0;
        int dxl_comm_result = COMM_TX_FAIL; // Communication result
        bool dxl_addparam_result = false;   // addParam result
        bool dxl_getdata_result = false;    // GetParam result

        uint8_t dxl_error = 0;       // Dynamixel error
        uint8_t param_sync_write[LEN_INDIRECTDATA_FOR_READ]; // (Length 8 including 0) 7 byte sync write is not supported for the DynamixelSDK
        uint8_t dxl_present_temperature = 0;

    
        uint16_t dxl_P = 0;
        uint16_t dxl_I = 0;
        uint16_t dxl_D = 0;
        uint16_t dxl_goal_current = 0;
        int16_t dxl_present_current = 0;
        uint32_t dxl_profile_acceleration = 0;
        uint32_t dxl_profile_velocity = 0;
        uint32_t dxl_goal_position = 0;
        uint32_t dxl_present_position = 0;

        
       
        if (!groupSyncRead->addParam(ID))
        {
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            return false;
        }
    

        // Sync read
        dxl_comm_result = groupSyncRead->txRxPacket();
        if (dxl_comm_result != COMM_SUCCESS)
        {
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            return false;
        }

        // Check if data is available
        dxl_comm_result = groupSyncRead->isAvailable(ID, INDIRECTDATA_FOR_READ, LEN_INDIRECTDATA_FOR_READ);
        if (!dxl_comm_result)
        {
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            return false;
        }

        // GroupSyncWrite: Torque Enable(1) + P(2) + I(2) + D(2)  + Goal current(2)+ Profile Acceleration (4) + Profile Velocity (4) +  Goal Position(4) 
        // GroupSyncRead: Present Temperature(1)+ P(2) + I(2) + D(2)  + Goal current(2)+ Present Current (2) Profile Acceleration (4) + Profile Velocity (4) +  Goal Position(4) + Present Position(4) 
        

        dxl_present_temperature = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ, 1); // Present temperature    
        dxl_present_position = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ+1, 4);    // Present position
        dxl_P = groupSyncRead->getData(ID, INDIRECTADDRESS_FOR_READ +1+4, 2); // P
        dxl_I = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2, 2); // I
        dxl_D = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2, 2); // D
        dxl_goal_current = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2, 2); // Goal current
        dxl_present_current = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2+2, 2); // Present current
        dxl_profile_acceleration = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2+2+2, 4); // Profile acceleration
        dxl_profile_velocity = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2+2+2+4, 4); // Profile velocity
        dxl_goal_position = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2+2+2+4+4, 4); // Goal position
        dxl_present_position = groupSyncRead->getData(ID, INDIRECTDATA_FOR_READ +1+4+2+2+2+2+2+4+4+4, 4); // Present position
        

        presentPosition[index] = dxl_present_position / 4095.0 * 360.0; // degrees
        presentCurrent[index] = dxl_present_current * 3.36;   // mA
        index++;
    

        // Send (sync write)
        index = IOIndex;
    
        param_sync_write[0] = 1; // Torque on

        if (parameterValues.connected())
        {   
            int parameterIndex = 0;
            //Goal position
            int value = parameterValues[parameterIndex] / 360.0 * 4096.0;
            param_sync_write[1] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[2] = DXL_HIBYTE(DXL_LOWORD(value));
            param_sync_write[3] = DXL_LOBYTE(DXL_HIWORD(value));
            param_sync_write[4] = DXL_HIBYTE(DXL_HIWORD(value));
            parameterIndex++;

            //Goal current
            value = goalCurrent[parameterIndex] / 3.36;
            param_sync_write[5] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[6] = DXL_HIBYTE(DXL_LOWORD(value));
            parameterIndex++;

            // P
            value = parameterValues[parameterIndex];
            param_sync_write[7] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[8] = DXL_HIBYTE(DXL_LOWORD(value));
            parameterIndex++;

            // I   
            value = parameterValues[parameterIndex];
            param_sync_write[9] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[10] = DXL_HIBYTE(DXL_LOWORD(value));
            parameterIndex++;

            // D
            value = parameterValues[parameterIndex];
            param_sync_write[11] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[12] = DXL_HIBYTE(DXL_LOWORD(value));
            parameterIndex++;    

            // Profile Acceleration
            value = parameterValues[parameterIndex];
            param_sync_write[13] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[14] = DXL_HIBYTE(DXL_LOWORD(value));
            param_sync_write[15] = DXL_LOBYTE(DXL_HIWORD(value));
            param_sync_write[16] = DXL_HIBYTE(DXL_HIWORD(value));
            parameterIndex++;

            // Profile Velocity
            value = parameterValues[parameterIndex];
            param_sync_write[17] = DXL_LOBYTE(DXL_LOWORD(value));
            param_sync_write[18] = DXL_HIBYTE(DXL_LOWORD(value));
            param_sync_write[19] = DXL_LOBYTE(DXL_HIWORD(value));
            param_sync_write[20] = DXL_HIBYTE(DXL_HIWORD(value));

        }
        else
        {
            Notify(msg_fatal_error, "Parameter values are not set\n");
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            return false;
        }

      
      

        dxl_addparam_result = groupSyncWrite->addParam(ID, param_sync_write, 7);
        if (!dxl_addparam_result)
        {
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            return false;
        }

        index++;
        

        // Syncwrite
        dxl_comm_result = groupSyncWrite->txPacket();
        if (dxl_comm_result != COMM_SUCCESS)
        {
            groupSyncWrite->clearParam();
            groupSyncRead->clearParam();
            std::cout << "Sync failed" << std::endl;
            return false;
        }

        groupSyncWrite->clearParam();
        groupSyncRead->clearParam();
        return true;
    }


    void Init()
    {
        
        // Robots configurations
        robot["EpiWhite"] = {.serialPortPupil = "/dev/cu.usbserial-FT66WV4A",
                             .serialPortHead = "/dev/cu.usbserial-FT6S4JL9",
                             .serialPortBody = "",
                             .serialPortLeftArm = "",
                             .serialPortRightArm = "",
                             .type = "EpiTorso"};

        robot["EpiRed"] = {.serialPortPupil = "/dev/cu.usbserial-FT4TCJXI",
                           .serialPortHead = "/dev/cu.usbserial-FT4TCGUT",
                           .serialPortBody = "",
                           .serialPortLeftArm = "",
                           .serialPortRightArm = "",
                           .type = "EpiTorso"};

        robot["EpiRedDemo"] = {.serialPortPupil = "/dev/ttyUSB0",
                               .serialPortHead = "/dev/ttyUSB1",
                               .serialPortBody = "",
                               .serialPortLeftArm = "",
                               .serialPortRightArm = "",
                               .type = "EpiTorso"};

        robot["EpiOrange"] = {.serialPortPupil = "/dev/cu.usbserial-FT3WI2WH",
                              .serialPortHead = "/dev/cu.usbserial-FT3WI2K2",
                              .serialPortBody = "",
                              .serialPortLeftArm = "",
                              .serialPortRightArm = "",
                              .type = "EpiTorso"};

        robot["EpiYellow"] = {.serialPortPupil = "/dev/cu.usbserial-FT6S4IFI",
                              .serialPortHead = "/dev/cu.usbserial-FT6RW7PH",
                              .serialPortBody = "",
                              .serialPortLeftArm = "",
                              .serialPortRightArm = "",
                              .type = "EpiTorso"};

        robot["EpiGreen"] = {.serialPortPupil = "/dev/cu.usbserial-FT6S4JMH",
                             .serialPortHead = "/dev/cu.usbserial-FT66WT6W",
                             .serialPortBody = "",
                             .serialPortLeftArm = "",
                             .serialPortRightArm = "",
                             .type = "EpiTorso"};

        robot["EpiBlue"] = {.serialPortPupil = "/dev/cu.usbserial-FT66WS1F",
                            .serialPortHead = "/dev/cu.usbserial-FT4THUNJ",
                            .serialPortBody = "/dev/cu.usbserial-FT4THV1M",
                            .serialPortLeftArm = "/dev/cu.usbserial-FT4TFSV0",
                            .serialPortRightArm = "/dev/cu.usbserial-FT4TCVTT",
                            .type = "Epi"};

        robot["EpiGray"] = {.serialPortPupil = "/dev/cu.usbserial-FT6S4JL5",
                            .serialPortHead = "/dev/cu.usbserial-FT66WV43",
                            .serialPortBody = "",
                            .serialPortLeftArm = "",
                            .serialPortRightArm = "",
                            .type = "EpiTorso"};

        robot["EpiBlack"] = {.serialPortPupil = "/dev/cu.usbserial-FT1SEOJY",
                             .serialPortHead = "/dev/cu.usbserial-FT3WHSCR",
                             .serialPortBody = "",
                             .serialPortLeftArm = "",
                             .serialPortRightArm = "",
                             .type = "EpiTorso"};

        robotName = GetValue("robot");

        // Check if robotname exist in configuration
        if (robot.find(robotName) == robot.end())
        {
            Notify(msg_fatal_error, std::string("%s is not supported") + robotName);
            return;
        }

        // Check type of robot
        EpiTorsoMode = (robot[robotName].type.compare("EpiTorso") == 0);
        EpiMode = (robot[robotName].type.compare("Epi") == 0);
        //Notify(msg_debug, "Connecting to %s (%s)\n", robotName.c_str(), robot[robotName].type.c_str());
        
        Notify(msg_debug, "before passing dynamixel parameters\n");

   

        
        dynamixel2bytesParameters = {ADDR_P, ADDR_I, ADDR_D, ADDR_GOAL_CURRENT, ADDR_PRESENT_CURRENT};
        dynamixel2bytesParameters.set_name("Dynamixel2bytes");
        dynamixel2bytesParameters.set_labels(0, "P", "I", "D", "Goal Current", "Present Current");
        
        dynamixel4bytesParameters = {ADDR_PROFILE_ACCELERATION, ADDR_PROFILE_VELOCITY, ADDR_GOAL_POSITION, ADDR_PRESENT_POSITION};
        dynamixel4bytesParameters.set_name("Dynamixel4bytes");
        dynamixel4bytesParameters.set_labels(0, "Profile Acceleration", "Profile Velocity", "Goal Position", "Present Position");
        

        Notify(msg_debug, "Passed dynamixel parameters\n");

        

        
        // Ikaros input
        Bind(servoParameters, "ServoParameters");
        Bind(minLimitPosiiton, "MinLimitPosition");
        Bind(maxLimitPosition, "MaxLimitPosition");
        Bind(servoToTuneName, "Servo");
        Bind(numberTransitions, "NumberTransitions");

        Notify(msg_debug, "Passed Bindings ");

       
        servoParameters.set_labels(0, "Goal Position, Goal Current, P, I, D, Profile Acceleration, Profile Velocity");
        
        Notify(msg_debug, "Passed servo parameters labels \n");
        std::string s = R"({"Necktilt": 2, "Neckpan": 3, "LeftEye": 4, "RightEye": 5, "PupilLeft": 2, "PupilRight": 3, "ArmJoint1": 2, "ArmJoint2": 3, "ArmJoint3": 4, "ArmJoint4": 5, "ArmJoint5": 6, "Hand": 7, "Body": 2})";
        servoIDs = parse_json(s);
        

        // Epi torso
        // =========
        if (EpiTorsoMode || EpiMode)
        {
            int dxl_comm_result;
            std::vector<uint8_t> vec;

            // Neck/Eyes (id 2,3) =  2x MX106R Eyes = 2xMX28R (id 3,4)

            // Init Dynaxmixel SDK
            portHandlerHead = dynamixel::PortHandler::getPortHandler(robot[robotName].serialPortHead.c_str());
            packetHandlerHead = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

            // Open port
            if (portHandlerHead->openPort())
                Notify(msg_debug, "Succeeded to open serial port!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to open serial port!\n");
                return;
            }

            // Set port baudrate
            if (portHandlerHead->setBaudRate(BAUDRATE3M))
                Notify(msg_debug, "Succeeded to change baudrate!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to change baudrate!\n");
                return;
            }

            // Ping all the servos to make sure they are all there.
            dxl_comm_result = packetHandlerHead->broadcastPing(portHandlerHead, vec);
            // if (dxl_comm_result != COMM_SUCCESS)
            //     Notify(msg_warning, "%s\n", packetHandlerHead->getTxRxResult(dxl_comm_result));

            Notify(msg_debug, "Detected Dynamixel (Head): \n");
            // for (int i = 0; i < (int)vec.size(); i++)
            //     Notify(msg_debug, "[ID:%03d]\n", vec.at(i));

            // Pupil (id 2,3) = XL320 Left eye, right eye

            // Init Dynaxmixel SDK
            portHandlerPupil = dynamixel::PortHandler::getPortHandler(robot[robotName].serialPortPupil.c_str());
            packetHandlerPupil = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

            // Open port
            if (portHandlerPupil->openPort())
                Notify(msg_debug, "Succeeded to open serial port!\n");
            else
            {
                Notify(msg_fatal_error, "\n");
                return;
            }
            // Set port baudrate
            if (portHandlerPupil->setBaudRate(BAUDRATE1M))
                Notify(msg_debug, "Succeeded to change baudrate!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to change baudrate!\n");
                return;
            }
            // Ping all the servos to make sure they are all there.
            dxl_comm_result = packetHandlerPupil->broadcastPing(portHandlerPupil, vec);
            // if (dxl_comm_result != COMM_SUCCESS)
            //     Notify(msg_warning, "%s\n", packetHandlerPupil->getTxRxResult(dxl_comm_result));

            Notify(msg_debug, "Detected Dynamixel (Pupil): \n");
            // for (int i = 0; i < (int)vec.size(); i++)
            //     Notify(msg_debug, "[ID:%03d]\n", vec.at(i));
        }
        else
        {
            Notify(msg_fatal_error, "Robot type is not yet implementet\n");
        }
        if (EpiMode)
        {
            int dxl_comm_result;
            std::vector<uint8_t> vec;

            // Left arm 6x MX106R 1 MX28R

            // Init Dynaxmixel SDK
            portHandlerLeftArm = dynamixel::PortHandler::getPortHandler(robot[robotName].serialPortLeftArm.c_str());
            packetHandlerLeftArm = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

            // Open port
            if (portHandlerLeftArm->openPort())
                Notify(msg_debug, "Succeeded to open serial port!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to open serial port!\n");
                return;
            }
            // Set port baudrate
            if (portHandlerLeftArm->setBaudRate(BAUDRATE3M))
                Notify(msg_debug, "Succeeded to change baudrate!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to change baudrate!\n");
                return;
            }
            // Ping all the servos to make sure they are all there.
            dxl_comm_result = packetHandlerLeftArm->broadcastPing(portHandlerLeftArm, vec);
            // if (dxl_comm_result != COMM_SUCCESS)
            //     Notify(msg_warning, "%s\n", packetHandlerLeftArm->getTxRxResult(dxl_comm_result));

            Notify(msg_debug, "Detected Dynamixel (Left arm): \n");
            // for (int i = 0; i < (int)vec.size(); i++)
            //     Notify(msg_debug, "[ID:%03d]\n", vec.at(i));

            // Right arm 6x MX106R 1 MX28R

            // Init Dynaxmixel SDK
            portHandlerRightArm = dynamixel::PortHandler::getPortHandler(robot[robotName].serialPortRightArm.c_str());
            packetHandlerRightArm = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

            // Open port
            if (portHandlerRightArm->openPort())
                Notify(msg_debug, "Succeeded to open serial port!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to open serial port!\n");
                return;
            }
            // Set port baudrate
            if (portHandlerRightArm->setBaudRate(BAUDRATE3M))
                Notify(msg_debug, "Succeeded to change baudrate!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to change baudrate!\n");
                return;
            }
            // Ping all the servos to make sure they are all there.
            dxl_comm_result = packetHandlerRightArm->broadcastPing(portHandlerRightArm, vec);
            // if (dxl_comm_result != COMM_SUCCESS)
            //     Notify(msg_warning, "%s\n", packetHandlerRightArm->getTxRxResult(dxl_comm_result));

            Notify(msg_debug, "Detected Dynamixel (Right arm): \n");
            // for (int i = 0; i < (int)vec.size(); i++)
            //     Notify(msg_debug, "[ID:%03d]\n", vec.at(i));

            // Body MX106R

            // Init Dynaxmixel SDK
            portHandlerBody = dynamixel::PortHandler::getPortHandler(robot[robotName].serialPortBody.c_str());
            packetHandlerBody = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

            // Open port
            if (portHandlerBody->openPort())
                Notify(msg_debug, "Succeeded to open serial port!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to open serial port!\n");
                return;
            }
            // Set port baudrate
            if (portHandlerBody->setBaudRate(BAUDRATE3M))
                Notify(msg_debug, "Succeeded to change baudrate!\n");
            else
            {
                Notify(msg_fatal_error, "Failed to change baudrate!\n");
                return;
            }
            // Ping all the servos to make sure they are all there.
            dxl_comm_result = packetHandlerBody->broadcastPing(portHandlerBody, vec);
            if (dxl_comm_result != COMM_SUCCESS)
                Notify(msg_warning, "%s\n", packetHandlerBody->getTxRxResult(dxl_comm_result));

            Notify(msg_debug, "Detected Dynamixel (Body): \n");
            // for (int i = 0; i < (int)vec.size(); i++)
            //     Notify(msg_debug, "[ID:%03d]\n", vec.at(i));
            
            
        }
        
        // Create dynamixel objects
        if (EpiTorsoMode || EpiMode)
        {
            groupSyncWriteHead = new dynamixel::GroupSyncWrite(portHandlerHead, packetHandlerHead, INDIRECTDATA_FOR_WRITE, LEN_INDIRECTDATA_FOR_WRITE); // Torque enable, goal position, goal current, P, I, D, Profile acceleration, Profile velocity
            groupSyncReadHead = new dynamixel::GroupSyncRead(portHandlerHead, packetHandlerHead, INDIRECTDATA_FOR_READ,LEN_INDIRECTDATA_FOR_READ );   // Present poistion, present current, temperature, goal current
        }
        if (EpiMode)
        {
            groupSyncWriteLeftArm = new dynamixel::GroupSyncWrite(portHandlerLeftArm, packetHandlerLeftArm, INDIRECTADDRESS_FOR_WRITE, LEN_INDIRECTDATA_FOR_WRITE);
            groupSyncReadLeftArm = new dynamixel::GroupSyncRead(portHandlerLeftArm, packetHandlerLeftArm, INDIRECTDATA_FOR_READ, LEN_INDIRECTDATA_FOR_READ);
            groupSyncWriteRightArm = new dynamixel::GroupSyncWrite(portHandlerRightArm, packetHandlerRightArm, INDIRECTADDRESS_FOR_WRITE, LEN_INDIRECTDATA_FOR_WRITE);
            groupSyncReadRightArm = new dynamixel::GroupSyncRead(portHandlerRightArm, packetHandlerRightArm, INDIRECTDATA_FOR_READ, LEN_INDIRECTDATA_FOR_READ);
            groupSyncWriteBody = new dynamixel::GroupSyncWrite(portHandlerBody, packetHandlerBody, INDIRECTADDRESS_FOR_WRITE, LEN_INDIRECTDATA_FOR_WRITE);
            groupSyncReadBody = new dynamixel::GroupSyncRead(portHandlerBody, packetHandlerBody, INDIRECTDATA_FOR_READ, LEN_INDIRECTDATA_FOR_READ);
            groupSyncWritePupil = new dynamixel::GroupSyncWrite(portHandlerPupil, packetHandlerPupil, 30, 2); // no read..
        }

        AutoCalibratePupil();

        Notify(msg_debug, "torque down servos and prepering servos for write defualt settings\n");
        if (!PowerOffRobot())
            Notify(msg_fatal_error, "Unable torque down servos\n");

        if (!SetParameterAddresses(servoIDs))
            Notify(msg_fatal_error, "Unable to set indirect addresses on servos\n");

        if (!PowerOnRobot())
            Notify(msg_fatal_error, "Unable torque up servos\n");
    }

    float PupilMMToDynamixel(float mm, int min, int max)
    {
        // Quick fix
        float minMM = 4.2;
        float maxMM = 19.7;
        float deltaMM = maxMM - minMM;

        float minDeg = min;
        float maxDeg = max;
        float deltDeg = maxDeg - minDeg;

        if (mm < minMM)
            mm = minMM;
        if (mm > maxMM)
            mm = maxMM;

        return (-(mm - minMM) / deltaMM * deltDeg + maxDeg); // Higher goal position gives smaller pupil
    }

    matrix GenerateServoTransitions(int numSequence, int minLimit, int maxLimit)
    {
        maxLimit = maxLimit * 360 / 4095; // Convert to degrees
        minLimit = minLimit * 360 / 4095; // Convert to degrees
        matrix Positions(numSequence, 1);
        // calculate the middle position
        float middle = (maxLimit + minLimit) / 2;
        // first transitions are from middle then oscillating in small steps that increase with time to the limits
        for (int i = 0; i < numSequence; i++)
        {
            if (i < numSequence / 2)
                Positions[i] = middle + (maxLimit - middle) * sin(i * 3.1415 / numSequence);
            else
                Positions[i] = middle + (minLimit - middle) * sin(i * 3.1415 / numSequence);
        }
        return Positions;

    }

    bool compare_parameter_to_string(parameter& p, const std::string& str) {
    return p.operator std::string() == str;
    }

    bool compare_parameter_to_string( parameter& p, const char* str) {
        return p.operator std::string() == std::string(str);
    }
    
    void Tick()
    {   
        servoToTune = servoIDs[servoToTuneName].copy();
        servoIndex = servoIDs.get_index(servoToTuneName.c_str());

        if (transitionIndex > numberTransitions)
        {
            transitionIndex = 0;
            Notify(msg_debug, "Restarting the servo transitions\n");
            
        }
        
        if (compare_parameter_to_string(servoToTuneName, "RightPupil") || compare_parameter_to_string(servoToTuneName, "LeftPupil")) 
        {
            Notify(msg_debug, "Compare parameter with strings method works");
            
            servoTransitions[transitionIndex] = clip(servoTransitions[transitionIndex], 5, 16); // Pupil size must be between 5 mm to 16 mm.

            // Special case. As pupil does not have any feedback we just return goal position
            presentPosition[PUPIL_INDEX_IO]    =     servoTransitions[transitionIndex];
            presentPosition[PUPIL_INDEX_IO+1]  =     servoTransitions[transitionIndex];
        
            // Special case for pupil uses mm instead of degrees
            servoTransitions[transitionIndex] = PupilMMToDynamixel(servoTransitions[transitionIndex], AngleMinLimitPupil[0], AngleMaxLimitPupil[0]);
            
        
            auto pupilThread = std::async(std::launch::async, &ServoControlTuning::CommunicationPupil, this, servoTransitions[transitionIndex] ); // Special!
            if (!pupilThread.get())
        {
            Notify(msg_warning, "Can not communicate with pupil");
            portHandlerPupil->clearPort();
        }
        }
        
        auto headThread = std::async(std::launch::async, &ServoControlTuning::Communication, this, servoToTune, HEAD_INDEX_IO,servoParameters, std::ref(portHandlerHead), std::ref(packetHandlerHead), std::ref(groupSyncReadHead), std::ref(groupSyncWriteHead));
        
        
        
    
        if (!headThread.get())
        {
            Notify(msg_warning,"Can not communicate with head");
            portHandlerHead->clearPort();
        }
        
        if(EpiMode)
        {
            auto leftArmThread = std::async(std::launch::async, &ServoControlTuning::Communication, this, servoToTune , LEFT_ARM_INDEX_IO, servoParameters, std::ref(portHandlerLeftArm), std::ref(packetHandlerLeftArm), std::ref(groupSyncReadLeftArm), std::ref(groupSyncWriteLeftArm));
            auto rightArmThread = std::async(std::launch::async, &ServoControlTuning::Communication, this, servoToTune , RIGHT_ARM_INDEX_IO,servoParameters, std::ref(portHandlerRightArm), std::ref(packetHandlerRightArm), std::ref(groupSyncReadRightArm), std::ref(groupSyncWriteRightArm));
            auto bodyThread = std::async(std::launch::async, &ServoControlTuning::Communication, this, servoToTune , BODY_INDEX_IO,servoParameters, std::ref(portHandlerBody), std::ref(packetHandlerBody), std::ref(groupSyncReadBody), std::ref(groupSyncWriteBody));
        
            if (!leftArmThread.get())
            {
                Notify(msg_warning, "Can not communicate with left arm");
                portHandlerLeftArm->clearPort();
            }
            if (!rightArmThread.get())
            {
                Notify(msg_warning, "Can not communicate with right arm");
                portHandlerRightArm->clearPort();
            }
            if (!bodyThread.get())
            {
                Notify(msg_warning, "Can not communicate with body");
                portHandlerBody->clearPort();
            }
        }
        transitionIndex++;
    }

    // A function that set importat parameters in the control table.
    // Baud rate and ID needs to be set manually.
    bool SetParameterAddresses(dictionary servoIDs) {
        uint32_t param_default_4Byte;
        uint16_t param_default_2Byte;
        uint8_t param_default_1Byte;
        uint8_t dxl_error = 0;
        // Vector of smart pointers to PortHandler
        std::vector<std::shared_ptr<dynamixel::PortHandler>> portHandlers;
        // Vector of smart pointers to PacketHandler
        std::vector<std::shared_ptr<dynamixel::PacketHandler>> packetHandlers;
       
        std::vector<int> maxMinPositionLimitIndex;
        std::vector<value> servoIDsInUse;

        uint16_t pupil_moving_speed = 150;
        uint8_t pupil_p_gain = 100;
        uint8_t pupil_i_gain = 20;
        uint8_t pupil_d_gain = 5;

        int dxl_comm_result = COMM_TX_FAIL;
        int idMin = HEAD_ID_MIN;
        int idMax;
        int addressRead = INDIRECTADDRESS_FOR_READ;
        int addressWrite = INDIRECTADDRESS_FOR_WRITE;

        Notify(msg_debug, "Setting control table on servos\n");

        if (EpiMode)
        {
            
            // Store the port handlers and packet handlers in the lists
            portHandlers.push_back(std::shared_ptr<dynamixel::PortHandler>(portHandlerHead));
            portHandlers.push_back(std::shared_ptr<dynamixel::PortHandler>(portHandlerLeftArm));
            portHandlers.push_back(std::shared_ptr<dynamixel::PortHandler>(portHandlerRightArm));
            portHandlers.push_back(std::shared_ptr<dynamixel::PortHandler>(portHandlerBody));
            packetHandlers.push_back(std::shared_ptr<dynamixel::PacketHandler>(packetHandlerHead));
            packetHandlers.push_back(std::shared_ptr<dynamixel::PacketHandler>(packetHandlerLeftArm));
            packetHandlers.push_back(std::shared_ptr<dynamixel::PacketHandler>(packetHandlerRightArm));
            packetHandlers.push_back(std::shared_ptr<dynamixel::PacketHandler>(packetHandlerBody));
    
            idMax = BODY_ID_MAX;
        }
        else
        {
            portHandlers.push_back(std::shared_ptr<dynamixel::PortHandler>(portHandlerHead));
            packetHandlers.push_back(std::shared_ptr<dynamixel::PacketHandler>(packetHandlerHead));
            idMax = HEAD_ID_MAX;
        }
        

       
        


        //loop through all portHandlers
        
        for (int p = 0; p < portHandlers.size(); p++) {

            // Ensure p is within the valid range
            if (p < 0 || p >= portHandlers.size() || p >= packetHandlers.size()) {
                std::cout << "Invalid index for portHandlers or packetHandlers: " << p << std::endl;
                return false;
            }

            // Ensure pointers are not null
            if (!portHandlers[p] || !packetHandlers[p]) {
                std::cout << "Null pointer encountered in portHandlers or packetHandlers at index: " << p << std::endl;
                return false;
            }
            int start = 0;
            int end = 0;
            //switch statement for different p values. 
            switch (p) {
                case 0:
                servoIDsInUse = {servoIDs["Necktilt"].copy(), servoIDs["Neckpan"].copy(), servoIDs["LeftEye"].copy(), servoIDs["RightEye"].copy()};
                maxMinPositionLimitIndex = {0, 1, 2, 3};
                idMax = HEAD_ID_MAX;
                break;
                //same for p==1 and p==2 (right and left arm)
                case 1:
                servoIDsInUse = {servoIDs["Armjoint1"].copy(), servoIDs["Armjoint2"].copy(), servoIDs["Armjoint3"].copy(), servoIDs["Armjoint4"].copy(), servoIDs["Armjoint5"].copy(), servoIDs["Hand"].copy()};
                maxMinPositionLimitIndex = {4, 5, 6, 7, 8, 9};
                idMin = ARM_ID_MIN;
                idMax = ARM_ID_MAX;
                break;
                case 2:
                servoIDsInUse = {servoIDs["Armjoint1"].copy(), servoIDs["Armjoint2"].copy(), servoIDs["Armjoint3"].copy(), servoIDs["Armjoint4"].copy(), servoIDs["Armjoint5"].copy(), servoIDs["Hand"].copy()};
                maxMinPositionLimitIndex = {10, 11, 12, 13, 14, 15};
                idMin = ARM_ID_MIN;
                idMax = ARM_ID_MAX;
                break;
                case 3:
                servoIDsInUse = {servoIDs["Body"].copy()};
                maxMinPositionLimitIndex = {16};
                idMin = BODY_ID_MIN;
                idMax = BODY_ID_MAX;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            for (int id = idMin; id <= idMax; id++) 
            {
                // Torque Enable
                dxl_comm_result = packetHandlers[p]->write2ByteTxRx(portHandlers[p].get(),id , addressWrite, ADDR_TORQUE_ENABLE, &dxl_error);
                if (dxl_comm_result != COMM_SUCCESS) {
                    std::cout << "Failed to set torque enable for servo ID: " << id 
                            << " of port:" << portHandlers[p]->getPortName() 
                            << " Error: " << packetHandlers[p]->getTxRxResult(dxl_comm_result)
                            << " DXL Error: " << packetHandlers[p]->getRxPacketError(dxl_error) << std::endl;
                    return false;
                }            
                Notify(msg_debug,"Indirect addresses Torque enable set for all servos" );
                // present temperature
                dxl_comm_result = packetHandlers[p]->write2ByteTxRx(portHandlers[p].get(), id, addressRead, ADDR_PRESENT_TEMPERATURE, &dxl_error);
                if (dxl_comm_result != COMM_SUCCESS) {
                    std::cout << "Failed to set present temperature for servo ID: " << id 
                            << " of port:" << portHandlers[p]->getPortName() 
                            << " Error: " << packetHandlers[p]->getTxRxResult(dxl_comm_result)
                            << " DXL Error: " << packetHandlers[p]->getRxPacketError(dxl_error) << std::endl;
                    return false;
                }
            }

            //A indir. address is 2 bytes long. Next address is 2 bytes ahead of the previous
            addressWrite += 2; 
            addressRead += 2;    
            
            // 2 byte parameters (P, I, D, goal current, present current)
            for (int id = idMin; id <= idMax; id++) {
                for (int param = 0; param < dynamixel2bytesParameters.size()-1; param++) {//present position (last in dynamixel4byteparameters) is not used for writing
                    for (int byte = 0; byte < 2; byte++) {
                        addressWrite += (2*byte);
                        //writing
                        if (COMM_SUCCESS != packetHandlers[p]->write2ByteTxRx(portHandlers[p].get(), id, addressWrite , dynamixel2bytesParameters[param] + byte, &dxl_error)) {
                            std::cout << "Failed to set indirect address for " << dynamixel2bytesParameters.labels()[param] << " for servo ID: " << id << " of port:" << portHandlers[p]->getPortName() << std::endl;
                            return false;
                            
                        }
                        //reading
                        addressRead += (2*byte);
                        if(COMM_SUCCESS != packetHandlers[p]->write2ByteTxRx(portHandlers[p].get(), id, addressRead, dynamixel2bytesParameters[param] + byte, &dxl_error)) {
                            std::cout << "Failed to set indirect address for "<< dynamixel2bytesParameters.labels()[param] << " for servo ID: " << id << " of port:" << portHandlers[p]->getPortName() << std::endl;
                            return false;
                        }
                    }
                    
                }
               
            }
            // 4 byte parameters (profile acceleration, profile velocity, goal position, present position), also min and max limits
            int maxMinIndex = 0;
            for (int id = idMin; id <= idMax; id++) {
                for (int param = 0; param < dynamixel4bytesParameters.size() -1; param++) { //present position (last in dynamixel4byteparameters) is not used for writing
                    for (int byte = 0; byte < 4; byte++) {
                        //writing
                        addressWrite += (2*byte);
                        if (COMM_SUCCESS != packetHandlers[p]->write4ByteTxRx(portHandlers[p].get(), id, addressWrite , dynamixel4bytesParameters[param] + byte, &dxl_error)) {
                            std::cout << "Failed to set indirect address for "<< dynamixel4bytesParameters.labels()[param] <<" head servo ID: " << id << std::endl;
                            return false;
                            
                        }
                        //reading
                        addressRead += (2*byte);
                        if(COMM_SUCCESS != packetHandlers[p]->write4ByteTxRx(portHandlers[p].get(), id, addressRead, dynamixel4bytesParameters[param] + byte, &dxl_error)) {
                            std::cout << "Failed to set indirect address for parameter: " << dynamixel4bytesParameters[param].labels()[param] <<", ID: " << id << std::endl;
                            return false;
                        }
                    }
                }
                
               
                
            }
            
            int i = 0;
            //min and max limits
            for (int id = idMin; id <= idMax; id++) 
            {
                std::cout << " max and min limits for servo ID: " << id << std::endl;
    
                std::cout << "maxMinPositionLimitIndex[i]: " << maxMinPositionLimitIndex[i] << std::endl;
                minLimitPosiiton[maxMinPositionLimitIndex[i]].print();
                maxLimitPosition[maxMinPositionLimitIndex[i]].print();
                //min and max limits
                param_default_4Byte = minLimitPosiiton[maxMinPositionLimitIndex[i]];
                std::cout << "min param_default_4Byte: " << param_default_4Byte << std::endl;
                if (COMM_SUCCESS != packetHandlers[p]->write4ByteTxRx(portHandlers[p].get(), id, ADDR_MIN_POSITION_LIMIT, param_default_4Byte, &dxl_error)){
                    std::cout << "Failed to set indirect address for min position limit for servo ID: " << id << " of port:" << portHandlers[p]->getPortName() << std::endl;
                    return false;
                }
                param_default_4Byte = maxLimitPosition[maxMinPositionLimitIndex[i]];
                std::cout << "max param_default_4Byte: " << param_default_4Byte << std::endl;
                if (COMM_SUCCESS != packetHandlers[p]->write4ByteTxRx(portHandlers[p].get(), id, ADDR_MIN_POSITION_LIMIT, param_default_4Byte, &dxl_error)){
                    std::cout << "Failed to set indirect address for max position limit for servo ID: " << id << " of port:" << portHandlers[p]->getPortName() << std::endl;
                    return false;
                }
                std::cout << "min and max limits set for servo ID: " << id << std::endl;
                i++;
                
                
            }
                
            
        }
      
        std::cout << "through p loop" << std::endl;


        Notify(msg_debug, "Settting parameters for pupils (servo XL320)\n");

        Timer t;
        double xlTimer = 0.01; // Timer in sec. XL320 need this. Not sure why.

        // PUPIL ID 2 (Left pupil)
        // Limit position min
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 6, AngleMinLimitPupil[0], &dxl_error))
            return false;
        Sleep(xlTimer);
        std::cout << "min limit set for pupil" << std::endl;
        // Limit position max
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 8, AngleMaxLimitPupil[0], &dxl_error))
            return false;
        Sleep(xlTimer);

        // Moving speed
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 32, pupil_moving_speed, &dxl_error))
            return false;
        Sleep(xlTimer);

        // P
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 29, pupil_p_gain, &dxl_error))
            return false;
        Sleep(xlTimer);


        // I
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 28, pupil_i_gain, &dxl_error))
            return false;
        Sleep(xlTimer);


        // D
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 27, pupil_d_gain, &dxl_error))
            return false;
        Sleep(xlTimer);


        // PUPIL ID 3 (Right pupil)
        // Limit position in
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 6, AngleMinLimitPupil[1], &dxl_error))
            return false;
        Sleep(xlTimer);


        // Limit position max
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 8, AngleMaxLimitPupil[1], &dxl_error))
            return false;
        Sleep(xlTimer);


        // Moving speed
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 32, pupil_moving_speed, &dxl_error))
            return false;
        Sleep(xlTimer);

        // P
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 29, pupil_p_gain, &dxl_error))
            return false;
        Sleep(xlTimer);


        // I
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 28, pupil_i_gain, &dxl_error))
            return false;
        Sleep(xlTimer);


        // D
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 27, pupil_d_gain, &dxl_error))
            return false;
        Sleep(xlTimer);

        Notify(msg_debug, "Indirect addresses set for all servos!!!!\n");
        return true; // Yay we manage to set everything we needed.
    }




    bool PowerOn(int IDMin, int IDMax, dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler)
    {
        if (portHandler == NULL) // If no port handler return true. Only return false if communication went wrong.
            return true;

        Notify(msg_debug, "Power on servos");

        Timer t;
        const int nrOfServos = IDMax - IDMin + 1;
        int dxl_comm_result = COMM_TX_FAIL; // Communication result
        uint8_t dxl_error = 0;              // Dynamixel error
        uint16_t start_p_value[7] = {0, 0, 0, 0, 0, 0, 0};
        uint32_t present_postition_value[7] = {0, 0, 0, 0, 0, 0, 0};

        // Get P values
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->read2ByteTxRx(portHandler, IDMin + i, 84, &start_p_value[i], &dxl_error))
                return false;

        // Set P value to 0
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, 0, &dxl_error))
                return false;
        // Set torque value to 1
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write1ByteTxRx(portHandler, IDMin + i, 64, 1, &dxl_error))
                return false;
        while (t.GetTime() < TIMER_POWER_ON)
        {
            // Get present position
            for (int i = 0; i < nrOfServos; i++)
                if (COMM_SUCCESS != packetHandler->read4ByteTxRx(portHandler, IDMin + i, 132, &present_postition_value[i], &dxl_error))
                    return false;
            // Set goal position to present postiion
            for (int i = 0; i < nrOfServos; i++)
                if (COMM_SUCCESS != packetHandler->write4ByteTxRx(portHandler, IDMin + i, 116, present_postition_value[i], &dxl_error))
                    return false;
            // Ramping up P
            for (int i = 0; i < nrOfServos; i++)
                if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, int(float(start_p_value[i]) / float(TIMER_POWER_ON) * t.GetTime()), &dxl_error))
                    return false;
        }

        // Set P value to start value
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, start_p_value[i], &dxl_error))
                return false;

        return true;
    }

    bool PowerOnPupil()
    {
        uint8_t dxl_error = 0; // Dynamixel error

        // Enable torque. No fancy rampiong
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 24, 1, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 24, 1, &dxl_error))
            return false;
        return true;
    }

    bool PowerOnRobot()
    {
        // Trying to torque up the power of the servos.
        // Dynamixel protocel 2.0
        // In current base position control mode goal current can be used.
        // In poistion control mode P can be used (PID).
        // Torqing up the servos? This can not be done in 2.0 and position mode only in position-current mode.
        // 1. Set P (PID) = 0. Store start P value
        // 2. Set goal poistion to present position
        // 3. Increase current or P (PID)
        // 4. Repeat 2,3 for X seconds.

        auto headThread = std::async(std::launch::async, &ServoControlTuning::PowerOn, this, HEAD_ID_MIN, HEAD_ID_MAX, std::ref(portHandlerHead), std::ref(packetHandlerHead));
        auto pupilThread = std::async(std::launch::async, &ServoControlTuning::PowerOnPupil, this); // Different control table.
        auto leftArmThread = std::async(std::launch::async, &ServoControlTuning::PowerOn, this, ARM_ID_MIN, ARM_ID_MAX, std::ref(portHandlerLeftArm), std::ref(packetHandlerLeftArm));
        auto rightArmThread = std::async(std::launch::async, &ServoControlTuning::PowerOn, this, ARM_ID_MIN, ARM_ID_MAX, std::ref(portHandlerRightArm), std::ref(packetHandlerRightArm));
        auto bodyThread = std::async(std::launch::async, &ServoControlTuning::PowerOn, this, BODY_ID_MIN, BODY_ID_MAX, std::ref(portHandlerBody), std::ref(packetHandlerBody));

        if (!headThread.get())
            Notify(msg_fatal_error, "Can not power on head");
        if (!pupilThread.get())
            Notify(msg_fatal_error, "Can not power on pupil");
        if (!leftArmThread.get())
            Notify(msg_fatal_error, "Can not power on left arm");
        if (!rightArmThread.get())
            Notify(msg_fatal_error, "Can not power on right arm");
        if (!bodyThread.get())
            Notify(msg_fatal_error, "Can not power on body");

        return true;
    }

    bool PowerOff(int IDMin, int IDMax, dynamixel::PortHandler *portHandler, dynamixel::PacketHandler *packetHandler)
    {
        if (portHandler == NULL) // If no port handler return true. Only return false if communication went wrong.
            return true;

        Timer t;
        const int nrOfServos = IDMax - IDMin + 1;
        int dxl_comm_result = COMM_TX_FAIL; // Communication result
        uint8_t dxl_error = 0;              // Dynamixel error
        uint16_t start_p_value[7] = {0, 0, 0, 0, 0, 0, 0};
        uint32_t present_postition_value[7] = {0, 0, 0, 0, 0, 0, 0};

        // Get P values
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->read2ByteTxRx(portHandler, IDMin + i, 84, &start_p_value[i], &dxl_error))
                return false;

        // t.Reset();
        Notify(msg_warning, "Power off servos. If needed, support the robot while power off the servos");

        // Ramp down p value
        // while (t.GetTime() < TIMER_POWER_OFF)
        //     for (int i = 0; i < nrOfServos; i++)
        //         if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, int(float(start_p_value[i]) * (float(TIMER_POWER_OFF) - t.GetTime()) / float(TIMER_POWER_OFF)), &dxl_error))
        //             return false;
        // Turn p to zero
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, 0, &dxl_error))
                return false;

        Sleep(TIMER_POWER_OFF);

        // Get present position
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->read4ByteTxRx(portHandler, IDMin + i, 132, &present_postition_value[i], &dxl_error))
                return false;
        // Set goal position to present postiion
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write4ByteTxRx(portHandler, IDMin + i, 116, present_postition_value[i], &dxl_error))
                return false;

        // t.Restart();
        Sleep(TIMER_POWER_OFF_EXTENDED);

        // Enable torque off
        Notify(msg_debug, "Enable torque off");
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write1ByteTxRx(portHandler, IDMin + i, 64, 0, &dxl_error))
                return false;
        // Set P value to start value
        for (int i = 0; i < nrOfServos; i++)
            if (COMM_SUCCESS != packetHandler->write2ByteTxRx(portHandler, IDMin + i, 84, start_p_value[i], &dxl_error))
                return false;

        return true;
    }

    bool PowerOffPupil()
    {
        uint8_t dxl_error = 0; // Dynamixel error

        // Torque off. No fancy rampiong
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 24, 0, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 24, 0, &dxl_error))
            return false;

        return true;
    }

    bool PowerOffRobot()
    {

        auto headThread = std::async(std::launch::async, &ServoControlTuning::PowerOff, this, HEAD_ID_MIN, HEAD_ID_MAX, std::ref(portHandlerHead), std::ref(packetHandlerHead));
        auto pupilThread = std::async(std::launch::async, &ServoControlTuning::PowerOffPupil, this);
        auto leftArmThread = std::async(std::launch::async, &ServoControlTuning::PowerOff, this, ARM_ID_MIN, ARM_ID_MAX, std::ref(portHandlerLeftArm), std::ref(packetHandlerLeftArm));
        auto rightArmThread = std::async(std::launch::async, &ServoControlTuning::PowerOff, this, ARM_ID_MIN, ARM_ID_MAX, std::ref(portHandlerRightArm), std::ref(packetHandlerRightArm));
        auto bodyThread = std::async(std::launch::async, &ServoControlTuning::PowerOff, this, BODY_ID_MIN, BODY_ID_MAX, std::ref(portHandlerBody), std::ref(packetHandlerBody));

        if (!headThread.get())
            Notify(msg_fatal_error, "Can not power off head");
        if (!pupilThread.get())
            Notify(msg_fatal_error, "Can not power off pupil");
        if (!leftArmThread.get())
            Notify(msg_fatal_error, "Can not power off left arm");
        if (!rightArmThread.get())
            Notify(msg_fatal_error, "Can not power off right arm");
        if (!bodyThread.get())
            Notify(msg_fatal_error, "Can not power off body");

        // Power down servos.
        // 1. Store P (PID) value
        // 2. Ramp down P
        // 3. Turn of torque enable
        // 4. Set P (PID) valued from 1.

        return (true);
    }
    bool AutoCalibratePupil()
    {
        int dxl_comm_result = COMM_TX_FAIL; // Communication result
        uint8_t dxl_error = 0;              // Dynamixel error
        Timer t;
        double xlTimer = 0.010; // Timer in sec. XL320 need this. Not sure why.

        // Torque off. No fancy rampiong
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 24, 0, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 24, 0, &dxl_error))
            return false;
        Sleep(xlTimer);

        // Reset min and max limit
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 6, 0, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 6, 0, &dxl_error))
            return false;
        Sleep(xlTimer);

        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 8, 1023, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 8, 1023, &dxl_error))
            return false;
        Sleep(xlTimer);

        // Turn down torue limit
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 35, 500, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 35, 500, &dxl_error))
            return false;
        Sleep(xlTimer);

        // Torque off. No fancy rampiong
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 24, 1, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 24, 1, &dxl_error))
            return false;
        Sleep(xlTimer);

        // Go to min pos
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 30, 0, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 30, 0, &dxl_error))
            return false;
        // Sleep for 300 ms
        Sleep(xlTimer);

        // Read pressent position
        uint16_t present_postition_value[2] = {0, 0};
        if (COMM_SUCCESS != packetHandlerPupil->read2ByteTxRx(portHandlerPupil, 2, 37, &present_postition_value[0], &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->read2ByteTxRx(portHandlerPupil, 3, 37, &present_postition_value[1], &dxl_error))
            return false;

        AngleMinLimitPupil[0] = present_postition_value[0] + 10;
        AngleMinLimitPupil[1] = present_postition_value[1] + 10;

        AngleMaxLimitPupil[0] = AngleMinLimitPupil[0] + 280;
        AngleMaxLimitPupil[1] = AngleMinLimitPupil[1] + 280;

       // Not implemented.
       //Notify(msg_debug, "Position limits pupil servos (auto calibrate): min %i %i max %i %i \n", AngleMinLimitPupil[0], AngleMinLimitPupil[1], AngleMaxLimitPupil[0], AngleMaxLimitPupil[1]);

        // Torque off. No fancy rampiong
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 2, 24, 0, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write1ByteTxRx(portHandlerPupil, 3, 24, 0, &dxl_error))
            return false;
        Sleep(xlTimer);

        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 2, 35, 1023, &dxl_error))
            return false;
        if (COMM_SUCCESS != packetHandlerPupil->write2ByteTxRx(portHandlerPupil, 3, 35, 1023, &dxl_error))
            return false;
        Sleep(xlTimer);


        return true;
    }
    ~ServoControlTuning()
    {
       

        // Torque down
        PowerOffRobot();

        // Close ports
        if (EpiTorsoMode || EpiMode)
        {
            portHandlerHead->closePort();
            portHandlerPupil->closePort();
        }
        if (EpiMode)
        {
            portHandlerLeftArm->closePort();
            portHandlerRightArm->closePort();
            portHandlerBody->closePort();
        }

        // Free memory
        delete groupSyncWriteHead;
        delete groupSyncReadHead;
        delete groupSyncWriteLeftArm;
        delete groupSyncReadLeftArm;
        delete groupSyncWriteRightArm;
        delete groupSyncReadRightArm;
        delete groupSyncWriteBody;
        delete groupSyncReadBody;
        delete groupSyncWritePupil;
    }

};

INSTALL_CLASS(ServoControlTuning)
