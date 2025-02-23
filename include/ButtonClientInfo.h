#pragma once

#include <string>

#include "IPAddress.h"

#define ID_LENGTH 32
#define LOST_CONTACT_TIME 20000

class ButtonClientInfo
{
    public: 
        ButtonClientInfo();
        ButtonClientInfo(IPAddress ip, std::string id);

        IPAddress GetIpAddress();
        void SetIpAddress(IPAddress newAddress);
        
        std::string& GetId();
        void SetId(std::string id);

        int GetScore();
        void SetScore(int score);
        int IncrementScore(int add);

        unsigned long GetTimeSinceLastContact();
        void ResetTimeSinceLastContact();

        bool IsActive() { return GetTimeSinceLastContact() < LOST_CONTACT_TIME; }

        unsigned int GetBatteryLevel();
        void SetBatteryLevel(unsigned int level);

    private:
        IPAddress _ip;
        int _score;   
        std::string _id;

        unsigned long _lastContactMillis;

        unsigned int _batteryLevel;;
};