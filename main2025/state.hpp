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

  // handle touch events
  void handleEvents( lv_event_t* e ){

      try{

        Serial.print("State:Touch");
        lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event

        // navigation
        handleNavClicks( target, "" );

        // windows
        if( currentScreenState != NULL ){ 
          Serial.println("state: ? Forwarding ...");    
          currentScreenState->handleEvents( e );          
        }

      }catch( const std::runtime_error& error ){
        Serial.println( "*** ERROR while handling event ***" );                    
        Serial.println( error.what() );                    
      }      
  }

  // key events -- ENTRY
  void keyboardEvent( String key ){
    try{

      Serial.print("State:Key:");
      Serial.println(key);

      // are we in system modal ?
      if ( overlay != nullptr ) {    
        Serial.println( "State: System modal" );
        // dismiss dialog
        if (key == "#" ) {    
          Serial.println("Keyboard # -> closing dialog");
          lv_obj_t * mbox = lv_obj_get_child(overlay, 0);  // If overlay only has the mbox.
          lv_msgbox_close(mbox);
          lv_obj_del(overlay);
          overlay = nullptr;
        }
        return; // eat the events, simulate modal
      }


      // window modal  
      if( currentScreenState !=  NULL  ){ 
        if( currentScreenState->modalActive() ) {
          Serial.println( "State: window modal" );
          return;
        }          
      }


      // NAV ??
      if( currentScreenState !=  NULL  ){ 
        if( lv_obj_t* target = currentScreenState->getFocusedButton() ){
          if( handleNavClicks( target, key ) ) return; // eat it
        }
      }


      // SCREEN TO SCREEN NAVI button shortcuts
      if (
          (key == "7" || key == "9") &&
          !(currentScreenState && 

            // and not inside a text area, bcs then pass the raw input
            lv_group_get_focused(currentScreenState->inputGroup) && 
            lv_obj_check_type(lv_group_get_focused(currentScreenState->inputGroup), &lv_textarea_class))
      ) {
        
          const char* target_text = NULL;
          if (key == "7") {
              target_text = "\xEF\x81\x93"; 
          } else if (key == "9") {
              target_text = "\xEF\x81\x94"; 
          }

          lv_obj_t* found = NULL;
          lv_obj_t* stack[64];
          int top = 0;
          stack[top++] = lv_scr_act();

          // find the NVAI buttons by label
          while (top > 0 && found == NULL) {
              lv_obj_t* parent = stack[--top];

              int i = 0;
              lv_obj_t* child = lv_obj_get_child(parent, i);
              while (child != NULL && found == NULL) {
                  if (lv_obj_is_visible(child)) {
                      if (lv_obj_get_class(child) == &lv_btn_class) {
                          int j = 0;
                          lv_obj_t* lbl = lv_obj_get_child(child, j);
                          while (lbl != NULL) {
                              if (lv_obj_check_type(lbl, &lv_label_class)) {
                                  const char* txt = lv_label_get_text(lbl);
                                  if (txt != NULL && strstr(txt, target_text) != NULL) {
                                      found = child;
                                      break;
                                  }
                              }
                              j++;
                              lbl = lv_obj_get_child(child, j);
                          }
                      }
                      if (top < 64) {
                          stack[top++] = child;
                      }
                  }
                  i++;
                  child = lv_obj_get_child(parent, i);
              }
          }

          if (found != NULL) {
              lv_event_send(found, LV_EVENT_PRESSED, NULL);
              Serial.println("Simulated PRESS on arrow button.");
          } else {
              Serial.println("Arrow button not found.");
          }

          return;
      }
 

      // no, well then pass the event to the current screen
      if( currentScreenState !=  NULL ){
        currentScreenState->keyboardEvent( key );
      }

    }catch( const std::runtime_error& error ){
      Serial.println( "*** ERROR while handling keyboard event ***" );                    
      Serial.println( error.what() );                    
    }  
  }



  // enter and touch common NAVI
  // so this is here so that screens do not know  each other, ie change in one screen ui will never impact another
  bool handleNavClicks( lv_obj_t *target, String key){

    bool handeled = false;

    try{

        // MAIN screen shortcuts
        if (lv_scr_act() == objects.main) {      

            // MAIN -> start inspection, select asset
            if( ( target == objects.do_inspect_button && key == ""  ) || ( target == objects.do_inspect_button && key == "#"  ) || key == "1" ){
              Serial.println("state: Open selectAssetScreenClass..");    
              openScreen( new selectAssetScreenClass() );
              handeled = true;
            }        

            // MAIN: sync
            if( ( target == objects.do_sync && key == ""  ) || ( target == objects.do_sync && key == "#"  ) || key == "2" ){
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

                handeled = true;        
            }

            // MAIN -> open settings
            if( ( target == objects.do_settings && key == ""  ) || ( target == objects.do_settings && key == "#"  ) || key == "3" ){
              Serial.println("state: Open settingsScreenClass..");            
              openScreen( new settingsScreenClass() );
              handeled = true;
            }        

        }

        //-- Gral NAVI << >> touch or # press
        if( key == "#" || key == "" ){

          // INSPE TYPE -> open form fields
          if(target == objects.do_inspection_form ){
            Serial.println("state: Open FF ..");            

                  // sync the inspe type
                  if (currentScreenState && currentScreenState->screenId == SCREEN_ID_SELECT_INSPECTION_TYPE) {          
                      static_cast<selectInspectionTypeScreenClass*>(currentScreenState)->syncToInspection();
                  } else {
                      Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
                  }

            handeled = true;
            openScreen( new formFieldsScreenClass() );

          }        


          // FORM -> open zones
          if(target == objects.do_zones ){
            Serial.println("state: Open Zones ..");            

                // sync the inspe type
                if (currentScreenState && currentScreenState->screenId == SCREEN_ID_INSPECTION_FORM) {          
                    static_cast<formFieldsScreenClass*>(currentScreenState)->syncToInspection();
                } else {
                    Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
                }

            handeled = true;
            openScreen( new inspectionZonesScreenClass() );

          }        

          // NAV

          // SELECT -> open select inspection type from select asset
          if(target == objects.do_select_inspection_type ){

            Serial.println("state: Open select inspe ..");            

                  // sync the assets
                  if (currentScreenState && currentScreenState->screenId == SCREEN_ID_SELECT_ASSET_SCREEN) {          
                      static_cast<selectAssetScreenClass*>(currentScreenState)->syncToInspection();
                      if (domainManagerClass::getInstance()->currentInspection.assets.size() == 0) {
                          createDialog("Error: No assets selected!");
                          handeled = true;
                          return handeled;  
                      }
                  } else {
                      Serial.println("Current screen is NOT selectAssetScreenClass, skipping sync.");
                  }

            openScreen( new selectInspectionTypeScreenClass() );
            handeled = true;

          }       
          
          //  back from asset or settings -> to main
          if( target == objects.back_from_select_asset || target == objects.back_from_settings ){
            Serial.println("state: Open mainScreenClass..");     
            handeled = true;                 
            openScreen( new mainScreenClass() );        
          }

          // back from select inspe
          if( target == objects.back_from_select_insp  ){
            Serial.println("state: Open select asset..");      
            handeled = true;                           
            openScreen( new selectAssetScreenClass() );        
          }

          // back from FF
          if( target == objects.back_from_form_fields  ){
            Serial.println("state: Open select inspe..");  
            handeled = true;                               
            openScreen( new selectInspectionTypeScreenClass() );        
          }

          // back from ZONES
          if( target == objects.back_from_form_zones  ){
            Serial.println("state: Open form ");      
            handeled = true;                           
            openScreen( new formFieldsScreenClass() );                
          }
        }
      
      return handeled;

    }catch( const std::runtime_error& error ){
      Serial.println( "*** ERROR while handling event ***" );                    
      Serial.println( error.what() );    

      createDialog( error.what() );     

      return handeled;           
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
        Serial.println( "*** window closed but not recycled ***" );   
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