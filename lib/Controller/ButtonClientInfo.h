#pragma once

#include <string>

#include "IPAddress.h"

#define ID_LENGTH 32
#define LOST_CONTACT_TIME 20000

class ButtonClientInfo
{
    public: 
        ButtonClientInfo();
        ButtonClientInfo(const uint8_t *mac, std::string id);

        void GetMacAddress(uint8_t mac[6]);
        void SetMacAddress(const uint8_t *mac);
        
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

        bool CompareMac(const uint8_t *mac);

    private:
        //IPAddress _ip;
        uint8_t _mac[6];
        
        int _score;   
        std::string _id;


        unsigned long _lastContactMillis;

        unsigned int _batteryLevel;;
};