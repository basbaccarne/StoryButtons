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

const int neopixelPin = 1;      
const int buttonPin = 2;      
const int onboardLED = LED_BUILTIN;

const int N_LEDs = 16;
const unsigned long countdownSeconds = 10;
const unsigned long delayPerLED = (countdownSeconds * 1000UL) / N_LEDs;

Adafruit_NeoPixel strip(N_LEDs, neopixelPin, NEO_GRB + NEO_KHZ800);

int currentLED = 0;
unsigned long lastUpdate = 0;
bool playing = false;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(onboardLED, OUTPUT); 

  strip.begin();
  strip.clear();
  strip.show();           
}

void loop() {
  bool buttonPressed = digitalRead(buttonPin) == LOW;

  // Debug LED: on when button is pressed
  digitalWrite(onboardLED, buttonPressed ? LOW : HIGH);

  // If button is held
  if (buttonPressed) {
    // If not already playing
    if(!playing){
      // Light up all LEDs blue initially
      for (int i = 0; i < N_LEDs; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }
      strip.show();
      playing = true;
      currentLED = 0;
    }

    // start the countdown
    unsigned long now = millis();

    // If enough time has passed, turn off the current LED
    // and move to the next one
    if (now - lastUpdate >= delayPerLED) {
      strip.setPixelColor(currentLED, 0);  // Turn off current LED
      strip.show();
      currentLED++;
      lastUpdate = now;
    }
  }

  // If button is released → reset strip
  if (!buttonPressed) {
    delay(100); // Optional debounce
    strip.clear();
    strip.show();
    playing = false;
  }

  // Small delay to avoid rapid looping
  delay(10);
}