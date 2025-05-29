// This is the main code for the button module
// It checkks wether the button is pressed
// At the start of the press, it send a signal to the hub and fills the led ring
// While pressed, it reveices the length of the selected audio file from the hub
// As long as the button is pressed, it counts down the time and reduces the number of lit LEDs accordingly
// When the button is released, it stops the audio and turns off the LEDs

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// Who are my peers?
uint8_t hub_mac[] = { 0x48, 0x27, 0xE2, 0xE7, 0x1B, 0xF4 };
int button_ID = 2;
int stopCode = 0;

// variable to audio length in milliseconds
volatile int lastReceivedAudioLength = 0;
volatile bool newAudioData = false;

// variable to track if the device is playing
bool playing = false;

// pins
const int buttonPin = 2;

// Function to handle incoming data (store audio length in lastReceivedAudioLength)
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(int)) {
    memcpy((void*)&lastReceivedAudioLength, data, sizeof(int));
    newAudioData = true;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  esp_now_init();

  // Initialize ESP-NOW (required to receive data)
  esp_now_register_recv_cb(OnDataRecv);

  // Initialize ESP-NOW (required to send data)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, hub_mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Ensure the peer is registered before sending.
  // Otherwise, esp_now_send() will silently fail.
  // esp_now_add_peer(&peerInfo); would assume success
  if (!esp_now_is_peer_exist(hub_mac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }

  pinMode(buttonPin, INPUT);
  Serial.println("Button sender ready");
}

void loop() {

  int buttonvalue = touchRead(buttonPin);
  // If button is held
  if (buttonvalue > 16000) {
    // If not already playing
    if (!playing) {
      playing = true;
      Serial.println("Button pressed...");

      // testhack
      button_ID = random(0, 6);
      
      // send the button ID to the hub
      esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&button_ID, sizeof(button_ID));

      // Check if the send was successful
      if (result == ESP_OK) {
        Serial.printf("Sent button ID: %d\n", button_ID);
      } else {
        Serial.printf("Send error: %d\n", result);
      }
    }
  // Print audio length if just received
    if (newAudioData) {
      Serial.printf("Received audio length: %d ms\n", lastReceivedAudioLength);
      newAudioData = false;
    }
  }

  // If button is released → reset strip
  else {
    if (playing) {
      delay(100);  // Optional debounce
      playing = false;
      Serial.println("Button released. Stopping playback...");
      esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&stopCode, sizeof(stopCode));
    }
  }

  // Small delay to avoid rapid looping
  delay(100);
}