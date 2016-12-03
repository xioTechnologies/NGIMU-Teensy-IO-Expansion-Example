/**
 * @file EventScheduler.cpp
 * @author Seb Madgwick
 * @brief Schedules events to repeat at a fixed frequency.
 */

//------------------------------------------------------------------------------
// Includes

#include "Arduino.h"

//------------------------------------------------------------------------------
// Definitions

/**
 * @brief Maximum number of scheduled events.
 */
#define MAX_NUMBER_OF_SCHEDULED_EVENTS 32

/**
 * @brief Structure of scheduled event variables.
 */
typedef struct {
    void (*function)();
    unsigned long repeatInterval; // in microseconds
    unsigned long previousMicros;
} ScheduledEvent;

//------------------------------------------------------------------------------
// Variable declarations

/**
 * @brief Array of scheduled events.
 */
static ScheduledEvent scheduledEvents[MAX_NUMBER_OF_SCHEDULED_EVENTS];

/**
 * @brief Current number of scheduled events.  Will increment each time a new
 * event is scheduled.
 */
static int numberOfSchduledEvents = 0;

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Do tasks.  This function should be called repeatedly within the main
 * program loop.
 */
void EventSchedulerDoTasks() {
    unsigned long currentMicros = micros();
    int i;
    for (i = 0; i < numberOfSchduledEvents; i++) {
        if ((currentMicros - scheduledEvents[i].previousMicros) >= scheduledEvents[i].repeatInterval) {
            scheduledEvents[i].function();
            scheduledEvents[i].previousMicros = currentMicros;
        }
    }
}

/**
 * @brief Adds new event to be repeated at fixed frequency.
 * @param function Address of function to be called at fixed frequency.
 * @param repeatFrequency Repeat frequency in Hz.
 */
void EventSchedulerAddEvent(void (*const function) (), const float repeatFrequency) {
    if (numberOfSchduledEvents >= MAX_NUMBER_OF_SCHEDULED_EVENTS) {
        return; // error: cannot exceed maximum number of scheduled events
    }
    scheduledEvents[numberOfSchduledEvents].function = function;
    scheduledEvents[numberOfSchduledEvents].repeatInterval = (unsigned long) (1000000.0f / repeatFrequency); // convert to microseconds
    numberOfSchduledEvents++;
}

//------------------------------------------------------------------------------
// End of file
