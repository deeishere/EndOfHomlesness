#include <Arduino.h>
#include <FPM.h>
#include <HardwareSerial.h>

// Pin definitions for R307
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial fpmSerial(2);
FPM finger(&fpmSerial);

void setup() {
  Serial.begin(9600);
  Serial.println("=== R307 Fingerprint Enrollment (FPM Library) ===");
  
  // Initialize UART2 for R307
  fpmSerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(100);
  
  // Initialize FPM library
  if (finger.begin()) {
    Serial.println("✓ R307 sensor found!");
  } else {
    Serial.println("✗ R307 sensor not found!");
    Serial.println("Check wiring and power supply");
    while (1) delay(1);
  }
  
  // Get sensor parameters
  FPM_System_Params params;
  if (finger.readParams(&params) == FPM_OK) {
    Serial.println("\n--- R307 Parameters ---");
    Serial.print("Status: 0x"); Serial.println(params.status_reg, HEX);
    Serial.print("System ID: 0x"); Serial.println(params.system_id, HEX);
    Serial.print("Capacity: "); Serial.println(params.capacity);
    Serial.print("Security Level: "); Serial.println(params.security_level);
    Serial.print("Device Address: 0x"); Serial.println(params.device_addr, HEX);
    Serial.print("Packet Length: "); Serial.println(params.packet_len);
    Serial.print("Baud Rate: "); Serial.println(params.baud_rate);
    Serial.println("--- End Parameters ---\n");
  }
}

void loop() {
  Serial.println("=== Fingerprint Enrollment ===");
  Serial.println("Enter ID (1-1000) to enroll fingerprint:");
  
  // Wait for user input
  while (!Serial.available()) delay(10);
  int id = Serial.parseInt();
  
  if (id < 1 || id > 1000) {
    Serial.println("Invalid ID! Use 1-1000");
    return;
  }
  
  Serial.print("Enrolling fingerprint with ID: ");
  Serial.println(id);
  
  if (enrollFingerprint(id)) {
    Serial.println("✓ Fingerprint enrolled successfully!");
  } else {
    Serial.println("✗ Fingerprint enrollment failed!");
  }
  
  Serial.println("Ready for next enrollment...\n");
  delay(2000);
}

bool enrollFingerprint(int id) {
  Serial.println("Place finger on sensor...");
  
  // Step 1: Get first image
  int result = finger.getImage();
  if (result != FPM_OK) {
    Serial.println("Failed to get first image");
    return false;
  }
  Serial.println("✓ First image captured");
  
  // Convert first image to template
  result = finger.image2Tz(1);
  if (result != FPM_OK) {
    Serial.println("Failed to convert first image");
    return false;
  }
  Serial.println("✓ First template created");
  
  Serial.println("Remove finger...");
  delay(2000);
  
  // Wait for finger removal
  while (finger.getImage() == FPM_OK) {
    delay(100);
  }
  
  Serial.println("Place same finger again...");
  
  // Step 2: Get second image
  while (finger.getImage() != FPM_OK) {
    delay(100);
  }
  Serial.println("✓ Second image captured");
  
  // Convert second image to template
  result = finger.image2Tz(2);
  if (result != FPM_OK) {
    Serial.println("Failed to convert second image");
    return false;
  }
  Serial.println("✓ Second template created");
  
  // Step 3: Create model from both templates
  result = finger.createModel();
  if (result != FPM_OK) {
    Serial.println("Failed to create model - fingerprints don't match");
    return false;
  }
  Serial.println("✓ Fingerprint model created");
  
  // Step 4: Store model
  result = finger.storeModel(id, 1);
  if (result != FPM_OK) {
    Serial.print("Failed to store model. Error: ");
    Serial.println(result);
    return false;
  }
  Serial.println("✓ Fingerprint stored successfully");
  
  return true;
}

// Additional utility functions
void searchFingerprint() {
  Serial.println("Place finger to search...");
  
  if (finger.getImage() != FPM_OK) {
    Serial.println("Failed to get image");
    return;
  }
  
  if (finger.image2Tz(1) != FPM_OK) {
    Serial.println("Failed to convert image");
    return;
  }
  
  FPM_Search_Params params;
  int result = finger.searchDatabase(1, 1, 1000, &params);
  
  if (result == FPM_OK) {
    Serial.print("Found fingerprint! ID: ");
    Serial.print(params.finger_id);
    Serial.print(", Confidence: ");
    Serial.println(params.score);
  } else {
    Serial.println("Fingerprint not found");
  }
}

void deleteFingerprint(int id) {
  int result = finger.deleteModel(id);
  if (result == FPM_OK) {
    Serial.print("Deleted fingerprint ID: ");
    Serial.println(id);
  } else {
    Serial.println("Failed to delete fingerprint");
  }
}

void emptyDatabase() {
  int result = finger.emptyDatabase();
  if (result == FPM_OK) {
    Serial.println("Database cleared successfully");
  } else {
    Serial.println("Failed to clear database");
  }
}
