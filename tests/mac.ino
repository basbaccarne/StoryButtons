// This script prints the MAC address of the XIAO ESP32S3 to the Serial Monitor.
// fast ESP32 systems use baud rate 115200, don't forget to set it in the Serial Monitor!

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // wait for Serial

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  String mac = WiFi.macAddress();  // e.g. "94:B9:7E:12:34:56"
  Serial.print("MAC Address: ");
  Serial.println(mac);

  int parts = 0;
  int start = 0;
  for (int i = 0; i < mac.length(); i++) {
    if (mac[i] == ':' || i == mac.length() - 1) {
      int end = (mac[i] == ':') ? i : i + 1;
      String part = mac.substring(start, end);
      start = i + 1;

      byte b = (byte)strtol(part.c_str(), NULL, 16);

      if (parts > 0) Serial.print(", ");
      if (b < 0x10) Serial.print("0x0");
      else Serial.print("0x");
      Serial.print(b, HEX);

      parts++;
    }
  }
}

void loop() {}