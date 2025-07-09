/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/


#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <exception>
#include <WiFi.h>

#include <NTPClient.h>
#include "RTClib.h"

extern RTC_DS3231* rtc;

class commsClass{
public:
    WiFiClient client;

    // daday phone
    String ssid = "DadyPhone";
    String pass = "Casiopea1";

    // house
    //String ssid = "irazu2G";
    //String pass = "casiocasio";

    // hot spot
    //String ssid = "irazu5g";
    //String pass = "Casiopea3";


    String serverURL = "zzz2025.duckdns.org";

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

    void setConnectionInfo(const String& ssidParam, const String& passParam, const String& serverURLParam) {
        ssid = ssidParam;
        pass = passParam;
        serverURL = serverURLParam;
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

        Serial.print("Starting connection to server... ");        
        Serial.println(serverURL);        
        for( int i = 0 ; i < 3; i++){
            if( !client.connect( serverURL.c_str(), 8080 ) ){
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
        client.println();
        
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
                    if( response.size() > 10000 ){
                        throw std::runtime_error( "Error more than 10,000 rows read" );
                    }                
                    Serial.println(currentRow );
                    currentRow = "";
                    continue;
                }

                // keep reading
                currentRow += c;
                if( currentRow.length() > 1000 ){
                    throw std::runtime_error( "Error more than 1000 chars in line" );
                }                

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

    // redo all this in domain
    String postInspection(const String& payload) {

        Serial.println("POST connectiong!!");

        up();
        
        const char* host = "zzz2025.duckdns.org";
        const int port = 8080;
        if (!client.connect(host, port)) {
            createDialog("Connection failed");
            throw std::runtime_error("Conn failed"); 
        }

        String url = "/server2025/inspections";

        // Compose HTTP POST request
        String request = "";
        request += "POST " + url + " HTTP/1.1\r\n";
        request += "Host: " + String(host) + "\r\n";
        request += "Content-Type: text/plain\r\n";
        request += "Content-Length: " + String(payload.length()) + "\r\n";
        request += "Connection: close\r\n";
        request += "\r\n";
        request += payload;

        client.print(request);

        // Wait for server response
        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                createDialog("Client Timeout");
                client.stop();
                throw std::runtime_error("Server did not respond!"); 
            }
        }

        // Read response
        String response = "";
        while (client.available()) {
            String line = client.readStringUntil('\n');
            response += line + "\n";
        }

        client.stop();

        // Extract only body (optional)
        int bodyIndex = response.indexOf("\r\n\r\n");
        if (bodyIndex != -1) {
            response = response.substring(bodyIndex + 4);
        }

        return response;
    }

    void syncClockWithNTP() {

        if (connState != ON) {
            throw std::runtime_error("WiFi must be ON to sync clock");
        }

        WiFiUDP ntpUDP;
        NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

        Serial.println("Starting NTP sync...");
        timeClient.begin();

        int tries = 0;
        while (!timeClient.update()) {
            timeClient.forceUpdate();
            delay(500);
            tries++;
            if (tries > 10) {
                throw std::runtime_error("NTP sync failed: timeout");
            }
        }

        unsigned long epochTime = timeClient.getEpochTime();
        DateTime ntpTime(epochTime);

        rtc->adjust(ntpTime);  // Uses the global rtc

        Serial.println("RTC synced!");
    }

};


#endif