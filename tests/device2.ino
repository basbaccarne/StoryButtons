// DEVICE B
// This scripts listens for data and responds on a received message
// beware: add wifi antenna or it won't work ;)

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// Who are my peers?
uint8_t peerAddress[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC6, 0x74};  // Device A

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Serial.print("Received from: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  Serial.println(macStr);

  Serial.print("Data: ");
  Serial.write(data, len);
  Serial.println();

  // Send back "Received" to sender
  const char *reply = "Received";
  esp_now_send(info->src_addr, (const uint8_t *)reply, strlen(reply));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(peerAddress)) {
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