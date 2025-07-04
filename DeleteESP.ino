#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Define pins for R307
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial serialPort(2); // use UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

uint8_t readNumber();
uint8_t deleteFingerprint(uint8_t id);

void setup()
{
  Serial.begin(9600);
  while (!Serial); // For boards like Zero, Yun, etc.
  delay(100);
  Serial.println("\n\nR307 Fingerprint sensor - DELETE MODE");

  serialPort.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN); // Start UART2
  delay(100);
  finger.begin(57600);
  delay(100);

  Serial.println("Testing sensor connection...");

  if (finger.verifyPassword()) {
    Serial.println("Found R307 fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    Serial.println("Check wiring:");
    Serial.println("VCC -> 5V, GND -> GND");
    Serial.println("TXD -> GPIO16, RXD -> GPIO17");
    while (1) {
      delay(1);
    }
  }

  Serial.println(F("Ready to delete stored fingerprint templates."));
}

void loop()
{
  Serial.println("\nType the ID # (1 to 127) you want to delete:");
  uint8_t id = readNumber();
  if (id == 0) {
    Serial.println("Invalid ID. Must be between 1 and 127.");
    return;
  }

  Serial.print("Deleting fingerprint ID #");
  Serial.println(id);

  uint8_t result = deleteFingerprint(id);

  switch (result) {
    case FINGERPRINT_OK:
      Serial.println("Successfully deleted.");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error.");
      break;
    case FINGERPRINT_BADLOCATION:
      Serial.println("Invalid location. No fingerprint exists at that ID.");
      break;
    case FINGERPRINT_FLASHERR:
      Serial.println("Flash write error.");
      break;
    default:
      Serial.print("Unknown error: 0x");
      Serial.println(result, HEX);
      break;
  }
}

uint8_t readNumber(void)
{
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t deleteFingerprint(uint8_t id)
{
  return finger.deleteModel(id);
}
