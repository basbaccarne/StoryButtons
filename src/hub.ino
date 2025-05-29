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
  const char* name;
  unsigned long duration;
};

// Metadata for the audio tracks (id, name, duration in ms)
const Track tracks[] = {
  {1, "HAL 9000",    16*1000},
  {2, "Sauruman",    28*1000},
  {3, "Dobby",       10*1000}
};
const uint8_t numTracks = sizeof(tracks) / sizeof(tracks[0]);

// ESP-NOW peer addresses (MAC addresses of the buttons - see "mac.ino" for details)
uint8_t button1_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x74};
// uint8_t button2_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x75};
// uint8_t button3_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x76};
// uint8_t button4_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x77};

// Variable to store the received button ID
int ReceivedButtonID = 0; 

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
    memcpy((void*)&ReceivedButtonID, data, sizeof(int));
    Serial.print("Received press from button ID: ");
    Serial.println(ReceivedButtonID);
    delay(10);

    // check if the received button ID is valid
    if (ReceivedButtonID == 0) {
      Serial.println("■ Stopping playback ...");
      myDFPlayer.stop();
      delay(200); 
    }
    else if (ReceivedButtonID > 0 && ReceivedButtonID <= numTracks) {
        audioLength = tracks[ReceivedButtonID - 1].duration; // Get the duration of the selected track
        Serial.printf("Sending back audio length: %d ms\n", audioLength);
        esp_now_send(info->src_addr, (uint8_t *)&audioLength, sizeof(audioLength));
        delay(10);
        Serial.print("➤ Starting track ");
        Serial.print(tracks[ReceivedButtonID - 1].number);
        Serial.print(" - ");
        Serial.println(tracks[ReceivedButtonID - 1].name);
        myDFPlayer.play(tracks[ReceivedButtonID - 1].number);
    }  
    else {
        Serial.println("Invalid button ID. Ignoring...");
    }
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

void setup()
{
  // initialize both Serial communications
  // TX = GPIO43 (D6), RX = GPIO44 (D7)
  DFSerial.begin(9600, SERIAL_8N1, 44, 43);
  Serial.begin(115200);

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
  // addPeer(button2_mac);
  // addPeer(button3_mac);
  // addPeer(button4_mac);

  Serial.println("Receiver ready");

}

void loop(){delay(100);}