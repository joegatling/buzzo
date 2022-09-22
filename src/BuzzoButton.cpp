
#include <Wifi.h>
#include <Arduino.h>

#include "BuzzoButton.h"
#include "Commands.h"

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
    button->_currentScore = 6;

    //TODO: Turn off the lights
    //TODO: Show current score

    button->_strip.setBrightness(50);
    button->_strip.clear();



    button->_lastButtonPressTime = 0;
}

void IdleUpdate(BuzzoButton* button)
{
    if(button->_lastButtonPressTime > 0 && millis() - button->_lastButtonPressTime < 200)
    {
        button->_strip.setBrightness(255);
        button->_strip.fill(button->_strip.Color(255,255,255));
        button->_strip.show();
    }
    else
    {
        if(button->_currentScore >= 6)
        {
            button->_strip.setBrightness(255);
            uint16_t hue = (millis() * 50) % 0xFFFF;
            button->_strip.rainbow(hue,1,255, 32);
        }
        else
        {
            button->_strip.setBrightness(64);

            for(int i = 0; i < button->_strip.numPixels(); i++)
            {
                if(i < button->_currentScore)
                {
                    button->_strip.setPixelColor(i, 255, 255, 255);                 
                }
                else
                {
                    button->_strip.setPixelColor(i, 0, 0, 0);                 
                }
            }
        }
        button->_strip.show();
    }
}

void IdleExit(BuzzoButton* button)
{
    button->_canBuzz = false;
}

void AnsweringEnter(BuzzoButton* button)
{}

void AnsweringUpdate(BuzzoButton* button)
{
    float timeLeft = (millis() - button->_stateEnterTime) / 30000.0f;
    uint8_t debugTimer = max(0.0f, 255 - (timeLeft * 255));
    float step = 255 / (float)button->_strip.numPixels();    

    Serial.println(debugTimer);

    button->_strip.setBrightness(255);

    const float pulseAmount = 64;
    float pulse = 255 - pulseAmount + sin(millis() / 100.0f) * pulseAmount;

    for(int i = 0; i < button->_strip.numPixels(); i++)
    {
        if(i < ceil(debugTimer / step))
        {
            button->_strip.setPixelColor(i, pulse, pulse, 0);
        }
        else
        {
            button->_strip.setPixelColor(i, 4, 4, 0);
        }
    }

    // if(debugTimer == 0)
    // {
    //     button->SetState(BuzzoButton::INCORRECT);
    // }

    button->_strip.show();
}

void AnsweringExit(BuzzoButton* button)
{}

void CorrectEnter(BuzzoButton* button)
{
    button->_strip.setBrightness(255);
    button->_strip.fill(button->_strip.Color(0, 255, 0));
    button->_strip.show();

    button->_currentScore = 6;

    button->_canBuzz = false;   
}

void CorrectUpdate(BuzzoButton* button)
{
    auto timeInState = millis() - max(button->_stateEnterTime, button->_lastButtonPressTime);

    const unsigned long fadeBegin = 3000;
    const unsigned long fadeDuration = 1000;

    if(timeInState < fadeBegin)
    {
        button->_strip.setBrightness(255);

        if(button->_currentScore >= 6)
        {
            uint16_t hue = (millis() * 50) % 0xFFFF;
            button->_strip.rainbow(hue,1,255, 255);
        }
        else
        {
            button->_strip.fill(button->_strip.Color(0,255,0));
        }

        button->_strip.show();
    }
    else if(timeInState < fadeBegin + fadeDuration)
    {
        if(button->_currentScore >= 6)
        {
            uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, 64);

            uint16_t hue = (millis() * 50) % 0xFFFF;
            button->_strip.rainbow(hue,1,255, brightness);
        }
        else
        {
            for(int i = 0; i < button->_strip.numPixels(); i++)
            {
                uint8_t targetBrightness = i < button->_currentScore ? 96 : 8;
                uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, targetBrightness);

                
                button->_strip.setPixelColor(i, 0, brightness, 0);
            }
        }


        button->_strip.show();
    }
    else if(button->_currentScore >= 6)
    {
        uint16_t hue = (millis() * 50) % 0xFFFF;
        button->_strip.rainbow(hue,1,255, 64);
        button->_strip.show();
    }
}

void CorrectExit(BuzzoButton* button)
{}


void IncorrectEnter(BuzzoButton* button)
{
    button->_strip.setBrightness(255);
    button->_strip.fill(button->_strip.Color(255,0,0));
    button->_strip.show();

    button->_canBuzz = false;
}

void IncorrectUpdate(BuzzoButton* button)
{
    auto timeInState = millis() - max(button->_stateEnterTime, button->_lastButtonPressTime);

    const unsigned long fadeBegin = 3000;
    const unsigned long fadeDuration = 1000;

    if(timeInState < fadeBegin)
    {
        button->_strip.setBrightness(255);
        button->_strip.fill(button->_strip.Color(255,0,0));
        button->_strip.show();
    }
    else if(timeInState < fadeBegin + fadeDuration)
    {
        for(int i = 0; i < button->_strip.numPixels(); i++)
        {
            uint8_t targetBrightness = i < button->_currentScore ? 96 : 8;
            uint8_t brightness = mapf((timeInState - fadeBegin) / (float)(fadeDuration), 0.0f, 1.0f, 255, targetBrightness);

            button->_strip.setPixelColor(i, brightness, 0, 0);
        }

        button->_strip.show();
    }
}

void IncorrectExit(BuzzoButton* button)
{}


void QueuedEnter(BuzzoButton* button)
{
    button->_strip.setBrightness(255);
    button->_placeInQueue = 1;
}

void QueuedUpdate(BuzzoButton* button)
{
    float offset = millis() / 2000.0f;
    offset = ((sin((offset + 0.5f)* TWO_PI) + (offset + 0.5f) * TWO_PI) / TWO_PI) - 0.5f;

    int count = button->_strip.numPixels();

    uint16_t colorA = 40000;
    uint16_t colorB = 45000;

    uint8_t patterns[3] = { 0b0000001, 0b00001001, 0b00000000 };

    int patternIndex = constrain(button->_placeInQueue - 1, 0, sizeof(patterns) - 1);
    
    // if(patternIndex > sizeof(patterns) - 1)
    // {
    //     patternIndex = sizeof(patterns) - 1;
    // }

    // Serial.print(patterns[button->_placeInQueue], BIN);
    // Serial.print(" -> ");
    for(int i = 0; i < button->_strip.numPixels(); i++)
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

        uint16_t hue = mapf(f, 0.0f, 1.0f, colorB, colorA);
        uint8_t sat = 255;
        uint8_t val = (uint8_t)mapf(f, 0.0f, 1.0f, 1.0f, 20.0f);

        auto color = button->_strip.ColorHSV(hue,sat,val);
        button->_strip.setPixelColor(i, color);
    }


    //Serial.println("");

    button->_strip.show();
}

void QueuedExit(BuzzoButton* button)
{
    button->_placeInQueue = -1;
}

void DisconnectedEnter(BuzzoButton* button)
{}

void DisconnectedUpdate(BuzzoButton* button)
{
    for(int i = 0; i < button->_strip.numPixels(); i++)
    {
        bool isOn = ((millis() / 200) % button->_strip.numPixels()) == i;
        button->_strip.setPixelColor(i, isOn ? button->_strip.Color(64,64,64) : button->_strip.Color(0,0,0));
    }    
    button->_strip.show();
}

void DisconnectedExit(BuzzoButton* button)
{
    button->_strip.clear();
    button->_strip.show();
}


#pragma endregion


void OnButtonPress(BuzzoButton* button)
{
    button->_lastButtonPressTime = millis();

    if(button->_canBuzz)
    {
        button->SendBuzzCommand();
    }
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
_strip(6, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800),
_lastButtonPressTime(0),
_currentState(BuzzoButton::NONE),
_nextState(BuzzoButton::NONE)
{
    _strip.begin();
    _strip.setBrightness(50);
    _strip.show(); // Initialize all pixels to 'off'

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

    //Serial.println("State Update End");
    if(_currentState != BuzzoButton::DISCONNECTED)
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

            for(int i = 0; i < 3; i++)
            {
                command[i] = packetBuffer[i];
            }

            std::string param;

            if(packetSize > 3)
            {
                param.assign(command+3);
                param = trim(param);
            }            

            // Process Packet
            if(strcmp(command, COMMAND_ANSWER) == 0)
            {
                if(param.length() > 0)
                {
                    ProcessAnswerCommand(atoi(param.c_str()));
                }
            }
            else if(strcmp(command, COMMAND_QUEUE) == 0)
            {
                if(param.length() > 0)
                {
                    ProcessQueueCommand(atoi(param.c_str()));
                }
            } 
            else if(strcmp(command, COMMAND_SCORE) == 0)
            {
                if(param.length() > 0)
                {
                    ProcessScoreCommand(atoi(param.c_str()));
                }
            }              
            else if(strcmp(command, COMMAND_CORRECT_RESPONSE) == 0)
            {
                ProcessCorrectResponseCommand();
            }
            else if(strcmp(command, COMMAND_INCORRECT_RESPONSE) == 0)
            {
                ProcessIncorrectResponseCommand();
            } 
            else if(strcmp(command, COMMAND_RESET) == 0)
            {
                if(param.length() > 0)
                {
                    ProcessResetCommand((bool)atoi(param.c_str()));
                }                
            }
            else if(strcmp(command, COMMAND_OFF) == 0)
            {
                ProcessOffCommand();
            }
            else if(strcmp(command, COMMAND_SELECT) == 0)
            {
                ProcessSelectCommand();
            }                                                                                                                          
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


void BuzzoButton::ProcessAnswerCommand(int timer)
{
    SetState(BuzzoButton::ANSWERING);
    _answeringTimeRemaining = timer;
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
    SetState(BuzzoButton::IDLE);
    _canBuzz = canBuzz;
}

void BuzzoButton::ProcessOffCommand()
{
    // TOOD
}

void BuzzoButton::ProcessSelectCommand()
{
    // TODO
}

void BuzzoButton::ProcessScoreCommand(int score)
{
    _currentScore = score;
}


void BuzzoButton::SendRegisterCommand(char* param)
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_REGISTER);
    udp.print(" ");
    udp.println(param);
    udp.endPacket();

    
}

void BuzzoButton::SendBuzzCommand()
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_BUZZ);
    udp.endPacket();
}
