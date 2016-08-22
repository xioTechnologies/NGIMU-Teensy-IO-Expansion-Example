/**
 * @file Send.cpp
 * @author Seb Madgwick
 * @brief Application tasks and functions for sending messages.
 */

//------------------------------------------------------------------------------
// Includes

#include "Arduino.h"
#include "EventScheduler.h"
#include "EventTrigger.h"
#include "Osc99.h"
#include <stdbool.h>

//------------------------------------------------------------------------------
// Function prototypes

static void SendJoystickXYMessage();
static void SendCounterMessage();
static void SendButtonAMessage();
static void SendButtonBMessage();
static void SendButtonCMessage();
void SendError(const char* const errorMessage);
static void SendOscContents(OscContents* oscContents);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises module.  This function should be called once on system
 * start up.
 */
void SendInitialise() {

    // Configure scheduled events
    EventSchedulerAddEvent(&SendJoystickXYMessage, 10.0f); // send joystick XY values at 10 Hz
    EventSchedulerAddEvent(&SendCounterMessage, 1.0f); // send counter at 1 Hz

    // Configure triggered events
    EventTriggerAddEvent(&SendButtonAMessage, 10); // send button A message with each falling edge of pin 10
    EventTriggerAddEvent(&SendButtonBMessage, 11); // send button B message with each falling edge of pin 11
    EventTriggerAddEvent(&SendButtonCMessage, 12); // send button C message with each falling edge of pin 12
}

/**
 * @brief Do tasks.  This function should be called repeatedly within the main
 * program loop.
 */
void SendDoTasks() {
    EventSchedulerDoTasks();
    EventTriggerDoTasks();
}

/**
 * @brief Sends joystick XY message containing analogue inputs measurements.
 */
static void SendJoystickXYMessage() {

    // Read analogue inputs
    int x = analogRead(A0);
    int y = analogRead(A1);

    // Remove offset
    static int xOffset = 0;
    static int yOffset = 0;
    static bool isFirstTime = true;
    if (isFirstTime == true) { // store offset on start up
        xOffset = x;
        yOffset = y;
        isFirstTime = false;
    }
    x -= xOffset;
    y -= yOffset;

    // Scale values to be between -1 and +1
    const static float scaleFactor = (2.0f / 8192.0f); // assumes 13-bit ADC resolution
    const float xScaled = (float) x * scaleFactor;
    const float yScaled = (float) y * scaleFactor;

    // Send message
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/joystick/xy");
    OscMessageAddFloat32(&oscMessage, xScaled);
    OscMessageAddFloat32(&oscMessage, yScaled);
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends counter message containing an integer that increments with each
 * send.
 */
static void SendCounterMessage() {
    static int counter = 0;
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/counter");
    OscMessageAddInt32(&oscMessage, counter++); // increment counter
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends button A message.
 */
static void SendButtonAMessage() {
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/button/a");
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends button B message.
 */
static void SendButtonBMessage() {
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/button/b");
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends button C message.
 */
static void SendButtonCMessage() {
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/button/c");
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends error message.
 * @param errorMessage Error message string.
 */
void SendError(const char* const errorMessage) {
    OscMessage oscMessage;
    OscMessageInitialise(&oscMessage, "/teensy/error");
    OscMessageAddString(&oscMessage, errorMessage);
    SendOscContents(&oscMessage);
}

/**
 * @brief Sends either an OSC message or OSC bundle through both serial
 * peripherals.
 * @param oscContents Address of either an OSC message or OSC bundle.
 */
static void SendOscContents(OscContents* oscContents) {

    // Create OSC packet from OSC message or bundle
    OscPacket oscPacket;
    if (OscPacketInitialiseFromContents(&oscPacket, oscContents)) {
        return; // error: unable to create an OSC packet from the OSC contents
    }

    // Encode SLIP packet
    char slipPacket[MAX_OSC_PACKET_SIZE];
    size_t slipPacketSize;
    if (OscSlipEncodePacket(&oscPacket, &slipPacketSize, slipPacket, sizeof (slipPacket))) {
        return; // error: the encoded SLIP packet is too long for the size of slipPacket
    }

    // Send SLIP packet
    Serial.write((uint8_t*) slipPacket, slipPacketSize); // Teensy USB
    Serial1.write((uint8_t*) slipPacket, slipPacketSize); // NGIMU auxiliary serial
}

//------------------------------------------------------------------------------
// End of file
