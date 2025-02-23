#include "ButtonClientInfo.h"
#include <Arduino.h>

#define MAX_SCORE 9

ButtonClientInfo::ButtonClientInfo() :
_ip(),
_score(0),
_lastContactMillis(millis())
{
    _id.assign("");
}

ButtonClientInfo::ButtonClientInfo(IPAddress ip, std::string id) :
_ip(ip),
_score(0),
_id(id),
_lastContactMillis(millis())
{
}

IPAddress ButtonClientInfo::GetIpAddress()
{
    return _ip;
}

void ButtonClientInfo::SetIpAddress(IPAddress newAddress)
{
    _ip = newAddress;
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