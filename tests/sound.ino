// sample script to test the DF player
// https://github.com/PowerBroker2/DFPlayerMini_Fast

// Signal wires:
// * XIAO RX to DF player TX
// * XIAO TX (via 1k resistor) to DF player RX

// Power wires: The XIAO and the DFmini need 5V regulated power (this requires diode protection)
/*
+5V (from PSU)
 ├───|>|─── XIAO 5V
 │     (Schottky diode)
 ├──────── DFPlayer VCC
    
GND (from PSU)
 ├──────── XIAO GND
 ├──────── DFPlayer GND
 */

// Speaker wires:
// * Speaker - (3.5 female jack) to DF player SPK1
// * Speaker + (3.5 female jack) to DF player SPK2

// file structure:
// files need to start with 4 digits: e.g. 0001.mp3, 0002.mp3, 0003.mp3, etc.
// after these 4 digits, the file name can be anything: e.g. 0001.mp3, 0001_hello.mp3, 0001_hello_world.mp3, etc.
// the folder name needs to be "MP3", placed under the SD card "root" directory

// Source: https://github.com/DFRobot/DFRobotDFPlayerMini/blob/master/examples/GetStarted/GetStarted.ino 

#include "DFRobotDFPlayerMini.h"

// On Arduino Nano 33IOT the TX & RX pins are added to the Serial1 object
// Name Serial1 as DFSerial to make it clear that it is used for the DFPlayer
// #define DFSerial Serial1

HardwareSerial mySerial(1);  // Use UART1
DFRobotDFPlayerMini myDFPlayer;

void setup()
{
  // initialize both Serial communications
  // TX = GPIO43 (D6), RX = GPIO44 (D7)
  mySerial.begin(115200, SERIAL_8N1, 44, 43); // RX, TX to the DF player
  Serial1.begin(9600); // for USB debugging

  // Open the SD card stream
  myDFPlayer.begin(mySerial);
  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  // myDFPlayer.play(1);  //Play the first mp3
}

void loop()
{
  // set a timer
  static unsigned long timer = millis();
  
  // every 10 seconds, play the first mp3 file
  if (millis() - timer > 10000) {
    timer = millis();
    myDFPlayer.play(1);
    Serial.println("audio playing!");
  }
}