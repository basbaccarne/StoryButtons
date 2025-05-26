// This simple code tests the button functionality on the XIAO ESP32.
// It uses the internal pull-up resistor to read the button state.
// Wiring:
// Button pin 1 → Arduino digital pin D2
// Button pin 2 → GND

const int buttonPin = 2; // Button pin connected to D2 (XIAO ESP32)

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Internal pull-up resistor enabled
}

void loop() {
  // Button logic: LOW when pressed, HIGH when released
  if (digitalRead(buttonPin) == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on LED when button is pressed
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn off LED otherwise
  }
  delay(10);
}