Arduino IR Remote Control (Midea Protocol)
This project implements a custom infrared (IR) transmitter using an Arduino to control devices that use the Midea protocol (commonly found in AC units from brands like Midea, Comfee, Carrier, and others).
Unlike standard implementations, this code uses a "Forced Manual Mode" to ensure precise timing for pulses (mark) and spaces (space), bypassing some of the library's high-level abstractions for better compatibility.

Features

    Manual Timing Control: Explicitly defines the duration of the Header and individual bits.
    48-bit Protocol: Sends 6-byte bursts following the standard Midea structure.
    Configurable Output: Optimized for digital pin 9 (PWM).
    Serial Debugging: Real-time feedback via the Serial Monitor at 115200 baud.

Hardware Requirements

    Arduino Board (Uno, Nano, or compatible).
    Infrared LED (940nm).
    Resistor (appropriate for your LED, usually 100Ω to 220Ω).
    Transistor (Optional but Recommended): A 2N2222 or similar to increase the IR signal range.

Wiring

IR LED Anode (+)	Pin 9 (via resistor)
IR LED Cathode (-)	GND

Dependencies

The code requires the IRremote library. You can install it via the Arduino IDE Library Manager:
    
    Name: IRremote
    Recommended Version: 4.x or higher.

Message Structure (Payload)

The code sends a 6-byte burst. The current example sends:
0xB2, 0x4D, 0x1F, 0xE0, 0x20, 0xDF.

To change the command (e.g., adjust temperature, mode, or power), you must modify the values inside the loop() function under the DATA section.

Protocol Specifications:

    Header: 4.5ms pulse followed by a 4.4ms space.
    Bit Logic: * Bit 1: 560µs pulse + 1600µs space.
        Bit 0: 560µs pulse + 560µs space.
    Stop Bit: A final 560µs pulse.

Setup and Installation

    Copy the code into your Arduino IDE.
    Install the IRremote library.
    Connect your Arduino and select the correct Port.
    Click Upload.
    Open the Serial Monitor (set to 115200 baud) to confirm that the bursts are firing every 5 seconds.
