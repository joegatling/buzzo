#pragma once

#include <string>

#define MAX_QUEUE_LENGTH 12

enum RespondantStatus
{
    NONE,
    INCORRECT,
    CORRECT,
    ANSWERING,
    QUEUED
};

class ClientResponseQueue
{
    public:
        ClientResponseQueue();
        
        /**
         * @brief Reset the queue
         */
        void Reset();

        /**
         * @brief Enque a respondant to the queue
         * 
         * @param respondantId 
         * @return true if the client we just enqueued is the current respondant
         * @return false if the client we just enqueued is not the current respondant
         */
        bool EnqueueRespondant(std::string respondantId);

        /**
         * @brief Get the number of respondants. This includes any previous, current, and pending respondants.
         * 
         * @return The number of respondants (the total number of buzzed clients)
         */
        int GetRespondantCount();   

        /**
         * @brief Get the number of pending respondants. This is the number of clients that have buzzed but have not yet had their turn.
         * 
         * @return The number of respondants waiting to answer 
         */
        int GetPendingRespondantCount();     
        
        /**
         * @brief Is there a respondant who could go next?
         * 
         * @return true if there is a respondant who could go next. 
         *         Note that this could be true even if the round is over. 
         * @return false if there is no respondant who could go next.
         */
        bool HasNextRespondant();

        /**
         * @brief Is there a current respondant?
         * 
         * @return true if the round is not over and there is a current respondant
         * @return false if there is no respondant. Either because the round is over or because no one else has buzzed yet.
         */
        bool HasCurrentRespondant();
        
        /**
         * @brief Is there a previous respondant?
         * 
         * @return true if there is a previous respondant
         * @return false if there is no previous respondant. This means that either no one has buzzed yet or the round is over.
         */
        bool HasPreviousRespondant();

        /**
         * @brief Move to the next respondant. This will not move to the next respondant if the round is over.
         */
        void MoveToNextRespondant();

        /**
         * @brief Set that the round is over.
         */
        void EndRound();

        /**
         * @brief Set that the round is not over.
         */
        void StartRound();        

        /**
         * @brief Get the id of the previous respondant
         * 
         * @return 0 if there is no previous respondant
         * @return std::string the id of the previous respondant
         */
        std::string GetPreviousRespondant();

        /**
         * @brief Get the id of the current respondant
         * 
         * @return 0 if there is no current respondant
         * @return 0 if the round is over
         * @return std::string the id of the current respondant
         */
        std::string GetCurrentRespondant();

        /**
         * @brief Get the id of the next respondant. This if functionally equvalent of GetNextRespondant(0).
         * 
         * @return 0 if there is no next respondant
         * @return std::string the id of the next respondant
         */
        std::string GetNextRespondant();

        /**
         * @brief Peeks at a respondant in the queue
         * 
         * @param index the index of the respondant to peek at. 0 is the next respondant.
         * @return std::string the id of the respondant at the given index.
         * @return 0 if there is no respondant at the given index
         */
        std::string GetNextRespondant(unsigned int index);
        
        /**
         * @brief Get the status of a respondant with the provided id. 
         * 
         * @param respondantId the client id of th respondant. 
         * @return RespondantStatus the status of the respondant.
         */
        RespondantStatus GetRespondantStatus(std::string respondantId);

        /**
         * @brief Get the index of the specified respondant in the queue
         * 
         * @param respondantId the string id of the respondant
         * @return int [0..x] the index of the respondant in the queue
         * @return -1 if the respondant is not in the queue
         */
        int GetQueuedRespondantIndex(std::string respondantId);

        /**
         * @brief Is the round over?
         * 
         * @return true if the round is over
         * @return false if the round is not over
         */
        bool IsRoundOver();

    private:
        unsigned int _currentIndex;
        unsigned int _respondantCount;

        std::string _queue[MAX_QUEUE_LENGTH];

        bool _isRoundOver;
    

};