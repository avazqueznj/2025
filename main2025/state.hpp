#ifndef STATE_H
#define STATE_H

#include "controlManager.hpp"

class stateClass{
public:

  screenClass* currentScreenState = NULL;

  stateClass( ){            
    currentScreenState =  NULL;  
  }    
          
  virtual ~stateClass(){   
  }

  // BASE DISPATCH ----------------------------------------

  void handleEvents( lv_event_t* e ){

      Serial.println("state: Event ...");    
      lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event

      // MAIN -> open inspect
      if(target == objects.do_inspect_button ){
        Serial.println("state: Open selectAssetScreenClass..");    
        openScreen( new selectAssetScreenClass() );
      }else        

      // MAIN -> open settings
      if(target == objects.do_settings ){
        Serial.println("state: Open settingsScreenClass..");            
        openScreen( new settingsScreenClass() );
      }else        

      // SELECT -> open select inspection type from select asset
      if(target == objects.do_select_inspection_type ){

        Serial.println("state: Open select inspe ..");            

        // sync the assets
        if (currentScreenState && currentScreenState->screenId == SCREEN_ID_SELECT_ASSET_SCREEN) {          
            static_cast<selectAssetScreenClass*>(currentScreenState)->syncToInspection();
            if (domainManagerClass::getInstance()->currentInspection.assets.size() == 0) {
                createDialog("Error: No assets selected!");
                return;  
            }
        } else {
            Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
        }
        openScreen( new selectInspectionTypeScreenClass() );


      }else        



      // -----------------------------------------------
      // NAV  BUTTONS
      // -----------------------------------------------

      //  SETTINGS or SELECT back to MAIN
      if( target == objects.back_from_select_asset || target == objects.back_from_settings ){
        Serial.println("state: Open mainScreenClass..");            
        openScreen( new mainScreenClass() );        
      }else

      // back from select inspe
      if( target == objects.back_from_select_insp  ){
        Serial.println("state: Open select asset..");      
        openScreen( new selectAssetScreenClass() );        
      }else


      // -------------------------------------------------------
      // else pass the event to the active screen
      if( currentScreenState != NULL ){ 
        Serial.println("state: ? Forwarding ...");    
        currentScreenState->handleEvents( e );          
      }
      
  }

  // SCREEN NAVIGATION

  void openScreen( screenClass* screen ){        
    try{

      // ok well see if it opens
      screen->open();         

      // ok replace and go
      delete currentScreenState;
      currentScreenState = screen;

    }catch( const std::runtime_error& error ){
        Serial.println( error.what() );            
        createDialog( error.what() );  
    }
  }

};


//------------------------------------------------------

extern stateClass* stateManager;
#include "src/actions.h"
extern "C" void action_main_event_dispatcher(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_PRESSED) return;  
  if( stateManager != NULL ){
      stateManager->handleEvents( e );
  }                      
}

//------------------------------------------------------

#endif

/*

Fields of lv_event_t

lv_event_t is the only parameter passed to the event callback and it contains all data about the event. The following values can be gotten from it:

    lv_event_get_code(e) get the event code

    lv_event_get_current_target(e) get the object to which an event was sent. I.e. the object whose event handler is being called.

    lv_event_get_target(e) get the object that originally triggered the event (different from lv_event_get_target if event bubbling is enabled)

    lv_event_get_user_data(e) get the pointer passed as the last parameter of lv_obj_add_event_cb.

    lv_event_get_param(e) get the parameter passed as the last parameter of lv_event_send


*/