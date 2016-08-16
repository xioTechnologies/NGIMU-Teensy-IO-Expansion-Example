/**
 * @file Send.h
 * @author Seb Madgwick
 * @brief Application tasks and functions for sending messages.
 */

#ifndef SEND_H
#define SEND_H

//------------------------------------------------------------------------------
// Function prototypes

void SendInitialise();
void SendDoTasks();
void SendError(const char* const errorMessage);

#endif

//------------------------------------------------------------------------------
// End of file
