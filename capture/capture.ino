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
//       Copy the printed raw timing array into sender.ino as AC_ON_RAW[].
//    4. Press the OFF button and copy that array as AC_OFF_RAW[].
// ============================================================

#include <Arduino.h>
#define IR_RECEIVE_PIN 2
#include <IRremote.hpp>

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

    // ---- Human-readable decoded result (protocol, address, command) ----
    Serial.println(F("\n===== DECODED ====="));
    IrReceiver.printIRResultShort(&Serial);

    // ---- Raw timing array, ready to paste into sender.ino ----
    Serial.println(F("\n===== RAW TIMINGS (copy into sender.ino) ====="));
    Serial.print(F("uint16_t rawData[] = {"));
    IRRawDataType *raw = IrReceiver.decodedIRData.rawDataPtr->rawbuf;
    uint16_t rawlen = IrReceiver.decodedIRData.rawDataPtr->rawlen;

    // rawbuf[0] is the leading space (gap before signal) — skip it;
    // values are in 50µs ticks, convert to microseconds
    for (uint16_t i = 1; i < rawlen; i++) {
        Serial.print((uint32_t)raw[i] * MICROS_PER_TICK);
        if (i < rawlen - 1) Serial.print(F(", "));
    }
    Serial.println(F("};"));
    Serial.print(F("uint16_t rawLen = "));
    Serial.print(rawlen - 1);
    Serial.println(F(";"));

    // ---- Byte-level dump (useful for Midea / NEC protocols) ----
    if (IrReceiver.decodedIRData.decodedRawData != 0) {
        Serial.println(F("\n===== DECODED BYTES ====="));
        uint32_t val = (uint32_t)IrReceiver.decodedIRData.decodedRawData;
        Serial.print(F("0x"));
        Serial.println(val, HEX);
    }

    Serial.println(F("-----------------------------------------------------"));
    Serial.println(F("Ready for next button press..."));

    IrReceiver.resume();
}
