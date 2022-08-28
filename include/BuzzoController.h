#pragma once

#include <string>
#include <WiFiudp.h>

#include "ButtonClientInfo.h"
#include "SimpleButton.h"
#include "ResponseQueue.h"

#define PACKET_MAX_SIZE 1024
#define MAX_CLIENTS 12

#define PORT 8888

#define CORRECT_BUTTON_PIN 10
#define INCORRECT_BUTTON_PIN 11
#define RESET_BUTTON_PIN 12

class BuzzoController
{
    public:
        enum ControllerState
        {
            PLAYING,
            SETUP
        };

        static BuzzoController* GetInstance();
        void Initialize();
        void Update();

    private:
        static BuzzoController* _instance;

        BuzzoController();

        void ProcessPacket();

        void ProcessRegisterCommand(IPAddress ip, std::string param);
        void ProcessBuzzCommand(IPAddress ip);

        void SendAnswerCommand(IPAddress ip, int timer);
        void SendQueueCommand(IPAddress ip, int placeInQueue);
        void SendResponseCommand(IPAddress ip, bool isCorrect);
        void SendResetCommand(IPAddress ip);
        void SendOffCommand(IPAddress ip);
        void SendSelectCommand(IPAddress ip);
        void SendErrorCommand(IPAddress ip, int errorCode);
        
        void AddClient(ButtonClientInfo* newClient);
        void RemoveClientAt(unsigned int index);
        ButtonClientInfo* GetClient(IPAddress ip);
        ButtonClientInfo* GetClient(std::string id);

        ButtonClientInfo* _clients[MAX_CLIENTS];
        unsigned int _clientCount = 0;

        char packetBuffer[PACKET_MAX_SIZE + 1]; 

        WiFiUDP udp;

        unsigned int _lastSendTime = 0;
        unsigned int _responseStartTime = 0;

        ControllerState _currentState = BuzzoController::PLAYING;

        SimpleButton _correctButton;
        SimpleButton _incorrectButton;
        SimpleButton _ResetButton;

        ResponseQueue<std::string> _responseQueue;
        std::string _currentRespondant;
};