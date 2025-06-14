#ifndef STATE_H
#define STATE_H

#include "controlManager.hpp"

screenClass* currentScreenState = NULL;

class stateClass{
public:
  
  stateClass( ){            
    currentScreenState =  NULL;  
  }    
          
  virtual ~stateClass(){   
  }

  void handleEvents( lv_event_t* e ){
      lv_obj_t *target = lv_event_get_target(e);  // The object that triggered the event
      if(target == objects.do_inspect_button ){
        openSelectAssetScreen();
      }else{
        if( currentScreenState != NULL ) currentScreenState->handleEvents( e );          
      }
  }

  void openMainScreen(){
    Serial.println("state: open main screen ....");        
    if( currentScreenState != NULL ){
          if( !currentScreenState->close() ) return;            
          delete currentScreenState;          
        }                      
    currentScreenState = new mainScreenClass();
    currentScreenState->open();
  }

  void openSelectAssetScreen(){
      Serial.println("state: select asset ....");        
      if( currentScreenState != NULL ){
            if( !currentScreenState->close() ) return;            
            delete currentScreenState;          
          }                      
      currentScreenState = new selectAssetScreenClass();
      currentScreenState->open();
    }      
};


//------------------------------------------------------

extern stateClass* stateManager;

#include "src/actions.h"
extern "C" void action_main_event_dispatcher(lv_event_t *e) {
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