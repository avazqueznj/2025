
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


#include <WiFi.h>

// configure lvgl
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"

#include "src/ui.h"
#include "controlManager.hpp"
#include "state.hpp"



//----------------------------------------------------------

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;
stateClass* stateManager;
void setup() {
  
  Display.begin();
  TouchDetector.begin();
    
  // initialize serial comms 
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    delayBlink();
  }

  create_screens();           
  stateManager = new stateClass();
  stateManager->openMainScreen();

  // done!!
  Serial.println("2025 init .... DONE!");  
}


//-----------------

// paint loop
void loop() {
  delayBlink();
  lv_timer_handler(); 
  ui_tick();       
}

//----------------------------------------------------------


#include "src/actions.h"

extern "C" void action_test_action1(lv_event_t *e) {
  lv_obj_t* btn = (lv_obj_t*) lv_event_get_target(e);
  lv_obj_t* label = (lv_obj_t*) lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "Clicked!");
  loadScreen( SCREEN_ID_SELECT_ASSET_SCREEN  );
}

extern "C" void action_main_menu_start_inspection(lv_event_t *e) {


}



//----------------------------------------------------------

void delayBlink(){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(50);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(50);                      // wait for a second  
}



//----------------------------------------------------------
//----------------------------------------------------------


// setup eez
// in eez peoject override location of lvgl heder -> "lvglInclude": "lvgl.h"
// in sketch add folder src, and copy the code
// set code output to sketch path, so it makes a src folder with all the files
// in ino, include ui.h
// lv_conf in C:\Users\alejandro.vazquez\AppData\Local\Arduino15\packages\arduino\hardware\mbed_giga\4.3.1\libraries\Arduino_H7_Video\src

