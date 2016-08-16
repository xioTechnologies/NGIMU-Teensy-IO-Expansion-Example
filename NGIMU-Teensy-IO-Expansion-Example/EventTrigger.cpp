/**
 * @file EventTrigger.cpp
 * @author Seb Madgwick
 * @brief Triggers events on each digital input falling edge.
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
    void (*function)();
    int pinNumber;
    unsigned long previousMicros;
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
    unsigned long currentMicros = micros();
    int i;
    for (i = 0; i < numberOfTriggerEvents; i++) {
        if (digitalRead(triggerEvents[i].pinNumber) == 0) {
            if (currentMicros >= (triggerEvents[i].previousMicros + 10000)) { // button must be released for 10 ms before being pressed again
                triggerEvents[i].function();
            }
            triggerEvents[i].previousMicros = currentMicros;
        }
    }
}

/**
 * @brief Adds new event to be triggered for each input pin falling edge.  This
 * function will also configure pin as an input with internal pull-up.
 * @param function Address of function to be triggered by input falling edge.
 * @param pinNumber Pin number to be configured as a digital input.
 */
void EventTriggerAddEvent(void (*const function) (), const int pinNumber) {
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
