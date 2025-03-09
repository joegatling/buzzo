#pragma once

#include <string>
//#include <WiFiudp.h>
//#include <WiFi.h>


#include <SimpleButton.h>

#include "ButtonClientInfo.h"
#include "ResponseQueue.h"

#define PACKET_MAX_SIZE 256
#define PACKET_MAX_QUEUE 4

#define MAX_CLIENTS 12

//#define PORT 8888


#ifdef NEW_PINS
    // WHITE
    #define PAUSE_BUTTON_PIN 33
    // GREY
    #define CORRECT_BUTTON_PIN 15
    // PURPLE
    #define INCORRECT_BUTTON_PIN 32
    // BLUE
    #define RESET_BUTTON_PIN 14
#else
    #define PAUSE_BUTTON_PIN 33
    #define CORRECT_BUTTON_PIN 32
    #define INCORRECT_BUTTON_PIN 14
    #define RESET_BUTTON_PIN 15
#endif

struct ControllerReceivedPacketData
{
    uint8_t mac[6];
    char packetBuffer[PACKET_MAX_SIZE + 1];
};

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


        void EndCurrentRespondantTurn(bool isCorrect);
        void BeginIncorrectButtonPress();
        void ResetButtonPress();      
        void HoldResetButton();  
        void ReleaseHoldResetButton();
        void ReleaseHoldIncorrectButton();

        void BeginPauseButtonPress();
        void EndPauseButtonPress();

        void AdjustPreviousRespondant(bool isCorrect);

        void SetAllClientsToSleep();

        unsigned long TimeSinceLastButtonPress() { return millis() - _lastButtonPressTime; } 

        int GetActiveClientCount();
        int GetActiveClientsYetToPlayCount();

        unsigned int GetMinBatteryLevelForClients();

        bool IsGoingToSleep() { return _shouldSleep; }

        void EnqueuePacketData(const uint8_t *mac, const char* packetBuffer);

    private:
        static BuzzoController* _instance;

        BuzzoController();

        void ProcessRegisterCommand(const uint8_t *mac, std::string paramId, std::string paramBattery);
        void ProcessBuzzCommand(const uint8_t *mac);

        void SendAnswerCommand(const uint8_t *mac, int timer, int totalTime);
        void SendQueueCommand(const uint8_t *mac, int placeInQueue);
        void SendResponseCommand(const uint8_t *mac, bool isCorrect);
        void SendResetCommand(const uint8_t *mac, bool canBuzz);
        void SendSelectCommand(const uint8_t *mac);
        void SendErrorCommand(const uint8_t *mac, int errorCode);
        void SendScoreCommand(const uint8_t *mac, int score);
        void SendSleepCommand(const uint8_t *mac);

        void UpdatePlaying();
        void UpdateSetup();

        void AddAllRemainingClientsToQueue();
        
        void AddClient(ButtonClientInfo* newClient);
        void RemoveClientAt(unsigned int index);
        ButtonClientInfo* GetClient(const uint8_t *mac);
        ButtonClientInfo* GetClient(std::string id);

        void ProcessPacketQueue();
        void ProcessPacket(const uint8_t *mac, const char* packetBuffer);
        
        ControllerReceivedPacketData packetQueue[PACKET_MAX_QUEUE];
        unsigned int _packetQueueIndex = 0;
        unsigned int _packetQueueCount = 0;
        


        ButtonClientInfo* _clients[MAX_CLIENTS];
        unsigned int _clientCount = 0;

        char packetBuffer[PACKET_MAX_SIZE + 1]; 

        //WiFiUDP udp;

        unsigned long _lastSendTime = 0;
        unsigned long _responseStartTime = 0;
        unsigned long _nextResponderDelayStartTime = 0;
        unsigned long _lastRespondantPingTime = 0;
        unsigned long _lastButtonPressTime = 0;
        unsigned long _responsePauseTime = 0;
        unsigned long _lastMillis = 0;

        unsigned long _autoResestTime = 0;

        bool _isAcceptingResponses = true;
        bool _isReset = false;
        bool _shouldSleep = false;
        bool _isInAdjustMode = false;
        bool _isPaused = false;

        ControllerState _currentState = BuzzoController::PLAYING;

        SimpleButton _correctButton;
        SimpleButton _incorrectButton;
        SimpleButton _resetButton;
        SimpleButton _pauseButton;

        ResponseQueue<std::string> _responseQueue;
        std::string _currentRespondant;
        std::string _previousRespondant;
        bool _previousRespondantWasCorrect = false;

        std::string _participants[MAX_CLIENTS];
        unsigned int _participantCount;

};