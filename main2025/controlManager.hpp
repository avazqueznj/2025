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

//****

//-------------------------------------------------

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
            // lv_obj_t* target = lv_event_get_target(e);
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
//_+_+_+

                
class inspectionZonesScreenClass : public screenClass {
public:

    assetClass* lastSelectedAsset = nullptr;

    inspectionZonesScreenClass() : screenClass(SCREEN_ID_INSPECTION_ZONES) {
    }

    void handleEvents(lv_event_t* e) override {
        
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event
        lv_obj_t* parent = lv_obj_get_parent(target);

        // =====================================================
        // On ASSET ----
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == objects.zone_asset_list ) {
            Serial.println("Asset clicked...");

            // iterate the list and reset
            uint32_t child_count = lv_obj_get_child_cnt(objects.zone_asset_list  ); // ZONE assetrs list
            for (uint32_t i = 0; i < child_count; ++i) {
                // reset the asset
                lv_obj_t* btn = lv_obj_get_child(objects.zone_asset_list, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                // f%^%k EEZ!
                if (btn != target) {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                }
                           
                // build zone list
                if (btn == target) {  // i know it is unecessary ... now -

                    assetClass* asset = static_cast<assetClass*>(lv_obj_get_user_data(btn));
                    if (!asset) {
                        throw std::runtime_error("inspectionZonesScreenClass: asset is null");
                    }

                    if (asset == lastSelectedAsset) {
                        return; // nothing to do
                    }else{
                        lastSelectedAsset = asset;                    
                    }
                    
                    domainManagerClass* domain = domainManagerClass::getInstance();
                    if (!domain) {
                        throw std::runtime_error("inspectionZonesScreenClass: domainManagerClass is null");
                    }

                    lv_obj_clean(objects.zone_list);
                    lv_obj_clean(objects.zone_component_list);

                    // Find the layout for the asset
                    layoutClass* layout = nullptr;
                    for (layoutClass& l : domain->layouts) {
                        if (l.name == asset->layoutName) {
                            layout = &l;
                            break;
                        }
                    }
                    if (!layout) {
                        throw std::runtime_error("inspectionZonesScreenClass: layout not found" );
                    }

                    //=================================================
                    // ZONE RENDER

                    // read the zones from the layout
                    bool foundZone = false;
                    for (layoutZoneClass& zone : layout->zones) {
                        foundZone = true;

                        // Add zone button
                        lv_obj_t* zbtn = lv_btn_create(objects.zone_list);
                        lv_obj_set_size(zbtn, 230, 50);
                        //lv_obj_add_flag(zbtn, LV_OBJ_FLAG_CHECKABLE);
                        lv_obj_set_style_bg_color(zbtn, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(zbtn, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);

                        lv_obj_add_event_cb(zbtn, action_main_event_dispatcher, LV_EVENT_PRESSED, this);

                            lv_obj_t* zlabel = lv_label_create(zbtn);
                            lv_obj_set_style_text_font(zlabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(zlabel, zone.name.c_str());
                            lv_obj_set_user_data(zbtn, (void*)&zone);    
                    }

                    if (!foundZone) {
                        throw std::runtime_error("inspectionZonesScreenClass: no zones found in layout: " );
                    }
                }

            }

            refreshZoneAndComponentLabels();                

            Serial.println("Asset clicked  DONE");
            return;
        }     


        // On ZONE click -->
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == objects.zone_list ) {

            Serial.println("Zone click");
            // iterate the zone list and reset
            uint32_t child_count = lv_obj_get_child_cnt(objects.zone_list  ); // ZONE assetrs list
            for (uint32_t i = 0; i < child_count; ++i) {
                // reset selection
                lv_obj_t* btn = lv_obj_get_child(objects.zone_list, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                if (btn != target) {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                }

                if (btn == target) {  // i know it is unecessary ... now -   

                    lv_obj_clean(objects.zone_component_list);

                    layoutZoneClass* zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(target));
                    if (!zone) {
                        throw std::runtime_error("Zone user_data is null in ZONE click handler");
                    }
                    if (!lastSelectedAsset) {
                        throw std::runtime_error("lastSelectedAsset is null in ZONE click handler");
                    }

                    domainManagerClass* domain = domainManagerClass::getInstance();
                    const std::vector<defectClass>& defects = domain->currentInspection.defects;

                    //=================================================
                    // COMPO RENDER ====================
                    for (size_t j = 0; j < zone->components.size(); ++j) {

                        const std::vector<String>& compVec = zone->components[j];
                                    
                        if (compVec.empty()) {
                            throw std::runtime_error("Component vector is empty");
                        }
                        if (compVec.size() <= 1) {
                            throw std::runtime_error("Component vector missing name");
                        }

                        String compName = compVec[1];
                        String labelText = String( " " ) + compName;

                        lv_obj_t* cbtn = lv_btn_create(objects.zone_component_list);
                        lv_obj_set_size(cbtn, 280, 50);
                        //lv_obj_add_flag(cbtn, LV_OBJ_FLAG_CHECKABLE);
                        lv_obj_set_style_bg_color(cbtn, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(cbtn, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);

                        lv_obj_set_user_data(cbtn, (void*)&compVec);
                        lv_obj_add_event_cb(cbtn, action_main_event_dispatcher, LV_EVENT_PRESSED, this);

                        lv_obj_t* clabel = lv_label_create(cbtn);
                        lv_obj_set_style_text_font(clabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(clabel, labelText.c_str());
                        
                    }

                }
            }
            Serial.println("Zone click DONE");

            refreshZoneAndComponentLabels();            

            return;
        }     

        // On COMPONENT click reset check -->
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == objects.zone_component_list ) {
            Serial.println("compo click");
            // iterate the  list and reset
            uint32_t child_count = lv_obj_get_child_cnt(objects.zone_component_list  ); 
            for (uint32_t i = 0; i < child_count; ++i) {
                // reset selection
                lv_obj_t* btn = lv_obj_get_child(objects.zone_component_list, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                if (btn != target) {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                }
            }
            Serial.println("compo click DONE");
            return;
        }     

                        

        // =====================================================
        // DEFECTO dialog ---

        // Open defect dialog
        if (target == objects.defect_button) {
            Serial.println("create defect click");

            // Find selected component in zone_component_list (index version)
            std::vector<String>* compVec = nullptr;
            uint32_t i = 0;
            lv_obj_t* btn = lv_obj_get_child(objects.zone_component_list, i);
            while (btn) {
                if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                    compVec = (std::vector<String>*) lv_obj_get_user_data(btn);            
                    break;
                }
                ++i;
                btn = lv_obj_get_child(objects.zone_component_list, i);                
            }

            if (compVec != nullptr) {
                openDefectDialog( compVec );
            }else{
                Serial.println("No component selected!");
                createDialog("Please select a component.");
            }
        }

        // close defecto
        if( close_btn != nullptr ){
            if (  (lv_event_get_code(e) == LV_EVENT_CLICKED)  &&  lv_obj_check_type(target, &lv_btn_class) &&  target == close_btn ) {
                lv_msgbox_close_async(dialog);   
                refreshZoneAndComponentLabels();
                return;                
            }
        }

        // defecto reset
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == defect_list ) {
            Serial.println("defecto click");
            
            uint32_t child_count = lv_obj_get_child_cnt( defect_list ); 
            for (uint32_t i = 0; i < child_count; ++i) {                
                lv_obj_t* btn = lv_obj_get_child( defect_list , i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                if (btn != target) {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                }

            }
            Serial.println("defecto click DONE");
            return;
        }     

        // POST DEFECTO
        if (  
                ( (lv_event_get_code(e) == LV_EVENT_CLICKED)  &&  lv_obj_check_type(target, &lv_btn_class) ) 
                &&  
                ( target == major_btn || target == minor_btn || target == del_btn )
            ){

            Serial.println("Defect post...");
            domainManagerClass* domain = domainManagerClass::getInstance();

            int severity = 1;
            if( target == major_btn ) severity = 10;
    
            String* selected_defect = static_cast<String*>(lv_obj_get_user_data(get_checked_child(defect_list)));
            if( selected_defect !=  nullptr ){

                // assemble defect object
                defectClass newDefect(
                    selected_asset,
                    selected_zone->tag,
                    selected_component_name,
                    selected_defect ? *selected_defect : String(""),
                    severity,
                    "<<a note>>"
                );

                // no dupes
                std::vector<defectClass>& defects = domain->currentInspection.defects;
                for (size_t i = 0; i < defects.size(); ) {
                    if (defects[i].isSameComponent(newDefect)) {
                        defects.erase(defects.begin() + i);
                    } else {
                        ++i;
                    }
                }

                // save it, or delete it
                if( target != del_btn ) domain->currentInspection.defects.push_back( newDefect );                

                lv_msgbox_close_async(dialog);   

                refreshZoneAndComponentLabels();                

                // debugo
                Serial.println( domain->currentInspection.toString().c_str() );                    
            }
        
            return;                
        }

        //-------
    

    }



    lv_obj_t *dialog = nullptr;    
    lv_obj_t *close_btn = nullptr;
    lv_obj_t *del_btn = nullptr;
    lv_obj_t *minor_btn = nullptr;
    lv_obj_t *major_btn = nullptr;
    lv_obj_t *defect_list = nullptr;

    assetClass* selected_asset = nullptr;
    layoutZoneClass* selected_zone = nullptr;
    std::vector<String>* selected_component_vec = nullptr;
    String selected_component_name;
    void openDefectDialog( std::vector<String>* compVec ){


        //=======================
        // while i fix the unselect issue

            lv_obj_t* selected_zone_item = nullptr;
            lv_obj_t* selected_component_item = nullptr;

                // Asset selection check
                lv_obj_t* selected_asset_item = get_checked_child(objects.zone_asset_list);
                if (!selected_asset_item) {
                    createDialog("Please select an asset.");
                    return;
                }
                selected_asset = static_cast<assetClass*>(lv_obj_get_user_data(selected_asset_item));
                if (!selected_asset) {
                    createDialog("Failed to resolve selected asset.");
                    return;
                }

                // Zone selection check
                selected_zone_item = get_checked_child(objects.zone_list);
                if (!selected_zone_item) {
                    createDialog("Please select a zone.");
                    return;
                }
                selected_zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(selected_zone_item));
                if (!selected_zone) {
                    createDialog("Failed to resolve selected zone.");
                    return;
                }

                // Component selection check
                selected_component_item = get_checked_child(objects.zone_component_list);
                if (!selected_component_item) {
                    createDialog("Please select a component.");
                    return;
                }
                selected_component_vec = static_cast<std::vector<String>*>(lv_obj_get_user_data(selected_component_item));
                if (!selected_component_vec) {
                    createDialog("Failed to resolve selected component.");
                    return;
                }
                if (selected_component_vec->size() <= 1) {
                    createDialog("Selected component data is incomplete.");
                    return;
                }
                selected_component_name = (*selected_component_vec)[1];
                if (selected_component_name.isEmpty()) {
                    createDialog("Selected component name is empty.");
                    return;
                }

        //===================

        domainManagerClass* domain = domainManagerClass::getInstance();
        defectClass* existingDefect = nullptr;

        // restore if this is an edit ....
        for (auto& d : domain->currentInspection.defects) {
            if (d.asset == selected_asset && d.zoneName == selected_zone->tag && d.componentName == selected_component_name) {
                existingDefect = &d;
                break;
            }
        }

        // Create defectDialog
        dialog = lv_msgbox_create(NULL, "", "", 0, true);                
        lv_obj_set_pos(dialog, 89, 39);
        lv_obj_set_size(dialog, 626, 400); // Make dialog taller to fit keyboard
        lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER |
                                    LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW);
        lv_obj_set_style_align(dialog, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_layout(dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        // add event handler
        close_btn = lv_msgbox_get_close_btn(dialog);
        if (close_btn) {
        Serial.println("set handler!");            
            lv_obj_add_event_cb(close_btn, action_main_event_dispatcher, LV_EVENT_PRESSED, (void*)0);
        }

        {
            lv_obj_t *parent_obj = dialog;

            // delete button
            del_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(del_btn, 20, 305);
            lv_obj_set_size(del_btn, 164, 40);
                lv_obj_add_event_cb(del_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(del_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *del_label = lv_label_create(del_btn);
            lv_obj_set_style_align(del_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(del_label, "delete");

            // minorButton
            minor_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(minor_btn, 202, 305);
            lv_obj_set_size(minor_btn, 164, 40);
                lv_obj_add_event_cb(minor_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(minor_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *minor_label = lv_label_create(minor_btn);
            lv_obj_set_style_align(minor_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(minor_label, "minor");

            // majorButton
            major_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(major_btn, 385, 305);
            lv_obj_set_size(major_btn, 164, 40);
                lv_obj_add_event_cb(major_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(major_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *major_label = lv_label_create(major_btn);
            lv_obj_set_style_align(major_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(major_label, "major");

                //----

            // (defect list)
            defect_list = lv_list_create(parent_obj);
            lv_obj_set_pos(defect_list, -1, 52);
            lv_obj_set_size(defect_list, 250, 227);
            lv_obj_clear_flag(defect_list, LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC);
            lv_obj_set_scrollbar_mode(defect_list, LV_SCROLLBAR_MODE_ON);
            lv_obj_set_scroll_dir(defect_list, LV_DIR_VER);
            lv_obj_set_style_pad_top(defect_list, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(defect_list, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(defect_list, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

            if (compVec->size() >= 2) {
                String compName = (*compVec)[1];
                Serial.print("Selected component: ");
                Serial.println(compName);

                // Defective component label
                lv_obj_t *defective_component_label = lv_label_create(parent_obj);
                lv_obj_set_style_text_font(defective_component_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

                lv_obj_set_pos(defective_component_label, 170, 6);
                lv_obj_set_style_text_color(defective_component_label, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(defective_component_label, compName.c_str());

                // Add defect buttons
                for (size_t i = 2; i < compVec->size(); ++i) {
                    String defectName = (*compVec)[i];

                    lv_obj_t* defect_btn = lv_btn_create(defect_list);
                    lv_obj_set_size(defect_btn, 230, 50);

                    lv_obj_set_user_data(defect_btn, (void*)&(*compVec)[i] );
                    
                    lv_obj_add_event_cb(defect_btn, action_main_event_dispatcher, LV_EVENT_PRESSED, (void*)&(*compVec)[i]);

                    lv_obj_set_style_bg_color(defect_btn, lv_color_hex(0xffdddddd), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(defect_btn, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(defect_btn, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(defect_btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

                    //lv_obj_add_flag(defect_btn, LV_OBJ_FLAG_CHECKABLE);

                    lv_obj_t* label = lv_label_create(defect_btn);
                    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(label, defectName.c_str());

                    if (existingDefect && defectName == existingDefect->defectType) {
                        lv_obj_add_state(defect_btn, LV_STATE_CHECKED);
                    }
                    else if (i == 2 && !existingDefect) {
                        lv_obj_add_state(defect_btn, LV_STATE_CHECKED);
                    }
                }
            }

            // Static defect label
            lv_obj_t *defect_label = lv_label_create(parent_obj);
            lv_obj_set_style_text_font(defect_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_pos(defect_label, 56, 6);
            lv_label_set_text(defect_label, "Defect:");

            // Notes label
            lv_obj_t *notes_label = lv_label_create(parent_obj);
            lv_obj_set_pos(notes_label, 255, 52);
            lv_label_set_text(notes_label, "Notes:");

            // Notes textarea
            lv_obj_t *notes_textarea = lv_textarea_create(parent_obj);
            lv_obj_set_style_text_font(notes_textarea, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_pos(notes_textarea, 255, 80);
            lv_obj_set_size(notes_textarea, 323, 190);
            lv_textarea_set_max_length(notes_textarea, 128);
            lv_textarea_set_one_line(notes_textarea, false);
            lv_textarea_set_password_mode(notes_textarea, false);
            if (existingDefect) {
                lv_textarea_set_text(notes_textarea, existingDefect->notes.c_str());
            }                    

            // Keyboard for textarea
            lv_obj_t *kb = lv_keyboard_create(parent_obj);
            lv_obj_set_size(kb, 323, 150);
            lv_obj_set_pos(kb, 255, 280); // below textarea
            lv_keyboard_set_textarea(kb, notes_textarea);

            // Show/hide keyboard on focus
            lv_obj_add_event_cb(notes_textarea, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t *kb_obj = (lv_obj_t *) lv_event_get_user_data(e);

                if (code == LV_EVENT_FOCUSED) {
                    Serial.println("Show kb");
                    lv_obj_clear_flag(kb_obj, LV_OBJ_FLAG_HIDDEN);
                }

            }, LV_EVENT_ALL, kb);

            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }

        Serial.println("defect click done!");

    }

    //----------

    void refreshZoneAndComponentLabels() {

    Serial.println("Refresh!");

    domainManagerClass* domain = domainManagerClass::getInstance();
    if (!domain) throw std::runtime_error("domainManagerClass is null");
    if (!lastSelectedAsset) throw std::runtime_error("lastSelectedAsset is null");

    // Find layout for selected asset
    layoutClass* layout = nullptr;
    for (layoutClass& l : domain->layouts) {
        if (l.name == lastSelectedAsset->layoutName) {
            layout = &l;
            break;
        }
    }
    if (!layout) throw std::runtime_error("Layout not found for selected asset");

    // Iterate over all zones shown in the UI
    uint32_t zone_count = lv_obj_get_child_cnt(objects.zone_list);
    for (uint32_t z = 0; z < zone_count; ++z) {

        lv_obj_t* zbtn = lv_obj_get_child(objects.zone_list, z);
        if (!zbtn) continue;

        layoutZoneClass* ui_zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(zbtn));
        if (!ui_zone) continue;

        // Find this zone in the DB by tag
        layoutZoneClass* db_zone = nullptr;
        for (layoutZoneClass& lz : layout->zones) {
            if (lz.tag == ui_zone->tag) {
                db_zone = &lz;
                break;
            }
        }
        if (!db_zone) continue;

        bool allHaveDefects = true;
        bool allAreSev0 = true;
        bool hasSev1 = false;
        bool hasSev10 = false;

        // For each expected component in this zone - find defects
        for (const std::vector<String>& compVec : db_zone->components) {
            if (compVec.size() <= 1) continue;
            String compName = compVec[1];
            bool found = false;
            int severity = -1;
            for (const auto& defect : domain->currentInspection.defects) {
                if (!defect.asset) continue;
                if (defect.asset->ID == lastSelectedAsset->ID &&
                    defect.zoneName == db_zone->tag &&
                    defect.componentName == compName) {
                    found = true;
                    severity = defect.severity;
                    break;
                }
            }

            if (!found) {
                allHaveDefects = false;
                allAreSev0 = false;
            } else {
                if (severity == 1) hasSev1 = true;
                if (severity == 10) hasSev10 = true;
                if (severity != 0) allAreSev0 = false;
            }

            // ---- ON COMPO --> compVec
            if (lv_obj_has_state(zbtn, LV_STATE_CHECKED)) {

                uint32_t comp_btn_count = lv_obj_get_child_cnt(objects.zone_component_list); // compo list
                lv_obj_t* matching_btn = nullptr;
                for (uint32_t c = 0; c < comp_btn_count; ++c) {
                    lv_obj_t* cbtn = lv_obj_get_child(objects.zone_component_list, c);
                    std::vector<String>* uiCompVec = static_cast<std::vector<String>*>(lv_obj_get_user_data(cbtn));
                    if (uiCompVec && uiCompVec->size() > 1 && (*uiCompVec)[1] == compName) {
                        matching_btn = cbtn;
                        break;
                    }
                }
                if (!matching_btn) continue; // skip if not found in UI

                String prefix;
                if (severity == 10)
                    prefix = LV_SYMBOL_CLOSE;
                else if (severity == 1)
                    prefix = LV_SYMBOL_WARNING;
                else if (severity == 0)
                    prefix = LV_SYMBOL_OK;
                else
                    prefix = "";

                // Update the label for this COMPO button
                uint32_t comp_child_count = lv_obj_get_child_cnt(matching_btn);
                for (uint32_t i = 0; i < comp_child_count; ++i) {
                    lv_obj_t* child = lv_obj_get_child(matching_btn, i);
                    if (lv_obj_check_type(child, &lv_label_class)) {
                        String labelText = (prefix.length() > 0 ? prefix + " " : "") + compName;
                        lv_label_set_text(child, labelText.c_str());
                    }
                }
            }else{

            }
        }

        // ---- ZONE LABEL UPDATE ----

        // Decide the symbol for this zone
        String prefix;
        if (hasSev10)
            prefix = LV_SYMBOL_CLOSE;      // Stop (blocker)
        else if (hasSev1)
            prefix = LV_SYMBOL_WARNING;    // Warning (minor)
        else if (allHaveDefects && allAreSev0)
            prefix = LV_SYMBOL_OK;         // Checked (all 0)
        else
            prefix = "";                   // Unmasked

        // Update the label for this zone button
        uint32_t zone_child_count = lv_obj_get_child_cnt(zbtn);
        for (uint32_t i = 0; i < zone_child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child(zbtn, i);
            if (lv_obj_check_type(child, &lv_label_class)) {
                String labelText = (prefix.length() > 0 ? prefix + " " : "") + ui_zone->name;
                lv_label_set_text(child, labelText.c_str());
            }
        }
    }
}        

//--------

    void open() override {
        domainManagerClass* domain = domainManagerClass::getInstance();

        // Clear existing items
        lv_obj_clean(objects.zone_asset_list);
        lv_obj_clean(objects.zone_list);
        lv_obj_clean(objects.zone_component_list);

        for (assetClass& asset : domain->currentInspection.assets) {
            lv_obj_t* btn = lv_btn_create(objects.zone_asset_list);

            lv_obj_set_size(btn, 182, 84);
            lv_obj_add_event_cb(btn, action_main_event_dispatcher, LV_EVENT_PRESSED, this);
            //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_user_data(btn, static_cast<void*>(&asset));

            // Style
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(btn, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

            // Label
            lv_obj_t* label = lv_label_create(btn);
            lv_obj_set_style_align(label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(label, asset.ID.c_str());
        }

        screenClass::open();
    }

    void syncToInspection() {
        // Placeholder if needed later
    }

    virtual ~inspectionZonesScreenClass() {
        domainManagerClass* domain = domainManagerClass::getInstance();
        domain->currentInspection.defects.clear();
    }
};

#endif 
