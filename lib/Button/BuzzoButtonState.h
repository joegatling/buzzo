#ifndef STATE_H
#define STATE_H

#include "BuzzoButton.h"

class BuzzoButton;

typedef void (*stateEnterFunction)(BuzzoButton* button);
typedef void (*stateUpdateFunction)(BuzzoButton* button);
typedef void (*stateExitFunction)(BuzzoButton* button);

class BuzzoButtonState 
{
  public:
    BuzzoButtonState(stateEnterFunction enterFunc, stateUpdateFunction updateFunc, stateExitFunction exitFunc);

    void Enter(BuzzoButton* button);
    void Update(BuzzoButton* button);
    void Exit(BuzzoButton* button);

  private:

    stateEnterFunction _enterFunction = 0;
    stateUpdateFunction _updateFunction = 0;
    stateExitFunction _exitFunction = 0;
};



#endif