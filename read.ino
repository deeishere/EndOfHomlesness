#include <AltSoftSerial.h>
#include <Adafruit_Fingerprint.h>

AltSoftSerial fpSerial;                 // D8 = RX  (sensor TX)
                                        // D9 = TX  (sensor RX)
Adafruit_Fingerprint finger(&fpSerial);

/*  ------  prototypes  ------  */
uint8_t getFingerprintID(void);         // ← الإعلان المسبق يحلّ الخطأ

void setup() {
  Serial.begin(115200);
  fpSerial.begin(57600);
  delay(350);                           // إقلاع المستشعر

  Serial.println(F("\n--- R307 READ TEST ---"));
  if (!finger.verifyPassword()) {
    Serial.println(F("✘ Sensor not found")); while (true);
  }
  Serial.println(F("Scan a finger…"));
  pinMode(LED_BUILTIN, OUTPUT);         // مؤشر نجاح
}

void loop() {
  uint8_t id = getFingerprintID();      // أصبح معروفًا الآن
  if (id) {
    Serial.print(F("✅  Found ID #")); Serial.println(id);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

/*  ------  function body  ------  */
uint8_t getFingerprintID(void) {
  if (finger.getImage()         != FINGERPRINT_OK)  return 0;
  if (finger.image2Tz()         != FINGERPRINT_OK)  return 0;
  if (finger.fingerFastSearch() != FINGERPRINT_OK)  return 0;
  return finger.fingerID;   // يُرجِع رقم الـ ID المطابق
}
