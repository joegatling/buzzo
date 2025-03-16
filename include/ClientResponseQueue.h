#pragma once

#include <string>

#define MAX_QUEUE_LENGTH 12

enum RespondantStatus
{
    NONE,
    INCORRECT,
    CORRECT,
    ANSWERING,
    PENDING
}

class ClientResponseQueue
{
    public:
        ClientResponseQueue();
        
        void Reset();

        void EnqueueRespondant(std::string respondantId);

        int GetRespondantCount();
        
        bool HasNextRespondant();
        bool HasCurrentRespondant();
        bool HasPreviousRespondant();

        void MoveToNextRespondant();
        void EndRound();

        std::string GetPreviousRespondant();
        std::string GetCurrentRespondant();
        std::string GetNextRespondant();
        
        RespondantStatus GetRespondantStatus(std::string respondantId)

        bool IsRoundOver();

    private:
        unsigned int _currentIndex;
        unsigned int _respondantCount;

        std::string _queue[MAX_QUEUE_LENGTH];

        bool _isRoundOver;
    

}