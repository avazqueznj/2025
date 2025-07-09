/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/

class selectAssetScreenClass:public screenClass{
public:
    std::deque< lv_obj_t* > listButtons;
    lv_obj_t* selectedButton = NULL;

    //----------------------------------

    selectAssetScreenClass(): screenClass( SCREEN_ID_SELECT_ASSET_SCREEN ){}

    virtual ~selectAssetScreenClass(){};    

    //----------------------------------

    void keyboardEvent(String key) override {

        //>>>>>>
        screenClass::keyboardEvent(key);

        // update if the selected item changed
        lv_obj_t* list = objects.asset_list;
        if (!list) return;
        selectedButton = nullptr;  // Reset
        uint32_t count = lv_obj_get_child_cnt(list);
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* btn = lv_obj_get_child(list, i);
            if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                selectedButton = btn;
                Serial.println("Updated selectedButton from CHECKED state.");
                break;
            }
        }
        if (!selectedButton) {
            Serial.println("No CHECKED button found in list.");
        }


    }

    //----------------------------------

    virtual void rfidEvent(byte *uid, byte length) override {

        // 1) Convert UID to same tag format
        String rfidTag = ":";
        for (byte i = 0; i < length; i++) {
            rfidTag += ":";
            rfidTag += String(uid[i]);
        }

        Serial.print("RFID event tag = ");
        Serial.println(rfidTag);

        // is this a double read?
        {
            uint32_t child_count = lv_obj_get_child_cnt(objects.selected_asset_list);
            for (uint32_t i = 0; i < child_count; ++i) {
                lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, i);

                if (lv_obj_check_type(child, &lv_btn_class)) {
                    assetClass* childAsset = static_cast<assetClass*>(lv_obj_get_user_data(child));
                    if (childAsset && childAsset->tag == rfidTag) {
                        Serial.println("RFID asset already in selected list — ignoring event.");
                        return; // Already selected, skip the rest
                    }
                }
            }        
        }

        // 2) Find matching asset and button
        assetClass* matchedAsset = nullptr;

        for (lv_obj_t* btn : listButtons) {
            assetClass* asset = static_cast<assetClass*>(lv_obj_get_user_data(btn));
            if (asset && asset->tag == rfidTag) {
            matchedAsset = asset;
            selectedButton = btn;
            break;
            }
        }

        if (!matchedAsset) {
            createDialog("No matching asset found for this tag!");
            return;
        }

        // 3) Mark button visually selected, unselect others
        for (lv_obj_t* btn : listButtons) {
            if (btn == selectedButton) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            } else {
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            }
        }

        Serial.print("RFID matched asset ID: ");
        Serial.println(matchedAsset->ID);

        // 4) Does asset already exist in selected_asset_list?
        uint32_t child_count = lv_obj_get_child_cnt(objects.selected_asset_list);
        for (uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* child = lv_obj_get_child(objects.selected_asset_list, i);

            if (lv_obj_check_type(child, &lv_btn_class)) {
                assetClass* childAsset = static_cast<assetClass*>(lv_obj_get_user_data(child));
                if (childAsset && childAsset == matchedAsset) {
                    Serial.println("Asset already in selected list.");
                    return; // Already added
                }
            }
        }

        // 5) Check inspection type
        if (!hasCommonInspectionType(matchedAsset)) {
            createDialog("Error: The assets selected do not have a common inspection type!");
            return;
        }

        // 6) Add it
        addAssetToList(objects.selected_asset_list, matchedAsset, false);

        Serial.println("RFID asset added to selected list!");
    }

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
            listButtons.push_back(addAssetToList(objects.asset_list, &asset, true));

            for (assetClass& selected : domain->currentInspection.assets) {
                if (asset.ID == selected.ID) {
                    addAssetToList(objects.selected_asset_list, &asset, false);
                    inInspection = true;
                    break;
                }
            }
        }


                // Create fresh group or reuse existing
        {
            //-------------------------------------
            // Add focusable widgets

            // default
            lv_group_add_obj(inputGroup, objects.asset_list  );
            lv_group_add_obj(inputGroup, objects.select_asset);
            lv_group_add_obj(inputGroup, objects.de_select_asset);
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
