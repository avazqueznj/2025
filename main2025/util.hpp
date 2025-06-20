#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include "lvgl.h"

//----------------------------------------------

void delayBlink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
}

//----------------------------------------------

LV_FONT_DECLARE(lv_font_montserrat_28);
static lv_obj_t * overlay = NULL;
static lv_style_t style_font;
static bool style_ready = false;

static void btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        if (overlay) {
            lv_async_call((lv_async_cb_t)lv_obj_del, overlay);
            overlay = NULL;
        }
    }
}

void createDialog( const char* message )
{
    static const char * btns[] = { "OK", "" };

    // Create overlay
    overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Create message box
    lv_obj_t * mbox = lv_msgbox_create(overlay, "", message, btns, false);
    lv_obj_center(mbox);

    // One-time style init
    if (!style_ready) {
        lv_style_init(&style_font);
        lv_style_set_text_font(&style_font, &lv_font_montserrat_28);
        style_ready = true;
    }

    // Apply style to text and buttons
    lv_obj_add_style(lv_msgbox_get_text(mbox), &style_font, 0);
    lv_obj_t * btnm = lv_msgbox_get_btns(mbox);
    lv_obj_add_style(btnm, &style_font, 0);

    // Set callback on button matrix
    lv_obj_add_event_cb(btnm, btn_event_cb, LV_EVENT_ALL, NULL);
}

//----------------------------------------------

#include <vector>

std::vector<String> tokenize(String input, char delimiter) {

    Serial.println( input );

    std::vector<String> result;
    int start = 0;
    int end = input.indexOf(delimiter);

    while (end != -1) {
        result.push_back(input.substring(start, end));
        start = end + 1;
        end = input.indexOf(delimiter, start);
    }

    result.push_back(input.substring(start));
    return result;
}

//----------------------------------------------

#endif

