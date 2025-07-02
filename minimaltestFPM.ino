#include <FPM.h>
HardwareSerial fpmSerial(2);
FPM finger(&fpmSerial);

void setup() {
  Serial.begin(9600);
  fpmSerial.begin(57600, SERIAL_8N1, 16, 17);
  
  if (finger.begin()) {
    Serial.println("R307 found with FPM library!");
  } else {
    Serial.println("Still not working - check hardware");
  }
}
