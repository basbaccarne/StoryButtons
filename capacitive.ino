/* The XIAO ESP32 S3 has 9 capacitive touch pads
This testscript reads the signals as a way to test and calibrate
The touchRead function reads the capacitance and lowers when touched
*/

void setup(){
    Serial.begin(9600);
}

void loop(){
    int touchValue = touchRead(2);
    Serial.print(touchValue);
    delay(50);
}