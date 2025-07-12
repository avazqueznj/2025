/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/

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

    void clockTic( String time ) override {
        lv_label_set_text( objects.clock, time.c_str());
    }

    void keyboardEvent(String key) override {        
        screenClass::keyboardEvent( key );
    }

    void handleEvents( lv_event_t* e ) override{
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event

        
    }

    void open() override {

        // Create fresh group or reuse existing
        {
            //-------------------------------------            
            // Add focusable widgets (  lv_group_focus_obj(objects.do_inspect_button);             )


            lv_group_add_obj(inputGroup, objects.do_inspect_button  );
            lv_group_add_obj(inputGroup, objects.do_sync);
            lv_group_add_obj(inputGroup, objects.do_settings);
            
        }
    
        // Add focusable widgets
        //-------------------------------------

        screenClass::open();
    }

    virtual ~mainScreenClass(){
    };
};




