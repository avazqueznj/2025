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
    ScreensEnum screenId;

    screenClass( ScreensEnum screenIdParam ): 
        screenId{screenIdParam}{
    }

    virtual void open(){
        loadScreen( screenId );        
    };

    virtual void handleEvents( lv_event_t* e ){
        Serial.println("basescreen: event unhandled ...");  
    }

    virtual ~screenClass(){
    }

};




//-------------------------------------------------
// SCREEN BACKING STATE
//-------------------------------------------------


class settingsScreenClass:public screenClass{
public:

    settingsScreenClass(): screenClass( SCREEN_ID_SETTINGS ){    
    }

    void handleEvents( lv_event_t* e ) override{       
    }

    void open() override {
        lv_textarea_set_text( objects.setting_wifi_name,  domainManagerClass::getInstance()->comms->ssid.c_str() );
        lv_textarea_set_text( objects.setting_wifi_password, domainManagerClass::getInstance()->comms->pass.c_str()  );
        lv_textarea_set_text( objects.setting_server_url, domainManagerClass::getInstance()->comms->serverURL.c_str()  );
        screenClass::open();
    }

    virtual ~settingsScreenClass(){};
};


//-------------------------------------------------

class mainScreenClass:public screenClass{
public:

    mainScreenClass(): screenClass( SCREEN_ID_MAIN ){            
    }

    void handleEvents( lv_event_t* e ) override{
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event
        if(target == objects.do_sync ){
            try{

                Serial.println("main: sync ...");  
                domainManagerClass::getInstance()->sync();

                String syncMessage = "Sync successful. \n";
                syncMessage += "Loaded: \n";

                syncMessage += domainManagerClass::getInstance()->assets.size();
                syncMessage += " assets \n";


                syncMessage += domainManagerClass::getInstance()->layouts.size();
                syncMessage += " layouts \n";


                syncMessage += domainManagerClass::getInstance()->inspectionTypes.size();
                syncMessage += " Inspection types \n";

                createDialog( syncMessage.c_str() );   

            }catch( const std::runtime_error& error ){
                Serial.println( error.what() );            
                createDialog( error.what() );     
            }
        }else{
            Serial.println("main: unkown event ?");  
        }
    }

    virtual ~mainScreenClass(){
    };
};


//-------------------------------------------------
// >> >> >>

class selectAssetScreenClass:public screenClass{
public:
    std::deque< lv_obj_t* > listButtons;
    lv_obj_t* selectedButton = NULL;

    //----------------------------------

    selectAssetScreenClass(): screenClass( SCREEN_ID_SELECT_ASSET_SCREEN ){}
    virtual ~selectAssetScreenClass(){};    

    //----------------------------------

    void handleEvents( lv_event_t* e ) override{     

        lv_obj_t* target = lv_event_get_target(e);

        // CLICK select
        if( target == objects.select_asset  ){
            Serial.println("select: do select");              
            if( selectedButton != NULL ){

                // get selected asset
                assetClass* asset = (assetClass*) lv_obj_get_user_data(selectedButton);
                if (!asset) return;
                
                // Check if asset is already in selected_asset_list
                uint32_t child_count = lv_obj_get_child_cnt(objects.selected_asset_list);
                for (uint32_t i = 0; i < child_count; ++i) {
                    lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, i);

                    if (lv_obj_check_type(child, &lv_btn_class)) {
                        assetClass* childAsset = static_cast<assetClass*>(lv_obj_get_user_data(child));                    
                        if (childAsset && childAsset == asset) {
                            return;  // Asset already in list, skip adding
                        }
                    }
                }

                if (!hasCommonInspectionType(asset)) {
                    createDialog("Error: The assets selected do have a common inspection type!");
                    return;
                }

                // else, add it
                addAssetToList( objects.selected_asset_list ,  asset, false ); 
            }       
            Serial.println("select: do select  OK");                           
            return;
        }

        // CLICK deselect selected
        if( target == objects.de_select_asset ){
            Serial.println("select: do deselect");              
            while (true) {
                lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, NULL);
                if (child == NULL) break;
                lv_obj_del(child);
            }
            Serial.println("select: do deselect OK");                          
            return;
        }

        // CLICK asset
        for (lv_obj_t* btn : listButtons) {     
            if (btn == target) {               
                Serial.println("select: click button ...");                              
                selectedButton = btn;
                for (lv_obj_t* btn : listButtons) {
                    if (btn != target) {
                        lv_obj_clear_state(btn, LV_STATE_CHECKED);
                    }
                }
                Serial.println("select: click button ... DONE" );                                              
                return;
            }
        }        

    }

    //----------------------------------

    void open() override {

        domainManagerClass* domain = domainManagerClass::getInstance();        
        if( !domain->isLoaded ){
            domain->loadConfigFromKVStore();
        }
    
        // clean
        listButtons.clear();
        lv_obj_clean(objects.asset_list); 
        lv_obj_clean(objects.selected_asset_list); 

        // sync with current inspection
        for (assetClass& asset : domain->assets) {
            bool inInspection = false;

            for (assetClass& selected : domain->currentInspection.assets) {
                if (asset.ID == selected.ID) {
                    addAssetToList(objects.selected_asset_list, &asset, false);
                    inInspection = true;
                    break;
                }
            }

            if (!inInspection) {
                listButtons.push_back(addAssetToList(objects.asset_list, &asset, true));
            }
        }
        
        screenClass::open(); // always last, only if no issues
    }

    void syncToInspection(){

        domainManagerClass* domain = domainManagerClass::getInstance();    
        domain->currentInspection.assets.clear();

        // Count selected asset buttons
        uint32_t child_count = lv_obj_get_child_cnt(objects.selected_asset_list);
        for (uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, i);

            if (lv_obj_check_type(child, &lv_btn_class)) {
                assetClass* asset = static_cast<assetClass*>(lv_obj_get_user_data(child));
                if (asset) {
                    domain->currentInspection.assets.push_back(*asset);
                }
            }
        }

    }

    lv_obj_t* addAssetToList( lv_obj_t* parent_obj, const assetClass* asset, bool checkable ){
                    
        lv_obj_t* button = lv_btn_create(parent_obj);
        lv_obj_set_pos(button, 503, 42);
        lv_obj_set_size(button, 293, 50);

        lv_obj_add_event_cb(button, action_main_event_dispatcher, LV_EVENT_PRESSED, const_cast<assetClass*>(asset) );

        if( checkable ) lv_obj_add_flag(button, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_set_user_data(button, const_cast<assetClass*>(asset));

        lv_obj_set_style_bg_color(button, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(button, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_layout(button, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_flex_track_place(button, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        {
            lv_obj_t *parent_obj = button;
            {
                lv_obj_t *obj = lv_label_create(parent_obj);
                lv_obj_set_pos(obj, 0, 0);
                lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(obj, asset->buttonName.c_str() );
            }
        }

        return( button );            
    }
    
    
    //-------------------------------


    bool hasCommonInspectionType(const assetClass* newAsset) {
        domainManagerClass* domain = domainManagerClass::getInstance();

        for (const inspectionTypeClass& type : domain->inspectionTypes) {
            bool newAssetOk = false;
            bool allSelectedOk = true;

            // --- Does this type cover the NEW asset?
            for (const String& layout : type.layouts) {
                if (layout == "ALL" || layout == newAsset->layoutName) {
                    newAssetOk = true;
                    break;
                }
            }

            if (!newAssetOk) continue;

            // --- Does this type cover ALL SELECTED assets?
            uint32_t child_count = lv_obj_get_child_cnt(objects.selected_asset_list);
            for (uint32_t i = 0; i < child_count; ++i) {
                lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, i);

                if (lv_obj_check_type(child, &lv_btn_class)) {
                    assetClass* childAsset = static_cast<assetClass*>(lv_obj_get_user_data(child));
                    if (childAsset) {
                        bool found = false;
                        for (const String& layout : type.layouts) {
                            if (layout == "ALL" || layout == childAsset->layoutName) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            allSelectedOk = false;
                            break;
                        }
                    }
                }
            }

            if (newAssetOk && allSelectedOk) {
                return true;  // Found valid inspection type
            }
        }

        return false;  // No valid type found
    }

};


//-------------------------------------------------

class selectInspectionTypeScreenClass:public screenClass{
public:

    selectInspectionTypeScreenClass(): screenClass( SCREEN_ID_SELECT_INSPECTION_TYPE ){            
    }

    void handleEvents( lv_event_t* e ) override{
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event

        // button clicks 
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

    void open() override {

        domainManagerClass* domain = domainManagerClass::getInstance();

        // LOAD the LIST from ASSETS ...

        // Clear the list in case it has old items
        lv_obj_clean(objects.inspection_types);

        // For each inspection type, check if it matches ALL selected assets
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

            if (valid) {
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

        screenClass::open(); 
    }


   void syncToInspection() {

        domainManagerClass* domain = domainManagerClass::getInstance();

        uint32_t child_count = lv_obj_get_child_cnt(objects.inspection_types);

        for (uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* btn = lv_obj_get_child(objects.inspection_types, i);

            if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

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
        Serial.println("syncToInspection: No inspection type selected — cleared.");
    }




};


//-------------------------------------------------

    class formFieldsScreenClass : public screenClass {
    public:
        lv_obj_t* kb = NULL;                   
        std::vector<lv_obj_t*> textareas;     

        formFieldsScreenClass() : screenClass(SCREEN_ID_INSPECTION_FORM) {
        }

        void handleEvents(lv_event_t* e) override {
            lv_obj_t* target = lv_event_get_target(e);
            // For now, no extra event handling
        }

        void open() override {
            domainManagerClass* domain = domainManagerClass::getInstance();
            inspectionTypeClass* currentType = domain->currentInspection.type;

            if (currentType == NULL) {
                throw std::runtime_error("Error: No inspection type selected!");
            }

            // Show the current inspection type name at top
            lv_label_set_text(objects.inspection_type_name, currentType->name.c_str());            

            // Clear old textarea handles
            textareas.clear();

            // Use your EEZ-generated list container
            lv_obj_t* parent_obj = objects.inspection_types_1;
            lv_obj_clean(parent_obj);

            // Create keyboard if needed
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

            for (size_t i = 0; i < currentType->formFields.size(); ++i) {
                const std::vector<String>& row = currentType->formFields[i];

                String fieldName = row[0];
                String fieldType = row[1];
                String fieldMax = row.size() >= 3 ? row[2] : "128";

                int maxLength = fieldMax.toInt();
                if (maxLength <= 0) maxLength = 128;

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
                if (i < currentType->formFieldValues.size()) {
                    prefill = currentType->formFieldValues[i];
                }
                lv_textarea_set_text(textarea, prefill.c_str());

                lv_textarea_set_one_line(textarea, false);
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

                // Save handle for sync
                textareas.push_back(textarea);

                // Hook: show keyboard on focus
                lv_obj_add_event_cb(textarea, [](lv_event_t* e) {
                    lv_obj_t* ta = lv_event_get_target(e);
                    formFieldsScreenClass* self = static_cast<formFieldsScreenClass*>(lv_event_get_user_data(e));
                    lv_obj_clear_flag(self->kb, LV_OBJ_FLAG_HIDDEN);
                    lv_keyboard_set_textarea(self->kb, ta);
                    Serial.println("Keyboard opened for textarea.");
                }, LV_EVENT_FOCUSED, this);
            }

            screenClass::open();
        }

        void syncToInspection() {
            domainManagerClass* domain = domainManagerClass::getInstance();
            inspectionTypeClass* currentType = domain->currentInspection.type;

            if (currentType == NULL) {
                Serial.println("syncToInspection: No inspection type selected!");
                return;
            }

            currentType->formFieldValues.clear();

            for (lv_obj_t* ta : textareas) {
                const char* input = lv_textarea_get_text(ta);
                currentType->formFieldValues.push_back(String(input));
            }

            Serial.println("syncToInspection: Form field values saved:");
            for (size_t i = 0; i < currentType->formFieldValues.size(); ++i) {
                Serial.print("  [");
                Serial.print(i);
                Serial.print("] ");
                Serial.println(currentType->formFieldValues[i]);
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

//-------------------------------------------------

    class inspectionZonesScreenClass : public screenClass {
    public:

        inspectionZonesScreenClass() : screenClass(SCREEN_ID_INSPECTION_ZONES) {
        }

        void handleEvents(lv_event_t* e) override {
            lv_obj_t* target = lv_event_get_target(e);
            // For now, no extra event handling
        }

        void open() override {
            
            screenClass::open();
        }

        void syncToInspection() {
        }

        virtual ~inspectionZonesScreenClass() {
        }
    };


#endif 
