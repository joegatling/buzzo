// #include <Arduino.h>

// #include "ResponseQueue.h"

// #define GET_INDEX(i) ((_currentResponseIndex + i) % MAX_RESPONSES)

// template <class T>
// ResponseQueue<T>::ResponseQueue(int maxResponses)
// {
//   _currentResponseIndex = 0;
//   _responseCount = 0;
//   _maxRespones = maxResponses;

//   _responses = new T[_maxRespones];
// }

// template <class T>
// ResponseQueue<T>::~ResponseQueue()
// {
//   delete[] _responses;
// }

// template <class T>
// bool ResponseQueue<T>::IsEmpty()
// {
//   return _responseCount == 0;
// }

// template <class T>
// bool ResponseQueue<T>::ContainsResponse(T response)
// {
//   for(int i = 0; i < _responseCount; i++)
//   {
//     if(_responses[GET_INDEX(i)] == response)
//     {
//       return true;
//     }
//   }

//   return false;
// }

// template <class T>
// T ResponseQueue<T>::DequeueNextResponse()
// {
//   if(IsEmpty())
//   {
//     return -1;
//   }

//   T response = _responses[_currentResponseIndex];

//   _responseCount--;
//   _currentResponseIndex = GET_INDEX(1);

//   return response;
// }

// template <class T>
// void ResponseQueue<T>::EnqueueResponse(T response)
// {
//   if(_responseCount < MAX_RESPONSES)
//   {
//     _responses[GET_INDEX(_responseCount)] = response;
//     _responseCount++;
//   }
// }

// template <class T>
// T ResponseQueue<T>::PeekNextResponse()
// {
//   if(IsEmpty())
//   {
//     return -1;
//   }

//   return _responses[_currentResponseIndex];
// }

// template <class T>
// void ResponseQueue<T>::Clear()
// { 
//   _responseCount = 0;
// }