// This code is for testing the touch button functionality.
// It uses the internal pull-up resistor of the Arduino to read the button state.
// The button is connected to ground, so when pressed, it will read LOW.

// Button pin 1 → Arduino digital pin D2
// Button pin 2 → GND

const int buttonPin = 3;  // BEWARE: on the XIAO, D2 is annotated as 3

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor
  pinMode(LED_BUILTIN, OUTPUT);      // Built-in LED pin
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {           // Button pressed (connected to GND)
    digitalWrite(LED_BUILTIN, HIGH); // Turn on LED
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn off LED
  }
}