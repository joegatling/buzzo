#include <WiFi.h>
#include <esp_now.h>
#include <string>
#include <Arduino.h>
#include <iostream>
#include <string>
#include <sstream>


#include "BuzzoController.h"
#include "CommandsController.h"

#define RESPONSE_TIME_LIMIT     20
#define TIME_BETWEEN_RESPONDERS (1 * 1000)
#define TIME_BEFORE_FIRST_RESPONSE (250)
#define RESPONDANT_PING_TIME    500
#define AUTO_RESET_TIME         10000

#define MAX_SCORE 6

#define MAX_DATA_SIZE 128

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

#pragma region ESP-NOW Callbacks
void OnControllerDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    // Serial.print("Last Packet Sent Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnControllerDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) 
{
    len = min(len, PACKET_MAX_SIZE);

    char packetBuffer[MAX_DATA_SIZE + 1];
    memcpy(packetBuffer, data, len);
    packetBuffer[len] = 0;

    BuzzoController::GetInstance()->EnqueuePacketData(info->src_addr, packetBuffer);

    // Serial.print("Last Packet Recv Data: ");
    // for(int i = 0; i < len; i++)
    // {
    //     Serial.print((char)data[i]);
    // }
    // Serial.println();

}
#pragma endregion

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
//_responseQueue(MAX_CLIENTS),
//_isAcceptingResponses(true),
_isReset(true),
_shouldSleep(false),
_isPaused(false),
_isInAdjustMode(false)
{
    //_clientResponses.GetCurrentRespondant().assign("");
    _clientResponses.Reset();
    
    _clientCount = 0;

    for(int i = 0; i < PACKET_MAX_QUEUE; i++)
    {
        memset(packetQueue[i].mac, 0, sizeof(packetQueue[i].mac));
        memset(packetQueue[i].packetBuffer, 0, sizeof(packetQueue[i].packetBuffer));
    }

    //udp.begin(PORT);
    WiFi.mode(WIFI_STA);
    WiFi.channel(0);    

    if(esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
    }

    uint8_t baseMac[6];

    WiFi.macAddress(baseMac);
    Serial.print("Mac Address: ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                baseMac[0], baseMac[1], baseMac[2],
                baseMac[3], baseMac[4], baseMac[5]);

    esp_now_register_send_cb(OnControllerDataSent);
    esp_now_register_recv_cb(OnControllerDataReceived);

    //_participantCount = 0;

    _lastMillis = millis();

    Serial.println("Init complete");
}

void BuzzoController::Initialize()
{
    _correctButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(true); });    
    _correctButton.SetBeginHoldCallback([](){ BuzzoController::GetInstance()->AdjustPreviousRespondant(true); });    
    _correctButton.SetLongPressDuration(1500);

    _incorrectButton.SetEndPressCallback([](){ BuzzoController::GetInstance()->EndCurrentRespondantTurn(false); });
    _incorrectButton.SetBeginHoldCallback([](){ BuzzoController::GetInstance()->AdjustPreviousRespondant(false); });
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
    // ProcessPacket();
    ProcessPacketQueue();

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

void BuzzoController::EnqueuePacketData(const uint8_t *mac, const char* packetBuffer)
{
    if(_packetQueueCount < PACKET_MAX_QUEUE)
    {
        unsigned int index = (_packetQueueIndex + _packetQueueCount) % PACKET_MAX_QUEUE;
        
        memcpy(packetQueue[index].mac, mac, sizeof(packetQueue[index].mac));
        strncpy(packetQueue[index].packetBuffer, packetBuffer, PACKET_MAX_SIZE);
        packetQueue[index].packetBuffer[PACKET_MAX_SIZE] = '\0'; // Ensure null-termination

        _packetQueueCount++;
    }
}


void BuzzoController::ProcessPacketQueue()
{
    if(_packetQueueCount > 0)
    {
        uint8_t mac[6];
        memcpy(mac, packetQueue[_packetQueueIndex].mac, sizeof(mac));
        
        char buffer[PACKET_MAX_SIZE + 1];
        
        memcpy(buffer, packetQueue[_packetQueueIndex].packetBuffer, PACKET_MAX_SIZE);
        buffer[PACKET_MAX_SIZE] = 0;

        ProcessPacket(mac, buffer);

        _packetQueueCount--;
        _packetQueueIndex = (_packetQueueIndex + 1) % PACKET_MAX_QUEUE;
    }
}

void BuzzoController::ProcessPacket(const uint8_t *mac, const char* packetBuffer)
{
    // int packetSize = udp.parsePacket();
    // if (packetSize) 
    // {    
    //     Serial.printf("Received packet of size %d from %s:%d\n \n", 
    //     packetSize, 

    //     udp.remoteIP().toString().c_str(), 
    //     udp.remotePort());

    //     // read the packet into packetBufffer
    //     int n = udp.read(packetBuffer, PACKET_MAX_SIZE);
    //     packetBuffer[n] = 0;

    //     Serial.println("Contents:");
    //     Serial.println(packetBuffer); 

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
                ProcessBuzzCommand(mac);
            }
            else if(command.compare(COMMAND_REGISTER) == 0)
            {
                if(paramCount > 0)
                {
                    if(params[1].length() > 0)
                    {
                        ProcessRegisterCommand(mac, params[0], params[1]);
                    }
                    else
                    {
                        ProcessRegisterCommand(mac, params[0], "100");
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
    //}
}

void BuzzoController::ProcessRegisterCommand(const uint8_t *mac, std::string paramId, std::string paramBattery)
{
    Serial.println("Registering Client");

    Serial.print("MAC ");
    for(int i = 0; i < 6; i++)
    {
        Serial.print(mac[i], HEX);
        Serial.print(":");
    }
    Serial.println();

    Serial.print("ID ");
    Serial.println(paramId.c_str());

    Serial.print("Battery ");
    Serial.print(paramBattery.c_str());
    Serial.println("%");

    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->CompareMac(mac))
        {
            Serial.println("Resetting Contact Time...");
            _clients[i]->ResetTimeSinceLastContact();

            int batteryLevel = atoi(paramBattery.c_str());
            _clients[i]->SetBatteryLevel(batteryLevel);

            //Serial.println(_clients[i]->GetTimeSinceLastContact());

            // // An entry with a different IP is associated with this ID. This
            // // means the button has been assigned a new IP.
            // _clients[i]->SetIpAddress(ip);     
            
            // Scan through the remaining clients and make sure any duplicate
            // entries with this IP don't exist
            // for(int j = 0; j<_clientCount; j++)
            // {    
            //     if(j != i && _clients[j]->GetIpAddress() == ip)
            //     {
            //         delete _clients[j];
            //         RemoveClientAt(j);                    
            //     }
            // }

            // Tell the client what it's current score is
            SendScoreCommand(mac, _clients[i]->GetScore());

            return;
        } 
        // else if(_clients[i]->GetIpAddress() == ip)
        // {
        //     _clients[i]->ResetTimeSinceLastContact();

        //     // If this IP is to be associated with a new ID, then this must
        //     // be an entirely new Button, so reset the score too.

            
        //     _clients[i]->SetId(paramId);
        //     _clients[i]->SetScore(0);

        //     int batteryLevel = atoi(paramBattery.c_str());
        //     _clients[i]->SetBatteryLevel(batteryLevel);            

        //     return;
        // }  
    }

    // If we got here, we don't have a registered client with the MAC
    // Let's first cull all inactive clients
    if(_clientCount >= MAX_CLIENTS)
    {
        for(int i = _clientCount-1; i >= 0; i--)
        {
            if(_clients[i]->IsActive() == false)
            {
                uint8_t inactiveMac[6];
                _clients[i]->GetMacAddress(inactiveMac);
                esp_now_del_peer(inactiveMac);

                delete _clients[i];
                RemoveClientAt(i);                
                break;
            }
        }        
    }

    // Add this client (if we can)
    if(_clientCount < MAX_CLIENTS)
    {
        ButtonClientInfo* newClient = new ButtonClientInfo(mac, paramId);
        AddClient(newClient);

        int batteryLevel = atoi(paramBattery.c_str());
        newClient->SetBatteryLevel(batteryLevel);       
        
        // Add as an ESP-NOW peer
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, mac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        if(esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Error adding peer");
        }

        SendScoreCommand(mac, newClient->GetScore());
    }
    else
    {
        SendErrorCommand(mac, ERROR_TOO_MANY_CLIENTS);
    }
}

void BuzzoController::ProcessBuzzCommand(const uint8_t *mac)
{
    Serial.print("Processing buzz command from ");
    for(int i = 0; i < 6; i++)
    {
        Serial.print(mac[i], HEX);
        Serial.print(":");
    }
    Serial.println();
    auto client = GetClient(mac);

    if(client != 0)
    {
        Serial.print("Client ID ");
        Serial.println(client->GetId().c_str());    

        if(_isInAdjustMode)
        {
            client->SetScore(max(0, client->GetScore() - 1));
            Serial.print("Score: ");
            Serial.println(client->GetScore());

            SendScoreCommand(mac, client->GetScore());
        }
        else
        {
            client->ResetTimeSinceLastContact();

            if(client->GetScore() < MAX_SCORE)
            {
                auto respondantStatus = _clientResponses.GetRespondantStatus(client->GetId());
                bool thisClientShouldAnswer = false;

                Serial.print("This respondant status: ");
                // Print the status enum name as a human readable string
                switch(respondantStatus)
                {
                    case RespondantStatus::NONE:
                        Serial.println("NONE");
                        break;
                    case RespondantStatus::INCORRECT:
                        Serial.println("INCORRECT");
                        break;
                    case RespondantStatus::CORRECT:
                        Serial.println("CORRECT");
                        break;
                    case RespondantStatus::ANSWERING:
                        Serial.println("ANSWERING");
                        break;
                    case RespondantStatus::QUEUED:
                        Serial.println("QUEUED");
                        break;
                }

                
                if(respondantStatus == RespondantStatus::NONE)
                {
                    Serial.print("Has current respondant: ");
                    Serial.println(_clientResponses.HasCurrentRespondant() ? "YES" : "NO");

                    Serial.print("Is round over: ");
                    Serial.println(_clientResponses.IsRoundOver() ? "YES" : "NO");

                    thisClientShouldAnswer = _clientResponses.EnqueueRespondant(client->GetId());

                    //_responseQueue.EnqueueResponse(client->GetId());
                
                    // _participants[_participantCount] = client->GetId();
                    // _participantCount++;

                    Serial.print("Can respondant answer: ");
                    Serial.println(thisClientShouldAnswer ? "YES" : "NO");
    
                    Serial.print("Pending Respondants: ");
                    Serial.println(_clientResponses.GetPendingRespondantCount());
    
                    // If there is more than one in the response queue, tell this button that 
                    // it needs to wait before it can respond.
                    if(thisClientShouldAnswer)
                    {
                        Serial.println("Setting response delay...");
                        _nextResponderDelayStartTime = millis() - TIME_BETWEEN_RESPONDERS + TIME_BEFORE_FIRST_RESPONSE;
                    }
                    else
                    {
                        Serial.println("Sending queue command...");
                        SendQueueCommand(mac, _clientResponses.GetPendingRespondantCount());
                    }                    
                }
                else if(respondantStatus == RespondantStatus::INCORRECT)
                {
                    SendResponseCommand(mac, false);
                }
                else if(respondantStatus == RespondantStatus::CORRECT)
                {
                    SendResponseCommand(mac, true);
                }
                else if(respondantStatus == RespondantStatus::QUEUED)
                {
                    int placeInQueue = _clientResponses.GetQueuedRespondantIndex(client->GetId());

                    if(placeInQueue >= 0)
                    {
                        SendQueueCommand(mac, placeInQueue);
                    }
                    else
                    {
                        SendQueueCommand(mac, MAX_QUEUE_LENGTH);
                    }
                }

                
                // if(_clientResponses.GetResponseCount() > 1 || _clientResponses.GetCurrentRespondant().length() > 0)
                // {
                //     SendQueueCommand(mac, _responseQueue.GetResponseCount());
                // }

                // Note: A respondant will be selected in the next update, so all we need to here is enqueue
            }
        }
    }
    

    _isReset = false;
}
#pragma endregion

#pragma region Command Functions
void BuzzoController::SendAnswerCommand(const uint8_t *mac, int timer, int totalTime)
{
    String message = String(COMMAND_ANSWER) + " " + timer + " " + totalTime;
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendQueueCommand(const uint8_t *mac, int placeInQueue)
{
    String message = String(COMMAND_QUEUE) + " " + placeInQueue;
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendResponseCommand(const uint8_t *mac, bool isCorrect)
{
    String message = String(isCorrect ? COMMAND_CORRECT_RESPONSE : COMMAND_INCORRECT_RESPONSE);
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendResetCommand(const uint8_t *mac, bool canBuzz)
{
    String message = String(COMMAND_RESET) + " " + canBuzz;
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendSelectCommand(const uint8_t *mac)
{
    String message = String(COMMAND_SELECT);
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendErrorCommand(const uint8_t *mac, int errorCode)
{
    String message = String(COMMAND_ERROR) + " " + errorCode;
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendScoreCommand(const uint8_t *mac, int score = 0)
{
    String message = String(COMMAND_SCORE) + " " + score;
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}

void BuzzoController::SendSleepCommand(const uint8_t *mac)
{
    String message = String(COMMAND_SLEEP);
    esp_now_send(mac, (uint8_t*)message.c_str(), message.length());
}
#pragma endregion

#pragma region Update Functions
void BuzzoController::UpdatePlaying()
{
    if(_clientResponses.HasCurrentRespondant())
    {
        // If we are waiting to move to the next respondant, check to see if enough time has passed.
        if(_nextResponderDelayStartTime > 0 && millis() - _nextResponderDelayStartTime > TIME_BETWEEN_RESPONDERS)
        {
            _nextResponderDelayStartTime = 0;

            // Find our next respondant
            auto respondantId = _clientResponses.GetCurrentRespondant();
            auto client = GetClient(respondantId);

            Serial.print("Next respondant: ");
            Serial.println(respondantId.c_str());

            Serial.print("Is respondant active? ");
            Serial.println(client->IsActive() ? "YES" : "NO");

            if(client != 0 && client->IsActive())
            {
                Serial.println("Answering");
                _responseStartTime = millis();
                _responsePauseTime = 0;

                uint8_t mac[6];
                client->GetMacAddress(mac);

                SendAnswerCommand(mac, RESPONSE_TIME_LIMIT, RESPONSE_TIME_LIMIT);
                _lastRespondantPingTime = millis();
            }

            // Tell all other buttons that they're in the queue
            Serial.println("Telling other buttons they're in the queue");
            Serial.print("Pending Respondants: ");
            Serial.println(_clientResponses.GetPendingRespondantCount());

            int placeInQueue = 0;
            for(int i = 0; i < _clientResponses.GetPendingRespondantCount(); i++)
            {
                client = GetClient(_clientResponses.GetNextRespondant(i));
                if(client != 0 && client->IsActive())
                {
                    placeInQueue++;

                    uint8_t mac[6];
                    client->GetMacAddress(mac);

                    SendQueueCommand(mac, placeInQueue);
                }
            }

            Serial.println("Done");
        }
        else if(_nextResponderDelayStartTime > 0)
        {
            // We're not ready to move to the next response yet
            return;
        }
        else
        {
            if(_isPaused)
            {
                _responsePauseTime += millis() - _lastMillis;
            }

            unsigned long millisResponding = millis() - _responseStartTime - _responsePauseTime;
            int secondsResponding = floor(millisResponding / 1000);      
            
            // Responder ran out of time
            if(millisResponding > (RESPONSE_TIME_LIMIT * 1000))
            {
                auto client = GetClient(_clientResponses.GetCurrentRespondant());

                if(client != 0)
                {
                    uint8_t mac[6];
                    client->GetMacAddress(mac);

                    SendResponseCommand(mac, false);
                }

                _clientResponses.MoveToNextRespondant();
                //_previousRespondant.assign(_clientResponses.GetCurrentRespondant());
                //_clientResponses.GetCurrentRespondant().assign("");

                _responsePauseTime = 0;
                _nextResponderDelayStartTime = millis();        
            }            
            else if(millis() - _lastRespondantPingTime > RESPONDANT_PING_TIME)
            {
                // Serial.print("Time responding: ");
                // Serial.println(millisResponding);

                auto client = GetClient(_clientResponses.GetCurrentRespondant());

                if(client != 0)
                {                    
                    int secondsRemaining = RESPONSE_TIME_LIMIT - secondsResponding;

                    if(_isPaused)
                    {
                        secondsRemaining = -secondsRemaining;
                    }

                    uint8_t mac[6];
                    client->GetMacAddress(mac);

                    SendAnswerCommand(mac, secondsRemaining, RESPONSE_TIME_LIMIT);
                }

                _lastRespondantPingTime = millis();
            }
        }
    }
    else // No current respondant
    {
        // if(_clientResponses.HasNextRespondant() && _clientResponses.IsRoundOver() == false)
        // {
        //     // Wait a short delay between respondants to increase gameplay tension
        //     if(millis() - _nextResponderDelayStartTime > TIME_BETWEEN_RESPONDERS)
        //     {
        //         // Find our next respondant
        //         auto nextRespondantId = _responseQueue.DequeueNextResponse();
        //         auto client = GetClient(nextRespondantId);

        //         Serial.print("Next respondant: ");
        //         Serial.println(nextRespondantId.c_str());

        //         Serial.print("Is respondant active? ");
        //         Serial.println(client->IsActive() ? "YES" : "NO");

        //         if(client != 0 && client->IsActive())
        //         {
        //             Serial.println("Answering");
        //             _clientResponses.GetCurrentRespondant().assign(client->GetId());
        //             _responseStartTime = millis();
        //             _responsePauseTime = 0;

        //             uint8_t mac[6];
        //             client->GetMacAddress(mac);

        //             SendAnswerCommand(mac, RESPONSE_TIME_LIMIT, RESPONSE_TIME_LIMIT);
        //             _lastRespondantPingTime = millis();
        //         }

        //         // Tell all other buttons that they're in the queue
        //         int placeInQueue = 0;
        //         for(int i = 0; i < _responseQueue.GetResponseCount(); i++)
        //         {
        //             client = GetClient(_responseQueue.PeekNextResponse(i));
        //             if(client != 0 && client->IsActive())
        //             {
        //                 placeInQueue++;

        //                 uint8_t mac[6];
        //                 client->GetMacAddress(mac);

        //                 SendQueueCommand(mac, placeInQueue);
        //             }
        //         }
        //     }
        // }
        // else if(_isAcceptingResponses == false)
        // {
        //     if(_autoResestTime > 0 && millis() > _autoResestTime)
        //     {
        //         ResetButtonPress();
        //     }
        // }
    }
    
    // Do auto reset
    if(_autoResestTime > 0 && millis() > _autoResestTime)
    {
        ResetButtonPress();
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

ButtonClientInfo* BuzzoController::GetClient(const uint8_t *mac)
{
    for(int i = 0; i < _clientCount; i++)
    {
        if(_clients[i]->CompareMac(mac))
        {
            return _clients[i];
        }
    }

    return 0;
}

ButtonClientInfo* BuzzoController::GetClient(std::string id)
{
    if(id.empty())
    {
        return 0;
    }

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

            auto respondantStatus = _clientResponses.GetRespondantStatus(_clients[i]->GetId());

            if(respondantStatus == RespondantStatus::NONE)
            {
                count++;
            }

            
            // for(int j = 0; j < _participantCount; j++)
            // {
            //     if(_participants[j].compare(_clients[i]->GetId()) == 0)
            //     {
            //         hasParticipated = true;
            //         break;
            //     }
            // }            

            // if(!hasParticipated)
            // {
            //     count++;
            // }
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
            auto respondantStatus = _clientResponses.GetRespondantStatus(_clients[i]->GetId());

            if(respondantStatus != RespondantStatus::NONE)
            {
                continue;
            }
            
            // for(int j = 0; j < _participantCount; j++)
            // {
            //     if(_participants[j].compare(_clients[i]->GetId()) == 0)
            //     {
            //         hasParticipated = true;
            //         break;
            //     }
            // }            

            Serial.print("Adding client to response queue: ");
            Serial.println(_clients[i]->GetId().c_str());
            
            // Pretend that this participant has buzzed
            uint8_t mac[6];
            _clients[i]->GetMacAddress(mac);

            ProcessBuzzCommand(mac);
        }
    }
}


#pragma endregion

#pragma region Button Functions
void BuzzoController::EndCurrentRespondantTurn(bool isCorrect)
{
    _lastButtonPressTime = millis();

    if(_currentState == BuzzoController::PLAYING && _clientResponses.IsRoundOver() == false)
    {
        Serial.print("Ending Turn - ");
        Serial.println(isCorrect ? "CORRECT" : "INCORRECT");

        if(!_clientResponses.HasCurrentRespondant())
        {
            Serial.println("There is no current respondant");
            return;
        }

        auto client = GetClient(_clientResponses.GetCurrentRespondant());

        if(client != 0)
        {        
            if(isCorrect)
            {
                client->SetScore(min(MAX_SCORE, client->GetScore() + 1));
            }
            
            _clientResponses.MoveToNextRespondant();

            Serial.print("Score: ");
            Serial.println(client->GetScore());

            u_int8_t mac[6];
            client->GetMacAddress(mac);

            SendScoreCommand(mac, client->GetScore());
            SendResponseCommand(mac, isCorrect);
        }

        if(isCorrect)
        {
            _clientResponses.EndRound();

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

            if(_clientResponses.HasCurrentRespondant() == false)
            {
                Serial.println("Response queue is empty");

                Serial.print("Clients yet to play: ");
                Serial.println(GetActiveClientsYetToPlayCount());
                
                // if(GetActiveClientsYetToPlayCount() == 1)
                // {
                //     AddAllRemainingClientsToQueue();
                // }
            }
        }

        // _previousRespondant.assign(_clientResponses.GetCurrentRespondant());
        // _previousRespondantWasCorrect = isCorrect;
        // _clientResponses.GetCurrentRespondant().assign("");            
        
    }
}

void BuzzoController::ResetButtonPress()
{
    _lastButtonPressTime = millis();

    if(_currentState == BuzzoController::PLAYING)
    {
        for(int i = 0; i < _clientCount; i++)
        {
            u_int8_t mac[6];
            _clients[i]->GetMacAddress(mac);

            SendResetCommand(mac, true);
        }

        // _clientResponses.GetCurrentRespondant().assign("");
        // _previousRespondant.assign("");

        //_isAcceptingResponses = true;
        _nextResponderDelayStartTime = 0;

        //_responseQueue.Clear();
        _clientResponses.Reset();

        Serial.print("Has current respondant? ");
        Serial.println(_clientResponses.HasCurrentRespondant() ? "YES" : "NO");

        //_participantCount = 0;
        
        _autoResestTime = 0;
        _nextResponderDelayStartTime = 0;
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
                u_int8_t mac[6];
                _clients[i]->GetMacAddress(mac);

                SendSleepCommand(mac);
            }

        }
        else
        {
            for(int i = 0; i < _clientCount; i++)
            {
                _clients[i]->SetScore(0);
                
                u_int8_t mac[6];
                _clients[i]->GetMacAddress(mac);                

                SendResetCommand(mac, true);
                SendScoreCommand(mac, _clients[i]->GetScore());
            }

            _clientResponses.Reset();
            // _clientResponses.GetCurrentRespondant().assign("");
            // _previousRespondant.assign("");

            // _isAcceptingResponses = true;
            _nextResponderDelayStartTime = 0;
            _isReset = true;

            // _responseQueue.Clear();
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
    if(isCorrect)
    {
        // Previous respondant was marked incorrect, but they should have been correct.

        // Check to see if all the conditions are correct
        // ==============================================

        if(_clientResponses.IsRoundOver()) // If the round is over, then the previous respondant must be correct already
        {
            Serial.println("Unable to adjust previous respondant to CORRECT because the round is already over");
            return;
        }

        if(!_clientResponses.HasPreviousRespondant()) // If there's no previous respondant then there's nothing to do
        {
            Serial.println("Unable to adjust previous respondant to CORRECT because there is no previous respondant");
            return;
        }        

        auto client = GetClient(_clientResponses.GetPreviousRespondant());
        if(client == 0) // If we can't find the client then something's gone wrong
        {
            Serial.println("Unable to adjust previous respondant to CORRECT because the client could not be found");
            return;
        }

        u_int8_t mac[6];
        client->GetMacAddress(mac);

        // Adjust the scores
        // =================

        // 1. Give a point to the previous respondant
        client->SetScore(min(MAX_SCORE, client->GetScore() + 1));
        Serial.print("New Score: ");
        Serial.println(client->GetScore());        

        SendScoreCommand(mac, client->GetScore());

        // 2. Tell the previous respondant that they were correct
        SendResponseCommand(mac, isCorrect);

        // 3. End the round (because a correct answer was given)
        _clientResponses.EndRound();

        // 4. Tell all other buttons that they're in the queue again
        int placeInQueue = 0;
        for(int i = 0; i < _clientResponses.GetPendingRespondantCount(); i++)
        {
            auto pendingClient = GetClient(_clientResponses.GetNextRespondant(i));
            u_int8_t pendingMac[6];
            pendingClient->GetMacAddress(pendingMac);

            if(pendingClient != 0 && pendingClient->IsActive())
            {
                placeInQueue++;
                SendQueueCommand(pendingMac, placeInQueue);
            }
        }

        // 5. Make sure auto reset is disabled
        _autoResestTime = 0;
        // ...And we're done
    
    }
    else
    {
        // Previous respondant was marked correct, but they should have been incorrect.

        // Check to see if all the conditions are correct
        // ==============================================

        if(!_clientResponses.IsRoundOver()) // If the round is not over, then the previous respondant must be incorrect already
        {
            Serial.println("Unable to adjust previous respondant to INCORRECT because the round is already over");
            return;
        }

        if(!_clientResponses.HasPreviousRespondant()) // If there's no previous respondant then there's nothing to do
        {
            Serial.println("Unable to adjust previous respondant to INCORRECT because there is no previous respondant");
            return;
        }

        auto client = GetClient(_clientResponses.GetPreviousRespondant());

        if(client == 0) // If we can't find the client then something's gone wrong
        {
            Serial.println("Unable to adjust previous respondant to INCORRECT because the client could not be found");
            return;
        }        

        u_int8_t mac[6];
        client->GetMacAddress(mac);
        
        // Adjust the score
        // =================

        // 1. If they were previously given a score, subtract it
        client->SetScore(max(0, client->GetScore() - 1));
        Serial.print("New Score: ");
        Serial.println(client->GetScore());        

        SendScoreCommand(mac, client->GetScore());

        // 2. Tell the previous respondant that they were incorrect
        SendResponseCommand(mac, isCorrect);

        // 3. Resume the game
        _clientResponses.StartRound();
        
        // 4. Set the next responder delay to give the moment time to breath
        _nextResponderDelayStartTime = millis();

        // 5. Make sure auto reset is disabled
        _autoResestTime = 0;

    }

    // //if(_previousRespondant.length() > 0 && _previousRespondantWasCorrect != isCorrect)
    // if(_clientResponses.HasPreviousRespondant())
    // {
    //     auto client = GetClient(_clientResponses.GetPreviousRespondant());

    //     if(client == 0)
    //     {
    //         return;
    //     }

    //     u_int8_t mac[6];
    //     client->GetMacAddress(mac);

    //     auto previousRespondantStatus = _clientResponses.GetRespondantStatus(client->GetId());

    //     if(!(previousRespondantStatus == CORRECT || previousRespondantStatus == INCORRECT))
    //     {
    //         // Previous respondant has an unexpected status. No way to handle this.

    //         Serial.print("Previous respondant has an unexpected status: ");
    //         Serial.println(previousRespondantStatus);
    //         return;
    //     }
        

    //     if(previousRespondantStatus == RespondantStatus::INCORRECT)
    //     {
    //         if(desiredStatus == RespondantStatus::INCORRECT)
    //         {
    //             return; // Nothing to do, prevous respondant is already incorrect
    //         }
    //         else
    //         {
    //             // The previous respondant was incorrect, but we're now saying they were correct. Let's fix:
    //             _clientResponses.EndRound();

                
    //         }
    //     } 



    //     if(client != 0)
    //     {        
    //         if(isCorrect)
    //         {                
    //             client->SetScore(min(MAX_SCORE, client->GetScore() + 1));

    //             Serial.print("New Score: ");
    //             Serial.println(client->GetScore());

    //             SendScoreCommand(mac, client->GetScore());
    //             SendResponseCommand(mac, isCorrect);

    //             if(_clientResponses.GetCurrentRespondant().length() > 0)
    //             {
    //                 // Push the current respondant to the start of the queue again, just in case
    //                 _responseQueue.PushResponse(_clientResponses.GetCurrentRespondant());
    //                 _clientResponses.GetCurrentRespondant().assign("");

    //                 int placeInQueue = 0;
    //                 for(int i = 0; i < _responseQueue.GetResponseCount(); i++)
    //                 {
    //                     auto currentRespondantClient = GetClient(_responseQueue.PeekNextResponse(i));
    //                     u_int8_t currentRespondantMac[6];
    //                     currentRespondantClient->GetMacAddress(currentRespondantMac);

    //                     if(currentRespondantClient != 0 && currentRespondantClient->IsActive())
    //                     {
    //                         placeInQueue++;
    //                         SendQueueCommand(currentRespondantMac, placeInQueue);
    //                     }
    //                 }  
    //             }    
    //         }
    //         else
    //         {
    //             // 1. If they were previously given a score, subtract it
    //             client->SetScore(max(0, client->GetScore() - 1));
    //             Serial.print("New Score: ");
    //             Serial.println(client->GetScore());

    //             // 2. Tell the previous respondant that they were incorrect
    //             SendScoreCommand(mac, client->GetScore());
    //             SendResponseCommand(mac, isCorrect);

    //             // 3. Tell all other buttons that they're in the queue again
    //             int placeInQueue = 0;
    //             for(int i = 0; i < _responseQueue.GetResponseCount(); i++)
    //             {
    //                 client = GetClient(_responseQueue.PeekNextResponse(i));
    //                 if(client != 0 && client->IsActive())
    //                 {
    //                     placeInQueue++;
    //                     SendQueueCommand(mac, placeInQueue);
    //                 }
    //             }  

    //             // 4. Add a bit of a delay befor the next turn. Just to give the moment time to breath
    //             _nextResponderDelayStartTime = millis() + 2000; // Add a bit more time here
    //         }

    //         _previousRespondantWasCorrect = isCorrect;
    //         _isAcceptingResponses = !isCorrect;
    //     }
    // }
    // else
    // {
    //     _isInAdjustMode = true;

    //     for(int i = 0; i < _clientCount; i++)
    //     {
    //         SendSelectCommand(_clients[i]->GetIpAddress());
    //     }
    // }
}

void BuzzoController::SetAllClientsToSleep()
{
    for(int i = 0; i < _clientCount; i++)
    {
        u_int8_t mac[6];
        _clients[i]->GetMacAddress(mac);

        SendSleepCommand(mac);
    }
}

void BuzzoController::ReleaseHoldIncorrectButton()
{
    _isInAdjustMode = false;

    SetAllClientsToSleep();
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

