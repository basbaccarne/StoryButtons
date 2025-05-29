// This is the main code for the button module

// Pushes on the button control audio playback on the hub module
// By sending the button ID or a stop signal to the hub via ESP-NOW.
// The LED ring on the button indicates the current state of the audio playback.

// The button can be in the following states:
// BOOT: animation to indicate that the button is ready [goes to IDLE after timer]
// IDLE: waiting for a button press (slow breathing animation) [goes to PLAYING after button press]
// PLAYING: the audio is playing [goes to MANUAL_STOP on button press / goes to AUTOMATIC_STOP after audio length has passed]
// MANUAL_STOP: the audio is stopped manually [goes to IDLE after timer]
// AUTOMATIC_STOP:after the audio length has passed [goes to IDLE after timer]

// Libraries
#include <esp_now.h>
#include <WiFi.h>

// SETTINGS
int button_ID = 1;

// ESP-NEW identification of hub
uint8_t hub_mac[] = { 0x48, 0x27, 0xE2, 0xE7, 0x1B, 0xF4 };
int stopCode = 0;

// pins
const int buttonPin = 2;
const int ledPin = 3;

// variables to audio length in milliseconds
volatile int AudioLength = 10000;
volatile bool newAudioData = false;
unsigned long startTime = 0;

// buttonPush tracking
bool canBepushed = false;

// states
enum ButtonState {
  BOOT,
  IDLE,
  PLAYING,
  MANUAL_STOP,
  AUTOMATIC_STOP
};
ButtonState currentState = BOOT;
ButtonState previousState = IDLE;

// Function to handle incoming data (store audio length in AudioLength)
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(int)) {
    memcpy((void*)&AudioLength, data, sizeof(int));
    newAudioData = true;
  }
}

void setup() {
  // initialize serial
  Serial.begin(115200);

  // initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  esp_now_init();

  // Initialize ESP-NOW to send data
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, hub_mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(hub_mac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }
  }

  // Initialize ESP-NOW to receive data
  esp_now_register_recv_cb(OnDataRecv);

  // Set pinmode for button and LED
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.println("Button sender ready");
}

void boot_sequence() {
  if(previousState != BOOT){
    // start timer
    startTime = millis();
    Serial.println("Entering BOOT mode ...");
    previousState = BOOT;
  }
  digitalWrite(ledPin, HIGH);

  // change state after 1 second
  if(millis() - startTime >= 1000){
    currentState = IDLE;
    Serial.println("Entering IDLE mode ...");
  }
}

void idle_sequence(bool buttonPushed) {
  if(previousState != IDLE){
    previousState = IDLE;
  }
  digitalWrite(ledPin, HIGH);
  if(buttonPushed == HIGH){
      canBepushed = true;
  }
  if(buttonPushed == LOW && canBepushed) {
    currentState = PLAYING;
    Serial.println("Button pressed, changing to PLAYING state...");
    canBepushed = false;
  }
}

void playing_sequence(int buttonPushed, int AudioLength) {
  // only once
  if(previousState != PLAYING){
    // start timer
    startTime = millis();
    // send the button ID to the hub
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&button_ID, sizeof(button_ID));
    if (result == ESP_OK) {
        Serial.printf("Sent button ID: %d\n", button_ID);
      } else {
        Serial.printf("Send error: %d\n", result);
      }
    previousState = PLAYING;
  }

  // play loop
  digitalWrite(ledPin, HIGH);
  // receive the audio length from the hub
  if (newAudioData) {
    Serial.printf("Received audio length: %d ms\n", AudioLength);
    newAudioData = false;
    }
  
  // If button is pressed, stop the audio manually
  if(buttonPushed == HIGH){
    // start detecting a new push only when the button has been released
    canBepushed = true;
  }
  if(buttonPushed == LOW && canBepushed){
    currentState = MANUAL_STOP;
    Serial.println("Button pressed, changing to MANUAL_STOP state...");
    digitalWrite(ledPin, LOW);
    canBepushed = false;
  } else {
    if(millis() - startTime >= AudioLength) {
      currentState = AUTOMATIC_STOP;
      Serial.println("Audio finished playing, changing to AUTOMATIC_STOP state...");
    }
  }
}

void manual_stop_sequence() {
  if(previousState != MANUAL_STOP){
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&stopCode, sizeof(stopCode));
    if (result == ESP_OK) {
      Serial.printf("Sent stop code: %d\n", stopCode);
    } else {
      Serial.printf("Send error: %d\n", result);
    }
    previousState = MANUAL_STOP;
  }
  digitalWrite(ledPin, LOW);
  Serial.println("Audio stopped manually, changing to IDLE state...");
  currentState = IDLE; 
}

void automatic_stop_sequence() {
  if(previousState != AUTOMATIC_STOP){
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&stopCode, sizeof(stopCode));
    if (result == ESP_OK) {
      Serial.printf("Sent stop code: %d\n", stopCode);
    } else {
      Serial.printf("Send error: %d\n", result);
    }
    previousState = AUTOMATIC_STOP;
  }
  digitalWrite(ledPin, LOW);
  Serial.println("Audio stopped automatically, changing to IDLE state...");
  currentState = IDLE; 
}

void loop() {

  int buttonPushed = digitalRead(buttonPin);

  switch(currentState) {
    case BOOT:
      boot_sequence();
      break;

    case IDLE:
      idle_sequence(buttonPushed);
      break;

    case PLAYING:
      playing_sequence(buttonPushed, AudioLength);
      break;

    case MANUAL_STOP:
      manual_stop_sequence();
      break;
      
    case AUTOMATIC_STOP:
      automatic_stop_sequence();
      break;
  }

  delay(10);
}