#include <SPI.h>
#include <MFRC522.h>

// Define RFID pins
#define SDA_PIN 10
#define RST_PIN 9

// Initialize RFID
MFRC522 rfid(SDA_PIN, RST_PIN);

void setup() {
  Serial.begin(9600); // Start Serial Monitor
  SPI.begin();        // Start SPI communication
  rfid.PCD_Init();    // Initialize RFID module

  Serial.println("Place your RFID card near the scanner...");
}

void loop() {
  // Check if a new card is detected
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Card UID: ");

  // Print the UID
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Halt the card for further processing
  rfid.PICC_HaltA();
}