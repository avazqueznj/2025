/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/


#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "src/screens.h"
#include "src/actions.h"
#include <exception>
#include <WiFi.h>
#include "util.hpp"
#include "domainManager.hpp"
#include <deque>

//-------------------------------------------------

class screenClass{
public:
    lv_group_t* inputGroup = nullptr;
    ScreensEnum screenId;

    screenClass( ScreensEnum screenIdParam ): 
        screenId{screenIdParam}{

        inputGroup = lv_group_create();
    }

    virtual void open(){
        loadScreen( screenId );        
    };

    virtual void handleEvents( lv_event_t* e ){
        Serial.println("basescreen: event unhandled ...");  
    }

    virtual void rfidEvent( byte *uid, byte length ){
    }

    virtual void clockTic( String time ){
    }

    //---

    lv_obj_t* get_prev_sibling(lv_obj_t* obj) {
        if (!obj) return nullptr;
        lv_obj_t* parent = lv_obj_get_parent(obj);
        if (!parent) return nullptr;

        lv_obj_t* prev = nullptr;
        uint32_t count = lv_obj_get_child_cnt(parent);
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* child = lv_obj_get_child(parent, i);
            if (child == obj) {
                return prev;
            }
            prev = child;
        }
        return nullptr;
    }

    lv_obj_t* get_next_sibling(lv_obj_t* obj) {
        if (!obj) return nullptr;
        lv_obj_t* parent = lv_obj_get_parent(obj);
        if (!parent) return nullptr;

        bool found = false;
        uint32_t count = lv_obj_get_child_cnt(parent);
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* child = lv_obj_get_child(parent, i);
            if (found) {
                return child;
            }
            if (child == obj) {
                found = true;
            }
        }
        return nullptr;
    }

    virtual void keyboardEvent(String key) {
        Serial.print("Key: ");
        Serial.println(key);

        lv_obj_t* focused = lv_group_get_focused(inputGroup);
        if (focused && lv_obj_check_type(focused, &lv_list_class)) {
            lv_obj_t* list = focused;

            // Fallback: find selected button
            lv_obj_t* selected = nullptr;
            uint32_t count = lv_obj_get_child_cnt(list);
            for (uint32_t i = 0; i < count; ++i) {
                lv_obj_t* btn = lv_obj_get_child(list, i);
                if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                    selected = btn;
                    break;
                }
            }

            if (key == "A" || key == "B") {
                if (!selected) {
                    lv_obj_t* first = lv_obj_get_child(list, 0);
                    if (first) {
                        lv_obj_add_state(first, LV_STATE_CHECKED);
                        lv_obj_scroll_to_view(first, LV_ANIM_ON);
                        Serial.println("No item selected — selected first.");
                    }
                } else {
                    lv_obj_t* next = nullptr;
                    if (key == "A") {
                        next = get_prev_sibling(selected);
                    } else if (key == "B") {
                        next = get_next_sibling(selected);
                    }
                    if (next) {
                        lv_obj_clear_state(selected, LV_STATE_CHECKED);
                        lv_obj_add_state(next, LV_STATE_CHECKED);
                        lv_obj_scroll_to_view(next, LV_ANIM_ON);
                    }
                }
                return;
            }
        } 
        
        if (key == "C") {
            lv_group_focus_prev(inputGroup);
        } else if (key == "D") {
            lv_group_focus_next(inputGroup);
        } else if (key == "#") {
            if (focused) {
                lv_event_send(focused, LV_EVENT_PRESSED, NULL);  // not ideal - figure
                lv_event_send(focused, LV_EVENT_CLICKED, NULL);
            }
        } else if (key == "*") {
            lv_group_send_data(inputGroup, LV_KEY_ESC);
        }
    }

//---

    virtual ~screenClass(){
        if (inputGroup) {
            lv_group_del(inputGroup);
            inputGroup = nullptr;
            Serial.println("mainScreenClass: inputGroup destroyed");
        }        
    }
};

//-------------------------------------------------
// SCREEN BACKING STATES
//-------------------------------------------------


#include "settingsAndMainScreen.hpp"

#include "selectAssetScreen.hpp"

#include "selectInspectionTypeAndFormScreen.hpp"

#include "inspectionZonesScreen.hpp"


#endif 
