// This script tests a neopixel led ring that counts down from a specified time.
// It lights up all LEDs in blue and then turns them off one by one.

// Wiring:
//  Din (Neopixel - data in) to D1 (XIAO ESP32)
//  5V (Neopixel - power) to 5V (external power supply)
//  GND (Neopixel - ground) to GND (external power supply)
//  GND (XIAO ESP32 - ground) to GND (external power supply)
// Button pin 1 → Arduino digital pin D2
// Button pin 2 → GND

#include <Adafruit_NeoPixel.h>

const int ledPin = 1; // led data on D1
const int buttonPin = 2; // Button pin connected to D2
const int N_LEDs = 16;
unsigned long countdownSeconds = 10; // length of the countdown time in seconds

// color settings
const uint8_t COLOR_R = 0;
const uint8_t COLOR_G = 0;
const uint8_t COLOR_B = 255;

// configure neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDs, ledPin, NEO_GRB + NEO_KHZ800);

// Breathing effect state
int breathBrightness = 0;
int breathDirection = 1;

void setup() {
  pinMode(buttonPin, INPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

// ----------- FUNCTIONS ------------

void setAllColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < N_LEDs; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void updateBreathingEffect(uint8_t step = 2) {
  breathBrightness += breathDirection * step;

  if (breathBrightness >= 255) {
    breathBrightness = 255;
    breathDirection = -1;
  } else if (breathBrightness <= 0) {
    breathBrightness = 0;
    breathDirection = 1;
  }

  setAllColor(
    (COLOR_R * breathBrightness) / 255,
    (COLOR_G * breathBrightness) / 255,
    (COLOR_B * breathBrightness) / 255
  );
}

void countdownEffect() {
  // Set full brightness blue first
  setAllColor(COLOR_R, COLOR_G, COLOR_B);

  unsigned long delayPerLED = (countdownSeconds * 1000UL) / N_LEDs;

  for (int i = 0; i < N_LEDs; i++) {
    delay(delayPerLED);
    strip.setPixelColor(i, 0); // Turn off LED
    strip.show();
  }
}

// ----------- MAIN LOOP ------------

void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    countdownEffect();
  } else {
    updateBreathingEffect();
    delay(20); // Breathing smoothness
  }
}