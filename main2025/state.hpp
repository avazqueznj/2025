/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/


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

   void rfidEvent( byte *uid, byte length ){
    try{
      if( currentScreenState !=  NULL ){
        Serial.println( "*** RFID event ***" );                    
        currentScreenState->rfidEvent( uid, length );
      }
    }catch( const std::runtime_error& error ){
      Serial.println( "*** ERROR while handling RFID event ***" );                    
      Serial.println( error.what() );                    
    }          
  }

  void clockTic( String time ){
    try{
      if( currentScreenState !=  NULL ){
        currentScreenState->clockTic(  time );
      }
    }catch( const std::runtime_error& error ){
      Serial.println( "*** ERROR while handling clock event ***" );                    
      Serial.println( error.what() );                    
    }         
  }  

  void keyboardEvent( String input ){
    try{
      if( currentScreenState !=  NULL ){
        currentScreenState->keyboardEvent( input );
      }
    }catch( const std::runtime_error& error ){
      Serial.println( "*** ERROR while handling keyboard event ***" );                    
      Serial.println( error.what() );                    
    }  
  }

  void handleEvents( lv_event_t* e ){

      try{

        Serial.println("state: Event ...");    
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event


        // MAIN -> open settings
        if(target == objects.do_settings ){
          Serial.println("state: Open settingsScreenClass..");            
          openScreen( new settingsScreenClass() );
        }else        


        // MAIN -> open select asset
        if(target == objects.do_inspect_button ){
          Serial.println("state: Open selectAssetScreenClass..");    
          openScreen( new selectAssetScreenClass() );
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


        // INSPE -> open form
        if(target == objects.do_inspection_form ){
          Serial.println("state: Open FF ..");            

          // sync the inspe type
          if (currentScreenState && currentScreenState->screenId == SCREEN_ID_SELECT_INSPECTION_TYPE) {          
              static_cast<selectInspectionTypeScreenClass*>(currentScreenState)->syncToInspection();
          } else {
              Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
          }

          openScreen( new formFieldsScreenClass() );

        }else        


        // FORM -> open zones
        if(target == objects.do_zones ){
          Serial.println("state: Open Zones ..");            

          // sync the inspe type
          if (currentScreenState && currentScreenState->screenId == SCREEN_ID_INSPECTION_FORM) {          
              static_cast<formFieldsScreenClass*>(currentScreenState)->syncToInspection();
          } else {
              Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
          }

          openScreen( new inspectionZonesScreenClass() );

        }else        


        // -----------------------------------------------
        // NAV  BUTTONS
        // -----------------------------------------------

        //  back from asset or settings -> to main
        if( target == objects.back_from_select_asset || target == objects.back_from_settings ){
          Serial.println("state: Open mainScreenClass..");            
          openScreen( new mainScreenClass() );        
        }else

        // back from select inspe
        if( target == objects.back_from_select_insp  ){
          Serial.println("state: Open select asset..");      
          openScreen( new selectAssetScreenClass() );        
        }else

        // back from FF
        if( target == objects.back_from_form_fields  ){
          Serial.println("state: Open select inspe..");      
          openScreen( new selectInspectionTypeScreenClass() );        
        }else


        // back from ZONES
        if( target == objects.back_from_form_zones  ){
          Serial.println("state: Open form ");      
          openScreen( new formFieldsScreenClass() );        
        }else


        // -------------------------------------------------------
        // else pass the event to the active screen
        if( currentScreenState != NULL ){ 
          Serial.println("state: ? Forwarding ...");    
          currentScreenState->handleEvents( e );          
        }

      }catch( const std::runtime_error& error ){
        Serial.println( "*** ERROR while handling event ***" );                    
        Serial.println( error.what() );                    
      }      
  }

  // SCREEN NAVIGATION

  // Note: if open() throws, we deliberately do NOT delete the screen here.
  // Rationale:
  //   - If open() fails, the object may be partially initialized.
  //   - Deleting a partially initialized object risks undefined behavior.
  //   - Small leak is safer than a risky destructor call.
  // In normal flow, open() should never fail — but this pattern ensures
  // fail-safe behavior if something unexpected happens.  

  void openScreen( screenClass* screen ){        
    try{

      // ok well see if it opens
      screen->open();         

      // ok replace and go
      delete currentScreenState;
      currentScreenState = screen;

    }catch( const std::runtime_error& error ){
        // no delete here, as designed, leak it, i dont care.
        Serial.println( error.what() );            
        createDialog( error.what() );  
    }
  }

};


//------------------------------------------------------

extern stateClass* stateManager;
#include "src/actions.h"
extern "C" void action_main_event_dispatcher(lv_event_t *e) {
  if ( (lv_event_get_code(e) != LV_EVENT_PRESSED) && (lv_event_get_code(e) != LV_EVENT_CLICKED)   )return;    
  
  if (stateManager != NULL) {
        stateManager->handleEvents(e);
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