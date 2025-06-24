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
#include "esp_wifi.h"


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
uint8_t button_macs[][6] = {
  { 0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x74 },  // button 1
  { 0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x58 },  // button 2
  { 0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x5C },  // button 3
  { 0xD8, 0x3B, 0xDA, 0x46, 0x64, 0x00 }   // button 4
};

uint8_t bulb_macs[][6] = {
  { 0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x88 },  // bulb1
  { 0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x20 },  // bulb2
  { 0xD8, 0x3B, 0xDA, 0x46, 0x59, 0x21 },  // bulb3
  { 0xD8, 0x3B, 0xDA, 0x46, 0x64, 0x22 }   // bulb4
};

// Variables to store the received button ID & sender IDs
volatile bool newButtonData = false;
int receivedButtonID = 0;
int currentPlayingButtonID = 0;
uint8_t currentSenderMac[6] = { 0 };
uint8_t activeSenderMac[6] = { 0 };

// Variable to store the audio length
unsigned long audioLength = 0;
uint8_t lengthBuffer[5] = {0x01, 0, 0, 0, 0};

// create a HardwareSerial object for the DFPlayer (the XIAO way)
HardwareSerial DFSerial(1);
DFRobotDFPlayerMini myDFPlayer;

// This function is called when data is received from a button
// (works like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&receivedButtonID, data, sizeof(int));
  if (receivedButtonID != currentPlayingButtonID || memcmp(info->src_addr, activeSenderMac, 6) != 0) {
    memcpy(currentSenderMac, info->src_addr, 6);
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

// Function to send with error handling
bool sendWithRetry(const uint8_t *mac, const uint8_t *data, size_t len, uint32_t timeoutMs = 3000) {
  delay(10);
  unsigned long start = millis();
  esp_err_t result;
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  while (millis() - start < timeoutMs) {
    result = esp_now_send(mac, data, len);
    Serial.printf("📤 Sending to %s | Data[0]: %d | Result: %d\n", macStr, data[0], result);

    if (result == ESP_OK) {
      return true;
    }
    delay(100);  // wait before retrying
  }

  Serial.printf("❌ Failed to send to %s after %lu ms\n", macStr, millis() - start);
  return false;
}

// Function to turn the invention lights on and off
void setLightBulb(uint8_t *mac, bool lightOn) {
  uint8_t lightTurnedOn = lightOn ? 1 : 0;  // Convert to uint8_t
  esp_err_t result = sendWithRetry(mac, &lightTurnedOn, sizeof(lightTurnedOn));
  if (result == ESP_OK) {
    Serial.printf("✅ Bulb %s successfully.\n", lightOn ? "turned on" : "turned off");
    Serial.println();
  } else {
    Serial.printf("❌ Send error: %d\n", result);
  }
}

void reaction(int id) {

  // retreive audio length
  audioLength = tracks[id - 1].duration;
  lengthBuffer[0] = 0x01;
    memcpy(&lengthBuffer[1], &audioLength, sizeof(audioLength));

  // send audio length or stop code
  Serial.printf("🎵 Sending audio length %lu ms to button %d\n", audioLength, id);
  sendWithRetry(button_macs[id - 1], lengthBuffer, sizeof(lengthBuffer));

  int N_buttons = 4;
  uint8_t stopBuffer[] = { 0x02, 's', 't', 'o', 'p', 0 };

  for (int i = 0; i < N_buttons; i++) {
    if (i != id - 1) {
      Serial.printf("🚫 Sending interrupt code to button %d\n", i + 1);
      sendWithRetry(button_macs[i], stopBuffer, sizeof(stopBuffer));
    }
  }
 

  // play the track
  Serial.print("▶️ Starting track ");
  Serial.print(tracks[receivedButtonID - 1].number);
  Serial.print(" - ");
  Serial.println(tracks[receivedButtonID - 1].name);
  myDFPlayer.play(tracks[receivedButtonID - 1].number);

  // turn on the light &nd turn of all the other lights
  //setLightBulb(bulb_macs[previousButtonID - 1], false);

  // set variables
  currentPlayingButtonID = id;
  memcpy(activeSenderMac, currentSenderMac, 6);
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
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_max_tx_power(78);  // 19.5 dBm
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  for (int i = 0; i < 4; i++) {
    addPeer(button_macs[i]);
    addPeer(bulb_macs[i]);
  }

  // light all the lights for 1 second
  Serial.println();
  Serial.println("➡️ Turning all lights on for 1 second ...");
  // setLightBulb(bulb_macs[0], true);
  delay(1000);
  // setLightBulb(bulb_macs[0], false);
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
  digitalWrite(ledPin, !digitalRead(busyPin));
  if (!newButtonData) return;  // skip loop if there is no new data
  newButtonData = false;

  // reaction on incoming data
  Serial.print("⬅️ Received button ID: ");
  Serial.println(receivedButtonID);
  Serial.println();
  delay(10);

  if (receivedButtonID < 0 || receivedButtonID > numTracks) {
    Serial.println("❌ Invalid button ID received!");  // skip loop if the code is invallid
    return;
  }

  // If the received button ID is 0, stop playback
  if (receivedButtonID == 0) {
    Serial.println("🛑 Stopping playback ...");
    myDFPlayer.stop();
    currentPlayingButtonID = 0;
    memset(activeSenderMac, 0, 6);
    delay(10);
    return;
  }

  // when a button ID is received that is not zero
  if (receivedButtonID > 0) {
    reaction(receivedButtonID);
  }
  newButtonData = false;
  delay(10);
}