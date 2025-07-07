
/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * 2025 
 * v1 (2025) Alejandro Vazquez 
 * 
 * This source code is the confidential and proprietary information of [Alejandro Vazquez]
 * ("Confidential Information"). You shall not disclose such Confidential Information and
 * shall use it only in accordance with the terms of the license agreement you entered into
 * with [Zonar Systems].
 * 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
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
#include <SDRAM.h>

#include <SPI.h>
#include <MFRC522.h>

// RFID Pins
#define SS_PIN 10  // SDA pin on RC522
#define RST_PIN 9  // RST pin on RC522

//----------------------------------------------------------
// LVGL pool guard
static void* lvgl_sdram_pool = nullptr;
extern "C" void* lvgl_get_sdram_pool() {
  if (!lvgl_sdram_pool) {
    Serial.println("[POOL] Alloc requested...");
    lvgl_sdram_pool = SDRAM.malloc(3U * 1024U * 1024U);
    if (!lvgl_sdram_pool) {
      Serial.println("SDRAM.malloc failed!");
      while (true);  // abort
    } else {
      Serial.print("LVGL pool at 0x");
      Serial.println((uintptr_t)lvgl_sdram_pool, HEX);
    }
  }
  return lvgl_sdram_pool;
}



//----------------------------------------------------------


// All peripherals as pointers to ensure any malloc after sdram is setup
Arduino_H7_Video* Display = nullptr;
Arduino_GigaDisplayTouch* TouchDetector = nullptr;
RTC_DS3231* rtc = nullptr;
MFRC522* mfrc522 = nullptr;

stateClass* stateManager = nullptr;  
bool rtcUp = false;
bool startedUp = false;

#include <Keypad.h>

// Keypad matrix
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};
Keypad* keypad = nullptr;
void my_keypad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data) {
  char key = keypad->getKey();
  uint32_t lvgl_key = 0;

  if (key) {
    switch (key) {
      case 'A': lvgl_key = LV_KEY_UP; break;
      case 'B': lvgl_key = LV_KEY_DOWN; break;
      case 'C': lvgl_key = LV_KEY_LEFT; break;
      case 'D': lvgl_key = LV_KEY_RIGHT; break;
      case '#': lvgl_key = LV_KEY_ENTER; break;
      case '*': lvgl_key = LV_KEY_ESC; break;
      default: lvgl_key = 0; break; // Numbers handled elsewhere
    }
    data->state = LV_INDEV_STATE_PR;
    data->key = lvgl_key;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

//----------------------------------------------------------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);    

  Serial.begin(9600);
  int serialWait = 0;
  while (!Serial) {    
    serialWait += 100;
    if (serialWait > 5000) break;
  } 

  Serial.println("Coming UP----------------->");

  Serial.println("SDRAM");
  SDRAM.begin();         // Must be FIRST!

  delay( 100 );

  Serial.println("Disp");
  Display = new Arduino_H7_Video(800, 480, GigaDisplayShield);  // AFTER SDRAM ready
  Display->begin();

  Serial.println("Touch");
  TouchDetector = new Arduino_GigaDisplayTouch();               // AFTER SDRAM ready
  TouchDetector->begin();

  Serial.println("SPI");
  SPI.begin();            // Uses default SPI bus

  Serial.println("RFID");
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);  // AFTER SPI ready
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

  keypad = new Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
  if( keypad ){
    for (byte i = 0; i < COLS; i++) {
      pinMode(colPins[i], INPUT_PULLUP);
    }

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = my_keypad_read_cb;  // Your read callback
    lv_indev_t * indev_keypad = lv_indev_drv_register(&indev_drv);
  }

  startedUp = true;
  Serial.println("Coming UP-----------------> Done!");

  delay(100);
  Serial.println("Start screens  ...");
  create_screens();           
  stateManager = new stateClass();
  stateManager->openScreen(new mainScreenClass());
  Serial.println("Start screens / 2025 init .... DONE!");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//-----------------
// Memory info helper
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

byte currentCardUID[10];     // Global buffer for UID
byte currentCardLength = 0;  // Global length

// Main loop
int RFIDrefreshCounts = 0;
int refreshCounts = 0;

void loop() {

  if (!startedUp) {
    delay(100);
    return;
  }

  delayBlink();
  lv_timer_handler(); 
  ui_tick();   

  // monitor memory
  refreshCounts += 1;
  if (refreshCounts == 200) {  
    getInternalHeapFreeBytes();
    refreshCounts = 0;   
  } 

  // monitor card scanner
  RFIDrefreshCounts += 1;
  if (RFIDrefreshCounts == 50) {  
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

    // update time
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

    // monitor keyboard
    if( keypad ){
      char key = keypad->getKey();
      if (key) {
        Serial.print("Key:");
        Serial.println(key);
        stateManager->keyboardEvent( String( key ) );
      }      
    }

    RFIDrefreshCounts = 0;   
  } 
}


//----------------------------------------------------------
//----------------------------------------------------------


// setup eez
// in eez peoject override location of lvgl heder -> "lvglInclude": "lvgl.h"
// in sketch add folder src, and copy the code
// set code output to sketch path, so it makes a src folder with all the files
// in ino, include ui.h
// lv_conf in C:\Users\alejandro.vazquez\AppData\Local\Arduino15\packages\arduino\hardware\mbed_giga\4.3.1\libraries\Arduino_H7_Video\src


