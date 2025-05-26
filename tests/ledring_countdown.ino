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

// configure neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDs, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {
  // initiate neopixel strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {
  // if the button is pressed
   while (digitalRead(buttonPin) == HIGH) {

      for (int i = 0; i < N_LEDs; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255)); // Blue
      }
      strip.show();

      // Calculate delay per LED
      unsigned long delayPerLED = (countdownSeconds * 1000UL) / N_LEDs;

      // Turn off one LED at a time
      for (int i = 0; i < N_LEDs; i++) {
        delay(delayPerLED);
        strip.setPixelColor(i, 0); // Turn off LED
        strip.show();
      }
   }

   strip.show();
  delay(10);
}