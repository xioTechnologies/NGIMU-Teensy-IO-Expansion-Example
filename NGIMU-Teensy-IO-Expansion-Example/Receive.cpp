/**
 * @file Receive.cpp
 * @author Seb Madgwick
 * @brief Application tasks and functions for receiving messages.
 */

//------------------------------------------------------------------------------
// Includes

#include "Arduino.h"
#include "Osc99.h"
#include "Send.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
// Variable declarations

static OscSlipDecoder oscSlipDecoderUsb;
static OscSlipDecoder oscSlipDecoderSerial;

//------------------------------------------------------------------------------
// Function prototypes

static void ProcessPacket(OscPacket * const oscMessage);
static void ProcessMessage(const OscTimeTag * const oscTimeTag, OscMessage * const oscMessage);
static OscError ProcessAddress(OscMessage * const oscMessage);

//------------------------------------------------------------------------------
// Functions

/**
 * @brief Initialises module.  This function should be called once on system
 * start up.
 */
void ReceiveInitialise() {
    OscSlipDecoderInitialise(&oscSlipDecoderUsb);
    oscSlipDecoderUsb.processPacket = ProcessPacket;
    OscSlipDecoderInitialise(&oscSlipDecoderSerial);
    oscSlipDecoderSerial.processPacket = ProcessPacket;
}

/**
 * @brief Do tasks.  This function should be called repeatedly within the main
 * program loop.
 */
void ReceiveDoTasks() {

    // Teensy USB
    while (Serial.available() > 0) {
        OscSlipDecoderProcessByte(&oscSlipDecoderUsb, Serial.read());
    }

    // NGIMU auxiliary serial
    while (Serial1.available() > 0) {
        OscSlipDecoderProcessByte(&oscSlipDecoderSerial, Serial1.read());
    }
}

/**
 * @brief Callback function executed for each OSC packet received by a SLIP
 * decoder.
 * @param oscPacket Address of the decoded OSC packet.
 */
static void ProcessPacket(OscPacket * const oscPacket) {
    oscPacket->processMessage = &ProcessMessage;
    OscError oscError = OscPacketProcessMessages(oscPacket);
    if (oscError != OscErrorNone) {
        SendError(OscErrorGetMessage(oscError)); // send error message
    }
}

/**
 * @brief Callback function executed for each message found within received OSC
 * packet.
 * @param oscTimeTag OSC time tag associated with message.
 * @param oscMessage Address of OSC message.
 */
static void ProcessMessage(const OscTimeTag * const oscTimeTag, OscMessage * const oscMessage) {
    const OscError oscError = ProcessAddress(oscMessage);
    if (oscError != OscErrorNone) {
        SendError(OscErrorGetMessage(oscError)); // send error message
    }
}

/**
 * @brief Process OSC message according to OSC address pattern.
 * @param oscMessage Address of OSC message.
 * @return Error code (0 if successful).
 */
static OscError ProcessAddress(OscMessage * const oscMessage) {

    // Reject non-literal OSC address patterns
    if (!OscAddressIsLiteral(oscMessage->oscAddressPattern)) {
        SendError("OSC address pattern cannot contain special characters: '?', '*', '[]', or '{}'.");
        return OscErrorNone;
    }

    // Teensy LED
    if (OscAddressMatch(oscMessage->oscAddressPattern, "/teensy/led")) {

        // Get LED state
        bool ledState;
        OscError oscError = OscMessageGetArgumentAsBool(oscMessage, &ledState);
        if (oscError != OscErrorNone) {
            return oscError;
        }

        // Write to pin
        pinMode(13, OUTPUT); // Teensy LED connected to pin 13
        digitalWrite(13, ledState);

        return OscErrorNone;
    }

    // RGB LED
    if (OscAddressMatch(oscMessage->oscAddressPattern, "/teensy/rgb")) {

        // Get red value
        int32_t red;
        OscError oscError = OscMessageGetArgumentAsInt32(oscMessage, &red);
        if (oscError != OscErrorNone) {
            return oscError;
        }

        // Get green value
        int32_t green;
        oscError = OscMessageGetArgumentAsInt32(oscMessage, &green);
        if (oscError != OscErrorNone) {
            return oscError;
        }

		// Get blue value
        int32_t blue;
        oscError = OscMessageGetArgumentAsInt32(oscMessage, &blue);
        if (oscError != OscErrorNone) {
            return oscError;
        }

        // Write to pins
        analogWrite(21, red); // Red LED cathode is pin 21
		analogWrite(22, green); // Green LED cathode is pin 22
		analogWrite(23, blue); // Blue LED cathode is pin 23

        return OscErrorNone;
    }

    // OSC address not recognised
    char string[256];
    snprintf(string, sizeof (string), "OSC address pattern not recognised: %s", oscMessage->oscAddressPattern);
    SendError(string);

    return OscErrorNone;
}

//------------------------------------------------------------------------------
// End of file
