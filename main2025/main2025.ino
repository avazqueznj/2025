
/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * 2025 
 * Zonar systems
 * v1 (2025) Alejandro Vazquez 
 * 
 * This source code is the confidential and proprietary information of [Zonar Systems]
 * ("Confidential Information"). You shall not disclose such Confidential Information and
 * shall use it only in accordance with the terms of the license agreement you entered into
 * with [Zonar Systems].
 * 
 * © [2025] [Zonar Systems]. All rights reserved.
 * 
 * NOTICE: All information contained herein is, and remains the property of [Zonar Systems]
 * and its suppliers, if any. The intellectual and technical concepts contained herein are
 * proprietary to [Zonar Systems] and its suppliers and may be covered by U.S. and Foreign
 * Patents, patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is strictly forbidden
 * unless prior written permission is obtained from [Zonar Systems].
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
      while (true);  // Halt safely
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
RTC_DS1307* rtc = nullptr;
MFRC522* mfrc522 = nullptr;

stateClass* stateManager = nullptr;  
bool rtcUp = false;
bool startedUp = false;

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
  rtc = new RTC_DS1307();
  if (!rtc->begin()) {
    Serial.println("Couldn't find RTC clock!!!!");
  } else {
    if (!rtc->isrunning()) {
      Serial.println("RTC is NOT running, default time ....");
      rtc->adjust(DateTime(2025, 7, 4, 12, 0, 0)); // YYYY, MM, DD, HH, MM, SS
    }
    rtcUp = true;
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

  refreshCounts += 1;
  if (refreshCounts == 50) {  
    // getInternalHeapFreeBytes();
    refreshCounts = 0;   
  } 

  RFIDrefreshCounts += 1;
  if (RFIDrefreshCounts == 5) {  
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


