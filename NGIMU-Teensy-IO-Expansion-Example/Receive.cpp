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
static OscError ProcesAddress(OscMessage * const oscMessage);
static OscError GetArgumentAsInt32(OscMessage * const oscMessage, int32_t * const value);
static OscError GetArgumentAsFloat32(OscMessage * const oscMessage, float * const value);
static OscError GetArgumentAsBool(OscMessage * const oscMessage, bool * const value);
static OscError GetArgumentAsString(OscMessage * const oscMessage, char* const destination, const size_t destinationSize);
static OscError GetArgumentAsBlob(OscMessage * const oscMessage, size_t * const blobSize, char* const destination, const size_t destinationSize);

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
    const OscError oscError = ProcesAddress(oscMessage);
    if (oscError != OscErrorNone) {
        SendError(OscErrorGetMessage(oscError)); // send error message
    }
}

/**
 * @brief Process OSC message according to OSC address pattern.
 * @param oscMessage Address of OSC message.
 * @return Error code (0 if successful).
 */
static OscError ProcesAddress(OscMessage * const oscMessage) {

    // Reject non-literal OSC address patterns
    if (!OscAddressIsLiteral(oscMessage->oscAddressPattern)) {
        SendError("OSC address pattern cannot contain special characters: '?', '*', '[]', or '{}'.");
        return OscErrorNone;
    }

    // Teensy LED
    if (OscAddressMatch(oscMessage->oscAddressPattern, "/teensy/led")) {

        // Get LED state
        bool ledState;
        OscError oscError = GetArgumentAsBool(oscMessage, &ledState);
        if (oscError != OscErrorNone) {
            return oscError;
        }

        // Write to pin
        pinMode(13, OUTPUT);
        digitalWrite(13, ledState);

        return OscErrorNone;
    }

    // Tone on piezo transducer
    if (OscAddressMatch(oscMessage->oscAddressPattern, "/teensy/tone")) {

        // Get frequency
        int32_t frequency;
        OscError oscError = GetArgumentAsInt32(oscMessage, &frequency);
        if (oscError != OscErrorNone) {
            return oscError;
        }

        // Write to pin
        if (frequency == 0) {
            noTone(9);
        } else {
            tone(9, frequency);
        }

        return OscErrorNone;
    }

    // OSC address not recognised
    char string[256];
    snprintf(string, sizeof (string), "OSC address pattern not recognised: %s", oscMessage->oscAddressPattern);
    SendError(string);

    return OscErrorNone;
}

//------------------------------------------------------------------------------
// Functions -  Argument type overloading

/**
 * @brief Interprets the next argument in OSC message as an int32.
 *
 * This function allows an int32, float32 or bool to be provided when an int32
 * argument is required.  This provides greater compatibility when communicating
 * with OSC libraries that do not support all argument types.  This also offers
 * a convenience to the user whom may reasonably expect to interchange integers
 * floats and booleans.  For example, to expect an integer value of zero to be
 * interpreted as boolean value of false.
 *
 * @param oscMessage Address of OSC message.
 * @param int32 Address value will be written to.
 * @return Error code (0 if successful).
 */
static OscError GetArgumentAsInt32(OscMessage * const oscMessage, int32_t * const int32) {
    if (!OscMessageIsArgumentAvailable(oscMessage)) {
        return OscErrorNoArgumentsAvailable;
    }
    switch (OscMessageGetArgumentType(oscMessage)) {
        case OscTypeTagInt32:
        {
            return OscMessageGetInt32(oscMessage, int32);
        }
        case OscTypeTagFloat32:
        {
            float float32;
            const OscError oscError = OscMessageGetFloat32(oscMessage, &float32);
            *int32 = (int32_t) float32;
            return oscError;
        }
        case OscTypeTagTrue:
        {
            *int32 = 1;
            break;
        }
        case OscTypeTagFalse:
        {
            *int32 = 0;
            break;
        }
        default:
            return OscErrorUnexpectedArgumentType; // error: unexpected argument type
    }
    return OscErrorNone;
}

/**
 * @brief Interprets the next argument in OSC message as a float32.
 *
 * This function allows an int32, float32 or bool to be provided when a float32
 * argument is required.  This provides greater compatibility when communicating
 * with OSC libraries that do not support all argument types.  This also offers
 * a convenience to the user whom may reasonably expect to interchange integers
 * floats and booleans.  For example, to expect an integer value of zero to be
 * interpreted as boolean value of false.
 *
 * @param oscMessage Address of OSC message.
 * @param float32 Address value will be written to.
 * @return Error code (0 if successful).
 */
static OscError GetArgumentAsFloat32(OscMessage * const oscMessage, float* const float32) {
    if (!OscMessageIsArgumentAvailable(oscMessage)) {
        return OscErrorNoArgumentsAvailable;
    }
    switch (OscMessageGetArgumentType(oscMessage)) {
        case OscTypeTagInt32:
        {
            int32_t int32;
            const OscError oscError = OscMessageGetInt32(oscMessage, &int32);
            *float32 = (float) int32;
            return oscError;
        }
        case OscTypeTagFloat32:
        {
            return OscMessageGetFloat32(oscMessage, float32);
        }
        case OscTypeTagTrue:
        {
            *float32 = 1.0f;
            break;
        }
        case OscTypeTagFalse:
        {
            *float32 = 0.0f;
            break;
        }
        default:
            return OscErrorUnexpectedArgumentType; // error: unexpected argument type
    }
    return OscErrorNone;
}

/**
 * @brief Interprets the next argument in OSC message as a bool.
 *
 * This function allows an int32, float32 or bool to be provided when a bool
 * argument is required.  This provides greater compatibility when communicating
 * with OSC libraries that do not support all argument types.  This also offers
 * a convenience to the user whom may reasonably expect to interchange integers
 * floats and booleans.  For example, to expect an integer value of zero to be
 * interpreted as boolean value of false.
 *
 * @param oscMessage Address of OSC message.
 * @param boolean Address value will be written to.
 * @return Error code (0 if successful).
 */
static OscError GetArgumentAsBool(OscMessage * const oscMessage, bool * const boolean) {
    if (!OscMessageIsArgumentAvailable(oscMessage)) {
        return OscErrorNoArgumentsAvailable;
    }
    switch (OscMessageGetArgumentType(oscMessage)) {
        case OscTypeTagInt32:
        {
            int32_t int32;
            const OscError oscError = OscMessageGetInt32(oscMessage, &int32);
            *boolean = int32 != 0;
            return oscError;
        }
        case OscTypeTagFloat32:
        {
            float float32;
            const OscError oscError = OscMessageGetFloat32(oscMessage, &float32);
            *boolean = float32 != 0.0f;
            return oscError;
        }
        case OscTypeTagTrue:
        {
            *boolean = true;
            break;
        }
        case OscTypeTagFalse:
        {
            *boolean = false;
            break;
        }
        default:
            return OscErrorUnexpectedArgumentType; // error: unexpected argument type
    }
    return OscErrorNone;
}

/**
 * @brief Interprets the next argument in OSC message as a string.
 *
 * This function allows a string or blob to be provided when a string argument
 * is required.  This provides greater compatibility when communicating with OSC
 * libraries that do not support all argument types.  This also offers a
 * convenience to the user whom may reasonably expect to interchange strings and
 * blobs.
 *
 * @param oscMessage Address of OSC message.
 * @param destination Destination address of string.
 * @param destinationSize Size of destination.
 * @return Error code (0 if successful).
 */
static OscError GetArgumentAsString(OscMessage * const oscMessage, char* const destination, const size_t destinationSize) {
    if (!OscMessageIsArgumentAvailable(oscMessage)) {
        return OscErrorNoArgumentsAvailable;
    }
    switch (OscMessageGetArgumentType(oscMessage)) {
        case OscTypeTagString:
        {
            return OscMessageGetString(oscMessage, destination, destinationSize);
        }
        case OscTypeTagBlob:
        {
            size_t blobSize;
            const OscError oscError = OscMessageGetBlob(oscMessage, &blobSize, destination, destinationSize);
            if (oscError != 0) {
                return oscError;
            }
            if (destination[blobSize - 1] != '\0') { // if blob not null terminated
                if (blobSize >= destinationSize) {
                    return OscErrorDestinationTooSmall; // error: destination too small
                }
                destination[blobSize] = '\0';
            }
            return OscErrorNone;
        }
        default:
            return OscErrorUnexpectedArgumentType; // error: unexpected argument type
    }
}

/**
 * @brief Interprets the next argument in OSC message as an OSC blob.
 *
 * This function allows a string or blob to be provided when a blob argument is
 * required.  This provides greater compatibility when communicating with OSC
 * libraries that do not support all argument types.  This also offers a
 * convenience to the user whom may reasonably expect to interchange strings and
 * blobs.
 *
 * @param oscMessage Address of OSC message.
 * @param blobSize Address of blob size.
 * @param destination Destination address of string.
 * @param destinationSize Size of destination.
 * @return Error code (0 if successful).
 */
static OscError GetArgumentAsBlob(OscMessage * const oscMessage, size_t * const blobSize, char* const destination, const size_t destinationSize) {
    if (!OscMessageIsArgumentAvailable(oscMessage)) {
        return OscErrorNoArgumentsAvailable;
    }
    switch (OscMessageGetArgumentType(oscMessage)) {
        case OscTypeTagString:
        {
            const OscError oscError = OscMessageGetString(oscMessage, destination, destinationSize);
            if (oscError != 0) {
                return oscError;
            }
            *blobSize = strlen(destination);
            return OscErrorNone;
        }
        case OscTypeTagBlob:
        {
            return OscMessageGetBlob(oscMessage, blobSize, destination, destinationSize);
        }
        default:
            return OscErrorUnexpectedArgumentType; // error: unexpected argument type
    }
}

//------------------------------------------------------------------------------
// End of file
