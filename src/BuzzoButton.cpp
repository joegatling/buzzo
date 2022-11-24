
#include <Wifi.h>
#include <Arduino.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>


#include "BuzzoButton.h"
#include "Commands.h"

//#define DEMO_MODE
#define FADEOUT_TIME 1500

float mapf(float x, float in_min, float in_max, float out_min, float out_max) 
{
    const float run = in_max - in_min;
    if(run == 0){
        return out_min;
    }
    const float rise = out_max - out_min;
    const float delta = x - in_min;
    return (delta * rise) / run + out_min;
}


#pragma region Update Functions

void IdleEnter(BuzzoButton* button)
{
    // REMOVE THIS LATER
    button->_canBuzz = true;

    //TODO: Turn off the lights
    //TODO: Show current score

    button->_strip.SetBrightness(50);
    button->_strip.ClearTo(RgbColor(0,0,0));
}

void IdleUpdate(BuzzoButton* button)
{
    #ifdef DEMO_MODE

    button->_currentScore = (millis() / 1000) % 7;

    #endif

    if(button->_wasScoreUpdated)
    {
        button->_lastButtonPressTime = millis();
        Serial.println("Score was updated");
    }

    auto timeInState = millis() - max(button->_stateEnterTime, button->_lastButtonPressTime);

    const unsigned long fadeBegin = FADEOUT_TIME;
    const unsigned long fadeDuration = 1000;

    if(timeInState < fadeBegin)
    {
        button->_strip.SetBrightness(255);
        button->_strip.ClearTo(RgbColor(255,255,255));
        button->_strip.Show();
    }
    else if(timeInState < fadeBegin + fadeDuration)
    {
        if(button->_currentScore >= 6)
        {
            button->_strip.SetBrightness(255);
            // uint16_t hue = (millis() * 50) % 0xFFFF;
            // button->_strip.rainbow(hue,1,255, 32);
            for(int i = 0; i < button->_strip.PixelCount(); i++)
            {            
                button->_strip.SetPixelColor(i, button->_wedgeColors[i]);                 
            }
            button->_strip.Show();
        }
        else
        {

            button->_strip.SetBrightness(128);

            for(int i = 0; i < button->_strip.PixelCount(); i++)
            {
                uint8_t targetBrightness = i < button->_currentScore ? 128 : 8;
                uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, targetBrightness);

                button->_strip.SetBrightness(255);
                button->_strip.SetPixelColor(i, RgbColor(brightness, brightness, brightness));
            }
            button->_strip.Show();
        }
    }
}

void IdleExit(BuzzoButton* button)
{
    button->_canBuzz = false;
}

void AnsweringEnter(BuzzoButton* button)
{
    button->_hasStartedWarning = false;
    button->_toneGenerator.DoSound(ToneGenerator::RESPOND, false);
}

void AnsweringUpdate(BuzzoButton* button)
{
    button->_strip.SetBrightness(255);

    const float pulseAmount = 64;
    float pulse = 255 - pulseAmount + sin(millis() / 100.0f) * pulseAmount;

    int lightsToShow = ceil(button->_strip.PixelCount() *  ((float)button->_answeringTimeRemaining / (float)button->_answeringTotalTime));

    for(int i = 0; i < button->_strip.PixelCount(); i++)
    {
        if(i < lightsToShow)
        {
            button->_strip.SetPixelColor(i, RgbColor(pulse, pulse, 0));
        }
        else
        {
            button->_strip.SetPixelColor(i, RgbColor(8, 8, 0));
        }
    }
    
    if(button->_answeringTimeRemaining < 10)
    {
        if(button->_hasStartedWarning == false)
        {
            button->_hasStartedWarning = true;
            button->_toneGenerator.StartTicking();
        }

        button->_toneGenerator.SetTickingUrgency(1.0f - (button->_answeringTimeRemaining / 10.0f));
    }



    button->_strip.Show();
}

void AnsweringExit(BuzzoButton* button)
{
    button->_toneGenerator.StopTicking();    
}

void CorrectEnter(BuzzoButton* button)
{
    button->_strip.SetBrightness(255);
    button->_strip.ClearTo(RgbColor(0, 255, 0));
    button->_strip.Show();

    //button->_currentScore = 6;

    button->_canBuzz = false;   

    if(button->_currentScore == 6)
    {
        button->_toneGenerator.DoSound(ToneGenerator::VICTORY);
    }
    else
    {
        button->_toneGenerator.DoSound(ToneGenerator::CORRECT);
    }

}

void CorrectUpdate(BuzzoButton* button)
{
    if(button->_wasScoreUpdated)
    {
        button->_lastButtonPressTime = millis();
    }

    auto timeInState = millis() - max(button->_stateEnterTime, button->_lastButtonPressTime);

    const unsigned long fadeBegin = FADEOUT_TIME;
    const unsigned long fadeDuration = 1000;

    if(timeInState < fadeBegin)
    {
        button->_strip.SetBrightness(255);

        if(button->_currentScore >= 6)
        {
            uint16_t hue = (millis() * 50) % 0xFFFF;
            //button->_strip.rainbow(hue,1,255, 255);
        }
        else
        {
            button->_strip.ClearTo(RgbColor(0,255,0));
        }

        button->_strip.Show();
    }
    else if(timeInState < fadeBegin + fadeDuration)
    {
        if(button->_currentScore >= 6)
        {
            uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, 64);

            uint16_t hue = (millis() * 50) % 0xFFFF;
           // button->_strip.rainbow(hue,1,255, brightness);
        }
        else
        {
            for(int i = 0; i < button->_strip.PixelCount(); i++)
            {
                uint8_t targetBrightness = i < button->_currentScore ? 96 : 8;
                uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, targetBrightness);

                
                button->_strip.SetPixelColor(i, RgbColor(0, brightness, 0));
            }
        }


        button->_strip.Show();
    }
    else if(button->_currentScore >= 6)
    {
        uint16_t hue = (millis() * 50) % 0xFFFF;
        //button->_strip.rainbow(hue,1,255, 64);
        button->_strip.Show();
    }
}

void CorrectExit(BuzzoButton* button)
{}


void IncorrectEnter(BuzzoButton* button)
{
    button->_strip.SetBrightness(255);
    button->_strip.ClearTo(RgbColor(255,0,0));
    button->_strip.Show();

    button->_canBuzz = false;

    button->_toneGenerator.DoSound(ToneGenerator::INCORRECT);
}

void IncorrectUpdate(BuzzoButton* button)
{
    if(button->_wasScoreUpdated)
    {
        button->_lastButtonPressTime = millis();
    }


    auto timeInState = millis() - max(button->_stateEnterTime, button->_lastButtonPressTime);

    const unsigned long fadeBegin = FADEOUT_TIME;
    const unsigned long fadeDuration = 1000;

    if(timeInState < fadeBegin)
    {
        button->_strip.SetBrightness(255);
        button->_strip.ClearTo(RgbColor(255,0,0));
        button->_strip.Show();
    }
    else if(timeInState < fadeBegin + fadeDuration)
    {
        for(int i = 0; i < button->_strip.PixelCount(); i++)
        {
            uint8_t targetBrightness = i < button->_currentScore ? 96 : 8;
            uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, targetBrightness);

            button->_strip.SetPixelColor(i, RgbColor(brightness, 0, 0));
        }

        button->_strip.Show();
    }
}

void IncorrectExit(BuzzoButton* button)
{}


void QueuedEnter(BuzzoButton* button)
{}

void QueuedUpdate(BuzzoButton* button)
{
    button->_strip.SetBrightness(255);
    
    float offset = millis() / 2000.0f;
    offset = ((sin((offset + 0.5f)* TWO_PI) + (offset + 0.5f) * TWO_PI) / TWO_PI) - 0.5f;

    int count = button->_strip.PixelCount();

    float colorA = 0.5f;
    float colorB = 0.6f;

    uint8_t patterns[4] = { 0b0000001, 0b00001001, 0b00010101, 0b00000000 };

    int patternIndex = constrain(button->_placeInQueue - 1, 0, sizeof(patterns) - 1);
    
    for(int i = 0; i < button->_strip.PixelCount(); i++)
    {
        float index = fmodf(offset + i, count);
        float t = fmodf(index, 1.0f);

        int a = (patterns[patternIndex] & (1 << (int)index)) > 0;
        int b = (patterns[patternIndex] & 1 << ((int)(index+1) % count)) > 0;

        // if((int)index == 0)
        // {
        //     Serial.print("|");
        // }
        // Serial.print(a);

        float f = a * (1.0f - t) + b * t;

        float hue = mapf(f, 0.0f, 1.0f, colorB, colorA);
        float sat = 1.0f;
        float val = mapf(f, 0.0f, 1.0f, 0.1f, 0.5f);

        button->_strip.SetPixelColor(i, HsbColor(hue,sat,val));
    }

    button->_strip.Show();
}

void QueuedExit(BuzzoButton* button)
{
    button->_placeInQueue = -1;
}

void DisconnectedEnter(BuzzoButton* button)
{
    if(button->_hasConnected)
    {
        button->_toneGenerator.DoSound(ToneGenerator::DISCONNECTED);
    }
}

void DisconnectedUpdate(BuzzoButton* button)
{
    button->_strip.SetBrightness(96);

    float t = (sin(millis() / 500.0f) + 1.0f) / 2.0f;

    for(int i = 0; i < button->_strip.PixelCount(); i++)
    {
        bool isOn = ((millis() / 200) % button->_strip.PixelCount()) == i;
        button->_strip.SetPixelColor(i, isOn ? RgbColor::LinearBlend(RgbColor(255,0,0), RgbColor(255,255,255), t) : RgbColor(0,0,0));
    }    
    button->_strip.Show();
}

void DisconnectedExit(BuzzoButton* button)
{
    if(button->_nextState != BuzzoButton::NONE)
    {
        button->_toneGenerator.DoSound(ToneGenerator::CONNECTED);
    }

    button->_strip.ClearTo(RgbColor(0));
    button->_strip.Show();

    button->_hasConnected = true;
}


#pragma endregion


void OnButtonPress(BuzzoButton* button)
{
    button->_lastButtonPressTime = millis();

    if(button->_canBuzz)
    {
        button->SendBuzzCommand();
    }

    bool shouldPlaySound = true;

    //shouldPlaySound &= button->GetState() != BuzzoButton::DISCONNECTED;
    shouldPlaySound &= button->GetState() == BuzzoButton::IDLE;
    shouldPlaySound &= button->GetScore() < NUM_LED;

    if(shouldPlaySound)
    {
        button->_toneGenerator.DoSound(ToneGenerator::BUZZ);
    }

}

void OnButtonHold(BuzzoButton* button)
{
    button->DisableLightsAndSound();
    button->SetState(BuzzoButton::NONE);
    delay(100);
}

void OnButtonHoldRelease(BuzzoButton* button)
{
    WiFi.disconnect(true);
    button->SetState(BuzzoButton::NONE);
        
    delay(100);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
    esp_deep_sleep_start();   
}

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

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
_controllerIp(192,168,1,1),
_currentScore(0),
_canBuzz(false),
_button(BUZZER_BUTTON_PIN),
_strip(6, NEOPIXEL_PIN),
_lastButtonPressTime(0),
_currentState(BuzzoButton::NONE),
_nextState(BuzzoButton::NONE),
_toneGenerator(),
_hasConnected(false),
_wasScoreUpdated(false)
{
    _strip.Begin();
    _strip.SetBrightness(50);
    _strip.Show(); // Initialize all pixels to 'off'

    _wedgeColors[0] = RgbColor(255,255,255);
    _wedgeColors[1] = RgbColor(0,255,0);
    _wedgeColors[2] = RgbColor(255,0,0);
    _wedgeColors[3] = RgbColor(0,255,255);
    _wedgeColors[4] = RgbColor(255,0,255);
    _wedgeColors[5] = RgbColor(255,96,0);
    _wedgeColors[6] = RgbColor(255,255,0);

    for(int i = 0; i < STATE_COUNT; i++)
    {
        _states[i] = 0;
    }    

    _states[IDLE] = new BuzzoButtonState(&IdleEnter, &IdleUpdate, &IdleExit);
    _states[ANSWERING] = new BuzzoButtonState(&AnsweringEnter, &AnsweringUpdate, &AnsweringExit);
    _states[CORRECT] = new BuzzoButtonState(&CorrectEnter, &CorrectUpdate, &CorrectExit);
    _states[INCORRECT] = new BuzzoButtonState(&IncorrectEnter, &IncorrectUpdate, &IncorrectExit);
    _states[QUEUED] = new BuzzoButtonState(&QueuedEnter, &QueuedUpdate, &QueuedExit);
    _states[DISCONNECTED] = new BuzzoButtonState(&DisconnectedEnter, &DisconnectedUpdate, &DisconnectedExit);

    _button.SetBeginPressCallback([]() { OnButtonPress(BuzzoButton::GetInstance()); });    
    _button.SetBeginHoldCallback([]() { OnButtonHold(BuzzoButton::GetInstance()); });    
    _button.SetEndHoldCallback([]() { OnButtonHoldRelease(BuzzoButton::GetInstance()); });    
    _button.SetLongPressDuration(3000);

    udp.begin(PORT);

    memset(_uniqueId, '\0', sizeof(_uniqueId));
    strcpy(_uniqueId, WiFi.macAddress().c_str());
    Serial.print("Unique ID: ");
    Serial.println(_uniqueId);

}

void BuzzoButton::Initialize()
{}

void BuzzoButton::Update()
{
    _button.Update();
    _toneGenerator.Update();
    
    if(_nextState != _currentState)
    {

        if(_states[_currentState] != 0)
        {
            _states[_currentState]->Exit(this);
        }
        
        _currentState = _nextState;
        _stateEnterTime = millis();

        if(_states[_currentState] != 0)
        {
            _states[_currentState]->Enter(this);
        }

        
    }

    if(_states[_currentState] != 0)
    {
        _states[_currentState]->Update(this);
    }

    _wasScoreUpdated = false;

    //Serial.println("State Update End");
    if(_currentState != BuzzoButton::DISCONNECTED && _currentState != BuzzoButton::NONE)
    {
        ProcessPacket();

        // Periodically send a register command
        if(millis() - _lastSendTime > PING_INTERVAL || _lastSendTime == 0) 
        {
            SendRegisterCommand(_uniqueId);
            _lastSendTime = millis();
        }
    }
}


void BuzzoButton::ProcessPacket()
{
    int packetSize = udp.parsePacket();
    if (packetSize) 
    {    
        // Serial.printf("Received packet of size %d from %s:%d\n \n", 
        // packetSize, 
        // udp.remoteIP().toString().c_str(), 
        // udp.remotePort());

        // read the packet into packetBufffer
        int n = udp.read(packetBuffer, PACKET_MAX_SIZE);
        packetBuffer[n] = 0;

        Serial.print("Received: ");
        Serial.println(packetBuffer); 

        std::stringstream data(packetBuffer);
        std::string command;

        data >> command;

        bool error = false;

        if(command.length() == 3)
        {
            std::string params[2];
            int paramCount = 0;

            while(!data.eof() && paramCount < sizeof(params))
            {
                if(paramCount == 0)
                {
                    Serial.println("Params:");
                }

                data >> params[paramCount];
                
                Serial.print(paramCount);
                Serial.print(": ");
                Serial.println(params[paramCount].c_str());

                paramCount++;
            }
            
            if(command.compare(COMMAND_ANSWER) == 0)
            {            
                if(paramCount >= 2)
                {
                    int timeLeft = atoi(params[0].c_str());
                    int totalTime = atoi(params[1].c_str());

                    // Some basic sanity checks
                    if(totalTime > 0 && timeLeft <= totalTime)
                    {
                        ProcessAnswerCommand(timeLeft, totalTime);
                    }
                    else
                    {
                        error = true;
                    }
                }
                else
                {
                    error = true;
                }               
            }
            else if(command.compare(COMMAND_QUEUE) == 0)
            {
                if(paramCount > 0)
                {
                    ProcessQueueCommand(atoi(params[0].c_str()));
                }
                else
                {
                    error = true;
                }
            } 
            else if(command.compare(COMMAND_SCORE) == 0)
            {
                if(paramCount > 0)
                {
                    ProcessScoreCommand(atoi(params[0].c_str()));
                }
                else
                {
                    error = true;
                }
            }              
            else if(command.compare(COMMAND_CORRECT_RESPONSE) == 0)
            {
                ProcessCorrectResponseCommand();
            }
            else if(command.compare(COMMAND_INCORRECT_RESPONSE) == 0)
            {
                ProcessIncorrectResponseCommand();
            } 
            else if(command.compare(COMMAND_RESET) == 0)
            {
                if(paramCount > 0)
                {
                    ProcessResetCommand((bool)atoi(params[0].c_str()));
                }
                else
                {
                    error = true;
                }
            }
            else if(command.compare(COMMAND_SELECT) == 0)
            {
                ProcessSelectCommand();
            }   
            else if(command.compare(COMMAND_SLEEP) == 0)
            {
                ProcessSleepCommand();
            }                                                                                                                                    
        }
        else
        {
            error = true;
        }

        if(error)
        {
            Serial.println("ERROR");
        }
    }
}


void BuzzoButton::SetState(StateId newState)
{
    _nextState = newState;
}

BuzzoButton::StateId BuzzoButton::GetState()
{
    return _currentState;
}

void BuzzoButton::DisableLightsAndSound()
{
    _strip.ClearTo(RgbColor(0));
    _strip.Show();

    _toneGenerator.ClearSoundQueue();
}

void BuzzoButton::ProcessAnswerCommand(int timeLeft, int totalTime)
{
    SetState(BuzzoButton::ANSWERING);
    _answeringTimeRemaining = timeLeft;
    _answeringTotalTime = totalTime;
}

void BuzzoButton::ProcessQueueCommand(int placeInQueue)
{
    _placeInQueue = placeInQueue;    
    SetState(BuzzoButton::QUEUED);
}

void BuzzoButton::ProcessCorrectResponseCommand()
{
    SetState(BuzzoButton::CORRECT);
}

void BuzzoButton::ProcessIncorrectResponseCommand()
{
    SetState(BuzzoButton::INCORRECT);
}

void BuzzoButton::ProcessResetCommand(bool canBuzz)
{
    if(GetState() == BuzzoButton::IDLE)
    {
        _lastButtonPressTime = millis();
    }
    else
    {
        SetState(BuzzoButton::IDLE);
    }
    
    _canBuzz = canBuzz;

    if(_canBuzz)
    {
        _toneGenerator.DoSound(ToneGenerator::ACKNOWLEDGE);
    }
}

void BuzzoButton::ProcessSelectCommand()
{
    // TODO
}

void BuzzoButton::ProcessScoreCommand(int score)
{
    _currentScore = score;
    _wasScoreUpdated = true;
}

void BuzzoButton::ProcessSleepCommand()
{
    DisableLightsAndSound();
    sleep(100);
    OnButtonHoldRelease(this);
}


void BuzzoButton::SendRegisterCommand(char* param)
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_REGISTER);
    udp.print(" ");
    udp.print(param);
    udp.endPacket();

    
}

void BuzzoButton::SendBuzzCommand()
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_BUZZ);
    udp.endPacket();
}
