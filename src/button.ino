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
#include <Adafruit_NeoPixel.h>

// SETTINGS
int button_ID = 1;

// ESP-NEW identification of hub
uint8_t hub_mac[] = { 0x48, 0x27, 0xE2, 0xE7, 0x1B, 0xF4 };
int stopCode = 0;

// pins
const int buttonPin = 3;
const int ledPin = 2;
const int N_LEDs = 16;

// LED settings and variables
int brightness = 0;
unsigned long lastFadeTime = 0;
unsigned long previousMillis_led;
float breathAngle = 0;

// configure neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDs, ledPin, NEO_GRBW + NEO_KHZ800);

// variables to audio length in milliseconds
volatile unsigned long AudioLength = 10000;
volatile bool newAudioData = false;
unsigned long startTime = 0;

// buttonPush tracking
bool canBepushed = false;

// states
enum ButtonState {
  BOOT,
  IDLE,
  WAIT_FOR_AUDIO_LENGTH,
  PLAYING,
  MANUAL_STOP,
  AUTOMATIC_STOP,
  OVERLAP_STOP
};
ButtonState currentState = BOOT;
ButtonState previousState = IDLE;

// Function to handle incoming data (store audio length in AudioLength)
// This function is called when data is received from the hub (like an interrupt handler)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(unsigned long)) {
    memcpy((void *)&AudioLength, data, sizeof(unsigned long));
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
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) delay(1000);
  }

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

  // initiate neopixel strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'

  Serial.print("✅ Button sender ready. ");
  Serial.print("Button [[");
  Serial.print(button_ID);
  Serial.println("]] at your service!");
  Serial.println();
}

void boot_sequence() {
  led_boot();

  if (previousState != BOOT) {
    // start timer
    startTime = millis();
    Serial.println("🔁 Entering BOOT mode ...");
    previousState = BOOT;
  }

  // change state after 2 seconds
  if (millis() - startTime >= 2000) {
    currentState = IDLE;
  }
}

void idle_sequence(bool buttonPushed) {
  if (previousState != IDLE) {
    // debounce
    delay(10);
    Serial.println();
    Serial.println("🔁 Entering IDLE mode ...");
    Serial.println();
    previousState = IDLE;
  }
  digitalWrite(ledPin, HIGH);
  if (buttonPushed == HIGH) {
    canBepushed = true;
  }
  if (buttonPushed == LOW && canBepushed) {
    currentState = WAIT_FOR_AUDIO_LENGTH;
    Serial.println("🔘 Button pressed.");
    canBepushed = false;
  }

  led_idle();
}

void wait_for_audio_length_sequence() {

  led_wait_for_audio_length();

  static bool sent = false;

  if (!sent) {
    delay(10);  // debounce
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&button_ID, sizeof(button_ID));
    if (result == ESP_OK) {
      Serial.printf("➡️ Sent button ID: %d\n", button_ID);
    } else {
      Serial.printf("❌ Send error: %d\n", result);
    }
    sent = true;
    startTime = millis();  // start timeout
  }

  // If new audio data received
  if (newAudioData) {
    Serial.printf("⬅️ Received audio length: %d ms\n", AudioLength);
    newAudioData = false;
    sent = false;

    if (AudioLength <= 0) {
      Serial.println();
      Serial.println("⬅️ Another button took over. Switching to OVERLAP_STOP.");
      currentState = OVERLAP_STOP;
    } else {
      currentState = PLAYING;
    }
  }

  // Timeout fallback
  if (millis() - startTime >= 1000) {
    Serial.println("❌ Timeout waiting for audio length! Defaulting to 10s...");
    AudioLength = 10000;
    newAudioData = false;
    sent = false;
    currentState = PLAYING;
  }
}

void playing_sequence(int buttonPushed) {

  led_playing();

  // only once
  static bool started = false;

  if (!started) {
    startTime = millis();
    previousState = PLAYING;
    started = true;
  }

  digitalWrite(ledPin, HIGH);

  // Manual stop
  if (buttonPushed == HIGH) {
    canBepushed = true;
  }
  if (buttonPushed == LOW && canBepushed) {
    currentState = MANUAL_STOP;
    Serial.println();
    Serial.println("🔘 Button pressed, changing to MANUAL_STOP state...");
    digitalWrite(ledPin, LOW);
    canBepushed = false;
    started = false;
    return;
  }

  // Automatic stop
  if (millis() - startTime >= AudioLength && AudioLength != 0) {
    currentState = AUTOMATIC_STOP;
    Serial.println();
    Serial.println("🛑 Audio finished playing, changing to AUTOMATIC_STOP state...");
    started = false;
  }

  // Interrupt by another button
  if (AudioLength <= 0) {
    Serial.println();
    Serial.println("⬅️ Another button took over.");
    Serial.println("🔁 Switching to OVERLAP_STOP.");
    currentState = OVERLAP_STOP;
  }
}

void manual_stop_sequence() {

  if (previousState != MANUAL_STOP) {
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&stopCode, sizeof(stopCode));
    if (result == ESP_OK) {
      Serial.printf("➡️ Sent stop code: %d\n", stopCode);
    } else {
      Serial.printf("❌ Send error: %d\n", result);
    }
    startTime = millis();
    previousState = MANUAL_STOP;
  }

   led_manual_stop();

    // change state after 1 second
  if (millis() - startTime >= 1000) {
    currentState = IDLE;
  }
}

void automatic_stop_sequence() {

  if (previousState != AUTOMATIC_STOP) {
    esp_err_t result = esp_now_send(hub_mac, (uint8_t *)&stopCode, sizeof(stopCode));
    if (result == ESP_OK) {
      Serial.printf("➡️ Sent stop code: %d\n", stopCode);
    } else {
      Serial.printf("❌ Send error: %d\n", result);
    }
    startTime = millis();
    previousState = AUTOMATIC_STOP;
  }

  led_automatic_stop();
    // change state after 1 second
  if (millis() - startTime >= 1000) {
    currentState = IDLE;
  }
}

void overlap_stop_sequence() {
  
  // This state is not used in this implementation, but can be used for future features
  // like overlapping audio playback or other advanced features.
  if (previousState != OVERLAP_STOP) {
    startTime = millis();
    previousState = OVERLAP_STOP;
  }

  led_overlap_stop();

  // change state after 1 seond
  if (millis() - startTime >= 1000) {
    currentState = IDLE;
  }
}

void loop() {

  int buttonPushed = digitalRead(buttonPin);

  switch (currentState) {
    case BOOT:
      boot_sequence();
      break;

    case IDLE:
      idle_sequence(buttonPushed);
      break;

    case WAIT_FOR_AUDIO_LENGTH:
      wait_for_audio_length_sequence();
      break;

    case PLAYING:
      playing_sequence(buttonPushed);
      break;

    case MANUAL_STOP:
      manual_stop_sequence();
      break;

    case AUTOMATIC_STOP:
      automatic_stop_sequence();
      break;

    case OVERLAP_STOP:
      overlap_stop_sequence();
      break;
  }

  delay(10);
}

void led_boot() {
  static unsigned long lastUpdate = 0;
  static int startHue = 0;

  const unsigned long updateInterval = 20;  // Controls speed
  const int hueStep = 3;                    // Controls swirl speed
  const int brightnessLimit = 30;           // Max brightness (0–255)

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    for (int i = 0; i < N_LEDs; i++) {
      int hue = (startHue + (i * 256 / N_LEDs)) % 256;
      uint32_t color = strip.gamma32(strip.ColorHSV(hue * 256));  // 16-bit HSV to RGB with gamma correction

      // Scale brightness down
      uint8_t r = (uint8_t)((uint32_t)(color >> 16) & 0xFF) * brightnessLimit / 255;
      uint8_t g = (uint8_t)((uint32_t)(color >> 8) & 0xFF) * brightnessLimit / 255;
      uint8_t b = (uint8_t)((uint32_t)color & 0xFF) * brightnessLimit / 255;

      strip.setPixelColor(i, r, g, b);
    }

    strip.show();
    startHue = (startHue + hueStep) % 256;
  }
}

void led_idle() {
static unsigned long lastUpdate = 0;
  static float angle = 0.0;

  const float maxBrightness = 0.05;        // 5% of full brightness
  const float gamma = 2.8;
  const float breathingSpeed = 0.02;       // Smaller = slower breathing
  const unsigned long updateInterval = 15; // ms between updates

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    float raw = (sin(angle) + 1.0) / 2.0;         // 0.0 to 1.0
    float adjusted = pow(raw, gamma);            // gamma correction
    int brightness = adjusted * 255 * maxBrightness;

    for (int i = 0; i < N_LEDs; i++) {
      strip.setPixelColor(i, strip.Color(brightness, brightness, brightness)); // breathing white
    }
    strip.show();

    angle += breathingSpeed;
    if (angle >= TWO_PI) angle -= TWO_PI;
  }
}

void led_wait_for_audio_length() {
  static unsigned long lastUpdate = 0;
  static float angle = 0.0;
  static int dotIndex = 0;

  const float pulseSpeed = 0.05;
  const float maxPulseBrightness = 0.05;
  const float tailFade = 0.5;
  const unsigned long updateInterval = 20;

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Pulsating base brightness
    float pulse = (sin(angle) + 1.0) / 2.0;
    float pulseCorrected = pow(pulse, 2.8);
    int baseBrightness = pulseCorrected * 255 * maxPulseBrightness;

    // Draw base ring
    for (int i = 0; i < N_LEDs; i++) {
      strip.setPixelColor(i, strip.Color(0, baseBrightness, 0));
    }

    // Rotating brighter dot with tail
    for (int t = 0; t < 3; t++) {
      int index = (dotIndex - t + N_LEDs) % N_LEDs;
      float tailFactor = pow(tailFade, t);
      int tailBrightness = baseBrightness + (100 * tailFactor);
      if (tailBrightness > 255) tailBrightness = 255;
      strip.setPixelColor(index, strip.Color(0, tailBrightness, 0));
    }

    strip.show();

    dotIndex = (dotIndex + 1) % N_LEDs;
    angle += pulseSpeed;
    if (angle >= TWO_PI) angle -= TWO_PI;
  }
}

void led_playing() {
  unsigned long elapsed = millis() - startTime;
  float progress = fmin((float)elapsed / AudioLength, 1.0);
  int ledsToLight = (1.0 - progress) * N_LEDs;

  for (int i = 0; i < N_LEDs; i++) {
    if (i < ledsToLight) {
      // Solid blue color at full brightness
      strip.setPixelColor(i, strip.Color(0, 0, 50));
    } else {
      strip.setPixelColor(i, 0); // turn off
    }
  }

  strip.show();
}


// Helper to set pulsating rotating dot with tail in any color
void led_pulsating_tail(uint8_t rBase, uint8_t gBase, uint8_t bBase) {
  static unsigned long lastUpdate = 0;
  static int dotIndex = 0;
  static float breathAngle = 0;

  const unsigned long updateInterval = 20;
  const float pulseSpeed = 0.05;
  const float maxPulseBrightness = 0.05;
  const float tailFade = 0.5;  // brightness reduction factor per tail LED
  const int tailLength = 3;

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Base pulsating brightness (0 to 1)
    float pulse = (sin(breathAngle) + 1.0) / 2.0;
    float pulseCorrected = pow(pulse, 2.8);
    float baseBrightnessFactor = pulseCorrected * maxPulseBrightness;

    // Draw base ring with pulsating brightness
    for (int i = 0; i < N_LEDs; i++) {
      uint8_t r = rBase * baseBrightnessFactor;
      uint8_t g = gBase * baseBrightnessFactor;
      uint8_t b = bBase * baseBrightnessFactor;
      strip.setPixelColor(i, r, g, b);
    }

    // Draw brighter rotating dot and tail
    for (int t = 0; t < tailLength; t++) {
      int idx = (dotIndex - t + N_LEDs) % N_LEDs;
      float tailBrightnessFactor = pow(tailFade, t);
      // Brighter dot and fading tail, capped at 255
      uint8_t r = min(255, (int)(rBase * 4 * tailBrightnessFactor));
      uint8_t g = min(255, (int)(gBase * 4 * tailBrightnessFactor));
      uint8_t b = min(255, (int)(bBase * 4 * tailBrightnessFactor));
      strip.setPixelColor(idx, r, g, b);
    }

    strip.show();

    dotIndex = (dotIndex + 1) % N_LEDs;
    breathAngle += pulseSpeed;
    if (breathAngle >= TWO_PI) breathAngle -= TWO_PI;
  }
}

void led_manual_stop() {
  // Orange color base (R, G, B)
  led_pulsating_tail(255, 100, 0);
}

void led_automatic_stop() {
  // Blue color base
  led_pulsating_tail(0, 0, 255);
}

void led_overlap_stop() {
  // White color base
  led_pulsating_tail(255, 255, 255);
}