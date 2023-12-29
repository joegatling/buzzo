#pragma once

#include <WiFiudp.h>
//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>
#include <SimpleButton.h>

#include "BuzzoButtonState.h"
#include "ToneGenerator.h"

#define PACKET_MAX_SIZE 1024
#define PING_INTERVAL 20000
#define PORT 8888

#define NUM_LED 6

#define BUZZER_BUTTON_PIN 32
#define NEOPIXEL_PIN 14

#define UNIQUE_ID_LEN 32

class BuzzoButtonState;

class BuzzoButton
{
    public:
        enum StateId
        {
            NONE = 0,
            IDLE, 
            ANSWERING,
            CORRECT,
            INCORRECT,
            QUEUED,
            SELECTED,
            DISCONNECTED,
            STATE_COUNT
        };

        static BuzzoButton* GetInstance();

        void Initialize();
        void Update();

        void SetBatteryLevel(float level) { _batteryLevel = level; }

        StateId GetState();
        void SetState(StateId newState);

        int GetScore() { return _currentScore; }
        int GetPlaceInQueue() { return _placeInQueue; }

        unsigned long TimeSinceLastButtonPress() { return millis() - _lastButtonPressTime; } 

        void ShowBatteryLevelOnButton();

        void DisableLightsAndSound();

    private:
        static BuzzoButton* _instance;

        BuzzoButton();
        void ProcessPacket();

        void ProcessAnswerCommand(int timeLeft, int totalTime);
        void ProcessQueueCommand(int placeInQueue);
        void ProcessCorrectResponseCommand();
        void ProcessIncorrectResponseCommand();
        void ProcessResetCommand(bool canBuzz);
        void ProcessSelectCommand();
        void ProcessScoreCommand(int score);
        void ProcessSleepCommand();

        void SendRegisterCommand(char* param);
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

        friend void DisconnectedEnter(BuzzoButton* button);
        friend void DisconnectedUpdate(BuzzoButton* button);
        friend void DisconnectedExit(BuzzoButton* button);    

        friend void SelectedEnter(BuzzoButton* button);
        friend void SelectedUpdate(BuzzoButton* button);
        friend void SelectedExit(BuzzoButton* button);      

        friend void OnButtonPress(BuzzoButton* button);

        char packetBuffer[PACKET_MAX_SIZE + 1]; 

        WiFiUDP udp;
        unsigned int _lastSendTime = 0;
        IPAddress _controllerIp;

        BuzzoButtonState* _states[STATE_COUNT];

        StateId _currentState;
        StateId _nextState;

        SimpleButton _button;

        char _uniqueId[UNIQUE_ID_LEN];

        int _currentScore;
        int _answeringTimeRemaining;
        int _answeringTotalTime;        
        int _placeInQueue;
        bool _isAnswerTimePaused;

        bool _hasStartedWarning;

        bool _isShowingScore;
        bool _wasScoreUpdated;

        bool _hasConnected;

        unsigned long _stateEnterTime;
        unsigned long _lastButtonPressTime;

        bool _canBuzz;

        float _batteryLevel;
        bool _isBlinking; 

        RgbColor _wedgeColors[7];

        //Adafruit_NeoPixel _strip;
        NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> _strip;

        ToneGenerator _toneGenerator;

};