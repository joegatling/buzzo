#include <string>
#include <Arduino.h>
#include <iostream>
#include <string>
#include <sstream>

#include "BuzzoController.h"
#include "Commands.h"

#define RESPONSE_TIME_LIMIT     20
#define TIME_BETWEEN_RESPONDERS (1 * 1000)
#define RESPONDANT_PING_TIME    500
#define AUTO_RESET_TIME         5000

#define MAX_SCORE 6

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
_resetButton(RESET_BUTTON_PIN),
_pauseButton(PAUSE_BUTTON_PIN),
_responseQueue(MAX_CLIENTS),
_isAcceptingResponses(true),
_isReset(true),
_shouldSleep(false),
_isPaused(false),
_isInAdjustMode(false)
{
    _currentRespondant.assign("");
    _clientCount = 0;
    udp.begin(PORT);

    _participantCount = 0;

    _lastMillis = millis();
}

void BuzzoController::Initialize()
{
    _correctButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(true); });    
    _correctButton.SetBeginHoldCallback([](){ BuzzoController::GetInstance()->AdjustPreviousRespondant(true); });    
    _correctButton.SetLongPressDuration(1500);

    _incorrectButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(false); });
    _incorrectButton.SetBeginHoldCallback([](){ BuzzoController::GetInstance()->AdjustPreviousRespondant(false); });
    _incorrectButton.SetEndHoldCallback([](){ BuzzoController::GetInstance()->ReleaseHoldIncorrectButton(); });
    _incorrectButton.SetLongPressDuration(1500);
    
    _resetButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->ResetButtonPress(); });    
    _resetButton.SetBeginHoldCallback([](){ BuzzoController::GetInstance()->HoldResetButton(); });
    _resetButton.SetEndHoldCallback([](){ BuzzoController::GetInstance()->ReleaseHoldResetButton(); });
    _resetButton.SetLongPressDuration(3000);

    _pauseButton.SetBeginPressCallback([](){ BuzzoController::GetInstance()->BeginPauseButtonPress(); });
    _pauseButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndPauseButtonPress(); });
}

void BuzzoController::Update()
{
    ProcessPacket();

    _correctButton.Update();
    _incorrectButton.Update();
    _resetButton.Update();
    _pauseButton.Update();

    if(_currentState == BuzzoController::PLAYING)
    {
        UpdatePlaying();        
    }
    else if(_currentState == BuzzoController::SETUP)
    {
        UpdateSetup();
    }

    _lastMillis = millis();    
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

        std::stringstream data(packetBuffer);
        std::string command;

        data >> command;        

        bool error = false;

        if(command.length() == 3)
        {
            std::string params[2];
            int paramCount = 0;

            while(data || paramCount >= sizeof(params))
            {
                data >> params[paramCount++];
                // Serial.print("Param ");
                // Serial.print(paramCount - 1);
                // Serial.print(" [");
                // Serial.print(params[paramCount - 1].c_str());
                // Serial.println("]");
            }            

            // Process Packet
            if(command.compare(COMMAND_BUZZ) == 0)
            {
                ProcessBuzzCommand(udp.remoteIP());
            }
            else if(command.compare(COMMAND_REGISTER) == 0)
            {
                if(paramCount > 0)
                {
                    if(params[1].length() > 0)
                    {
                        ProcessRegisterCommand(udp.remoteIP(), params[0], params[1]);
                    }
                    else
                    {
                        ProcessRegisterCommand(udp.remoteIP(), params[0], "100");
                    }
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

void BuzzoController::ProcessRegisterCommand(IPAddress ip, std::string paramId, std::string paramBattery)
{
    Serial.println("Registering Client");
    Serial.print("ID ");
    Serial.println(paramId.c_str());

    Serial.print("Battery ");
    Serial.print(paramBattery.c_str());
    Serial.println("%");

    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->GetId().compare(paramId) == 0)
        {
            Serial.println("Resetting Contact Time...");
            _clients[i]->ResetTimeSinceLastContact();

            int batteryLevel = atoi(paramBattery.c_str());
            _clients[i]->SetBatteryLevel(batteryLevel);

            //Serial.println(_clients[i]->GetTimeSinceLastContact());

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

            // Tell the client what it's current score is
            SendScoreCommand(ip, _clients[i]->GetScore());

            return;
        } 
        else if(_clients[i]->GetIpAddress() == ip)
        {
            _clients[i]->ResetTimeSinceLastContact();

            // If this IP is to be associated with a new ID, then this must
            // be an entirely new Button, so reset the score too.

            
            _clients[i]->SetId(paramId);
            _clients[i]->SetScore(0);

            int batteryLevel = atoi(paramBattery.c_str());
            _clients[i]->SetBatteryLevel(batteryLevel);            

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
        ButtonClientInfo* newClient = new ButtonClientInfo(ip, paramId);
        AddClient(newClient);

        int batteryLevel = atoi(paramBattery.c_str());
        newClient->SetBatteryLevel(batteryLevel);            

    }
    else
    {
        SendErrorCommand(ip, ERROR_TOO_MANY_CLIENTS);
    }
}

void BuzzoController::ProcessBuzzCommand(IPAddress ip)
{
    Serial.print("Processing buzz command from ");
    Serial.println(ip.toString());

    auto client = GetClient(ip);



    if(client != 0)
    {
        Serial.print("Client ID ");
        Serial.println(client->GetId().c_str());    

        if(_isInAdjustMode)
        {
            client->SetScore(max(0, client->GetScore() - 1));
            Serial.print("Score: ");
            Serial.println(client->GetScore());

            SendScoreCommand(client->GetIpAddress(), client->GetScore());
        }
        else
        {
            client->ResetTimeSinceLastContact();

            if(client->GetScore() < MAX_SCORE)
            {
                if(!_responseQueue.ContainsResponse(client->GetId()))
                {
                    _responseQueue.EnqueueResponse(client->GetId());
                
                    _participants[_participantCount] = client->GetId();
                    _participantCount++;
                }

                // If there is more than one in the response queue, tell this button that 
                // it needs to wait before it can respond.
                if(_responseQueue.GetResponseCount() > 1 || _currentRespondant.length() > 0)
                {
                    SendQueueCommand(ip, _responseQueue.GetResponseCount());
                }

                // Note: A respondant will be selected in the next update, so all we need to here is enqueue
            }
        }
    }
    

    _isReset = false;
}
#pragma endregion

#pragma region Command Functions
void BuzzoController::SendAnswerCommand(IPAddress ip, int timer, int totalTime)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_ANSWER);
    udp.print(" ");
    udp.print(timer);
    udp.print(" ");
    udp.print(totalTime);
    udp.endPacket();
}

void BuzzoController::SendQueueCommand(IPAddress ip, int placeInQueue)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_QUEUE);
    udp.print(" ");
    udp.print(placeInQueue);
    udp.endPacket();
}

void BuzzoController::SendResponseCommand(IPAddress ip, bool isCorrect)
{
    udp.beginPacket(ip, PORT);
    udp.print( isCorrect ? COMMAND_CORRECT_RESPONSE : COMMAND_INCORRECT_RESPONSE );
    udp.endPacket();
}

void BuzzoController::SendResetCommand(IPAddress ip, bool canBuzz)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_RESET);
    udp.print(" ");
    udp.print(canBuzz);    
    udp.endPacket();
}

void BuzzoController::SendSelectCommand(IPAddress ip)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_SELECT);
    udp.endPacket();
}

void BuzzoController::SendErrorCommand(IPAddress ip, int errorCode)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_ERROR);
    udp.print(" ");
    udp.print(errorCode);    
    udp.endPacket();
}

void BuzzoController::SendScoreCommand(IPAddress ip, int score = 0)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_SCORE);
    udp.print(" ");
    udp.print(score);    
    udp.endPacket();
}

void BuzzoController::SendSleepCommand(IPAddress ip)
{
    udp.beginPacket(ip, PORT);
    udp.print(COMMAND_SLEEP);
    udp.endPacket();
}
#pragma endregion

#pragma region Update Functions
void BuzzoController::UpdatePlaying()
{
    if(_currentRespondant.length() > 0)
    {
        if(_isAcceptingResponses) // If this is false, we have already received the correct answer
        {
            if(_isPaused)
            {
                _responsePauseTime += millis() - _lastMillis;
            }

            unsigned long millisResponding = millis() - _responseStartTime - _responsePauseTime;
            int secondsResponding = floor(millisResponding / 1000);            

            // Update respondant timer
            if(millis() - _lastRespondantPingTime > RESPONDANT_PING_TIME)
            {
                Serial.print("Time responding: ");
                Serial.println(millisResponding);

                auto client = GetClient(_currentRespondant);

                if(client != 0)
                {                    
                    int secondsRemaining = RESPONSE_TIME_LIMIT - secondsResponding;

                    if(_isPaused)
                    {
                        secondsRemaining = -secondsRemaining;
                    }

                    SendAnswerCommand(client->GetIpAddress(), secondsRemaining, RESPONSE_TIME_LIMIT);
                }

                _lastRespondantPingTime = millis();
            }

            // Responder ran out of time
            if(millisResponding > (RESPONSE_TIME_LIMIT * 1000))
            {
                auto client = GetClient(_currentRespondant);

                if(client != 0)
                {
                    SendResponseCommand(client->GetIpAddress(), false);
                }

                _previousRespondant.assign(_currentRespondant);
                _currentRespondant.assign("");

                _responsePauseTime = 0;
                _nextResponderDelayStartTime = millis();
            }
        }
    }
    else if(_currentRespondant.length() == 0)
    {
        if(!_responseQueue.IsEmpty() && _isAcceptingResponses == true)
        {
            // Wait a short delay between respondants to increase gameplay tension
            if(millis() - _nextResponderDelayStartTime > TIME_BETWEEN_RESPONDERS)
            {
                // Find our next respondant
                auto nextRespondantId = _responseQueue.DequeueNextResponse();
                auto client = GetClient(nextRespondantId);

                Serial.print("Next respondant: ");
                Serial.println(nextRespondantId.c_str());

                Serial.print("Is respondant active? ");
                Serial.println(client->IsActive() ? "YES" : "NO");

                if(client != 0 && client->IsActive())
                {
                    Serial.println("Answering");
                    _currentRespondant.assign(client->GetId());
                    _responseStartTime = millis();
                    _responsePauseTime = 0;

                    SendAnswerCommand(client->GetIpAddress(), RESPONSE_TIME_LIMIT, RESPONSE_TIME_LIMIT);
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
        else if(_isAcceptingResponses == false)
        {
            if(_autoResestTime > 0 && millis() > _autoResestTime)
            {
                ResetButtonPress();
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

int BuzzoController::GetActiveClientCount()
{
    int count = 0;

    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->IsActive())
        {
            count++;
        }
    }

    return count;

}

int BuzzoController::GetActiveClientsYetToPlayCount()
{
    int count = 0;

    for(int i = 0; i < _clientCount; i++)
    {

        if(_clients[i]->IsActive())
        {
            bool hasParticipated = false;
            
            for(int j = 0; j < _participantCount; j++)
            {
                if(_participants[j].compare(_clients[i]->GetId()) == 0)
                {
                    hasParticipated = true;
                    break;
                }
            }            

            if(!hasParticipated)
            {
                count++;
            }
        }
    }

    return count;
}

unsigned int BuzzoController::GetMinBatteryLevelForClients()
{
    unsigned int minBatteryLevel = 100;

    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->IsActive())
        {
            minBatteryLevel = min(minBatteryLevel, _clients[i]->GetBatteryLevel());
        }
    }

    return minBatteryLevel;
}

void BuzzoController::AddAllRemainingClientsToQueue()
{
    for(int i = 0; i < _clientCount; i++)
    {

        if(_clients[i]->IsActive())
        {
            bool hasParticipated = false;
            
            for(int j = 0; j < _participantCount; j++)
            {
                if(_participants[j].compare(_clients[i]->GetId()) == 0)
                {
                    hasParticipated = true;
                    break;
                }
            }            

            if(!hasParticipated)
            {
                Serial.print("Adding client to response queue: ");
                Serial.println(_clients[i]->GetId().c_str());
                
                // Pretend that this participant has buzzed
                ProcessBuzzCommand(_clients[i]->GetIpAddress());
            }
        }
    }
}


#pragma endregion

#pragma region Button Functions
void BuzzoController::EndCurrentRespondantTurn(bool isCorrect)
{
    _lastButtonPressTime = millis();

    if(_currentState == BuzzoController::PLAYING && _isAcceptingResponses == true)
    {
        Serial.print("Ending Turn - ");
        Serial.println(isCorrect ? "CORRECT" : "INCORRECT");

        if(_currentRespondant.length() > 0)
        {
            auto client = GetClient(_currentRespondant);

            if(client != 0)
            {        
                if(isCorrect)
                {
                    client->SetScore(min(MAX_SCORE, client->GetScore() + 1));
                    _isAcceptingResponses = false; // The round is over
                }

                Serial.print("Score: ");
                Serial.println(client->GetScore());

                SendScoreCommand(client->GetIpAddress(), client->GetScore());
                SendResponseCommand(client->GetIpAddress(), isCorrect);
            }

            if(isCorrect)
            {
                _isAcceptingResponses = false;
                _autoResestTime = millis() + AUTO_RESET_TIME;
            
                // for(int i = 0; i < _responseQueue.GetResponseCount(); i++)
                // {
                //     auto queuedClient = GetClient(_responseQueue.PeekNextResponse(i));
                //     if(queuedClient != 0 && queuedClient->IsActive())
                //     {
                //         SendResetCommand(queuedClient->GetIpAddress(), false);              
                //     }
                // }  
            }
            else
            {
                _nextResponderDelayStartTime = millis();

                if(_responseQueue.GetResponseCount() == 0)
                {
                    Serial.println("Response queue is empty");

                    Serial.print("Clients yet to play: ");
                    Serial.println(GetActiveClientsYetToPlayCount());
                    
                    if(GetActiveClientsYetToPlayCount() == 1)
                    {
                        AddAllRemainingClientsToQueue();
                    }
                }
            }

            _previousRespondant.assign(_currentRespondant);
            _previousRespondantWasCorrect = isCorrect;

            _currentRespondant.assign("");            
        }
    }
}

void BuzzoController::ResetButtonPress()
{
    _lastButtonPressTime = millis();

    if(_currentState == BuzzoController::PLAYING)
    {
        for(int i = 0; i < _clientCount; i++)
        {
            SendResetCommand(_clients[i]->GetIpAddress(), true);
        }

        _currentRespondant.assign("");
        _previousRespondant.assign("");

        _isAcceptingResponses = true;
        _nextResponderDelayStartTime = 0;

        _responseQueue.Clear();

        _participantCount = 0;
        
        _autoResestTime = 0;
    }    
}

void BuzzoController::HoldResetButton()
{
    if(_currentState == BuzzoController::PLAYING)
    {
        if(_isReset)
        {
            _shouldSleep = true;
            digitalWrite(13, LOW);

            for(int i = 0; i < _clientCount; i++)
            {
                SendSleepCommand(_clients[i]->GetIpAddress());
            }

        }
        else
        {
            for(int i = 0; i < _clientCount; i++)
            {
                _clients[i]->SetScore(0);
                
                SendResetCommand(_clients[i]->GetIpAddress(), true);
                SendScoreCommand(_clients[i]->GetIpAddress(), _clients[i]->GetScore());
            }

            _currentRespondant.assign("");
            _previousRespondant.assign("");

            _isAcceptingResponses = true;
            _nextResponderDelayStartTime = 0;
            _isReset = true;

            _responseQueue.Clear();
        }
    }    
}

void BuzzoController::ReleaseHoldResetButton()
{
    if(_shouldSleep)
    {

        WiFi.disconnect(true);

        // esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
        // esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);    
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW);

        esp_deep_sleep_start();  
    } 
}

void BuzzoController::AdjustPreviousRespondant(bool isCorrect)
{
    if(_previousRespondant.length() > 0 && _previousRespondantWasCorrect != isCorrect)
    {
        auto client = GetClient(_previousRespondant);

        if(client != 0)
        {        
            if(isCorrect)
            {                
                client->SetScore(min(MAX_SCORE, client->GetScore() + 1));

                Serial.print("New Score: ");
                Serial.println(client->GetScore());

                SendScoreCommand(client->GetIpAddress(), client->GetScore());
                SendResponseCommand(client->GetIpAddress(), isCorrect);

                if(_currentRespondant.length() > 0)
                {
                    // Push the current respondant to the start of the queue again, just in case
                    _responseQueue.PushResponse(_currentRespondant);
                    _currentRespondant.assign("");

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
            else
            {
                // 1. If they were previously given a score, subtract it
                client->SetScore(max(0, client->GetScore() - 1));
                Serial.print("New Score: ");
                Serial.println(client->GetScore());

                // 2. Tell the previous respondant that they were incorrect
                SendScoreCommand(client->GetIpAddress(), client->GetScore());
                SendResponseCommand(client->GetIpAddress(), isCorrect);

                // 3. Tell all other buttons that they're in the queue again
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

                // 4. Add a bit of a delay befor the next turn. Just to give the moment time to breath
                _nextResponderDelayStartTime = millis() + 2000; // Add a bit more time here
            }

            _previousRespondantWasCorrect = isCorrect;
            _isAcceptingResponses = !isCorrect;
        }
    }
    // else
    // {
    //     _isInAdjustMode = true;

    //     for(int i = 0; i < _clientCount; i++)
    //     {
    //         SendSelectCommand(_clients[i]->GetIpAddress());
    //     }
    // }
}

void BuzzoController::ReleaseHoldIncorrectButton()
{
    _isInAdjustMode = false;

    for(int i = 0; i < _clientCount; i++)
    {
        SendResetCommand(_clients[i]->GetIpAddress(), true);
    }
}

void BuzzoController::BeginPauseButtonPress()
{
    Serial.println("PAUSE");
    _isPaused = true;
}

void BuzzoController::EndPauseButtonPress()
{
    Serial.println("UNPAUSE");
        _isPaused = false;
}
#pragma endregion