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

    virtual bool close(){
        return( true );
    }

    virtual void handleEvents( lv_event_t* e ){
        Serial.println("basescreen: event unhandled ...");  
    }

    virtual ~screenClass(){
    }
};




//-------------------------------------------------
// SCREEN BACKING STATE
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
                createDialog( "Sync successful." );   

            }catch( const std::runtime_error& error ){
                Serial.println( error.what() );            
                createDialog( error.what() );     
            }
        }else
        if(target == objects.do_settings ){

            Serial.println("main: settings ...");  
            
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
    std::deque< String > listButtonNames;

    //----------------------------------

    selectAssetScreenClass(): screenClass( SCREEN_ID_SELECT_ASSET_SCREEN ){}
    virtual ~selectAssetScreenClass(){};    

    //----------------------------------

    lv_obj_t* selectedButton = NULL;
    void handleEvents( lv_event_t* e ) override{     

        lv_obj_t* target = lv_event_get_target(e);

        if( target == objects.select_asset ){
            Serial.println("select");  
            return;
        }


        if( target == objects.de_select_asset ){
            Serial.println("de select");  
            return;
        }

        // only allow one selected asset at a time
        for (lv_obj_t* btn : listButtons) {     
            if (btn == target) {                
                selectedButton = btn;
                for (lv_obj_t* btn : listButtons) {
                    if (btn != target) {
                        lv_obj_clear_state(btn, LV_STATE_CHECKED);
                    }
                }
                break;
            }
        }        

    }

    //----------------------------------

    void open() override {

        domainManagerClass* domain = domainManagerClass::getInstance();        
        if( !domain->isLoaded ) throw std::runtime_error( "Error, no config has been loaded" );
    
        // reset
        listButtonNames.clear();
        listButtons.clear();
        lv_obj_clean(objects.asset_list);   

        // add buttons
        for (const assetClass& asset : domain->assets) {

            String buttonName;
            buttonName += asset.ID;
            buttonName += ":";
            buttonName += asset.layoutName;
            listButtonNames.push_back( buttonName );
            addAssetToList( &listButtonNames.back(), &asset );            
        }
        
        screenClass::open(); // always last, only if no issues
    }


    void addAssetToList( const String* assetLinePtr, const assetClass* asset ){
            
        lv_obj_t *parent_obj = objects.asset_list;
        {
            lv_obj_t* button = lv_btn_create(parent_obj);
            lv_obj_set_pos(button, 503, 42);
            lv_obj_set_size(button, 293, 50);

            lv_obj_add_event_cb(button, action_main_event_dispatcher, LV_EVENT_PRESSED, (void *) asset );

            lv_obj_add_flag(button, LV_OBJ_FLAG_CHECKABLE);

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
                    lv_label_set_text(obj, assetLinePtr->c_str() );
                }
            }

            listButtons.push_back( button );
        }        
    }
    

};


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

#endif 
