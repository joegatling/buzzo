#ifndef RESPONSE_QUEUE_H
#define RESPONSE_QUEUE_H

#include <Wifi.h>

#define MAX_RESPONSES 8
#define GET_INDEX(i) ((_currentResponseIndex + i) % MAX_RESPONSES)


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
        if(_responses[GET_INDEX(i)] == response)
        {
          return true;
        }
      }

      return false;
    }    

    T DequeueNextResponse()
    {
      if(IsEmpty())
      {
        return {};
      }

      T response = _responses[_currentResponseIndex];

      _responseCount--;
      _currentResponseIndex = GET_INDEX(1);

      return response;
    }   

    void EnqueueResponse(T response)
    {
      if(_responseCount < MAX_RESPONSES)
      {
        _responses[GET_INDEX(_responseCount)] = response;
        _responseCount++;
      }
    }   

    T PeekNextResponse(int index = 0)
    {
      if(IsEmpty())
      {
        return {};
      }

      return _responses[GET_INDEX(index)];
    }    

    void Clear()
    { 
      _responseCount = 0;
    }    

    int GetResponseCount() { return _responseCount; }

  private:
    T* _responses;
    int _currentResponseIndex;
    int _responseCount;
    int _maxRespones;

};


#endif