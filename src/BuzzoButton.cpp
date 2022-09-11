
#include <Wifi.h>

#include "BuzzoButton.h"
#include "Commands.h"


#pragma region Update Functions

void IdleEnter(BuzzoButton* button)
{
    // REMOVE THIS LATER
    button->_canBuzz = true;

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
        uint16_t hue = (millis() * 20) % 0xFFFF;
        button->_strip.rainbow(hue,1,255,64);
        button->_strip.show();
    }
}

void IdleExit(BuzzoButton* button)
{
    button->_canBuzz = false;
}

void WaitingEnter(BuzzoButton* button)
{}

void WaitingUpdate(BuzzoButton* button)
{}

void WaitingExit(BuzzoButton* button)
{}

void AnsweringEnter(BuzzoButton* button)
{}

void AnsweringUpdate(BuzzoButton* button)
{}

void AnsweringExit(BuzzoButton* button)
{}

void CorrectEnter(BuzzoButton* button)
{}

void CorrectUpdate(BuzzoButton* button)
{}

void CorrectExit(BuzzoButton* button)
{}


void IncorrectEnter(BuzzoButton* button)
{}

void IncorrectUpdate(BuzzoButton* button)
{}

void IncorrectExit(BuzzoButton* button)
{}


void QueuedEnter(BuzzoButton* button)
{}

void QueuedUpdate(BuzzoButton* button)
{}

void QueuedExit(BuzzoButton* button)
{
    button->_placeInQueue = -1;
}

void DisconnectedEnter(BuzzoButton* button)
{}

void DisconnectedUpdate(BuzzoButton* button)
{

}

void DisconnectedExit(BuzzoButton* button)
{
    button->_strip.clear();
    button->_strip.show();
}


#pragma endregion


void OnButtonPress(BuzzoButton* button)
{
    Serial.println("Buzz");
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
    
    ProcessPacket();

    // Periodically send a register command
    if(millis() - _lastSendTime > PING_INTERVAL || _lastSendTime == 0) 
    {
        SendRegisterCommand(_uniqueId);
        _lastSendTime = millis();
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
    SetState(BuzzoButton::QUEUED);
    _placeInQueue = placeInQueue;
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
    _lastButtonPressTime = millis();

    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_BUZZ);
    udp.endPacket();
}
