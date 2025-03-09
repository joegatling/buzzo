#include "ClientResponseQueue.h"

ClientResponseQueue::ClientResponseQueue() :
_currentResponseIndex(0),
_responseCount(0),
{
    for(int i = 0; i < MAX_QUEUE_LENGTH; i++)
    {
        _queue[i].assign("");
    }
}

void ClientResponseQueue::EnqueueRespondant(std::string respondantId)
{
    if(_respondantCount >= MAX_QUEUE_LENGTH)
    {
        return;
    }

    _queue[_currentIndex].assign(respondantId);
    _respondantCount++;
}

void ClientResponseQueue::GetRespondantCount()
{
    return _respondantCount;
}

bool ClientResponseQueue::HasNextRespondant()
{
    return _currentIndex < _respondantCount - 1;
}

bool ClientResponseQueue::HasCurrentRespondant()
{
    return _currentIndex < _respondantCount;
}

bool ClientResponseQueue::HasPreviousRespondant()
{
    return _currentIndex > 0;
}

void ClientResponseQueue::MoveToNextRespondant()
{    
    _currentIndex = min(MAX_QUEUE_LENGTH-1, _currentIndex + 1);
}

void ClientResponseQueue::SetCurrentRespondantStatus

std::string ClientResponseQueue::GetPreviousRespondant()
{
    if(_currentIndex == 0)
    {
        return "";
    }

    return _queue[_currentIndex - 1];
}

std::string ClientResponseQueue::GetCurrentRespondant()
{
    if(_currentIndex >= _respondantCount)
    {
        return "":
    }

    return _queue[_currentIndex];
}

std::string ClientResponseQueue::GetNextRespondant()
{
    if(!HasNextRespondant())
    {
        return "";
    }

    return _queue[_currentIndex + 1];
}

bool clientresponsequeue::IsIncorrect(std::string respondantId)
{
    for(int i = 0; i < _currentIndex; i++)
    {
        if(_queue[i].compare(respondantId) == 0)
        {
            return true;
        }
    }
    
    return false;
}

bool clientresponsequeue::IsCorrect(std::string respondantId)
{

}

bool clientresponsequeue::IsCurrentRespondant(std::string respondantId)
{}

bool clientresponsequeue::IsInQueue(std::string respondantId)
{}
