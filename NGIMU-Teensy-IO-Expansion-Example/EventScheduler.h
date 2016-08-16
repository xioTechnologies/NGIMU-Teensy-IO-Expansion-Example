/**
 * @file EventScheduler.h
 * @author Seb Madgwick
 * @brief Schedules events to repeat at a fixed frequency.
 */

#ifndef EVENT_SCHEDULER_H
#define EVENT_SCHEDULER_H

//------------------------------------------------------------------------------
// Function prototypes

void EventSchedulerDoTasks();
void EventSchedulerAddEvent(void (*const function) (), const float repeatFrequency);

#endif

//------------------------------------------------------------------------------
// End of file
