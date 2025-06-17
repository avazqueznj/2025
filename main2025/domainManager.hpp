#ifndef DOMAIN_H
#define DOMAIN_H

#include <Arduino.h>
#include <exception>

//-------------------------------------------------

extern WiFiClient client;
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

    void getConfigFromServer(){

        if( connState != ON ){
            throw std::runtime_error( "Error attempt read when connection is not ON" );
        }    

        Serial.println("Starting connection to server... ttp://155.138.194.237:8080/server2025/config ");
        IPAddress server(155,138,194,237);
        for( int i = 0 ; i < 3; i++){
            if( !client.connect( server, 8080 ) ){
                Serial.println("Starting connection to server... FAILED!");
                Serial.println("Starting connection to server... retry....");                
                delay( 5000 );  
            }else{
                connState =  CONNECTED1;
                break;
            }
        }

        Serial.println("GET /server2025/config ...");

        client.println("GET /server2025/config");
        client.println("Accept: */*");
        client.println("\r\n");


        for( int i = 0 ; i < 3; i++){
            if( client.available() ) break;
            Serial.println("No data ready ....");
            delay( 1000 ); 
        }

        if( !client.available() ) throw std::runtime_error( "Data ready timeout" );

        Serial.println("READ ----------------------");

            while ( client.available() ) {
                char c = client.read();
                Serial.write(c);
            }

        Serial.println("READ---------------------- done!");

        Serial.println("Disconnect!");
        if (!client.connected()) {
            Serial.println();
            Serial.println("disconnecting from server.");
            client.stop();            
        }        

        connState =  ON;
    }
    
    virtual ~commsClass(){
    }

};

//-------------------------------------------------

#endif 
