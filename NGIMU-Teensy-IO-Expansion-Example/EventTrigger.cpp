/**
 * @file EventTrigger.cpp
 * @author Seb Madgwick
 * @brief Triggers events on each digital input rising and falling edge.
 */

//------------------------------------------------------------------------------
// Includes

#include "Arduino.h"

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Maximum number of trigger events.
 */
#define MAX_NUMBER_OF_TRIGGER_EVENTS 32

/**
 * @brief Structure of trigger event variables.
 */
typedef struct {
    void (*function)(const bool pinState);
    int pinNumber;
    bool prevousPinState;
} TriggerEvent;

//------------------------------------------------------------------------------
// Variable declarations

/**
 * @brief Array of trigger events.
 */
static TriggerEvent triggerEvents[MAX_NUMBER_OF_TRIGGER_EVENTS];

/**
 * @brief Current number of trigger events.  Will increment each time a new
 * event is added.
 */
static int numberOfTriggerEvents = 0;

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Do tasks.  This function should be called repeatedly within the main
 * program loop.
 */
void EventTriggerDoTasks() {
    int i;
    for (i = 0; i < numberOfTriggerEvents; i++) {
		const bool pinState = digitalRead(triggerEvents[i].pinNumber) == 1;
        if (pinState == false) {
            if (triggerEvents[i].prevousPinState == true) {
                triggerEvents[i].function(pinState);
            }
        }
		else {
            if (triggerEvents[i].prevousPinState == false) {
                triggerEvents[i].function(pinState);
            }
		}
		triggerEvents[i].prevousPinState = pinState;
    }
}

/**
 * @brief Adds new event to be triggered for each input pin rising and falling
 * edge.  This function will also configure pin as an input with internal pull-
 * up.
 * @param function Address of function to be triggered by input falling edge.
 * @param pinNumber Pin number to be configured as a digital input.
 */
void EventTriggerAddEvent(void (*const function) (const bool pinState), const int pinNumber){
    if (numberOfTriggerEvents >= MAX_NUMBER_OF_TRIGGER_EVENTS) {
        return; // error: cannot exceed maximum number of trigger events
    }
    pinMode(pinNumber, INPUT_PULLUP);
    triggerEvents[numberOfTriggerEvents].function = function;
    triggerEvents[numberOfTriggerEvents].pinNumber = pinNumber;
    numberOfTriggerEvents++;
}

//------------------------------------------------------------------------------
// End of file
