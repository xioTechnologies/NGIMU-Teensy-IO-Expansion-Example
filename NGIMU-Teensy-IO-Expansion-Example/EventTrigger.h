/**
 * @file EventTrigger.h
 * @author Seb Madgwick
 * @brief Triggers events on each digital input falling edge.
 */

#ifndef EVENT_TRIGGER_H
#define EVENT_TRIGGER_H

//------------------------------------------------------------------------------
// Function prototypes

void EventTriggerDoTasks();
void EventTriggerAddEvent(void (*const function) (), const int pin);

#endif

//------------------------------------------------------------------------------
// End of file
