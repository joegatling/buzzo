#pragma once

#include <string>

#define MAX_QUEUE_LENGTH 12

enum RespondantStatus
{
    NONE,
    CURRENTLY_RESPONDING,
    INCORRECT,
    CORRECT
}

class ClientResponseQueue
{
    public:
        ClientResponseQueue();

        void EnqueueRespondant(std::string respondantId);

        int GetRespondantCount();
        
        bool HasNextRespondant();
        bool HasCurrentRespondant();
        bool HasPreviousRespondant();

        void MoveToNextRespondant();
        void CurrentRespondnat(bool isCorrect);

        std::string GetPreviousRespondant();
        std::string GetCurrentRespondant();
        std::string GetNextRespondant();
        

        bool IsIncorrect(std::string respondantId);
        bool IsCorrect(std::string respondantId);
        bool IsCurrentRespondant(std::string respondantId);
        bool IsInQueue(std::string respondantId);

        RespondantStatus GetRespondantStatus(std::string respondantId)

    private:
        unsigned int _currentIndex;
        unsigned int _respondantCount;

        std::string _queue[MAX_QUEUE_LENGTH];
    

}