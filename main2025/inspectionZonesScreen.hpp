/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/

                
class inspectionZonesScreenClass : public screenClass {
public:


	/*
	zone tags, thanks kyle sparks for having to hardcode this
	
	zone1 ::4:98:28:2:177:115:128
	zone2 ::4:59:59:2:177:115:128
	zone3 ::4:249:13:2:177:115:128
	zone4 ::4:63:37:2:177:115:129
	zone5 ::233:112:67:194
	.. and i have no more tags
	*/


    assetClass* lastSelectedAsset = nullptr; // its the current asset not last!!

    inspectionZonesScreenClass() : screenClass(SCREEN_ID_INSPECTION_ZONES) {
    }

    //-------------------------------------------------

    void clockTic ( String time ) override {
        lv_label_set_text( objects.clock_insp, time.c_str());
    }

    void rfidEvent(byte *uid, byte length) override {

        // Build tag string in your style
        String rfidTag = ":";
        for (byte i = 0; i < length; i++) {
            rfidTag += ":";
            rfidTag += String(uid[i]);
        }

        Serial.print("RFID event tag = ");
        Serial.println(rfidTag);

        // Match to the 4 known zone tags → map to zone tag
        // are all zone tags the same ? idk
        String targetZoneTag;

        if (rfidTag == "::4:98:28:2:177:115:128") {  //conti
            targetZoneTag = "1";

        } else if (rfidTag == "::4:59:59:2:177:115:128") {  //conti
            targetZoneTag = "2";

        } else if (rfidTag == "::4:249:13:2:177:115:128") {  //conti
            targetZoneTag = "3";

        } else if (rfidTag == "::4:63:37:2:177:115:129") {  //conti
            targetZoneTag = "4";

        } else if (rfidTag == "::233:112:67:194") { // kfob
            targetZoneTag = "5";

        } else {
            createDialog("Unknown tag read");
            return;
        }

        Serial.print("Matched to zone tag: ");
        Serial.println(targetZoneTag);

        // Find zone button in UI and select it
        uint32_t zone_count = lv_obj_get_child_cnt(objects.zone_list);
        lv_obj_t* matchingZoneButton = nullptr;

        for (uint32_t i = 0; i < zone_count; ++i) {
            lv_obj_t* zbtn = lv_obj_get_child(objects.zone_list, i);
            layoutZoneClass* zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(zbtn));
            if (!zone) continue;

            if (zone->tag == targetZoneTag) {
                lv_obj_add_state(zbtn, LV_STATE_CHECKED);
                matchingZoneButton = zbtn;   // Save the match
            } else {
                lv_obj_clear_state(zbtn, LV_STATE_CHECKED);
            }
        }

        if (!matchingZoneButton) {
            createDialog("Read zone tag, but zone was not found or no asset selected.");
            return;
        }

        Serial.println("Zone selected by RFID OK.");

        // render
        renderComponents();
        refreshZoneAndComponentFlags();                        

        lv_group_focus_obj(objects.zone_component_list);             
    }


    //----------------


    void handleModalkeyboardEvent( String key ){

        lv_obj_t* list = defect_list;
        lv_obj_t* selected = nullptr;

        // prepare defect list
        uint32_t count = lv_obj_get_child_cnt(list);
        if( count == 0 ) return;

        // find current selection in the defect list
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* btn = lv_obj_get_child(list, i);
            if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                selected = btn;
                break;
            }        
        }

        // nothing then default to first
        if(!selected) {
            lv_obj_t* selected = lv_obj_get_child(list, 0);
            if (selected) {
                lv_obj_add_state(selected, LV_STATE_CHECKED);
                lv_obj_scroll_to_view(selected, LV_ANIM_ON);
            }    
        }    

        if( !selected ){
            return;
        }



        if (key == "A" || key == "B") {            
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

               return;  
        }


        if( key == "1" ){
            saveDefect( -1 );
            refreshZoneAndComponentFlags();
            return;  
        }
        if( key == "2" ){
            saveDefect( 1 );
            refreshZoneAndComponentFlags();
            return;              
        }
        if( key == "3" ){
            saveDefect( 10 );
            refreshZoneAndComponentFlags();  
            return;                        
        }
        if( key == "*" ){
            closeDefectDialog();
            refreshZoneAndComponentFlags();
            return;                
        }

    }


    //----------------------------------

    void keyboardEvent(String key) override {

        Serial.print("Inspection key event:");
        Serial.println(key);

        // are we under defecto modal? 
        if ( defectDialogOpen ) {
            Serial.print("Modal is up, handle modal.");
            handleModalkeyboardEvent( key );
            return; // simulate modal, eat events
        }     

        screenClass::keyboardEvent(key);
      

        /*
            updateAssetSeverityLabels();
            updateZoneSeverityLabels();
            updateComponentSeverityLabels();
        */

        Serial.println("ABCD refresh?");                                    
        if (key == "A" || key == "B" || key == "C" || key == "D") {
            lv_obj_t* focused = lv_group_get_focused(inputGroup);

            if (focused == objects.zone_asset_list) {
                assetClass* asset = nullptr;

                uint32_t count = lv_obj_get_child_cnt(objects.zone_asset_list);
                for (uint32_t i = 0; i < count; ++i) {
                    lv_obj_t* btn = lv_obj_get_child(objects.zone_asset_list, i);
                    if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
                        asset = static_cast<assetClass*>(lv_obj_get_user_data(btn));
                        break;
                    }
                }
                if (asset) {
                    Serial.println("Refresh zones...");
                    lastSelectedAsset = asset;
                    renderAssetZones();
                    updateZoneSeverityLabels();
                }

            }
            else if (focused == objects.zone_list) {
                Serial.println("Refresh compos...");
                renderComponents();
                updateComponentSeverityLabels();
            }
            // else: do nothing
        }


        Serial.println("Defect buttons?");                                    
        
        if (key == "1") {
            Serial.println("allokDefectClick");                                                
            allokDefectClick();
            refreshZoneAndComponentFlags();
            return;              
        }
        if (key == "2") {
            Serial.println("okDefectClick");                                                
            okDefectClick();
            refreshZoneAndComponentFlags();
            return;              
        }
        if (key == "3") {
            Serial.println("defectClick");                                                
            defectClick();
            refreshZoneAndComponentFlags();
            return;              
        }
        if (key == "4") {
            Serial.println("submitInspection");                                                
            submitInspection();
            return;              
        }



        Serial.println("DONE key handling");                                    
    }

    //--------------------------------

    void handleEvents(lv_event_t* e) override {
        
        lv_obj_t* target = lv_event_get_target(e);  // The object that triggered the event
        lv_obj_t* parent = lv_obj_get_parent(target);

        // =====================================================
        // CLICK ASSET ----
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == objects.zone_asset_list ) {
            Serial.println("Asset clicked...");

            // go over the asset list
            bool render  = false;
            uint32_t child_count = lv_obj_get_child_cnt(objects.zone_asset_list  ); // ZONE assetrs list
            for (uint32_t i = 0; i < child_count; ++i) {

                // next child
                lv_obj_t* btn = lv_obj_get_child(objects.zone_asset_list, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;

                // clear not selected
                if (btn != target) {                    
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);

                } else {
                    // set selected!
                    lv_obj_add_state(btn, LV_STATE_CHECKED);

                    // get the domain asset in the asset button
                    assetClass* asset = static_cast<assetClass*>(lv_obj_get_user_data(btn));
                    if (!asset) {
                        throw std::runtime_error("inspectionZonesScreenClass: asset in button is null ?");
                    }

                    Serial.println( (*asset).ID );

                    // has the asset changed from last ?
                    if (asset == lastSelectedAsset) {
                        return; // nothing to do                        
                    }else{
                        // yes update and render
                        lastSelectedAsset = asset;  // new selection   
                        render  = true;
                    }
                }                        
            }

            if( render ){
                renderAssetZones();
                refreshZoneAndComponentFlags();                
            }

            return;
        }     


        // On ZONE click -->
        if (  lv_obj_check_type(target, &lv_btn_class) &&  parent == objects.zone_list ) {

            Serial.println("Zone clicked...");

            // go over zones
            uint32_t child_count = lv_obj_get_child_cnt(objects.zone_list  ); // ZONE assetrs list
            if( child_count == 0 ) return;

            for (uint32_t i = 0; i < child_count; ++i) {
                // reset selection
                lv_obj_t* btn = lv_obj_get_child(objects.zone_list, i);
                if (!lv_obj_check_type(btn, &lv_btn_class)) continue;
                if (btn != target) {
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                } else {
                    lv_obj_add_state(btn, LV_STATE_CHECKED);
                } 
            }           

            renderComponents();
            refreshZoneAndComponentFlags();                        
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


        //  top bar---------------------------------------------------------------------------------------------------

        // save 0 sev 
        if (target == objects.comp_ok_button) {
            okDefectClick();
            refreshZoneAndComponentFlags();
            return;
        }


        if (target == objects.all_ok_button) {  
            allokDefectClick();
            refreshZoneAndComponentFlags();
            return;
        }

        // =====================================================
        // DEFECTO dialog ---

        // defect list scroll reset 
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


        // Buttons        

        if (target == objects.defect_button) {  
            defectClick();
            refreshZoneAndComponentFlags();
            return;
        }

        // close defecto dialog <-- just call close below
        if( close_btn != nullptr ){
            if (  (lv_event_get_code(e) == LV_EVENT_CLICKED)  &&  lv_obj_check_type(target, &lv_btn_class) &&  target == close_btn ) {
                closeDefectDialog();
                refreshZoneAndComponentFlags();
                return;                
            }
        }

  
        if (  
                ( (lv_event_get_code(e) == LV_EVENT_CLICKED)  &&  lv_obj_check_type(target, &lv_btn_class) ) 
                &&  
                ( target == major_btn || target == minor_btn || target == del_btn )
            ){

            int severity = 0;
            if( target == major_btn ) severity = 10;
            if( target == minor_btn ) severity = 1;
            if( target == del_btn ) severity = -1;

            saveDefect( severity );
            refreshZoneAndComponentFlags();

            return;                
        }

        //-------

        if (target == objects.submit) {
            submitInspection();
            return;  
        }

    }



//==================================================================================================================================
//==================================================================================================================================
//==================================================================================================================================

    void okDefectClick(){

        Serial.println("OK defect");

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

            // Validate caset
            if (!lastSelectedAsset) {
                createDialog("No asset selected.");
                return;
            }

            // zone
            lv_obj_t* selected_zone_item = get_checked_child(objects.zone_list);
            if (!selected_zone_item) {
                createDialog("No zone selected.");
                return;
            }

            // layout
            layoutZoneClass* selected_zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(selected_zone_item));
            if (!selected_zone) {
                createDialog("Failed to resolve selected zone.");
                return;
            }

            if (compVec->size() <= 1) {
                createDialog("Component vector is incomplete.");
                return;
            }

            // compo
            String compName = (*compVec)[1];
            if (compName.isEmpty()) {
                createDialog("Component name is empty.");
                return;
            }

            domainManagerClass* domain = domainManagerClass::getInstance();
            std::vector<defectClass>& defects = domain->currentInspection.defects;

            // Check if a defect already exists for this component
            bool exists = false;
            for (const auto& defect : defects) {
   
                if (
                    defect.asset.ID == (*lastSelectedAsset).ID &&
                    defect.zoneName == selected_zone->tag &&
                    defect.componentName == compName) {
                    exists = true;
                    break;
                }
            }

            if (exists) {
                Serial.println("Defect already exists — ignoring OK post.");
                return; // Skip adding severity 0 if any defect exists
            }

            // Make severity 0 defect
            defectClass newDefect(
                *lastSelectedAsset,
                selected_zone->tag,
                compName,
                "GOOD",
                0,
                "<< NOTES >>"
            );

            defects.push_back(newDefect);

            Serial.println("OK defect saved!");
            Serial.println(domain->currentInspection.toString().c_str());

        } else {
            Serial.println("No component selected!");
            createDialog("Please select a component.");
        }

    }


    void allokDefectClick(){

            Serial.println("OK ALL defects for zone");

            if (!lastSelectedAsset) {
                createDialog("No asset selected.");
                return;
            }

            lv_obj_t* selected_zone_item = get_checked_child(objects.zone_list);
            if (!selected_zone_item) {
                createDialog("No zone selected.");
                return;
            }

            layoutZoneClass* selected_zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(selected_zone_item));
            if (!selected_zone) {
                createDialog("Failed to resolve selected zone.");
                return;
            }

            domainManagerClass* domain = domainManagerClass::getInstance();
            std::vector<defectClass>& defects = domain->currentInspection.defects;

            uint32_t comp_btn_count = lv_obj_get_child_cnt(objects.zone_component_list);
            uint32_t newDefectsCount = 0;

            for (uint32_t c = 0; c < comp_btn_count; ++c) {
                lv_obj_t* cbtn = lv_obj_get_child(objects.zone_component_list, c);
                std::vector<String>* uiCompVec = static_cast<std::vector<String>*>(lv_obj_get_user_data(cbtn));

                if (!uiCompVec || uiCompVec->size() <= 1) continue;

                String compName = (*uiCompVec)[1];
                if (compName.isEmpty()) continue;

                // Check if defect exists
                bool exists = false;
                for (const auto& defect : defects) {
                    if (
                        defect.asset.ID == (*lastSelectedAsset).ID &&
                        defect.zoneName == selected_zone->tag &&
                        defect.componentName == compName) {
                        exists = true;
                        break;
                    }
                }

                if (exists) {
                    Serial.print("Defect already exists for ");
                    Serial.println(compName);
                    continue; // Skip adding
                }

                // Add severity 0 defect
                defectClass newDefect(
                    (*lastSelectedAsset),
                    selected_zone->tag,
                    compName,
                    "GOOD",
                    0,
                    "<< NOTES >>"
                );
                defects.push_back(newDefect);
                ++newDefectsCount;

                Serial.print("OK defect saved for ");
                Serial.println(compName);
            }

            if (newDefectsCount == 0) {
                Serial.println("No new OK defects needed — all already have defects.");
            } else {
                Serial.print("Added OK defects for ");
                Serial.print(newDefectsCount);
                Serial.println(" components.");
            }

            Serial.println(domain->currentInspection.toString().c_str());
    }

    void defectClick(){

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



    void saveDefect( int severity ){

        Serial.println("Defect post...");

        domainManagerClass* domain = domainManagerClass::getInstance();

        // get selection
        String* selected_defect = static_cast<String*>(lv_obj_get_user_data(get_checked_child(defect_list)));

        // assemble a tentative defect
        if( selected_defect !=  nullptr ){        
            defectClass newDefect(
                *selected_asset,
                selected_zone->tag,
                selected_component_name,
                selected_defect ? *selected_defect : String(""),
                severity,
                "<<a note>>"
            );

            // is there a sibling already there - delete it
            std::vector<defectClass>& defects = domain->currentInspection.defects;
            for (size_t i = 0; i < defects.size(); ) {
                if (defects[i].isSameComponent(newDefect)) {
                    defects.erase(defects.begin() + i);
                } else {
                    ++i;
                }
            }

            // save it, or delete it
            if( severity != -1 ) domain->currentInspection.defects.push_back( newDefect );                

            closeDefectDialog();

            // debugo
            Serial.println( domain->currentInspection.toString().c_str() );                    
        }

    }

    void submitInspection(){

        Serial.println("Submit ...");
        spinnerStart();

        try{
            domainManagerClass* domain = domainManagerClass::getInstance(); 

            // Check if there are any defects BEFORE submitting
            if (domain->currentInspection.defects.size() == 0) {
                spinnerEnd(); 
                createDialog("ERROR: Cannot submit empty inspection.");
            } else {
                domain->comms->up();   
                domain->currentInspection.submitTime = String(lv_label_get_text(objects.clock_insp));
                createDialog(domain->comms->postInspection(domain->currentInspection.toString()).c_str());
                domain->comms->down();
            }                
            
        }catch( const std::runtime_error& error ){
            spinnerEnd();       
            String chainedError = String( "ERROR: Could not POST: " ) + error.what();           
            createDialog( chainedError.c_str() );
        }

        spinnerEnd();       
        Serial.println("Submit ... done!");
}


//==================================================================================================================================
//==================================================================================================================================
//==================================================================================================================================


    void renderAssetZones(){

        Serial.println("Render asset zones...");
            
        domainManagerClass* domain = domainManagerClass::getInstance();

        // reset zones and compos
        lv_obj_clean(objects.zone_list);
        lv_obj_clean(objects.zone_component_list);

        // Find the layout for the asset
        layoutClass* layout = nullptr;
        for (layoutClass& l : domain->layouts) {
            if (l.name == lastSelectedAsset->layoutName) {
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
    
                
        Serial.println("Render asset zones...done!");
        return;
    }   

    void renderComponents(){  

        Serial.println("Render components ...");

        lv_obj_t* btn = NULL;

        // find the selected zone btn
        uint32_t child_count = lv_obj_get_child_cnt(objects.zone_list);
        if (child_count == 0) return;
        for (uint32_t i = 0; i < child_count; ++i) {
            lv_obj_t* b = lv_obj_get_child(objects.zone_list, i);
            if (!lv_obj_check_type(b, &lv_btn_class)) continue;

            if (lv_obj_has_state(b, LV_STATE_CHECKED)) {
                btn = b;
                break;
            }
        }    

        if(!btn) return;

        // clean compos
        lv_obj_clean(objects.zone_component_list);

        // get the zone
        layoutZoneClass* zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(btn));
        if (!zone) {
            throw std::runtime_error("Zone user_data is null in ZONE click handler");
        }
        if (!lastSelectedAsset) {
            throw std::runtime_error("lastSelectedAsset is null in ZONE click handler");
        }

        domainManagerClass* domain = domainManagerClass::getInstance();
        const std::vector<defectClass>& defects = domain->currentInspection.defects;


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
    
        Serial.println("Render components ...done!");

        return;

    }  



//-----------------------------------------------------------


    void refreshZoneAndComponentFlags() {

        updateAssetSeverityLabels();
        updateZoneSeverityLabels();
        updateComponentSeverityLabels();
    }        

    void updateAssetSeverityLabels() {
        domainManagerClass* domain = domainManagerClass::getInstance();

        Serial.println("Refresh Asset flags:");

        uint32_t count = lv_obj_get_child_cnt(objects.zone_asset_list);
        for (uint32_t i = 0; i < count; ++i) {

            // get the asset button
            lv_obj_t* assetButton = lv_obj_get_child(objects.zone_asset_list, i);
            if (!assetButton) continue;

            // get the cargo
            assetClass* asset = static_cast<assetClass*>(lv_obj_get_user_data(assetButton));
            if (!asset) continue;

            Serial.print(asset->ID);
            Serial.print(" Defect?");

            // check against defects
            int maxSeverity = -1;
            for (const defectClass& defect : domain->currentInspection.defects) {
                if (defect.asset.ID == asset->ID) {

                    Serial.print( defect.severity );  // report over fix
                    Serial.print(" ");

                    if (defect.severity > maxSeverity) {
                        maxSeverity = defect.severity;
                    }
                }
            }

            String prefix;
            if (maxSeverity == 10) prefix = String(LV_SYMBOL_CLOSE) + " ";
            else if (maxSeverity == 1) prefix = String(LV_SYMBOL_WARNING) + " ";
            else if (maxSeverity == 0) prefix = String(LV_SYMBOL_OK) + " ";
            else prefix = "";

            lv_obj_t* label = lv_obj_get_child(assetButton, 0);
            if (label) {
                String newText = prefix + asset->buttonName;
                lv_label_set_text(label, newText.c_str());

                Serial.print(" -> ");
                Serial.println(newText);
            }
        }
    }


    void updateZoneSeverityLabels() {

        Serial.println("Refresh Zone flags:");

        domainManagerClass* domain = domainManagerClass::getInstance();
        if (!lastSelectedAsset) return;

        uint32_t count = lv_obj_get_child_cnt(objects.zone_list);
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* zoneButton = lv_obj_get_child(objects.zone_list, i);
            if (!zoneButton) continue;

            layoutZoneClass* zone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(zoneButton));
            if (!zone) continue;

            Serial.print("Zone:");
            Serial.print(zone->name);
            Serial.print(" def:");

            // Get max severity for this zone
            int maxSeverity = -1;
            for (const defectClass& defect : domain->currentInspection.defects) {
                if (defect.asset.ID == lastSelectedAsset->ID && defect.zoneName == zone->tag) {
                    Serial.print(defect.severity);
                    Serial.print(" ");
                    if (defect.severity > maxSeverity) {
                        maxSeverity = defect.severity;
                    }
                }
            }

            String prefix;
            if (maxSeverity == 10) prefix = String(LV_SYMBOL_CLOSE) + " ";
            else if (maxSeverity == 1) prefix = String(LV_SYMBOL_WARNING) + " ";
            else if (maxSeverity == 0) prefix = String(LV_SYMBOL_OK) + " ";
            else prefix = "";

            // Update label text
            lv_obj_t* label = lv_obj_get_child(zoneButton, 0); // Assuming first child is label
            if (label) {
                String newText = prefix + zone->name;
                lv_label_set_text(label, newText.c_str());
                Serial.print(" label:");
                Serial.println(newText);
            }
        }
    }

    void updateComponentSeverityLabels() {
        domainManagerClass* domain = domainManagerClass::getInstance();
        if (!lastSelectedAsset) return;

        Serial.println("Refresh Compo flags:");

        // --- Find the selected zone ---
        layoutZoneClass* selectedZone = nullptr;
        uint32_t zoneCount = lv_obj_get_child_cnt(objects.zone_list);
        for (uint32_t i = 0; i < zoneCount; ++i) {
            lv_obj_t* zoneButton = lv_obj_get_child(objects.zone_list, i);
            if (!zoneButton) continue;

            if (lv_obj_has_state(zoneButton, LV_STATE_CHECKED)) {
                selectedZone = static_cast<layoutZoneClass*>(lv_obj_get_user_data(zoneButton));
                break;
            }
        }
        if (!selectedZone) return; // no zone selected, nothing to update

        Serial.print("Zone:");
        Serial.println(selectedZone->tag);

        // --- Update all component labels for selected zone ---
        uint32_t count = lv_obj_get_child_cnt(objects.zone_component_list);
        for (uint32_t i = 0; i < count; ++i) {
            lv_obj_t* compButton = lv_obj_get_child(objects.zone_component_list, i);
            if (!compButton) continue;

            // get the name form the comp vec
            std::vector<String>* compVec = static_cast<std::vector<String>*>(lv_obj_get_user_data(compButton));
            if (!compVec || compVec->size() < 2) continue;
            String componentName = (*compVec)[1];

            Serial.print(" Comp:");
            Serial.print(componentName);
            Serial.print(" ");                                    

            int maxSeverity = -1;
            for (const defectClass& defect : domain->currentInspection.defects) {            

                if (defect.asset.ID == lastSelectedAsset->ID &&
                    defect.zoneName == selectedZone->tag &&
                    defect.componentName == componentName) 
                    {

                        if (defect.severity > maxSeverity) maxSeverity = defect.severity;
                        Serial.print(defect.severity);
                        Serial.print(" ");                        
                    }

            }

            String prefix;
            if (maxSeverity == 10) prefix = String(LV_SYMBOL_CLOSE) + " ";
            else if (maxSeverity == 1) prefix = String(LV_SYMBOL_WARNING) + " ";
            else if (maxSeverity == 0) prefix = String(LV_SYMBOL_OK) + " ";
            else prefix = "";

            lv_obj_t* label = lv_obj_get_child(compButton, 0);
            if (label) {
                String newText = prefix + componentName;
                lv_label_set_text(label, newText.c_str());

                Serial.print("  -> Label:");
                Serial.println(newText);
            }
        }
    }    




//==================================================================================================================================
//==================================================================================================================================
//==================================================================================================================================



    void open() override {
        domainManagerClass* domain = domainManagerClass::getInstance();

        // Clear existing items
        lv_obj_clean(objects.zone_asset_list);
        lv_obj_clean(objects.zone_list);
        lv_obj_clean(objects.zone_component_list);

        bool defaulted =  false;
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

        { // key nav

            // default
            lv_group_add_obj(inputGroup, objects.zone_asset_list  );
            lv_group_add_obj(inputGroup, objects.zone_list  );            
            lv_group_add_obj(inputGroup, objects.zone_component_list  );            

            // too many items in the group, lets only do number shortcuts
            //lv_group_add_obj(inputGroup, objects.all_ok_button  );
            //lv_group_add_obj(inputGroup, objects.comp_ok_button  );            
            //lv_group_add_obj(inputGroup, objects.defect_button  );            

            // nav bar -- also too complicated, use numbers this will never work
            lv_group_add_obj(inputGroup, objects.back_from_form_zones );            
            lv_group_add_obj(inputGroup, objects.submit);       
        }

        screenClass::open();

        // activate the first asset
        uint32_t count = lv_obj_get_child_cnt(objects.zone_asset_list);
        if (count > 0) {
            lv_obj_t* first_asset_btn = lv_obj_get_child(objects.zone_asset_list, 0);
            if (lv_obj_check_type(first_asset_btn, &lv_btn_class)) {
                Serial.println("Activate first asset in list.");

                //tbd                
            }
        }


    }

    void syncToInspection() {
        // Placeholder if needed later
    }

    virtual ~inspectionZonesScreenClass() {
        domainManagerClass* domain = domainManagerClass::getInstance();
        domain->currentInspection.defects.clear();
    }


    //---------------------------------------------

    bool defectDialogOpen = false;
    void closeDefectDialog(){
        Serial.println("Close defect dialog ...");                
        if( defectDialogOpen ) lv_msgbox_close_async(dialog1);       
        defectDialogOpen = false;
    }

        
    //---------------------------------------------    

    lv_obj_t *dialog1 = nullptr;
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
            if (d.asset.ID == (*selected_asset).ID && d.zoneName == selected_zone->tag && d.componentName == selected_component_name) {
                existingDefect = &d;
                break;
            }
        }

        // Create defectDialog
        dialog1 = lv_msgbox_create(NULL, "", "", 0, true);      
        defectDialogOpen = true;

        lv_obj_set_pos(dialog1, 89, 39);
        lv_obj_set_size(dialog1, 626, 400); // Make dialog taller to fit keyboard
        lv_obj_clear_flag(dialog1, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER |
                                    LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW);
        lv_obj_set_style_align(dialog1, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_layout(dialog1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        // add event handler
        close_btn = lv_msgbox_get_close_btn(dialog1);
        if (close_btn) {
            Serial.println("set handler!");            
            lv_obj_add_event_cb(close_btn, action_main_event_dispatcher, LV_EVENT_PRESSED, (void*)0);
        }

        {
            lv_obj_t *parent_obj = dialog1;

            // delete button
            del_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(del_btn, 20, 305);
            lv_obj_set_size(del_btn, 164, 40);
                lv_obj_add_event_cb(del_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(del_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *del_label = lv_label_create(del_btn);
            lv_obj_set_style_align(del_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(del_label, "1 delete");

            // minorButton
            minor_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(minor_btn, 202, 305);
            lv_obj_set_size(minor_btn, 164, 40);
                lv_obj_add_event_cb(minor_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(minor_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *minor_label = lv_label_create(minor_btn);
            lv_obj_set_style_align(minor_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(minor_label, "2 minor");

            // majorButton
            major_btn = lv_btn_create(parent_obj);
            lv_obj_set_pos(major_btn, 385, 305);
            lv_obj_set_size(major_btn, 164, 40);
                lv_obj_add_event_cb(major_btn, action_main_event_dispatcher, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(major_btn, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *major_label = lv_label_create(major_btn);
            lv_obj_set_style_align(major_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(major_label, "3 major");

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
                    else if (i == 2 && existingDefect) {
                        if( existingDefect->defectType == "GOOD" ){ 
                            lv_obj_add_state(defect_btn, LV_STATE_CHECKED);
                        }
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

//==============================================
   
};


//----






