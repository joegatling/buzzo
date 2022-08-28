#include "BuzzoButton.h"
#include "Commands.h"

BuzzoButton* BuzzoButton::_instance = 0;
BuzzoButton* BuzzoButton::GetInstance()
{
    if(_instance == 0)
    {
        _instance = new BuzzoButton();
    }

    return _instance;
}

BuzzoButton::BuzzoButton() :
_controllerIp(192,168,1,1)
{
    udp.begin(PORT);
}

void BuzzoButton::Initialize()
{}

void BuzzoButton::Update()
{
    ProcessPacket();

    if(millis() - _lastSendTime > 1000)
    {
        _lastSendTime = millis();

        udp.beginPacket(_controllerIp, PORT);
        udp.println(COMMAND_BUZZ);
        udp.endPacket();
    }
}


void BuzzoButton::ProcessPacket()
{
    int packetSize = udp.parsePacket();
    if (packetSize) 
    {    
        Serial.printf("Received packet of size %d from %s:%d\n \n", 
        packetSize, 
        udp.remoteIP().toString().c_str(), 
        udp.remotePort());

        // read the packet into packetBufffer
        int n = udp.read(packetBuffer, PACKET_MAX_SIZE);
        packetBuffer[n] = 0;

        Serial.println("Contents:");
        Serial.println(packetBuffer); 

        if(packetSize >= 3)
        {
            char command[LEN_COMMAND+1];
            command[LEN_COMMAND] = 0;

            // char command[LEN_COMMAND+1];
            // command[LEN_COMMAND] = 0;            

            for(int i = 0; i < 3; i++)
            {
                command[i] = packetBuffer[i];
            }

            // Process Packet
            if(strcmp(command, COMMAND_ANSWER) == 0)
            {
                // Put button in answering mode, indicating the player should answer now

            }
            else if(strcmp(command, COMMAND_QUEUE) == 0)
            {
                // Put button in queue mode, indicating that the player will answer soon
            } 
            else if(strcmp(command, COMMAND_CORRECT_RESPONSE) == 0)
            {
                // Put button in correct response mode, indicating the player got the correct answer

            }
            else if(strcmp(command, COMMAND_INCORRECT_RESPONSE) == 0)
            {
                // Put button in correct response mode, indicating the player anwered incorrectly
            } 
            else if(strcmp(command, COMMAND_RESET) == 0)
            {
                // Put button in idle mode

            }
            else if(strcmp(command, COMMAND_OFF) == 0)
            {
                // Turn off the button and put it in standby mode

            }
            else if(strcmp(command, COMMAND_SELECT) == 0)
            {
                // Put the button in selected mode, for debug purposes
            }                                                                                                                          
        }
    }
}
