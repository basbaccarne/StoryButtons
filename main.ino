// This is the main code for the installation

/*
** WIRES **

Power:
* 5V -- External power supply > diode >|| XIAO 5V
* 5V -- External power supply > DFplayer VCC
* 3.3V -- XIAO 3.3V > touch sensor VCC

Ground:
* External power supply > GND
* Touch sensor > GND
* XIAO > GND
* DFplayer > GDN

Data 1: touch sensor
* Touch sensor signal > XIAO D2

Data 2: DFPlayer commands
* XIAO TX > (via 1k resistor) > DFplayer RX
* XIAO RX > DFplayer TX

Data 3: Audio signal
* Speaker - > DFplayer SPK2
* Speaker + > DFplayer SPK1
 */

#include "DFRobotDFPlayerMini.h"

HardwareSerial mySerial(1);  // Use UART1
DFRobotDFPlayerMini myDFPlayer;

const int buttonPin = 3;
bool isPlaying = false;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize both Serial communications
  // TX = GPIO43 (D6), RX = GPIO44 (D7)
  mySerial.begin(115200, SERIAL_8N1, 44, 43);  // RX, TX to the DF player
  Serial1.begin(9600);                         // for USB debugging

  // Open the SD card stream
  myDFPlayer.begin(mySerial);
  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  delay(10);
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && !isPlaying) {  // Button is pressed
    digitalWrite(LED_BUILTIN, HIGH);
    myDFPlayer.play(1);  // Play the first MP3
    Serial.println("Audio started");
    isPlaying = true;
  } 
  else if (buttonState == LOW) {  // Button is released
    digitalWrite(LED_BUILTIN, LOW);
    myDFPlayer.stop();  // Stop playback
    Serial.println("Audio stopped");
    isPlaying = false;
  }

  delay(10);
}