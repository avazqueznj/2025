#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10  // SDA pin on RC522
#define RST_PIN 9  // RST pin on RC522

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {

  Serial.begin(9600);
  int serialWait = 0;
  while (!Serial) {
    delayBlink();   
    serialWait += 100;
    if( serialWait > 5000 ) break;
  } 

  Serial.println("UP----------------->");

  SPI.begin();            // Uses default SPI bus
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  Serial.println("UP-----------------> Done!");

}

void loop() {
  //Serial.println("Looking for card...");


  if (mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("Card detected!");

    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();
      delay(1000);  // Small delay so it doesn't flood the output
    }
  }
  

  delayBlink();
}

void delayBlink() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
}

