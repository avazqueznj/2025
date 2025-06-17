#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "src/screens.h"
#include <exception>
#include <WiFi.h>

WiFiClient client;

//-------------------------------------------------------------------------------------

class commsClass{
public:

    String ssid = "irazu5g";
    String pass = "Casiopea3";

    enum connectionState{
        OFF,
        ON,
        CONNECTED1
    };
    connectionState connState = OFF;

    commsClass(){
    }


    void printWifiStatus() {

        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());

        IPAddress ip = WiFi.localIP();
        Serial.print("IP Address: ");
        Serial.println(ip);

        long rssi = WiFi.RSSI();
        Serial.print("signal strength (RSSI):");
        Serial.print(rssi);
        Serial.println(" dBm");
    }

    void up(){

        Serial.println("WIFI connect....");  

        if( connState == ON ){
            Serial.println( "Error attempt connect when connection is ON, ignoring ..." );
            return;
        }    
        if( connState != OFF ){
            throw std::runtime_error( "Error attempt connect when connection is not OFF or ON" );
        }    
        if (WiFi.status() == WL_NO_SHIELD) {
            throw std::runtime_error("Fatal WL_NO_SHIELD" );
        }

        Serial.println("Attempting to connect to SSID: ");
        Serial.println( ssid.c_str() );
        Serial.println( pass.c_str() );
        
        int status = 4;
        for( int i = 0 ; i < 20; i++){
            status = WiFi.begin( ssid.c_str(), pass.c_str() );
            if( ( status ) != WL_CONNECTED ){
                Serial.print("Waiting ... status: ");             
                Serial.println( status );             
                delay(1000);
            }else{
                break;
            }
        }

        if( status != WL_CONNECTED ){
            throw std::runtime_error("While connecting: not [WL_CONNECTED] after 10 sec"); 
        } else
        {
            Serial.println("Connected !!");             
            printWifiStatus();
            connState = ON;
        }        
    }

    void down(){        
    }

    String readNextRow(){

        int wait = 0;
        String currentRow;
        while (true) {            

            if( !client.connected() ){
                Serial.println("Server disconnected!!");
                break;
            }

            if (client.available()) {
                char c = client.read();                                 
                if( c == '\r' or c == '\n' ){                    
                    if( currentRow == "" ) continue;
                    return( currentRow );
                }
                currentRow += c;
            }else{
                Serial.println("{no data wait}");
                delay(100);
                wait += 100;
                if( wait >= 10000 ) throw std::runtime_error( "*** ERROR *** read timeout -" );
            }                        
        }

        Serial.println("READ---------------------- done!");

        Serial.println("Release client!");
        client.stop();            
        connState =  ON;

        return("");
    }

    void connectToServer( ){

        if( connState != ON ){
            throw std::runtime_error( "Error attempt read when connection is not ON" );
        }    

        Serial.println("Starting connection to server... http://155.138.194.237:8080/server2025/config ");
        IPAddress server(155,138,194,237);
        for( int i = 0 ; i < 3; i++){
            if( !client.connect( server, 8080 ) ){
                Serial.println("Starting connection to server... FAILED!");
                Serial.println("Starting connection to server... retry....");                
                delay( 5000 );  
            }else{
                Serial.println("Connected to server!");     
                connState =  CONNECTED1;
                break;
            }
        }

        Serial.println("GET /server2025/config ...");
        // send request
        client.println("GET /server2025/config");
        client.println("Accept: */*");
        client.println("\r\n");
        
    }

  
    virtual ~commsClass(){
    }

};


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
                comms->connectToServer();
                String row;
                while( ( row = comms->readNextRow() ) != ""  ){
                    Serial.println( row );  
                }

                Serial.println("Sync is done !!!!!!!");  

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
