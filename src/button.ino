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
uint8_t hub_mac[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x58};
int button_ID = 1;

// variable to audio length in milliseconds
int lastReceivedAudioLength = 0; 

// Function to handle incoming data (store audio length in lastReceivedAudioLength)
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(int)) {
    memcpy(&lastReceivedAudioLength, data, sizeof(int));  // Copy raw bytes into int
    Serial.print("Received Audio Length: ");
    Serial.println(lastReceivedAudioLength);
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

  int touchValueBase = touchRead(2); // Read initial touch value for calibration
  Serial.println("Button sender ready"); 
}

void loop() {
  
  // send the button ID to the hub
  esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&button_ID, sizeof(button_ID));

  // Check if the send was successful
  if (result == ESP_OK) {
    Serial.print("Sent button ID to hub: ");
    Serial.println(button_ID);
  } else {
    Serial.printf("Send error: %d\n", result);
  }

  delay(1000);
}