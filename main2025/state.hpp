#ifndef STATE_H
#define STATE_H

#include "controlManager.hpp"

class stateClass{
private:
public:

    //uses
    // has
    screenClass* currentScreen;
    
    stateClass( ){            
      currentScreen =  NULL;  
    }    
            
    virtual ~stateClass(){   
    }

    void openMainScreen(){
      Serial.println("state: open main screen ....");  
      if( currentScreen != NULL ){
            if( !currentScreen->close() ) return;            
            delete currentScreen;          
          }                      
      currentScreen = new mainScreenClass();
      currentScreen->open();
    }
    
};


#endif