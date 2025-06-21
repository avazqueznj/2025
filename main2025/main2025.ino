
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

#include <SDRAM.h>
#include <WiFi.h>

// configure lvgl
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"
#include "src/ui.h"

#include "util.hpp"
#include "state.hpp"

//----------------------------------------------------------
#include <SDRAM.h>
extern "C" void* lv_sdram_malloc(size_t size) {
  return SDRAM.malloc(size);
}
extern "C" void lv_sdram_free(void* ptr) {
  SDRAM.free(ptr);
}
extern "C" void* lv_sdram_realloc(void* ptr, size_t size) {
  // Optional: optimize this for your own allocator if needed
  void* new_ptr = SDRAM.malloc(size);
  if (ptr && new_ptr) {
    memcpy(new_ptr, ptr, size);  // may copy more than old size
    SDRAM.free(ptr);
  }
  return new_ptr;
}
//----------------------------------------------------------

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;
stateClass* stateManager = NULL;  
void setup() {
  SDRAM.begin();  // Initialize SDRAM first

  Serial.begin(9600);
  while (!Serial) {
    delayBlink();
  } 

  Display.begin();
  TouchDetector.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  
  create_screens();           
  stateManager = new stateClass();
  stateManager->openScreen( new mainScreenClass() );

  Serial.println("2025 init .... DONE!");  
}

//-----------------

extern char _end;
extern "C" char* sbrk(int incr);

int getFreeRam() {
    char stack_dummy;
    uintptr_t stack_top = (uintptr_t)&stack_dummy;
    uintptr_t heap_end = (uintptr_t)sbrk(0);
    return (int)(stack_top > heap_end ? stack_top - heap_end : heap_end - stack_top);
}

// paint loop
int refreshCounts = 0;
void loop() {
  delayBlink();
  lv_timer_handler(); 
  ui_tick();       

  refreshCounts += 1;
  if( refreshCounts == 50 ){
    
    Serial.println(getFreeRam());
    refreshCounts = 0;
   
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

