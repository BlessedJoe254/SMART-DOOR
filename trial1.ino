#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define SDA_PIN 10
#define RST_PIN 9
#define GREEN_LED 6
#define RED_LED 7
#define BUZZER 8
#define SERVO_PIN 5

MFRC522 rfid(SDA_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD Address 0x27 (check your LCD address)
Servo doorServo;

bool doorOpen = false;  // Track state of the door
int currentServoPos = 0; // Track current servo position
String lastCardUID = ""; // Track the last card used
bool justWelcomed = false; // Track if we just showed welcome message

// List of authorized UIDs
String authorizedUIDs[] = {
  "C2DA54D9",  // Eng. Blessed Joe
  "12345678",  // Add other UIDs as needed
};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.begin(16, 2);  // Initialize LCD with 16 columns and 2 rows
  lcd.backlight();   // Turn on backlight

  doorServo.attach(SERVO_PIN);
  doorServo.write(0); // Door closed position
  currentServoPos = 0;

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initial beep sequence
  beep(1000, 200);  // High beep
  delay(100);
  beep(800, 200);   // Medium beep
  delay(100);
  beep(600, 200);   // Low beep

  // Initial message
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Project JP");
  delay(2000);  // Show the welcome message for 2 seconds
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Place your card");
  lcd.setCursor(0, 1);
  lcd.print("to open door");
}

void beep(int frequency, int duration) {
  tone(BUZZER, frequency, duration);
  delay(duration);
  noTone(BUZZER);
}

void moveServoSmoothly(int targetPos) {
  int step = (targetPos > currentServoPos) ? 1 : -1;
  
  while (currentServoPos != targetPos) {
    currentServoPos += step;
    doorServo.write(currentServoPos);
    delay(15);  // Adjust this delay to change the speed
  }
}

void loop() {
  // Check for new card
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    Serial.print("Card UID: ");
    Serial.println(uid);
    lcd.clear();
    beep(800, 100);  // Short beep when card is detected

    // Check if UID is authorized
    if (isAuthorized(uid)) {
      if (!doorOpen) {
        // Opening the door
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        beep(1200, 300);  // High-pitched success beep

        lcd.setCursor(0, 0);
        lcd.print("Door opening");
        moveServoSmoothly(90); // Open door slowly
        doorOpen = true;
        lastCardUID = uid;
        justWelcomed = false;

        // Play opening sequence beeps
        for (int i = 0; i < 3; i++) {
          beep(1000 + (i * 100), 100);
          delay(50);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Place your card");
        lcd.setCursor(0, 1);
        lcd.print("to close door");
      } else {
        // Closing the door
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        beep(1200, 300);  // High-pitched success beep

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Door closing");
        moveServoSmoothly(0);  // Close door slowly
        doorOpen = false;

        // Play closing sequence beeps
        for (int i = 3; i > 0; i--) {
          beep(1000 + (i * 100), 100);
          delay(50);
        }

        lcd.clear();
        if (uid == lastCardUID && justWelcomed) {
          lcd.setCursor(0, 0);
          lcd.print("See you again");
          lcd.setCursor(0, 1);
          lcd.print("Engineer");
          beep(1000, 100);
          delay(50);
          beep(800, 100);
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Welcome Eng. Joe");
          beep(1000, 100);  // Welcome beep
          delay(50);
          beep(1200, 100);
          justWelcomed = true;
        }

        // 2-second delay before starting again
        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Place your card");
        lcd.setCursor(0, 1);
        lcd.print("to open door");
      }
    } else {
      // Unauthorized card
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      beep(400, 100);  // Low error beep
      delay(50);
      beep(300, 500);  // Longer low error beep

      lcd.setCursor(0, 0);
      lcd.print("Access Denied");

      delay(1000); // Wait for 1 second
      digitalWrite(RED_LED, LOW);

      lcd.clear();
      if (doorOpen) {
        lcd.setCursor(0, 0);
        lcd.print("Place your card");
        lcd.setCursor(0, 1);
        lcd.print("to close door");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Place your card");
        lcd.setCursor(0, 1);
        lcd.print("to open door");
      }
    }

    // Halt the card to prevent it from being read again
    rfid.PICC_HaltA();
  }
}

bool isAuthorized(String scannedUID) {
  // Check if the scanned UID matches any of the authorized UIDs
  for (String uid : authorizedUIDs) {
    if (uid == scannedUID) return true;
  }
  return false;
}