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
//    Use capture.ino to capture your remote's ON and OFF signals,
//    then paste the printed rawData arrays below.
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
//  RAW IR TIMINGS — paste values from capture.ino here
//
//  The arrays below contain the timings (in microseconds) of
//  alternating marks and spaces captured from the real remote.
//  First value = first mark after the header gap.
//
//  IMPORTANT: rawLen must equal the number of elements in the array.
// ============================================================

// --- AC ON command (captured from remote) ---
// TODO: replace with values printed by capture.ino when you press ON
uint16_t AC_ON_RAW[] = {
    4500, 4400,                                  // Header: 4.5ms mark, 4.4ms space
    560, 1600, 560, 560,  560, 560,  560, 1600,  // 0xB2 LSB-first: 0,1,0,0,1,1,0,1  wait — Midea sends LSB first
    560, 560,  560, 1600, 560, 1600, 560, 560,   // 0x4D
    560, 560,  560, 560,  560, 560,  560, 560,   // 0x1F low nibble placeholder
    560, 1600, 560, 1600, 560, 1600, 560, 1600,  // 0x1F high nibble placeholder
    560, 1600, 560, 1600, 560, 1600, 560, 1600,  // 0xE0
    560, 560,  560, 560,  560, 560,  560, 560,   // 0xE0
    560, 560,  560, 560,  560, 560,  560, 1600,  // 0x20
    560, 1600, 560, 1600, 560, 1600, 560, 560,   // 0xDF
    560, 1600, 560, 1600, 560, 1600, 560, 1600,  // 0xDF
    560, 560,  560, 560,  560, 560,  560, 560,   // 0xDF
    560                                           // Stop bit
};
uint16_t AC_ON_RAW_LEN = sizeof(AC_ON_RAW) / sizeof(AC_ON_RAW[0]);

// --- AC OFF command (captured from remote) ---
// TODO: replace with values printed by capture.ino when you press OFF
uint16_t AC_OFF_RAW[] = {
    4500, 4400,
    560, 1600, 560, 560,  560, 560,  560, 1600,  // 0xB2
    560, 560,  560, 1600, 560, 1600, 560, 560,   // 0x4D
    560, 560,  560, 560,  560, 560,  560, 560,   // byte 2 — OFF payload (fill from capture)
    560, 1600, 560, 1600, 560, 1600, 560, 1600,
    560, 1600, 560, 1600, 560, 1600, 560, 1600,  // byte 3 (inverse of byte 2)
    560, 560,  560, 560,  560, 560,  560, 560,
    560, 560,  560, 560,  560, 560,  560, 560,   // byte 4 — OFF payload (fill from capture)
    560, 1600, 560, 1600, 560, 1600, 560, 1600,
    560, 1600, 560, 1600, 560, 1600, 560, 1600,  // byte 5 (inverse of byte 4)
    560, 560,  560, 560,  560, 560,  560, 560,
    560, 560,  560, 560,  560, 560,  560, 560,
    560
};
uint16_t AC_OFF_RAW_LEN = sizeof(AC_OFF_RAW) / sizeof(AC_OFF_RAW[0]);

// ============================================================
//  Internal state
// ============================================================
bool acIsOn = false;           // tracks current AC state
unsigned long lastCheck = 0;

// ---- Send an IR burst from a raw timing array ----
void sendRaw(uint16_t *timings, uint16_t len) {
    IrSender.sendRaw(timings, len, 38); // 38 kHz carrier
}

void setup() {
    Serial.begin(115200);
    IrSender.begin(IR_SEND_PIN);
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
        sendRaw(AC_ON_RAW, AC_ON_RAW_LEN);
        acIsOn = true;

    } else if (acIsOn && temp < TARGET_TEMP - HYSTERESIS) {
        Serial.println(F("-> Cool enough. Sending AC OFF..."));
        sendRaw(AC_OFF_RAW, AC_OFF_RAW_LEN);
        acIsOn = false;

    } else {
        Serial.println(F("-> No action needed."));
    }
}
