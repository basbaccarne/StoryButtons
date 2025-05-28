// This is the main code for the audio hub
// It listens for button presses from the buttons
// Then calculated the length of the selected audio file
// and sends the length of the selected audio file to the buttons
// It also starts playing the audio file

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// Who are my peers?
uint8_t button1_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x74};

// Variable to store the last received button ID
int lastReceivedButtonID = 0; 

// Function to handle incoming data (store audio length in lastReceivedButtonID)
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(int)) {
    memcpy((void*)&lastReceivedButtonID, data, sizeof(int));
    Serial.printf("Received press from button ID: %d\n", lastReceivedButtonID);
  }

  int audioLength = random(5000, 15000);  // 5–15 seconds
  Serial.printf("Sending audio length: %d ms\n", audioLength);

  // Send back the audio length to the button
  esp_now_send(info->src_addr, (uint8_t *)&audioLength, sizeof(audioLength));
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
  memcpy(peerInfo.peer_addr, button1_mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Ensure the peer is registered before sending. 
  // Otherwise, esp_now_send() will silently fail.
  // esp_now_add_peer(&peerInfo); would assume success
  if (!esp_now_is_peer_exist(button1_mac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }

  Serial.println("Receiver ready");
}

void loop() {
  delay(100);
}