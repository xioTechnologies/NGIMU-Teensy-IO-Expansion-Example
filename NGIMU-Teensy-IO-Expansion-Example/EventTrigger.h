/**
 * @file EventTrigger.h
 * @author Seb Madgwick
 * @brief Triggers events on each digital input rising and falling edge.
 */

#ifndef EVENT_TRIGGER_H
#define EVENT_TRIGGER_H

//------------------------------------------------------------------------------
// Includes

#include <stdbool.h>

//------------------------------------------------------------------------------
// Function prototypes

void EventTriggerDoTasks();
void EventTriggerAddEvent(void (*const function) (const bool pinState), const int pinNumber);

#endif

//------------------------------------------------------------------------------
// End of file
