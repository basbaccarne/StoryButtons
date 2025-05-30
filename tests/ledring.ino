// This script tests a neopixel led ring with a simple chase animation
// Wiring:
//  Din (Neopixel - data in) to D1 (XIAO ESP32)
//  5V (Neopixel - power) to 5V (external power supply)
//  GND (Neopixel - ground) to GND (external power supply)
//  GND (XIAO ESP32 - ground) to GND (external power supply)

#include <Adafruit_NeoPixel.h>

const int ledPin = 2; // led data on D1
const int N_LEDs = 16;

// configure neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDs, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {
  // initiate neopixel strip
  strip.begin();
}

void loop() {
  chase(strip.Color(255, 0, 0)); // Red
  chase(strip.Color(0, 255, 0)); // Green
  chase(strip.Color(0, 0, 255)); // Blue
}

static void chase(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      strip.setPixelColor(i  , c); // Draw new pixel
      strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
      strip.show();
      delay(25);
  }
}
