#include "ButtonClient.h"
#include <Arduino.h>


ButtonClient::ButtonClient() :
_score(0)
{}

ButtonClient::ButtonClient(IPAddress ip) :
_ip(ip),
_score(0)
{}

IPAddress ButtonClient::GetIpAddress()
{
    return _ip;
}

int ButtonClient::GetScore()
{
    return _score;
}

void ButtonClient::SetScore(int score)
{
    _score = max(0, score);
}

int ButtonClient::IncrementScore(int add)
{
    _score += add;
    _score = max(0, _score);
    return _score;
}