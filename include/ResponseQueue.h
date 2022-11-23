#ifndef RESPONSE_QUEUE_H
#define RESPONSE_QUEUE_H

#include <Wifi.h>

template <class T>
class ResponseQueue
{
  public:
    ResponseQueue(int maxResponses)
    {
      _currentResponseIndex = 0;
      _responseCount = 0;
      _maxRespones = maxResponses;

      _responses = new T[_maxRespones];
    }

    ~ResponseQueue()
    {
      delete[] _responses;
    }    

    bool IsEmpty()
    {
      return _responseCount == 0;
    }     

    bool ContainsResponse(T response)
    {
      for(int i = 0; i < _responseCount; i++)
      {
        if(_responses[GetIndex(i)] == response)
        {
          return true;
        }
      }

      return false;
    }    

    /// @brief Removes the next response from the queue and returns it.
    /// @return The next response, or NULL if the queue is empty
    T DequeueNextResponse()
    {
      if(IsEmpty())
      {
        return {};
      }

      T response = _responses[_currentResponseIndex];

      _responseCount--;
      _currentResponseIndex = GetIndex(1);

      return response;
    }   

    /// @brief Adds a new response to the queue
    void EnqueueResponse(T response)
    {
      if(_responseCount < _maxRespones)
      {
        _responses[GetIndex(_responseCount)] = response;
        _responseCount++;
      }
    }   

    /// @brief Pushes a response to the front of the queue, so that it will be the next one dequeued
    void PushResponse(T response)
    {
      if(_responseCount < _maxRespones)
      {
        _currentResponseIndex = GetIndex(_maxRespones-1);
        _responses[_currentResponseIndex] = response;
        _responseCount++;
      }
    }

    T PeekNextResponse(int index = 0)
    {
      if(IsEmpty())
      {
        return {};
      }

      return _responses[GetIndex(index)];
    }    

    void Clear()
    { 
      _responseCount = 0;
    }    

    int GetResponseCount() { return _responseCount; }

  private:
    unsigned int GetIndex(unsigned int i)
    {
      return (_currentResponseIndex + i) % _maxRespones;
    }

    T* _responses;
    int _currentResponseIndex;
    int _responseCount;
    int _maxRespones;

};


#endif