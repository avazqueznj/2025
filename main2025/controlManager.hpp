#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "src/screens.h"
#include <exception>
#include "domainManager.hpp"

//-------------------------------------------------

class screenClass{
public:
    ScreensEnum screenId;

    screenClass( ScreensEnum screenIdParam ): screenId{screenIdParam}{}
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
//-------------------------------------------------
//-------------------------------------------------


class mainScreenClass:public screenClass{
public:

    commsClass* comms = NULL;

    mainScreenClass(): screenClass( SCREEN_ID_MAIN ){    
        comms = new commsClass();
    }

    void handleEvents( lv_event_t* e ) override{
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event
        if(target == objects.do_sync ){
            Serial.println("main: sync ...");  
            try{

                comms->up();
                comms->getConfigFromServer();

            }catch( const std::runtime_error& error ){
                Serial.println("** ERROR **");  
                Serial.println( error.what() );  
            }

        }else
        if(target == objects.do_settings ){
            Serial.println("main: settings ...");  

            
        }else{
            Serial.println("main: unkown event ?");  
        }
    }

    virtual ~mainScreenClass(){
        delete comms;
    };
};


//-------------------------------------------------


class selectAssetScreenClass:public screenClass{
public:
    selectAssetScreenClass(): screenClass( SCREEN_ID_SELECT_ASSET_SCREEN ){    
    }

    void handleEvents( lv_event_t* e ) override{       
    }

    virtual ~selectAssetScreenClass(){};
};


//-------------------------------------------------


#endif 
