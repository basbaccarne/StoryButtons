// quick test to check if the LED strip is working correctly
// This script tests a NeoPixel LED strip by lighting up each LED in red one by one.

#include <Adafruit_NeoPixel.h>

#define LED_PIN    2        // D2 on XIAO ESP32S3
#define NUM_LEDS   16       // 16 RGBW LEDs
#define DELAY_MS   500      // Delay between LEDs

// Create NeoPixel object (RGBW type)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.println("Starting LED test...");
}

void loop() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear(); // Turn off all LEDs
    strip.setPixelColor(i, strip.Color(50, 0, 0, 0)); // Red only, dimmed
    strip.show();
    Serial.print("LED ");
    Serial.print(i);
    Serial.println(" is ON");
    delay(DELAY_MS);
  }
}
