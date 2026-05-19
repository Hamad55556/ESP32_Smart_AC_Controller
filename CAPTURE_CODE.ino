#include <Arduino.h>
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 18 
#define RAW_BUFFER_LENGTH 400

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  Serial.println("----------------------------------------");
  Serial.println("READY TO CAPTURE.");
  Serial.println("Point your remote at the ESP32 receiver.");
  Serial.println("1. Cover the remote LED and press OFF.");
  Serial.println("2. Uncover, point at ESP32, and press ON once.");
  Serial.println("----------------------------------------");
}

void loop() {
  if (IrReceiver.decode()) {

    if (IrReceiver.decodedIRData.rawlen < 100) {
      Serial.println("Signal too short, likely noise. Ignoring.");
      IrReceiver.resume();
      return;
    }

    Serial.println();
    Serial.println(">>> SIGNAL RECEIVED! <<<");
    
    IrReceiver.printIRResultShort(&Serial);
    
    Serial.println("--- RAW DATA START ---");
    IrReceiver.printIRResultRawFormatted(&Serial, true);

    Serial.print("Raw data length: ");
    Serial.println(IrReceiver.decodedIRData.rawlen - 1);

    Serial.println("--- RAW DATA END ---");

    IrReceiver.resume();
  }
}