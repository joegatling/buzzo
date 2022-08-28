#include "SimpleButton.h"

#define DEBOUNCE_TIME   20
#define HOLD_TIME  1200

#define BUTTON_DOWN (_usePullDownLogic ? HIGH : LOW)
#define BUTTON_UP (_usePullDownLogic ? LOW : HIGH)

SimpleButton::SimpleButton(int pin)
{
  _usePullDownLogic = false;

  _pin = pin;
  pinMode(pin, INPUT_PULLUP);

  _lastPinReading = BUTTON_UP;
  _buttonState = BUTTON_UP;
}

SimpleButton::SimpleButton(int pin, bool usePullDownLogic)
{
  _usePullDownLogic = usePullDownLogic;

  _pin = pin;
  pinMode(pin, INPUT);

  _lastPinReading = BUTTON_UP;
  _buttonState = BUTTON_UP;
}

void SimpleButton::Update()
{
  int reading = digitalRead(_pin);

  if(reading != _lastPinReading)
  {
    _debounceTime = millis();
  }
  _lastPinReading = reading;

  if ((millis() - _debounceTime) > DEBOUNCE_TIME) 
  {
      // if the button state has changed:
      if (reading != _buttonState) 
      {
        _buttonState = reading;

        if(_buttonState == BUTTON_DOWN)
        {
          _pressedTime = _debounceTime;

          if(_beginPressFunction)
          {
            _beginPressFunction();
          }
        }
        else
        {
          if(!_didHoldAction)
          {
            // We didn't do a hold this press, so send a click callback
            if(_clickFunction)
            {
              _clickFunction();
            }
          }
          else
          {
            // If we did a hold action, then do nothing here.
            _didHoldAction = false;
          }
        }        
      }
      else
      {
        if(_buttonState == LOW)
        {
          // Check for hold (If there is a hold action)
          if(_holdFunction && !_didHoldAction)
          {
            if(millis() - _pressedTime > HOLD_TIME)
            {
              _holdFunction();
              _didHoldAction = true;
            }
          }
        }
      }
    }    
}

void SimpleButton::SetEndPressCallback(simpleButtonCallbackFunction function)
{
  _clickFunction = function;
}

void SimpleButton::SetHoldCallback(simpleButtonCallbackFunction function)
{
  _holdFunction = function;
}

void SimpleButton::SetBeginPressCallback(simpleButtonCallbackFunction function)
{
  _beginPressFunction = function;
}
