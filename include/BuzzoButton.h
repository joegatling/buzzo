#pragma once

#include <WiFiudp.h>

#define PACKET_MAX_SIZE 1024

#define PORT 8888

class BuzzoButton
{
    public:
        static BuzzoButton* GetInstance();

        void Initialize();

        void Update();

    private:
        static BuzzoButton* _instance;

        BuzzoButton();

        char packetBuffer[PACKET_MAX_SIZE + 1]; 

        WiFiUDP udp;

        void ProcessPacket();

        unsigned int _lastSendTime = 0;

        IPAddress _controllerIp;
};