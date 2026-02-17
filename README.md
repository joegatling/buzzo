# Buzzo

![Buzzo Hero Image](/img/Buzzo.png)
Buzzo is a DIY open source set of game show buttons.

# Making A Buzzo Set
For instructions on how to build your own Buzzo can be found [here](/docs/Button_Assembly.md).

# How to Use
Buzzo is a set of buttons for running a fast paced answering game. Each contestant can hit their button to "buzz in". The system will queue up buttons in the order in which they buzzed. Each button is then given 30 seconds to provide an answer. With a set of Buzzo buttons, a standard game of _Trivial Pursuit_ can be turned into an exciting gameshow. 

Buzzo uses a wireless technology called ESP-Now. This allows them to communicate without the need for an existing WiFi network. However, only one Buzzo set can be operating in an area at a time, as all devices will try to communicate to any available controlelr that they can find.

## Buzzo Button

### Power
- Press the button on the Buzzo to turn it on. 
- Press and hold to turn it off.

A Buzzo Button will switch off automatically if left idle for a few minutes.

### Answering
To answer a question, press the button.  This will add the button into a queue, and each button will have 30 seconds to answer before moving to the next button in the queue.
If the timer expires, the device turns **red** and the button cannot be buzzed until the controller resets the round.

When it is a particular Buzzo's turn to answer, the button will light up yellow. As the timer expires, wedges of the button will go dark. Once they are all dark, the turn is over.

Buzzo devices that are in the answer queue will show a blue light. The number of bright wedges indicates device's place in the queue (eg, one bright wedge means 1st in the queue, two bright wedges means 2nd, etc...).

### Scoring
Each button will automatically keep track of correct answers, and display them as wedges on the button. Once a device has accumulated 6 correct answers, it has won.

## Buzzo Controller
The controller has four buttons:

`↺`/`⏻` - Power and Reset  
`⛌` - Incorrect  
`✓` - Correct  
`⃦`- Pause  

### Power
- Press this to turn on the device
- If the device is already powered on, press this to reset current round. This cleares

The Buzzo Controller will switch of automatically afer 15 minutes if there are devices connected, or after 2 minutes if nothing is connected.  
**WARNING: The Buzzo Controller is responsible for keeping track of scores. If it powers off, all devices will reset their score.**

### Judging Questions
If a Buzzo Button is answering, press the button marked `⛌` to judge the answer **incorrect**. Use `✓` to mark the answer as **correct**.
If a previously judged answer needs to be changed, press and hold either `⛌` or `✓`. This will retroactively change the last response.

### Pausing the Countdown
If for any reason the answering countdown needs to be paused (eg, if there is a dispute over a response, or an interruption to play), then hold the pause button. For as long as the button is held, the timer will be paused.

### Reseting
Press the `↺` button to reset the round. This will clear the answer queue, and reset all buttons so that they are ready for the next question.  
After resetting a round, the `↺` button can be pressed and held in order to reset the entire game. This will reset all the scores on all buttons.
Finally, after resetting around the `↺` button can be pressed and held once more to turn off the controller and all devices.


