#include "ButtonClientInfo.h"
#include <Arduino.h>

#define MAX_SCORE 9

ButtonClientInfo::ButtonClientInfo() :
_score(0),
_lastContactMillis(millis())
{
    memset(_mac, 0, sizeof(_mac));

    _id.assign("");
}

ButtonClientInfo::ButtonClientInfo(const uint8_t *mac, std::string id) :
_score(0),
_id(id),
_lastContactMillis(millis())
{
    for(int i = 0; i < 6; i++)
    {
        _mac[i] = mac[i];
    }
}

void ButtonClientInfo::GetMacAddress(uint8_t mac[6])
{
    for(int i = 0; i < 6; i++)
    {
        mac[i] = _mac[i];
    }
}

void ButtonClientInfo::SetMacAddress(const uint8_t *newMac)
{
    for(int i = 0; i < 6; i++)
    {
        _mac[i] = newMac[i];
    }
}

std::string& ButtonClientInfo::GetId()
{
    return _id;
}

void ButtonClientInfo::SetId(std::string id)
{
    _id.assign(id);
}

int ButtonClientInfo::GetScore()
{
    return _score;
}

void ButtonClientInfo::SetScore(int score)
{
    _score = max(0, min(MAX_SCORE, score));
}

int ButtonClientInfo::IncrementScore(int add)
{
    _score += add;
    _score = max(0, _score);
    return _score;
}

unsigned long ButtonClientInfo::GetTimeSinceLastContact()
{
    return millis() - _lastContactMillis;
}

void ButtonClientInfo::ResetTimeSinceLastContact()
{
    _lastContactMillis = millis();
}

unsigned int ButtonClientInfo::GetBatteryLevel()
{
    return _batteryLevel;
}

void ButtonClientInfo::SetBatteryLevel(unsigned int level)
{
    _batteryLevel = level;
}

bool ButtonClientInfo::CompareMac(const uint8_t *mac)
{
    for(int i = 0; i < 6; i++)
    {
        if(_mac[i] != mac[i])
        {
            return false;
        }
    }

    return true;
}