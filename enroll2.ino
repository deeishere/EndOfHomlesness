/*****************
  Adapted for AltSoftSerial on Arduino-Uno (D8/D9)
  R307 baud = 19200 USB monitor = 115200
******************/

#include <AltSoftSerial.h>
#include <Adafruit_Fingerprint.h>

/* ---------- Serial objects ---------- */
AltSoftSerial fpSerial;                  // D8 = RX (sensor TX), D9 = TX (sensor RX)
Adafruit_Fingerprint finger(&fpSerial);  // Fingerprint library uses AltSoftSerial

/* ---------- prototypes ---------- */
uint8_t getFingerprintEnroll(void);
uint8_t readnumber(void);

uint8_t id;   // ID chosen from Serial Monitor (1‒127)

/* ---------- setup ---------- */
void setup() {
  Serial.begin(115200);
  while (!Serial);                 // wait for USB
  delay(100);
  Serial.println(F("\nAdafruit Fingerprint sensor enrollment (AltSoftSerial D8/D9)"));

  fpSerial.begin(57600);           // ← الباود الذي نجح معك
  if (finger.verifyPassword()) {
    Serial.println(F("✔︎ Found fingerprint sensor!"));
  } else {
    Serial.println(F("✘ Sensor not found — check wiring"));
    while (true) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters:"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));   Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));   Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));   Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));  Serial.println(finger.baud_rate);
}

/* ---------- loop ---------- */
void loop() {
  Serial.println(F("\nReady to enroll a fingerprint!"));
  Serial.println(F("Type an ID (1–127) and press <Enter>:"));
  id = readnumber();
  if (id == 0) return;             // 0 not allowed
  Serial.print(F("Enrolling ID #")); Serial.println(id);

  while (! getFingerprintEnroll() );   // loop until success
}

/* ---------- helper: read number from USB ---------- */
uint8_t readnumber(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());   // wait for data
    num = Serial.parseInt();
  }
  return num;
}

/* ---------- helper: enrollment routine ---------- */
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print(F("Waiting for valid finger to enroll as #")); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:             Serial.println(F("Image taken")); break;
      case FINGERPRINT_NOFINGER:       Serial.print('.'); break;
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println(F("Communication error")); break;
      case FINGERPRINT_IMAGEFAIL:      Serial.println(F("Imaging error")); break;
      default:                         Serial.println(F("Unknown error")); break;
    }
  }

  /* 1st image → template buffer 1 */
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) { Serial.println(F("Image conversion failed")); return p; }

  Serial.println(F("Remove finger"));
  delay(1000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println(F("Place same finger again"));
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_PACKETRECIEVEERR) { Serial.println(F("Comm error")); return p; }
    if (p == FINGERPRINT_IMAGEFAIL)        { Serial.println(F("Imaging error")); return p; }
  }

  /* 2nd image → template buffer 2 */
  if (finger.image2Tz(2) != FINGERPRINT_OK) { Serial.println(F("2nd conversion failed")); return p; }

  /* Create model */
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println(F("Fingerprints did not match"));
    return p;
  }

  /* Store model */
  if (finger.storeModel(id) == FINGERPRINT_OK) {
    Serial.println(F("Stored successfully!"));
    return true;
  } else {
    Serial.println(F("Store failed"));
    return p;
  }
}
