#ifndef COMMS_H
#define COMMS_H


#include <Arduino.h>
#include <exception>
#include <WiFi.h>
#include "util.hpp"

//-------------------------------------------------------------------------------------

WiFiClient client;
class commsClass{
public:

    String ssid = "irazu2G";
    String pass = "casiocasio";

    enum connectionState{
        OFF,
        ON,
        CONNECTED1
    };
    connectionState connState = OFF;

    commsClass(){
    }

    virtual ~commsClass(){
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
                delayBlink();     
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

    std::vector<String> getContent(){

        if( connState != ON ){
            throw std::runtime_error( "Error attempt read when connection is not ON" );
        }    

        Serial.println("Starting connection to server... http://155.138.194.237:8080/server2025/config ");
        IPAddress server(155,138,194,237);
        for( int i = 0 ; i < 3; i++){
            if( !client.connect( server, 8080 ) ){
                Serial.println("Starting connection to server... FAILED!");
                Serial.println("Starting connection to server... retry....");                
                delay( 3000 );  
                delayBlink();
            }else{
                Serial.println("Connected to server!");     
                connState =  CONNECTED1;
                break;
            }
        }

        if( connState != CONNECTED1 ){
            throw std::runtime_error( "Error could not connect to server !" );
        }

        // ok send content request
        Serial.println("GET /server2025/config ...");
        // send request
        client.println("GET /server2025/config");
        client.println("Accept: */*");
        client.println("\r\n");
        
        // read the response ...
        int wait = 0;
        std::vector<String> response;        
        String currentRow;
        while (true) {            

            if( !client.connected() ){
                Serial.println("Server disconnected!!");
                connState =  ON;
                break;
            }

            if (client.available()) {

                // read pending data
                char c = client.read();              

                // end of row   ...                 
                if( c == '\r' or c == '\n' ){                    
                    if( currentRow == "" ) continue;  // null line
                    response.push_back( currentRow );  // add, next ...       
                    Serial.println(currentRow );
                    currentRow = "";
                    continue;
                }

                // keep reading
                currentRow += c;

            }else{

                // no data ... wait
                Serial.println("{no data wait}");
                delayBlink();
                wait += 100;
                if( wait >= 10000 ) throw std::runtime_error( "*** ERROR *** read timeout -" );
            }                        
        }

        Serial.println("READ---------------------- done!");

        Serial.println("Release client!");
        client.stop();            
        connState =  ON;

        return response;
    }

    

};


#endif