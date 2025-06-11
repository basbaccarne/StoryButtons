// This is the main code for the hub module
// It receives button presses from the buttons via ESP-NOW
// and plays audio files based on the received ID.
// It also sends back the audio length to the buttons so they can display the remaining time.
// playback stops on ID 0.

// For ESP-NOW: beware: add wifi antenna or it won't work ;)
// Signal wires:
// * XIAO RX to DF player TX
// * XIAO TX (via 1k resistor) to DF player RX

// Power wires:
/*
The XIAO and the DFmini need 5V regulated power (this requires diode protection)

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
// change the contents of the struct Track to match your audio files

// Source: https://github.com/DFRobot/DFRobotDFPlayerMini/blob/master/examples/GetStarted/GetStarted.ino

// Libraries
#include "DFRobotDFPlayerMini.h"
#include <esp_now.h>
#include <WiFi.h>

// Struct to hold track information
struct Track {
  uint8_t number;
  const char *name;
  unsigned long duration;
};

// Metadata for the audio tracks (id, name, duration in ms)
const Track tracks[] = {
  { 1, "groeilood", 22 * 1000 },
  { 2, "clockmaker", 28 * 1000 },
  { 3, "ultraoortje", 10 * 1000 },
  { 4, "gevoelensradio", 16 * 1000 }
};
const uint8_t numTracks = sizeof(tracks) / sizeof(tracks[0]);

// pins
const int busyPin = 2;
const int ledPin = 8;

// ESP-NOW peer addresses (MAC addresses of the buttons - see "mac.ino" for details)
uint8_t button1_mac[] = { 0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x74 };
uint8_t button2_mac[] = { 0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x58 };
uint8_t button3_mac[] = {0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x5C};
uint8_t button4_mac[] = {0xD8, 0x3B, 0xDA, 0x46, 0x64, 0x00};
uint8_t bulb_macs[][6] = {
  { 0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x88 }, // bulb1
  { 0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x58 }, // bulb2
  { 0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x5C }, // bulb3
  { 0xD8, 0x3B, 0xDA, 0x46, 0x64, 0x00 }  // bulb4
};

// Variables to store the received button ID & sender IDs
volatile int ReceivedButtonID = 0;
volatile bool newButtonData = false;
int previousButtonID = 0;
uint8_t lastSenderMac[6];
uint8_t penultimateSenderMac[6];

// Variable to store the audio length
unsigned long audioLength = 0;

// create a HardwareSerial object for the DFPlayer (the XIAO way)
HardwareSerial DFSerial(1);
DFRobotDFPlayerMini myDFPlayer;

// This function is called when data is received from a button
// (works like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // on valid message
  if (len == sizeof(int)) {
    previousButtonID = ReceivedButtonID; // store the previous button ID
    memcpy(penultimateSenderMac, lastSenderMac, 6); // store the previous sender's MAC address
    memcpy((void *)&ReceivedButtonID, data, sizeof(int)); // store the received button ID
    memcpy(lastSenderMac, info->src_addr, 6); // store the sender's MAC address
    newButtonData = true;
  }
}

// Function to registrer all the ESP-NOW peers
void addPeer(uint8_t *mac) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(mac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
    }
  }
}

// Function to turn the invention lights on and off
void setLightBulb(uint8_t *mac, bool lightOn) {
  uint8_t lightTurnedOn = lightOn ? 1 : 0; // Convert to uint8_t
  esp_err_t result = esp_now_send(mac, &lightTurnedOn, sizeof(lightTurnedOn));
  if (result == ESP_OK) {
    Serial.printf("✅ Bulb %s successfully.\n", lightOn ? "turned on" : "turned off");
  } else {
    Serial.printf("❌ Send error: %d\n", result);
  }
}

void setup() {
  // initialize both Serial communications
  // TX = GPIO43 (D6), RX = GPIO44 (D7)
  DFSerial.begin(9600, SERIAL_8N1, 44, 43);
  Serial.begin(115200);

  // set status pin as input with pull-up resistor
  pinMode(busyPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // Open the SD card stream
  myDFPlayer.begin(DFSerial);
  delay(10);
  // Set volume value. From 0 to 30
  myDFPlayer.volume(10);

  // initiatlize ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  addPeer(button1_mac);
  addPeer(button2_mac);
  addPeer(button3_mac);
  addPeer(button4_mac);
  addPeer(bulb_macs[0]); // bulb1
  // add others ... when ready

  // light all the lights for 1 second
  Serial.println("➡️ Turning all lights on for 1 second ...");
  setLightBulb(bulb_macs[0], true);
  delay(1000);
  setLightBulb(bulb_macs[0], false);
  delay(1000);
  // add others when ready

  // boot blink led
  for (int i = 0; i < 5; i++) {
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
  }

  Serial.println("✅ Listening hub ready");
  Serial.println();
}

void loop() {
  bool busy = !digitalRead(busyPin);
  digitalWrite(ledPin, busy);

  // reaction on incoming data
  if (newButtonData) {
    Serial.print("⬅️ Received button ID: ");
    Serial.println(ReceivedButtonID);
    
    // If the received button ID is 0, stop playback
    if (ReceivedButtonID == 0) {
      Serial.println("🛑 Stopping playback ...");
      Serial.println("");
      myDFPlayer.stop();
      delay(100);
      // Turn off light bulb
      setLightBulb(bulb_macs[previousButtonID - 1], false); 
    }

    // If the ID is within valid range
    else if (ReceivedButtonID > 0 && ReceivedButtonID <= numTracks) {

      // if something was playing tell that device a new button ID was received
      if (busy) {
        int interruptCode = 0;
        Serial.println("➡️ Sending interrupt code to previous sender.");
        esp_now_send(penultimateSenderMac, (uint8_t *)&interruptCode, sizeof(interruptCode));
        setLightBulb(bulb_macs[previousButtonID - 1], false); // turn off the previous light bulb
        delay(10);

      } 
      // Get and send the duration of the selected track ...
      audioLength = tracks[ReceivedButtonID - 1].duration;
      Serial.printf("➡️ Sending back audio length: %d ms\n", audioLength);
      esp_now_send(lastSenderMac, (uint8_t *)&audioLength, sizeof(audioLength));
      delay(10);

      // .. and play the track
      Serial.print("▶️ Starting track ");
      Serial.print(tracks[ReceivedButtonID - 1].number);
      Serial.print(" - ");
      Serial.println(tracks[ReceivedButtonID - 1].name);
      myDFPlayer.play(tracks[ReceivedButtonID - 1].number);
      Serial.println("");

      // ... and light the associated light bulb
      setLightBulb(bulb_macs[ReceivedButtonID - 1], true);
      
    }
    // if the ID is out of range
    else {
      Serial.println("❌ Invalid button ID received. Ignoring...");
    }
    newButtonData = false;  // Reset the flag
  }
  delay(100);
}