/**
 * @file NGIMU-Teensy-IO-Expansion-Example.ino
 * @author Seb Madgwick
 * @brief Example for interfacing a Teensy with the NGIMU's auxiliary serial
 * interface with OSC passthrough mode enabled.
 *
 * Device:
 * Teensy 3.2
 *
 * IDE/compiler:
 * Arduino 1.6.7 and Teensy Loader 1.27
 *
 * The OSC99 source files (i.e. the "Osc99" directory) must be added to the
 * Arduino libraries folder.  See: https://www.arduino.cc/en/guide/libraries
 */

//------------------------------------------------------------------------------
// Includes

#include "Receive.h"
#include "Send.h"

//------------------------------------------------------------------------------
// Functions

void setup() {

    // Initialise Teensy peripherals
    Serial.begin(115200); // Teensy USB (baud rate irrelevant)
    Serial1.begin(115200); // NGIMU auxiliary serial (baud rate must match NGIMU settings)
    analogReadRes(13); // use 13-bit ADC resolution

    // Initialise application modules
    ReceiveInitialise();
    SendInitialise();
}

void loop() {
    ReceiveDoTasks();
    SendDoTasks();
}

//------------------------------------------------------------------------------
// End of file
