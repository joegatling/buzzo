#include "BuzzoButtonState.h"

BuzzoButtonState::BuzzoButtonState(stateEnterFunction enterFunc, stateUpdateFunction updateFunc, stateExitFunction exitFunc)
{
  _enterFunction = enterFunc;
  _updateFunction = updateFunc;
  _exitFunction = exitFunc;
}

void BuzzoButtonState::Enter(BuzzoButton* button)
{
  if(_enterFunction != 0)
  {
    _enterFunction(button);
  }
}

void BuzzoButtonState::Update(BuzzoButton* button)
{
  if(_updateFunction != 0)
  {
    _updateFunction(button);
  }
}

void BuzzoButtonState::Exit(BuzzoButton* button)
{
  if(_exitFunction != 0)
  {
    _exitFunction(button);
  }
}