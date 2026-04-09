// ============================================================
//  Test Sketch — sends AC ON signal every 5 seconds
//  Hardware: Arduino UNO + IR LED on Pin 9
// ============================================================

#include <Arduino.h>

#define IR_SEND_PIN 9
#include <IRremote.hpp>

byte codeON[6] = { 0x4D, 0x4D, 0x1F, 0xE0, 0x40, 0xBF };

void sendMideaByte(byte b) {
    for (int i = 0; i < 8; i++) {   // LSB first
        IrSender.mark(500);
        IrSender.space(bitRead(b, i) ? 1600 : 550);
    }
}

void sendMideaPacket(byte* code) {
    IrSender.mark(4400);
    IrSender.space(4400);
    for (int i = 0; i < 6; i++) {
        sendMideaByte(code[i]);
    }
    IrSender.mark(500);
    IrSender.space(5200);
}

void setup() {
    Serial.begin(115200);
    IrSender.begin(IR_SEND_PIN);
    IrSender.enableIROut(38);
    Serial.println(F("Test sender ready."));
}

void loop() {
    Serial.println(F("Sending AC ON..."));
    sendMideaPacket(codeON);
    delay(9);
    sendMideaPacket(codeON);
    delay(5000);
}
