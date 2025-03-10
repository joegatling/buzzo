#include "ClientResponseQueue.h"

ClientResponseQueue::ClientResponseQueue() :
_currentIndex(0),
_responseCount(0),
_isRoundOver(false)
{
    for(int i = 0; i < MAX_QUEUE_LENGTH; i++)
    {
        _queue[i].assign("");
    }
}

void ClientResponseQueue::Reset()
{
    _isRoundOver = false;
    
    for(int i = 0; i < MAX_QUEUE_LENGTH; i++)
    {
        _queue[i].assign("");
    }
    
    _currentIndex = 0;
    _respondantCount = 0;
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
    if(_isRoundOver)
    {
        return;
    }    

    _currentIndex = min(MAX_QUEUE_LENGTH - 1, _currentIndex + 1);
}

void ClientResponseQueue::EndRound()
{
    if(_isRoundOver)
    {
        return;
    }

    MoveToNextRespondant();
    _isRoundOver = true;
}

std::string ClientResponseQueue::GetPreviousRespondant()
{
    if(!HasPreviousRespondant())
    {
        return 0;
    }

    return _queue[_currentIndex - 1];
}

std::string ClientResponseQueue::GetCurrentRespondant()
{
    if(!HasCurrentRespondant())
    {
        return 0;
    }

    return _queue[_currentIndex];
}

std::string ClientResponseQueue::GetNextRespondant()
{
    if(!HasNextRespondant())
    {
        return 0;
    }

    return _queue[_currentIndex + 1];
}

RespondantStatus clientresponsequeue::GetRespondantStatus(std::string respondantId)
{
    RespondantStatus result = RespondantStatus::NONE;

    for(int i = 0; i < _respondantCount; i++)
    {
        if(_queue[i].compare(respondantId) == 0)
        {
            if(i < _currentIndex - 1)
            {
                result = RespondantStatus::INCORRECT;
            }
            else if(i == _currentIndex - 1)
            {
                result = _isRoundOver ? RespondantStatus::CORRECT : RespondantStatus::INCORRECT;
            }
            else if(i == _currentIndex)
            {
                result = _isRoundOver ? RespondantStatus::PENDING : RespondantStatus::ANSWERING;
            }
            else
            {
                result = RespondantStatus::PENDING;
            }
        }
    }

    return result;
}

bool IsRoundOver()
{
    return _isRoundOver;
}

// Possible States
// [ ] [Y] [p] [p]
//          ^
// [ ] [N] [A] [p]
//          ^