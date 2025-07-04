
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

#define SS_PIN 10  // SDA pin on RC522
#define RST_PIN 9  // RST pin on RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

//----------------------------------------------------------
static void* lvgl_sdram_pool = nullptr;
extern "C" void* lvgl_get_sdram_pool() {
  if (!lvgl_sdram_pool) {
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

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;
stateClass* stateManager = NULL;  
void setup() {
  
  Serial.begin(9600);
  int serialWait = 0;
  while (!Serial) {
    delayBlink();   
    serialWait += 100;
    if( serialWait > 5000 ) break;
  } 

  Serial.println("Coming UP----------------->");

  Serial.println("SDRAM");
  SDRAM.begin();         // Must be FIRST

  Serial.println("Disp");
  Display.begin();
  TouchDetector.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("SPI");
  SPI.begin();            // Uses default SPI bus

  Serial.println("RFID");
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println("Coming UP-----------------> Done!");

  create_screens();           
  stateManager = new stateClass();
  stateManager->openScreen( new mainScreenClass() );

  Serial.println("2025 init .... DONE!");  
}

//-----------------


void getInternalHeapFreeBytes() {
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);    
    const int total_internal_ram = 491520;  
    int heap_used = heap_stats.current_size;
    int heap_free = total_internal_ram - heap_used;
    if (heap_free < 0) heap_free = 0;
    Serial.print( "Heap free:" );
    Serial.println( heap_free );
}

byte currentCardUID[10];     // Global buffer for UID
byte currentCardLength = 0;  // Global length

// paint loop
int RFIDrefreshCounts = 0;
int refreshCounts = 0;
void loop() {
  delayBlink();
  lv_timer_handler(); 
  ui_tick();       

  refreshCounts += 1;
  if( refreshCounts == 50 ){  
    getInternalHeapFreeBytes();
    refreshCounts = 0;   
  } 

  RFIDrefreshCounts += 1;
  if( RFIDrefreshCounts == 5 ){  
   
    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
        currentCardLength = mfrc522.uid.size;  // Save length
        for (byte i = 0; i < currentCardLength; i++) {
          currentCardUID[i] = mfrc522.uid.uidByte[i];  // Save bytes
        }

        Serial.print("Card UID:");
        for (byte i = 0; i < currentCardLength; i++) {
          Serial.print(":");
          Serial.print(currentCardUID[i]);
        }
        Serial.println();

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();

        stateManager->rfidEvent( currentCardUID, currentCardLength );
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


