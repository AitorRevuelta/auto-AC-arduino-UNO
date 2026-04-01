# Auto AC Arduino UNO

Automatic air conditioning controller using an Arduino UNO. The system reads room temperature via a DHT11 sensor and sends IR commands to a Midea-compatible AC unit to keep the room at a target temperature.

The project is split into two independent sketches:

| Sketch | Folder | Purpose |
|--------|--------|---------|
| Capture | `capture/` | Receive and decode IR signals from the original remote |
| Sender  | `sender/`  | Monitor temperature and send ON/OFF commands automatically |

---

## Hardware Requirements

| Component | Notes |
|-----------|-------|
| Arduino UNO (or Nano) | Main controller |
| IR receiver module (VS1838B or similar, 940 nm) | For capture sketch |
| IR LED (940 nm) | For sender sketch |
| Resistor 100–220 Ω | In series with IR LED |
| 2N2222 transistor | Optional — increases IR range |
| DHT11 temperature sensor | For sender sketch |

---

## Wiring

### Capture sketch (`capture/`)

| IR Receiver Pin | Arduino |
|-----------------|---------|
| OUT (signal)    | Pin 2   |
| VCC             | 5V      |
| GND             | GND     |

### Sender sketch (`sender/`)

| Component        | Arduino |
|-----------------|---------|
| IR LED anode (via 100–220 Ω resistor) | Pin 9 |
| IR LED cathode  | GND     |
| DHT11 DATA      | Pin 4   |
| DHT11 VCC       | 5V      |
| DHT11 GND       | GND     |

> **Tip:** For longer IR range, wire the IR LED through a 2N2222 transistor with the base driven from pin 9 via a 1 kΩ resistor.

---

## Step 1 — Capture your remote's signals

Before deploying the sender you must capture the exact ON and OFF IR codes from your real remote:

1. Wire the IR receiver to pin 2 (see wiring table above).
2. Open `capture/capture.ino` in the Arduino IDE and upload it.
3. Open **Serial Monitor** at **115200 baud**.
4. Point the AC remote at the sensor and press the **ON** button.  
   The Serial Monitor will print a `uint16_t rawData[]` array — copy it.
5. Press the **OFF** button and copy that array too.
6. Paste the two arrays into `sender/sender.ino` as `AC_ON_RAW[]` and `AC_OFF_RAW[]`, and update the corresponding `AC_ON_RAW_LEN` / `AC_OFF_RAW_LEN` values.

---

## Step 2 — Configure the sender

Open `sender/sender.ino` and adjust the defines at the top:

```cpp
#define TARGET_TEMP   24.0f   // Desired room temperature (°C)
#define HYSTERESIS     1.0f   // Dead band (°C) — avoids rapid switching
#define CHECK_INTERVAL_MS  10000UL  // How often to read the sensor (ms)
```

| Condition | Action |
|-----------|--------|
| temp > TARGET_TEMP + HYSTERESIS | Send AC ON  |
| temp < TARGET_TEMP − HYSTERESIS | Send AC OFF |
| otherwise | No action |

State is tracked, so the same command is never sent twice in a row.

---

## Step 3 — Upload the sender

1. Wire the IR LED and DHT11 (see wiring table above).
2. Install the required libraries via **Arduino IDE → Library Manager**:
   - `IRremote` ≥ 4.x
   - `DHT sensor library` (Adafruit)
   - `Adafruit Unified Sensor` (required by DHT library)
3. Upload `sender/sender.ino`.
4. Open **Serial Monitor** at **115200 baud** to monitor temperature readings and commands sent.

---

## Protocol Reference (Midea IR)

| Field      | Timing |
|------------|--------|
| Header mark  | 4500 µs |
| Header space | 4400 µs |
| Bit mark     | 560 µs  |
| Bit 1 space  | 1600 µs |
| Bit 0 space  | 560 µs  |
| Stop bit     | 560 µs mark |
| Carrier      | 38 kHz  |
| Payload      | 48 bits (6 bytes), each byte followed by its bitwise inverse |

Bits are transmitted LSB-first.

---

## Dependencies

- [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) ≥ 4.x
- [DHT sensor library](https://github.com/adafruit/DHT-sensor-library) (Adafruit)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Unified_Sensor)
