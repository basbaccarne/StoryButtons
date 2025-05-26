// This scripts sends "hello" every second to the second ESP32S3 device
// beware: add wifi antenna or it won't work ;)

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// Who are my peers?
uint8_t peerAddress[] = {0xD8, 0x3B, 0xDA, 0x73, 0xC4, 0x58};  // Device B


void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Serial.print("Reply from ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  Serial.print(macStr);

  Serial.print(" | Message: ");
  Serial.write(data, len);
  Serial.println();
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

  Serial.println("Sender ready");
}

void loop() {
  const char *msg = "Hello";
  esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)msg, strlen(msg));

  if (result == ESP_OK) {
    Serial.println("Sent: Hello");
  } else {
    Serial.printf("Send error: %d\n", result);
  }

  delay(1000);
}
