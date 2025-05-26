/* The XIAO ESP32 S3 has 9 capacitive touch pads
This testscript reads the signals as a way to test and calibrate
What is this data? It is an indication of the capacitor discharge time.
*/

void setup(){
    Serial.begin(9600);
}

void loop(){
    int touchValue = touchRead(2);
    Serial.println(touchValue);
    delay(50);
}