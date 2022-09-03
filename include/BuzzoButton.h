#pragma once

#include <WiFiudp.h>
#include "BuzzoButtonState.h"

#define PACKET_MAX_SIZE 1024
#define PING_INTERVAL 20000
#define PORT 8888

class BuzzoButtonState;

class BuzzoButton
{
    public:
        enum StateId
        {
            UNSET = -1,
            NONE = 0,
            IDLE, 
            ANSWERING,
            CORRECT,
            INCORRECT,
            QUEUED,
            STATE_COUNT = QUEUED
        };

        static BuzzoButton* GetInstance();

        void Initialize();
        void Update();

        StateId GetState();
        void SetState(StateId newState);

        int GetScore() { return _currentScore; }
        int GetPlaceInQueue() { return _placeInQueue; }

    private:
        static BuzzoButton* _instance;

        BuzzoButton();
        void ProcessPacket();

        void ProcessAnswerCommand(int timer);
        void ProcessQueueCommand(int placeInQueue);
        void ProcessCorrectResponseCommand();
        void ProcessIncorrectResponseCommand();
        void ProcessResetcCommand(bool canBuzz);
        void ProcessOffCommand();
        void ProcessSelectCommand();
        void ProcessScoreCommand(int score);

        void SendRegisterCommand(std::string param);
        void SendBuzzCommand();

        friend void IdleEnter(BuzzoButton* button);
        friend void IdleUpdate(BuzzoButton* button);
        friend void IdleExit(BuzzoButton* button);

        friend void AnsweringEnter(BuzzoButton* button);
        friend void AnsweringUpdate(BuzzoButton* button);
        friend void AnsweringExit(BuzzoButton* button);

        friend void CorrectEnter(BuzzoButton* button);
        friend void CorrectUpdate(BuzzoButton* button);
        friend void CorrectExit(BuzzoButton* button);

        friend void IncorrectEnter(BuzzoButton* button);
        friend void IncorrectUpdate(BuzzoButton* button);
        friend void IncorrectExit(BuzzoButton* button);

        friend void QueuedEnter(BuzzoButton* button);
        friend void QueuedUpdate(BuzzoButton* button);
        friend void QueuedExit(BuzzoButton* button);

        char packetBuffer[PACKET_MAX_SIZE + 1]; 

        WiFiUDP udp;
        unsigned int _lastSendTime = 0;
        IPAddress _controllerIp;

        BuzzoButtonState* _states[STATE_COUNT];
        StateId _currentState = NONE;
        StateId _nextState = UNSET;

        std::string _uniqueId;

        int _currentScore;
        int _answeringTimeRemaining;
        int _placeInQueue;

        bool _canBuzz;
};