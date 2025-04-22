#include "ClientResponseQueue.h"
#include <bits/stdc++.h>
using namespace std;

ClientResponseQueue::ClientResponseQueue() :
_currentIndex(0),
_respondantCount(0),
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

bool ClientResponseQueue::EnqueueRespondant(std::string respondantId)
{

    bool isCurrentRespondant = HasCurrentRespondant() == false && IsRoundOver() == false;
    

    if(_respondantCount >= MAX_QUEUE_LENGTH)
    {
        return false;
    }

    _queue[_respondantCount].assign(respondantId);
    _respondantCount++;

    // Return true if the client we just enqueued is the current respondant
    return isCurrentRespondant;
}

int ClientResponseQueue::GetRespondantCount()
{
    return _respondantCount;
}

int ClientResponseQueue::GetPendingRespondantCount()
{
    if(_isRoundOver)
    {
        return max((unsigned int)0, (_respondantCount) - _currentIndex);
    }
    else
    {
        return max((unsigned int)0, (_respondantCount - 1) - _currentIndex);
    }
}

bool ClientResponseQueue::HasNextRespondant()
{
    if(_isRoundOver)
    {
        // If the round is over, the respondant at the current index counts as the next respondant.        
        return _currentIndex < _respondantCount;
    }
    else
    {
        return _currentIndex < _respondantCount - 1;
    }
}

bool ClientResponseQueue::HasCurrentRespondant()
{
    if(_isRoundOver)
    {
        return false;
    }
    else
    {
        return _currentIndex < _respondantCount;
    }
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

    _currentIndex = min(MAX_QUEUE_LENGTH - 1, (int)_currentIndex + 1);
}

void ClientResponseQueue::EndRound()
{
    _isRoundOver = true;
}

void ClientResponseQueue::StartRound()
{
    _isRoundOver = false;
}

std::string ClientResponseQueue::GetPreviousRespondant()
{
    if(!HasPreviousRespondant())
    {
        return "";
    }

    return _queue[_currentIndex - 1];
}

std::string ClientResponseQueue::GetCurrentRespondant()
{
    if(!HasCurrentRespondant())
    {
        return "";
    }

    return _queue[_currentIndex];
}

std::string ClientResponseQueue::GetNextRespondant()
{
    if(!HasNextRespondant())
    {
        return "";
    }

    return GetNextRespondant(0);
}

std::string ClientResponseQueue::GetNextRespondant(unsigned int index)
{
    int offset = _isRoundOver ? 0 : 1;
    index += _currentIndex + offset;
    
    if(index >= _respondantCount)
    {
        return "";
    }

    return _queue[index];
}

RespondantStatus ClientResponseQueue::GetRespondantStatus(std::string respondantId)
{
    RespondantStatus result = RespondantStatus::NONE;

    for(int i = 0; i < _respondantCount; i++)
    {
        if(_queue[i].compare(respondantId) == 0)
        {
            if(_currentIndex > 0 && i < _currentIndex - 1)
            {
                result = RespondantStatus::INCORRECT;
            }
            else if(_currentIndex > 0 && i == _currentIndex - 1)
            {
                result = _isRoundOver ? RespondantStatus::CORRECT : RespondantStatus::INCORRECT;
            }
            else if(i == _currentIndex)
            {
                result = _isRoundOver ? RespondantStatus::QUEUED : RespondantStatus::ANSWERING;
            }
            else
            {
                result = RespondantStatus::QUEUED;
            }

            break;
        }
    }

    return result;
}

int ClientResponseQueue::GetQueuedRespondantIndex(std::string respondantId)
{
    int startIndex = _isRoundOver ? _currentIndex : _currentIndex + 1;
    for(int i = startIndex; i < _respondantCount; i++)
    {
        if(_queue[i].compare(respondantId) == 0)
        {
            return i;
        }
    }

    return -1;
}

bool ClientResponseQueue::IsRoundOver()
{
    return _isRoundOver;
}

// Possible States
// [ ] [Y] [p] [p]
//          ^
// [ ] [N] [A] [p]
//          ^