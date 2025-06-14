#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "src/screens.h"
#include <exception>

//-------------------------------------------------

class screenClass{
public:
    // uses
    ScreensEnum screenId;
    // has

    screenClass( ScreensEnum screenIdParam ): screenId{screenIdParam}
    {
    }

    virtual void open(){
        loadScreen( screenId );        
    };

    virtual bool close(){
        return( true );
    }

    virtual ~screenClass(){
    }
};

//-------------------------------------------------


class mainScreenClass:public screenClass{
public:
    mainScreenClass(): screenClass( SCREEN_ID_MAIN ){    
    }

    virtual ~mainScreenClass(){};
};


//-------------------------------------------------


#endif 
