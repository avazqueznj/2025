#include "core/lv_event.h"
/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/

class selectInspectionTypeScreenClass:public screenClass{
public:

    selectInspectionTypeScreenClass(): screenClass( SCREEN_ID_SELECT_INSPECTION_TYPE ){            
    }

    void handleEvents( lv_event_t* e ) override{
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event

        // make sure one button is selected at a time
        if (lv_obj_check_type(target, &lv_btn_class)) {

            uint32_t child_count = lv_obj_get_child_cnt(objects.inspection_types);
            for (uint32_t i = 0; i < child_count; ++i) {
                lv_obj_t* btn = lv_obj_get_child(objects.inspection_types, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                if (btn == target) {
                    Serial.println("inspection type: click button ...");
                    // LVGL toggles state automatically
                } else {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                }
            }

            Serial.println("inspection type: click button DONE");
            return;
        }     

        //------------------

    }


    void keyboardEvent(String key) override {        
     
        // Check for # key and if inspection_types is focused
        if (key == "#") {
            lv_obj_t* focused = lv_group_get_focused(inputGroup);
            if (focused == objects.inspection_types) {
                lv_event_send(objects.do_inspection_form, LV_EVENT_PRESSED, NULL);
                return;
            }
        }

        screenClass::keyboardEvent(key);
    }


    void open() override {

        domainManagerClass* domain = domainManagerClass::getInstance();

        // LOAD the LIST from ASSETS ...

        // Clear the list in case it has old items
        lv_obj_clean(objects.inspection_types);

        // For each inspection type, check if asset matches ALL selected assets
        for (const inspectionTypeClass& type : domain->inspectionTypes) {
            bool valid = true;

            for (const assetClass& asset : domain->currentInspection.assets) {
                bool found = false;

                for (const String& layout : type.layouts) {
                    if (layout == "ALL" || layout == asset.layoutName) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    valid = false;
                    break;
                }
            }

            if (valid) { // ie all assets share the type

                // Create button for this inspection type
                lv_obj_t* btn = lv_btn_create(objects.inspection_types);
                lv_obj_set_size(btn, 411, 84);
                lv_obj_add_event_cb(btn, action_main_event_dispatcher, LV_EVENT_PRESSED, static_cast<void*>(const_cast<inspectionTypeClass*>(&type)));
                lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(btn, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(btn, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

                lv_obj_set_user_data(btn, static_cast<void*>(const_cast<inspectionTypeClass*>(&type)));

                if (domain->currentInspection.type == &type) {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                }

                // Add label with inspection type name
                {
                    lv_obj_t* label = lv_label_create(btn);
                    lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(label, type.name.c_str());
                }
            }
        }

        {
            //-------------------------------------
            // Add focusable widgets

            // default
            lv_group_add_obj(inputGroup, objects.inspection_types  );

            // nav bar
            lv_group_add_obj(inputGroup, objects.do_inspection_form);            
            lv_group_add_obj(inputGroup, objects.back_from_select_insp);            

        }

        screenClass::open(); 
    }


   void syncToInspection() {

        domainManagerClass* domain = domainManagerClass::getInstance();

        // check inspe type ui
        uint32_t child_count = lv_obj_get_child_cnt(objects.inspection_types);
        for (uint32_t i = 0; i < child_count; ++i) {

            lv_obj_t* btn = lv_obj_get_child(objects.inspection_types, i);
            if (!lv_obj_check_type(btn, &lv_btn_class)) continue;
            // find selected
            if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                inspectionTypeClass* selectedType =
                    static_cast<inspectionTypeClass*>(lv_obj_get_user_data(btn));

                if (selectedType != NULL) {
                    domain->currentInspection.type = selectedType;
                    Serial.println("syncToInspection: Selected inspection type set by user_data.");
                } else {
                    Serial.println("syncToInspection: WARNING — user_data is NULL!");
                }
                return; // found checked → done
            }
        }

        // If none found checked, clear current type
        domain->currentInspection.type = NULL;
        throw std::runtime_error("Error: Please select an inspection type.");

    }

};


//-------------------------------------------------

    class formFieldsScreenClass : public screenClass {
    public:

        lv_obj_t* kb = NULL;                   
        std::vector<lv_obj_t*> textareas;     

        formFieldsScreenClass() : screenClass(SCREEN_ID_INSPECTION_FORM) {
        }

        void keyboardEvent(String key) override {        
            screenClass::keyboardEvent( key );

            // add key to values if it is numeric
            if( key != "A" && key != "B" && key != "C" && key != "D" && key != "*" && key != "#"  ){
                lv_obj_t* focused = lv_group_get_focused(inputGroup);
                if (focused && lv_obj_check_type(focused, &lv_textarea_class)) {
                    lv_textarea_add_text(focused, key.c_str());                    
                }
            }

            // use * as backspace
            if (key == "*") {
                lv_obj_t* focused = lv_group_get_focused(inputGroup);
                if (focused && lv_obj_check_type(focused, &lv_textarea_class)) {
                    String txt = lv_textarea_get_text(focused);  // copy the text
                    int len = txt.length();
                    if (len > 0) {
                        txt = txt.substring(0, len - 1);  // remove last character
                        //lv_textarea_set_text(focused, txt.c_str());
                        //lv_textarea_add_text(focused, "*" );   

                        // lvgl bug ??
                        // One for the ghost, one for the real char
                        lv_textarea_del_char( focused );
                        lv_textarea_del_char( focused );                                         
                    }
                }
            }

        }

        void handleEvents(lv_event_t* e) override {
        }

        void open() override {
            domainManagerClass* domain = domainManagerClass::getInstance();
            inspectionTypeClass* currentType = domain->currentInspection.type;
            inspectionClass* currentInspection = &domain->currentInspection;

            if (currentType == NULL) {
                throw std::runtime_error("Error: No inspection type selected!");
            }

            // Show the current inspection type name at top
            lv_label_set_text(objects.inspection_type_name, currentType->name.c_str());            

            // Clear old textarea handles
            textareas.clear();

            // Use  EEZ-generated list container
            lv_obj_t* parent_obj = objects.form_fields;
            lv_obj_clean(parent_obj);

            // Create floating keyboard 
            if (kb == NULL) {
                kb = lv_keyboard_create(objects.inspection_form);
                lv_obj_set_size(kb, 800, 200);
                lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
                lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

                lv_obj_add_event_cb(kb, [](lv_event_t* e) {
                    lv_event_code_t code = lv_event_get_code(e);
                    formFieldsScreenClass* self = static_cast<formFieldsScreenClass*>(lv_event_get_user_data(e));
                    if (code == LV_EVENT_CANCEL || code == LV_EVENT_READY) {
                        lv_obj_add_flag(self->kb, LV_OBJ_FLAG_HIDDEN);
                        Serial.println("Keyboard hidden (OK/Cancel).");
                    }
                }, LV_EVENT_ALL, this);
            }

            // Form Fields ====================================================

            for (size_t i = 0; i < currentType->formFields.size(); ++i) {
                const std::vector<String>& row = currentType->formFields[i];

                String fieldName = row[0];
                String fieldType = row[1];
                String fieldMax = row.size() >= 3 ? row[2] : "30";

                int maxLength = fieldMax.toInt();
                if (maxLength <= 0) maxLength = 30;

                // === Flex container ===
                lv_obj_t* rowContainer = lv_obj_create(parent_obj);
                lv_obj_set_size(rowContainer, 700, 72);
                lv_obj_set_style_pad_right(rowContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(rowContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(rowContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(rowContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(rowContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(rowContainer, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(rowContainer, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_left(rowContainer, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

                // === Label ===
                lv_obj_t* label = lv_label_create(rowContainer);
                lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(label, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label, fieldName.c_str());

                        // === Textarea ===
                        lv_obj_t* textarea = lv_textarea_create(rowContainer);
                        lv_obj_set_size(textarea, 388, 54);
                        lv_textarea_set_max_length(textarea, maxLength);

                        // If there’s a previous value, restore it
                        String prefill = "";
                        if (i < currentInspection->inspectionFormFieldValues.size()) {
                            prefill = currentInspection->inspectionFormFieldValues[i];
                        }
                        lv_textarea_set_text(textarea, prefill.c_str());

                        lv_textarea_set_one_line(textarea, true);
                        lv_textarea_set_password_mode(textarea, false);
                        lv_obj_clear_flag(textarea,
                            LV_OBJ_FLAG_SCROLLABLE |
                            LV_OBJ_FLAG_SCROLL_CHAIN_HOR |
                            LV_OBJ_FLAG_SCROLL_CHAIN_VER |
                            LV_OBJ_FLAG_SCROLL_ELASTIC |
                            LV_OBJ_FLAG_SCROLL_MOMENTUM |
                            LV_OBJ_FLAG_SCROLL_ON_FOCUS |
                            LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                        lv_obj_set_style_text_font(textarea, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(textarea, lv_color_hex(0xff7095c8), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_border_color(textarea, lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_FOCUSED);                        

                        // Save handle for sync
                        textareas.push_back(textarea);
                        lv_group_add_obj(inputGroup, textarea );

                        // Hook: show keyboard on focus
                        lv_obj_add_event_cb(textarea, 
                        
                            [](lv_event_t* e) { 

                                // get the form field
                                lv_obj_t* ta = lv_event_get_target(e);
                                formFieldsScreenClass* self = static_cast<formFieldsScreenClass*>(lv_event_get_user_data(e));

                                // unhide kb
                                lv_obj_clear_flag(self->kb, LV_OBJ_FLAG_HIDDEN);
                                lv_keyboard_set_textarea(self->kb, ta);                                

                                Serial.println("Keyboard opened for textarea.");
                            }

                        , LV_EVENT_PRESSED, this);
            }

            // keyboard spacer ============
            lv_obj_t* spacer = lv_obj_create( objects.form_fields );  
            lv_obj_set_size(spacer, LV_PCT(100), LV_PCT(50));  
            lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE); 
            lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);                 

            {
                // default
                //lv_group_add_obj(inputGroup, objects.form_fields  );

                // nav bar
                lv_group_add_obj(inputGroup, objects.do_zones );            
                lv_group_add_obj(inputGroup, objects.back_from_form_fields);       
            }

            screenClass::open();
        }

        void syncToInspection() {
            domainManagerClass* domain = domainManagerClass::getInstance();
            inspectionClass* currentInspection = &domain->currentInspection; // FIXED
            inspectionTypeClass* currentType = currentInspection->type;

            if (currentType == NULL) {
                Serial.println("syncToInspection: No inspection type selected!");
                return;
            }

            currentInspection->inspectionFormFieldValues.clear(); // FIXED

            for (lv_obj_t* ta : textareas) {
                const char* input = lv_textarea_get_text(ta);
                currentInspection->inspectionFormFieldValues.push_back(String(input)); // FIXED
            }

            Serial.println("syncToInspection: Form field values saved:");
            for (size_t i = 0; i < currentInspection->inspectionFormFieldValues.size(); ++i) {
                Serial.print("  [");
                Serial.print(i);
                Serial.print("] ");
                Serial.println(currentInspection->inspectionFormFieldValues[i]);
            }
        }

        virtual ~formFieldsScreenClass() {
            if (kb != NULL) {
                lv_obj_del(kb);
                kb = NULL;
                Serial.println("Keyboard destroyed by formFieldsScreenClass destructor.");
            }
        }
    };

