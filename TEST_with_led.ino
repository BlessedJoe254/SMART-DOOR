#include <SPI.h>
#include <MFRC522.h>

#define SDA_PIN 10
#define RST_PIN 9
#define LED_PIN 8  // LED connected to pin 8

MFRC522 rfid(SDA_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED off

  Serial.println("Place your RFID card near the scanner...");
}

void loop() {
  // Wait for a new card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Card UID: ");
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");

    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Match against known UID: C2 DA 54 D9
  if (uid.equalsIgnoreCase("C2DA54D9")) {
    Serial.println("Access Granted");
    digitalWrite(LED_PIN, HIGH); // Turn on LED
  } else {
    Serial.println("Access Denied");
    digitalWrite(LED_PIN, LOW); // Turn off LED
  }

  rfid.PICC_HaltA();
}
