
#include <Wifi.h>

#include "BuzzoButton.h"
#include "Commands.h"


#pragma region Update Functions

void IdleEnter(BuzzoButton* button)
{
    //TODO: Turn off the lights
    //TODO: Show current score

    button->_canBuzz = true;
}

void IdleUpdate(BuzzoButton* button)
{

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

#pragma endregion

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
_canBuzz(false)
{
    udp.begin(PORT);
    _uniqueId.assign(WiFi.macAddress().c_str());
}

void BuzzoButton::Initialize()
{
    for(int i = 0; i < STATE_COUNT; i++)
    {
        _states[i] = 0;
    }    

    _states[IDLE] = new BuzzoButtonState(&IdleEnter, &IdleUpdate, &IdleExit);
    _states[ANSWERING] = new BuzzoButtonState(&AnsweringEnter, &AnsweringUpdate, &AnsweringExit);
    _states[CORRECT] = new BuzzoButtonState(&CorrectEnter, &CorrectUpdate, &CorrectExit);
    _states[INCORRECT] = new BuzzoButtonState(&IncorrectEnter, &IncorrectUpdate, &IncorrectExit);
    _states[QUEUED] = new BuzzoButtonState(&QueuedEnter, &QueuedUpdate, &QueuedExit);
}

void BuzzoButton::Update()
{
    if(_nextState != BuzzoButton::UNSET)
    {
        _states[_currentState]->Exit(this);
        _currentState = _nextState;
        _states[_currentState]->Enter(this);
    }

    _states[_currentState]->Update(this);
    
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
                    ProcessResetcCommand((bool)atoi(param.c_str()));
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

void BuzzoButton::ProcessResetcCommand(bool canBuzz)
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


void BuzzoButton::SendRegisterCommand(std::string param)
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_ANSWER);
    udp.print(" ");
    udp.println(param.c_str());
    udp.endPacket();
}

void BuzzoButton::SendBuzzCommand()
{
    udp.beginPacket(_controllerIp, PORT);
    udp.print(COMMAND_BUZZ);
    udp.endPacket();
}

