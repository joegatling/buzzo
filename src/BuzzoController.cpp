#include <string>
#include <Arduino.h>

#include "BuzzoController.h"
#include "Commands.h"

#define RESPONSE_TIME_LIMIT (60 * 1000)
#define TIME_BETWEEN_RESPONDERS (1 * 1000)
#define RESPONDANT_PING_TIME 500

#define CLIENT_TIMER_MAX 255


inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

BuzzoController* BuzzoController::_instance = 0;
BuzzoController* BuzzoController::GetInstance()
{
    if(_instance == 0)
    {
        _instance = new BuzzoController();
    }

    return _instance;
}

BuzzoController::BuzzoController():
_correctButton(CORRECT_BUTTON_PIN),
_incorrectButton(INCORRECT_BUTTON_PIN),
_ResetButton(RESET_BUTTON_PIN),
_responseQueue(MAX_CLIENTS),
_isAcceptingResponses(true)
{
    _currentRespondant.assign("");
    _clientCount = 0;
    udp.begin(PORT);
}

void BuzzoController::Initialize()
{
    _correctButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(true); });
    
    _incorrectButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(false); });
    
    _ResetButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->BeginResetButtonPress(); });
    _ResetButton.SetHoldCallback([](){ BuzzoController::GetInstance()->HoldResetButton(); });
}

void BuzzoController::Update()
{
    ProcessPacket();

    _correctButton.Update();
    _incorrectButton.Update();
    _ResetButton.Update();

    if(_currentState == BuzzoController::PLAYING)
    {
        UpdatePlaying();        
    }
    else if(_currentState == BuzzoController::SETUP)
    {
        UpdateSetup();
    }
}

#pragma region Command Processing
void BuzzoController::ProcessPacket()
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

            // Process Packet
            if(strcmp(command, COMMAND_BUZZ) == 0)
            {
                ProcessBuzzCommand(udp.remoteIP());
            }
            else if(strcmp(command, COMMAND_REGISTER) == 0)
            {
                if(packetSize > 3)
                {
                    std::string param(command+3);
                    param = trim(param);
                    
                    ProcessRegisterCommand(udp.remoteIP(), param);                    
                }
            }                                                                                     
        }
    }
}

void BuzzoController::ProcessRegisterCommand(IPAddress ip, std::string param)
{
    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->GetId().compare(param) == 0)
        {
            _clients[i]->ResetTimeSinceLastContact();

            // An entry with a different IP is associated with this ID. This
            // means the button has been assigned a new IP.
            _clients[i]->SetIpAddress(ip);            

            // Scan through the remaining clients and make sure any duplicate
            // entries with this IP don't exist
            for(int j = 0; j<_clientCount; j++)
            {    
                if(j != i && _clients[j]->GetIpAddress() == ip)
                {
                    delete _clients[j];
                    RemoveClientAt(j);                    
                }
            }

            return;
        } 
        else if(_clients[i]->GetIpAddress() == ip)
        {
            _clients[i]->ResetTimeSinceLastContact();

            // If this IP is to be associated with a new ID, then this must
            // be an entirely new Button, so reset the score too.

            _clients[i]->SetId(param);
            _clients[i]->SetScore(0);

            return;
        }  
    }

    // If we got here, we don't have a registered client with the IP
    if(_clientCount >= MAX_CLIENTS)
    {
        for(int i = _clientCount-1; i >= 0; i--)
        {
            if(_clients[i]->IsActive() == false)
            {
                delete _clients[i];
                RemoveClientAt(i);                
                break;
            }
        }        
    }

    // Add this client (if we can)
    if(_clientCount < MAX_CLIENTS)
    {
        ButtonClientInfo* newClient = new ButtonClientInfo(ip, param);
        AddClient(newClient);
    }
    else
    {
        SendErrorCommand(ip, ERROR_TOO_MANY_CLIENTS);
    }
}

void BuzzoController::ProcessBuzzCommand(IPAddress ip)
{
    auto client = GetClient(ip);

    if(client != 0)
    {
        client->ResetTimeSinceLastContact();

        if(!_responseQueue.ContainsResponse(client->GetId()))
        {
            _responseQueue.EnqueueResponse(client->GetId());
        }

        // If there is more than one in the response queue, tell this button that 
        // it needs to wait before it can respond.
        if(_responseQueue.GetResponseCount() > 1)
        {
            SendQueueCommand(ip, _responseQueue.GetResponseCount());
        }

        // Note: A respondant will be selected in the next update, so all we need to here is enqueue
    }
}
#pragma endregion

#pragma region Command Functions
void BuzzoController::SendAnswerCommand(IPAddress ip, int timer)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_ANSWER);
    udp.print(" ");
    udp.println(timer);
    udp.endPacket();
}

void BuzzoController::SendQueueCommand(IPAddress ip, int placeInQueue)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_QUEUE);
    udp.print(" ");
    udp.println(placeInQueue);
    udp.endPacket();
}

void BuzzoController::SendResponseCommand(IPAddress ip, bool isCorrect)
{
    udp.beginPacket(ip, PORT);
    udp.println( isCorrect ? COMMAND_CORRECT_RESPONSE : COMMAND_INCORRECT_RESPONSE );
    udp.endPacket();
}

void BuzzoController::SendResetCommand(IPAddress ip, bool canBuzz)
{
    udp.beginPacket(ip, PORT);
    udp.println(COMMAND_RESET);
    udp.print(" ");
    udp.println(canBuzz);    
    udp.endPacket();
}

void BuzzoController::SendOffCommand(IPAddress ip)
{
    udp.beginPacket(ip, PORT);
    udp.println(COMMAND_OFF);
    udp.endPacket();
}

void BuzzoController::SendSelectCommand(IPAddress ip)
{
    udp.beginPacket(ip, PORT);
    udp.println(COMMAND_SELECT);
    udp.endPacket();
}

void BuzzoController::SendErrorCommand(IPAddress ip, int errorCode)
{
    udp.beginPacket(ip, PORT);
    udp.println(COMMAND_ERROR);
    udp.print(" ");
    udp.println(errorCode);    
    udp.endPacket();
}

void BuzzoController::SendScoreCommand(IPAddress ip, int score = 0)
{
    udp.beginPacket(ip, PORT);
    udp.println(COMMAND_SCORE);
    udp.print(" ");
    udp.println(score);    
    udp.endPacket();
}
#pragma endregion

#pragma region Update Functions
void BuzzoController::UpdatePlaying()
{
    if(_currentRespondant.length() > 0)
    {
        unsigned long timeResponding = millis() - _responseStartTime;

        // Update respondant timer
        if(millis() - _lastRespondantPingTime > RESPONDANT_PING_TIME)
        {
            auto client = GetClient(_currentRespondant);

            if(client != 0)
            {
                float timeRemaining = timeResponding / RESPONSE_TIME_LIMIT;
                SendAnswerCommand(client->GetIpAddress(), max(0, (int)(1.0f - timeRemaining) * 255));
            }

            _lastRespondantPingTime = millis();
        }

        // Responder ran out of time
        if(timeResponding > RESPONSE_TIME_LIMIT)
        {
            auto client = GetClient(_currentRespondant);

            if(client != 0)
            {
                SendResponseCommand(client->GetIpAddress(), false);
            }

            _currentRespondant.assign("");
            _nextResponderDelayStartTime = millis();
        }
    }
    else if(_currentRespondant.length() == 0 && !_responseQueue.IsEmpty() && _isAcceptingResponses == true)
    {
        // Wait a short delay between respondants to increase gameplay tension
        if(millis() - _nextResponderDelayStartTime > TIME_BETWEEN_RESPONDERS)
        {
            // Find our next respondant
            auto nextRespondantId = _responseQueue.DequeueNextResponse();
            auto client = GetClient(nextRespondantId);

            if(client != 0 && client->IsActive())
            {
                _currentRespondant.assign(client->GetId());
                _responseStartTime = millis();

                SendAnswerCommand(client->GetIpAddress(), CLIENT_TIMER_MAX);
                _lastRespondantPingTime = millis();
            }

            // Tell all other buttons that they're in the queue
            int placeInQueue = 0;
            for(int i = 0; i < _responseQueue.GetResponseCount(); i++)
            {
                client = GetClient(_responseQueue.PeekNextResponse(i));
                if(client != 0 && client->IsActive())
                {
                    placeInQueue++;
                    SendQueueCommand(client->GetIpAddress(), placeInQueue);
                }
            }
        }
    }    
}

void BuzzoController::UpdateSetup()
{}
#pragma endregion

#pragma region Client Management Functions
void BuzzoController::AddClient(ButtonClientInfo* newClient)
{
    if(_clientCount < MAX_CLIENTS)
    {
        _clients[_clientCount++] = newClient;
    }
}

void BuzzoController::RemoveClientAt(unsigned int index)
{
    if(index < _clientCount)
    {
        if(index < _clientCount - 1)
        {
            _clients[index] = _clients[_clientCount-1];
        }

        _clientCount--;
    }
}

ButtonClientInfo* BuzzoController::GetClient(IPAddress ip)
{
    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->GetIpAddress() == ip)
        {
            return _clients[i];
        }
    }

    return 0;
}

ButtonClientInfo* BuzzoController::GetClient(std::string id)
{
    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->GetId().compare(id) == 0)
        {
            return _clients[i];
        }
    }

    return 0;
}
#pragma endregion

#pragma region Button Functions
void BuzzoController::EndCurrentRespondantTurn(bool isCorrect)
{
    if(_currentState == BuzzoController::PLAYING)
    {
        if(_currentRespondant.length() > 0)
        {
            auto client = GetClient(_currentRespondant);

            if(client != 0)
            {        
                if(isCorrect)
                {
                    client->SetScore(client->GetScore() + 1);
                    _isAcceptingResponses = false; // The round is over
                }

                SendScoreCommand(client->GetIpAddress(), client->GetScore());
                SendResponseCommand(client->GetIpAddress(), isCorrect);
            }

            _currentRespondant.assign("");

            if(isCorrect)
            {
                _isAcceptingResponses = false;

                for(int i = 0; i < _clientCount; i++)
                {
                    if(_clients[i] != client)
                    {
                        SendResetCommand(_clients[i]->GetIpAddress(), false);
                    }
                }
            }
            else
            {
                _nextResponderDelayStartTime = millis();
            }
        }
    }
}

void BuzzoController::BeginResetButtonPress()
{
    if(_currentState == BuzzoController::PLAYING)
    {
        for(int i = 0; i < _clientCount; i++)
        {
            SendResetCommand(_clients[i]->GetIpAddress(), true);

        }

        _currentRespondant.assign("");

        _isAcceptingResponses = true;
        _nextResponderDelayStartTime = 0;
    }    
}

void BuzzoController::HoldResetButton()
{
    if(_currentState == BuzzoController::PLAYING)
    {
        for(int i = 0; i < _clientCount; i++)
        {
            _clients[i]->SetScore(0);
            
            SendResetCommand(_clients[i]->GetIpAddress(), true);
            SendScoreCommand(_clients[i]->GetIpAddress(), _clients[i]->GetScore());
        }

        _currentRespondant.assign("");

        _isAcceptingResponses = true;
        _nextResponderDelayStartTime = 0;
    }    
}
#pragma endregion