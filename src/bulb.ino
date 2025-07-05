// This is the main code for the invention light bulb modules

// The module listens for signals from the hub module
// The module can receive lightTurnedOn = true or false and acts accordingly.

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// SETTINGS
int bulb_ID = 1;

// ESP-NOW identification of hub
uint8_t hub_mac[] = { 0x48, 0x27, 0xE2, 0xE7, 0x1B, 0xF4 };

// Pins
const int relayPin = 2;

// Main variable
volatile bool lightTurnedOn = false;

// Function to handle incoming data
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(uint8_t)) {
        uint8_t received = *data;
        lightTurnedOn = (received != 0);
    }
}

void setup() {
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register receive callback
    esp_now_register_recv_cb(OnDataRecv);

    // Set relay pin as output
    pinMode(relayPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT); 
    digitalWrite(relayPin, LOW);

    Serial.print("✅ Light bulb ready. ");
    Serial.print("Bulb [[");
    Serial.print(bulb_ID);
    Serial.println("]] at your service!");
    Serial.println();
}

void loop() {
    // set relay state
    digitalWrite(relayPin, lightTurnedOn ? HIGH : LOW);
    // set LED state
    digitalWrite(LED_BUILTIN, lightTurnedOn ? HIGH : LOW);
    delay(100);
}