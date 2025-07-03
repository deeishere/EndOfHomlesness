#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define RX_PIN 16
#define TX_PIN 17

HardwareSerial serialPort(2);
Adafruit_Fingerprint finger(&serialPort);

uint8_t id = 0;

uint8_t readNumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t enrollFingerprint(uint8_t id) {
  int p = -1;
  Serial.print("Waiting for finger to enroll as ID ");
  Serial.println(id);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK: Serial.println("Image taken"); break;
      case FINGERPRINT_NOFINGER: Serial.print("."); break;
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); break;
      case FINGERPRINT_IMAGEFAIL: Serial.println("Imaging error"); break;
      default: Serial.println("Unknown error"); break;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image");
    return p;
  }
  Serial.println("Remove finger");
  delay(2000);

  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place same finger again");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK: Serial.println("Image taken"); break;
      case FINGERPRINT_NOFINGER: Serial.print("."); break;
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println("Communication error"); break;
      case FINGERPRINT_IMAGEFAIL: Serial.println("Imaging error"); break;
      default: Serial.println("Unknown error"); break;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert second image");
    return p;
  }

  Serial.println("Creating model...");
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to create model");
    return p;
  }

  Serial.print("Storing model at ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to store model");
    return p;
  }

  Serial.println("Enrollment successful!");
  return FINGERPRINT_OK;
}

void setup() {
  Serial.begin(9600);
  serialPort.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  finger.begin(57600);

  Serial.println("\n\nFingerprint sensor starting...");
  if (!finger.verifyPassword()) {
    Serial.println("Could not find fingerprint sensor :(");
    while (1) delay(1);
  }
  Serial.println("Fingerprint sensor found!");
}

void loop() {
  if (Serial.available()) {
    Serial.println("Enter ID (1-127) to enroll, or 0 to skip:");
    id = readNumber();
    if (id >= 1 && id <= 127) {
      enrollFingerprint(id);
    } else {
      Serial.println("Skipping enrollment.");
    }
    while (Serial.available()) Serial.read(); // clear input buffer
  } else {
    // Detection mode
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return;

    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK) {
      Serial.print("Found ID #");
      Serial.print(finger.fingerID);
      Serial.print(" with confidence ");
      Serial.println(finger.confidence);
    } else {
      Serial.println("No match found");
    }
    delay(1000);
  }
}
