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

    lv_obj_t* getFocusedButton() {
        if (!inputGroup) return nullptr;

        lv_obj_t* focused = lv_group_get_focused(inputGroup);
        if (!focused) return nullptr;

        const lv_obj_class_t* obj_class = lv_obj_get_class(focused);
        if (!obj_class) return nullptr;

        return (obj_class == &lv_btn_class) ? focused : nullptr;
    }

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

    // screenClass >>>>>>>>>>>>>>>>>>>
    virtual void keyboardEvent(String key) {

        // get the focused thing
        lv_obj_t* focused = lv_group_get_focused(inputGroup);

        // is it a list, is it scrolling ...
        if (focused && lv_obj_check_type(focused, &lv_list_class)) {
            lv_obj_t* list = focused;
            lv_obj_t* selected = nullptr;

            // find current selection in the list
            uint32_t count = lv_obj_get_child_cnt(list);
            for (uint32_t i = 0; i < count; ++i) {
                lv_obj_t* btn = lv_obj_get_child(list, i);
                if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                    selected = btn;
                    break;
                }
            }

            // start scrolling...
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
                //return; // test
            }

            // no enter # or * on selected item as it might not be what we want
            // let the child choose what to do on list enter
        } 
        
        // no scrolling, then are we navigating ?
        if (key == "C") {
            lv_group_focus_prev(inputGroup);
        } else if (key == "D") {
            lv_group_focus_next(inputGroup);

        // else are we clicking ENTER ... ?            
        } else if (key == "#") {
            if (focused) {
                lv_event_send(focused, LV_EVENT_PRESSED, NULL);  
            }

        // else esc ... 
        } else if (key == "*") {
            lv_group_send_data(inputGroup, LV_KEY_ESC);
        }

        // else we are done, child class could do something special to the screen
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
