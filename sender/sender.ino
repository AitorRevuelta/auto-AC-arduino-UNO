// ============================================================
//  Auto AC Sender Sketch
//  Hardware: Arduino UNO + IR LED + DHT11 temperature sensor
//  Libraries:
//    - IRremote >= 4.x        (install via Library Manager)
//    - DHT sensor library     (by Adafruit, via Library Manager)
//    - Adafruit Unified Sensor (dependency of DHT library)
//
//  Wiring:
//    IR LED anode    -> Pin 9 (via 100-220Ω resistor)
//    IR LED cathode  -> GND
//    (Optional) 2N2222 transistor on pin 9 for longer range
//
//    DHT11 DATA      -> Pin 4
//    DHT11 VCC       -> 5V
//    DHT11 GND       -> GND
//
//  Logic:
//    - If temperature rises above (TARGET_TEMP + HYSTERESIS) -> send AC ON
//    - If temperature drops below (TARGET_TEMP - HYSTERESIS) -> send AC OFF
//    - State is tracked so the same command is never sent twice in a row.
//
//  HOW TO GET YOUR RAW CODES:
//    Use capture.ino (with RAW_BUFFER_LENGTH 750) to capture your remote's
//    ON and OFF signals. Print the 6 bytes for each and XOR them byte-by-byte
//    to find the power bit. Paste the values into codeON and codeOFF below.
// ============================================================

#include <Arduino.h>
#include <DHT.h>

#define IR_SEND_PIN   9
#include <IRremote.hpp>

// ---- DHT sensor config ----
#define DHT_PIN       4
#define DHT_TYPE      DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ---- Temperature thresholds (°C) ----
#define TARGET_TEMP   24.0f   // Desired room temperature
#define HYSTERESIS     1.0f   // Dead band to avoid rapid switching
// AC turns ON  when temp > TARGET_TEMP + HYSTERESIS  (i.e. > 25°C)
// AC turns OFF when temp < TARGET_TEMP - HYSTERESIS  (i.e. < 23°C)

// ---- How often to check temperature ----
#define CHECK_INTERVAL_MS  10000UL   // every 10 seconds

// ============================================================
//  MIDEA IR PAYLOADS — paste 6-byte captures from capture.ino
//
//  Midea protocol: LSB first, bytes come in complement pairs
//  (byte N and byte N+1 are bitwise inverses of each other).
//
//  To find the power bit: XOR codeON and codeOFF byte-by-byte.
//  The byte(s) that differ contain the power bit.
// ============================================================

// ON packet  (power bit = 1)
byte codeON[6]  = { 0x4D, 0xB2, 0xFD, 0x02, 0x00, 0xFF }; // TODO: fill after full capture

// OFF packet (power bit = 0)
byte codeOFF[6] = { 0x4D, 0xB2, 0xFD, 0x02, 0x00, 0xFF }; // TODO: fill after full capture

// ============================================================
//  IR sending — Midea protocol
// ============================================================

void sendMideaByte(byte b) {
    for (int i = 0; i < 8; i++) {        // LSB first
        IrSender.mark(500);
        IrSender.space(bitRead(b, i) ? 1600 : 550);
    }
}

void sendMideaPacket(byte* code) {
    // Header
    IrSender.mark(4350);
    IrSender.space(4400);

    // 6 data bytes
    for (int i = 0; i < 6; i++) {
        sendMideaByte(code[i]);
    }

    // Stop bit
    IrSender.mark(500);
    IrSender.space(5000);
}

void sendAC(byte* code) {
    // Midea sends the frame twice with ~9ms gap
    sendMideaPacket(code);
    delay(9);
    sendMideaPacket(code);
}

// ============================================================
//  Internal state
// ============================================================
bool acIsOn = false;
unsigned long lastCheck = 0;

void setup() {
    Serial.begin(115200);
    IrSender.begin(IR_SEND_PIN);
    IrSender.enableIROut(38);   // 38 kHz carrier — set once
    dht.begin();

    Serial.println(F("Auto AC controller starting..."));
    Serial.print(F("Target temp: ")); Serial.print(TARGET_TEMP); Serial.println(F(" C"));
    Serial.print(F("Hysteresis:  ")); Serial.print(HYSTERESIS);   Serial.println(F(" C"));
}

void loop() {
    unsigned long now = millis();
    if (now - lastCheck < CHECK_INTERVAL_MS) {
        return;
    }
    lastCheck = now;

    float temp = dht.readTemperature();

    if (isnan(temp)) {
        Serial.println(F("ERROR: Failed to read from DHT sensor. Check wiring."));
        return;
    }

    Serial.print(F("Temperature: "));
    Serial.print(temp, 1);
    Serial.print(F(" C  |  AC is "));
    Serial.println(acIsOn ? F("ON") : F("OFF"));

    if (!acIsOn && temp > TARGET_TEMP + HYSTERESIS) {
        Serial.println(F("-> Too hot. Sending AC ON..."));
        sendAC(codeON);
        acIsOn = true;

    } else if (acIsOn && temp < TARGET_TEMP - HYSTERESIS) {
        Serial.println(F("-> Cool enough. Sending AC OFF..."));
        sendAC(codeOFF);
        acIsOn = false;

    } else {
        Serial.println(F("-> No action needed."));
    }
}
