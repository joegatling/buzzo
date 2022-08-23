#pragma once

#include "IPAddress.h"

class ButtonClient
{
    public: 
        ButtonClient();
        ButtonClient(IPAddress ip);

        IPAddress GetIpAddress();

        int GetScore();
        void SetScore(int score);
        int IncrementScore(int add);

    private:
        IPAddress _ip;
        int _score;    
};