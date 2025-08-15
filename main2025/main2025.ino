/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 * Portions of this software are based on LVGL (https://lvgl.io),
 * which is licensed under the MIT License.
 *
 ********************************************************************************************/

#include "mbed.h"
#include <SDRAM.h>
#include <WiFi.h>

// configure lvgl
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"
#include "src/ui.h"

#include "util.hpp"
#include "state.hpp"

#include <SPI.h>
#include <MFRC522.h>

// RFID Pins
#define SS_PIN 10  // SDA pin on RC522
#define RST_PIN 9  // Back to D9 for RST

//----------------------------------------------------------
// LVGL pool guard (global)
static void* lvgl_sdram_pool = nullptr;

extern "C" void* lvgl_get_sdram_pool() {
  return lvgl_sdram_pool;  // Return whatever we reserved at setup
}
//----------------------------------------------------------

Arduino_H7_Video* Display = nullptr;
Arduino_GigaDisplayTouch* TouchDetector = nullptr;
RTC_DS3231* rtc = nullptr;
MFRC522* mfrc522 = nullptr;

stateClass* stateManager = nullptr;  
bool rtcUp = false;
bool startedUp = false;

// === FIXED KEYPAD ===
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Using A0–A7 for keypad
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, A6, A7};

const int ANALOG_THRESHOLD = 900;
const int DEBOUNCE_MS = 300;
unsigned long lastPressTime = 0;

void setup() {

  Serial.begin(9600);
  int serialWait = 0;
  while (!Serial) {    
    serialWait += 100;
    if (serialWait > 10) break;
    delay( 100 );
  } 

  Serial.println("Coming UP----------------->");

// SDRAM -----------------------------

  //----------------------------------------------------------
  // SDRAM INIT + LVGL POOL alloc

  Serial.println("SDRAM: begin...");
  bool sdram_ok = SDRAM.begin();
  Serial.print("SDRAM.begin() = ");
  Serial.println(sdram_ok ? "OK" : "FAIL");

  if (!sdram_ok) {
    Serial.println("SDRAM init failed! Will halt.");
    sosBlink();
  }

  // Let SDRAM settle
  delay(500);

  // Try to allocate big pool
  const size_t POOL_SIZE = 3U * 1024U * 1024U; // 3 MB
  Serial.print("Allocating LVGL pool of ");
  Serial.print(POOL_SIZE);
  Serial.println(" bytes...");

  for (int tries = 0; tries < 3; ++tries) {
    lvgl_sdram_pool = SDRAM.malloc(POOL_SIZE);
    if (lvgl_sdram_pool) break;
    Serial.println("SDRAM.malloc failed, retrying...");
    delay(100);
  }

  if (!lvgl_sdram_pool) {
    Serial.println("SDRAM.malloc failed after retries! HALT.");
    sosBlink();
  }

  Serial.print("LVGL pool at 0x");
  Serial.println((uintptr_t)lvgl_sdram_pool, HEX);
  //----------------------------------------------------------
  
// /SDRAM -----------------------------

  pinMode(LED_BUILTIN, OUTPUT);   
  Serial.println("Disp");
  Display = new Arduino_H7_Video(800, 480, GigaDisplayShield);
  Display->begin();

  Serial.println("Touch");
  TouchDetector = new Arduino_GigaDisplayTouch();
  TouchDetector->begin();

  Serial.println("SPI");
  SPI.begin();

  Serial.println("RFID");
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init();
  mfrc522->PCD_DumpVersionToSerial();

  Serial.println("RTC");
  rtc = new RTC_DS3231();
  if (!rtc->begin()) {
    Serial.println("Couldn't find RTC clock!!!!");
  } else {
    if (rtc->lostPower()) {
      Serial.println("RTC lost power, setting default time ...");
      rtc->adjust(DateTime(2025, 7, 4, 12, 0, 0));
    }
    rtcUp = true;
  }

  // Init keypad pins
  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], LOW);
  }
  for (byte i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT);
  }

  startedUp = true;
  Serial.println("Coming UP-----------------> Done!");

  delay(100);
  Serial.println("Start screens  ...");
  create_screens();           

  stateManager = new stateClass();  
  stateManager->openScreen(new loginScreenClass());

  try{
      domainManagerClass::getInstance()->loadConfigFromKVStore();
  }catch( const std::runtime_error& error ){
      Serial.println( error.what() );            
      createDialog( error.what() );     
  }  


  Serial.println("Start screens / 2025 init .... DONE!");
}

// -----------------------------

void getInternalHeapFreeBytes() {
  mbed_stats_heap_t heap_stats;
  mbed_stats_heap_get(&heap_stats);    
  const int total_internal_ram = 491520;  
  int heap_used = heap_stats.current_size;
  int heap_free = total_internal_ram - heap_used;
  if (heap_free < 0) heap_free = 0;
  Serial.print("Heap free: ");
  Serial.println(heap_free);
}

byte currentCardUID[10];
byte currentCardLength = 0;

int slowPollInterval = 0;
int memStatReportInterval = 0;

void loop() {

  // wait for start
  if (!startedUp) {
    delay(100);
    return;
  }

  // general delay  - render
  delayBlink();  // 50MSEC *********************
  lv_timer_handler(); 
  ui_tick();   

  memStatReportInterval += 1;
  if (memStatReportInterval == ( 20 + 20 + 20 ) ) {  // 3 sec 
    getInternalHeapFreeBytes();
    memStatReportInterval = 0;   
  }

  slowPollInterval += 1;
  if (slowPollInterval ==  2 ) { // x main delay = 100msec

    slowPollInterval = 0;   

    //---------------------------------------------------------
    // rfid
    if (mfrc522 && mfrc522->PICC_IsNewCardPresent()) {
      if (mfrc522->PICC_ReadCardSerial()) {
        currentCardLength = mfrc522->uid.size;
        for (byte i = 0; i < currentCardLength; i++) {
          currentCardUID[i] = mfrc522->uid.uidByte[i];
        }

        Serial.print("Card UID:");
        for (byte i = 0; i < currentCardLength; i++) {
          Serial.print(":");
          Serial.print(currentCardUID[i]);
        }
        Serial.println();

        mfrc522->PICC_HaltA();
        mfrc522->PCD_StopCrypto1();

        stateManager->rfidEvent(currentCardUID, currentCardLength);
      }
    }

    //---------------------------------------------------------
    // rtc
    if (rtcUp && rtc) {
      DateTime now = rtc->now();
      char buffer[20];
      sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
              now.year(),
              now.month(),
              now.day(),
              now.hour(),
              now.minute(),
              now.second());
      String time = String(buffer);
      stateManager->clockTic(time); 
    }

    //---------------------------------------------------------
    // keypad voltage check
    for (byte row = 0; row < ROWS; row++) {
      digitalWrite(rowPins[row], HIGH);
      for (byte col = 0; col < COLS; col++) {
        int val = analogRead(colPins[col]);
        if (val > ANALOG_THRESHOLD) {
          unsigned long now = millis();
          if (now - lastPressTime > DEBOUNCE_MS) {
            char key = hexaKeys[row][col];
            stateManager->keyboardEvent(String(key));
            lastPressTime = now;
          }
        }
      }

      digitalWrite(rowPins[row], LOW);
    }

    //-----------------------------------------------

    // LVGL should do this!!
    if (stateManager != nullptr && stateManager->currentScreenState != nullptr) {
        // get screen
        screenClass* screen = stateManager->currentScreenState;
        // is kb open ??
        formFieldsScreenClass* ff = static_cast<formFieldsScreenClass*>(screen);
        if (ff != nullptr && ff->kb != nullptr && !lv_obj_has_flag(ff->kb, LV_OBJ_FLAG_HIDDEN)) {
            screen->checkTextAreaInView(); // check that lvgl did not overlap the entry area with the kb
        }
    }

  //------------------------------------------

    
  }



} //<< END



