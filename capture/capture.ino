// ============================================================
//  IR Signal Capture Sketch
//  Hardware: Arduino UNO + IR receiver module (e.g. VS1838B)
//  Library:  IRremote >= 4.x  (install via Library Manager)
//
//  Wiring:
//    IR receiver OUT  -> Pin 2
//    IR receiver VCC  -> 5V
//    IR receiver GND  -> GND
//
//  Usage:
//    1. Upload this sketch.
//    2. Open Serial Monitor at 115200 baud.
//    3. Point your AC remote at the sensor and press the ON button.
//    4. Copy the printed "MIDEA BYTES" line into codeON[] in sender.ino.
//    5. Press the OFF button and copy that line into codeOFF[].
// ============================================================

// Must be defined BEFORE including IRremote — increases capture buffer
// so the full Midea 6-byte frame (sent twice) is not truncated.
#define RAW_BUFFER_LENGTH 750

#include <Arduino.h>
#define IR_RECEIVE_PIN 2
#include <IRremote.hpp>

// Decode raw timings into Midea bytes (LSB first, 500µs mark,
// 1600µs space = 1, ~550µs space = 0).
// Returns number of bytes decoded (up to maxBytes).
uint8_t decodeMideaBytes(IRRawDataType *raw, uint16_t rawlen,
                         byte *out, uint8_t maxBytes) {
    uint8_t byteCount = 0;
    uint8_t bitCount  = 0;
    byte    current   = 0;

    // raw[0] = leading gap — skip it.
    // Odd indices = marks, even indices = spaces (after gap skip).
    // We only care about spaces to determine bit value.
    for (uint16_t i = 2; i < rawlen && byteCount < maxBytes; i += 2) {
        uint32_t spaceUs = (uint32_t)raw[i] * MICROS_PER_TICK;

        bool bit = (spaceUs > 1000);   // >1000µs → 1, otherwise → 0
        if (bit) bitWrite(current, bitCount, 1);
        bitCount++;

        if (bitCount == 8) {
            out[byteCount++] = current;
            current  = 0;
            bitCount = 0;
        }
    }
    return byteCount;
}

void setup() {
    Serial.begin(115200);
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    Serial.println(F("Ready. Point remote at sensor and press a button."));
    Serial.println(F("-----------------------------------------------------"));
}

void loop() {
    if (!IrReceiver.decode()) {
        return;
    }

    IRRawDataType *raw    = IrReceiver.decodedIRData.rawDataPtr->rawbuf;
    uint16_t      rawlen  = IrReceiver.decodedIRData.rawDataPtr->rawlen;

    // ---- Protocol guess ----
    Serial.println(F("\n===== DECODED ====="));
    IrReceiver.printIRResultShort(&Serial);

    // ---- Midea byte decode ----
    byte decoded[12];   // up to 12 bytes (frame sent twice = 12 bytes)
    uint8_t n = decodeMideaBytes(raw, rawlen, decoded, 12);

    Serial.println(F("\n===== MIDEA BYTES (copy first 6 into sender.ino) ====="));
    Serial.print(F("Bytes decoded: ")); Serial.println(n);

    // Print first frame (6 bytes)
    if (n >= 6) {
        Serial.print(F("Frame 1: { "));
        for (uint8_t i = 0; i < 6; i++) {
            Serial.print(F("0x"));
            if (decoded[i] < 0x10) Serial.print(F("0"));
            Serial.print(decoded[i], HEX);
            if (i < 5) Serial.print(F(", "));
        }
        Serial.println(F(" }"));
    }

    // Print second frame if present (sanity check — should match frame 1)
    if (n >= 12) {
        Serial.print(F("Frame 2: { "));
        for (uint8_t i = 6; i < 12; i++) {
            Serial.print(F("0x"));
            if (decoded[i] < 0x10) Serial.print(F("0"));
            Serial.print(decoded[i], HEX);
            if (i < 11) Serial.print(F(", "));
        }
        Serial.println(F(" }"));
        Serial.println(F("(Frame 1 and Frame 2 should be identical)"));
    }

    // ---- Raw timings (for debugging) ----
    Serial.println(F("\n===== RAW TIMINGS (µs) ====="));
    Serial.print(F("Count: ")); Serial.println(rawlen - 1);
    for (uint16_t i = 1; i < rawlen; i++) {
        Serial.print((uint32_t)raw[i] * MICROS_PER_TICK);
        if (i < rawlen - 1) Serial.print(F(", "));
    }
    Serial.println();

    Serial.println(F("-----------------------------------------------------"));
    Serial.println(F("Ready for next button press..."));

    IrReceiver.resume();
}
