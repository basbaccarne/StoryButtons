// This script tests the relay setup
/*
Wiring:

* AC 220V  Blue (N) > sleeve of lightbulb
* AC 220V  Brown (L) > COM on relay
* NO (normally open) on relay > tip of lightbulb

* XIAO 5V > HV on TTL
* XIAO GND > HV GND on TTL
* XIAO 3.3V > LV on TTL
* XIAO GND > LV GND on TTL
* XIAO D1 (GPIO2) > LV TXI on TTL

* TTL HV RXO > relay D
* TTL HV VCC > relay +
* TTL HV GND > relay -

* AC 220V Blue (N) > Mean Well N
* AC 220V Brown(L) > Mean Well L
* Mean Well V- to XIAO GND
* Mean well V+ to XIAO 5V

*/

// Pins
const int relayPin = 2;
const int ledPin = LED_BUILTIN; 

void setup() {
    pinMode(relayPin, OUTPUT);
    pinMode(ledPin, OUTPUT); 
}

void loop() {
    digitalWrite(relayPin, HIGH); // Turn on relay
    digitalWrite(ledPin, HIGH); // Turn on LED
    delay(1000);
    digitalWrite(relayPin, LOW); // Turn off relay
    digitalWrite(ledPin, LOW); // Turn off LED
    delay(1000); 
}

