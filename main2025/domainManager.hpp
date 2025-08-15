/********************************************************************************************
 * CONFIDENTIAL AND PROPRIETARY
 * 
 * ZZZ2025 
 * © [2025] [Alejandro Vazquez]. All rights reserved.
 * 
 ********************************************************************************************/


#ifndef DOMAIN_H
#define DOMAIN_H

#include"comms.hpp"


#include "KVStore.h"
#include "kvstore_global_api.h"

#include "domain.hpp"


class domainManagerClass {
public:
    // has
    bool isLoaded = false;
    commsClass* comms = NULL;

    // database
    std::vector<userClass> users;    
    std::vector<assetClass> assets;
    std::vector<layoutClass> layouts;
    std::vector<inspectionTypeClass> inspectionTypes;

    // inspection
    inspectionClass currentInspection;

    //logged user
    userClass loggedUser;

    // singleton
    static domainManagerClass* getInstance() {
        static domainManagerClass instance;  // Guaranteed to be created once (thread-safe in C++11+)
        return &instance;
    }    

    domainManagerClass(){        
        comms = new commsClass();            
    }

    virtual ~domainManagerClass(){        
        delete comms;
    }

    void emptyAll() {

        currentInspection.clear();

        assets.clear();
        layouts.clear();
        inspectionTypes.clear();
        users.clear();
    }

    void login(String usernameParam, String passwordParam) {

        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].username == usernameParam && users[i].password == passwordParam) {

                loggedUser = users[i];

                Serial.print("Login successful: ");
                Serial.println(users[i].name);
                return; // Exit once we find a match
            }
        }

        // else..
        throw std::runtime_error("Invalid credentials");
    }

    void sync(){
        
        spinnerStart();

        try{

            comms->up();

                std::vector<String> config = comms->getContent();
                parse( &config );
                saveConfigToKVStore( &config );
                comms->syncClockWithNTP();

                    String syncMessage = "Sync successful. \n";
                    syncMessage += "Loaded: \n";

                    syncMessage += domainManagerClass::getInstance()->assets.size();
                    syncMessage += " assets \n";


                    syncMessage += domainManagerClass::getInstance()->layouts.size();
                    syncMessage += " layouts \n";


                    syncMessage += domainManagerClass::getInstance()->inspectionTypes.size();
                    syncMessage += " Inspection types \n";

                    syncMessage += domainManagerClass::getInstance()->users.size();
                    syncMessage += " Users \n";       

                    spinnerEnd();        
                    
                    createDialog( syncMessage.c_str() );                                

            comms->down();

        }catch( const std::runtime_error& error ){
            spinnerEnd();
            String chainedError = String( "ERROR: Could not sync: " ) + error.what();            
            throw std::runtime_error( chainedError.c_str() );
        }


    }


    void parse( std::vector<String>* config ){        
        Serial.println( "Parsing ..." );

        emptyAll();

        std::vector<String>::iterator iterator = config->begin();    
        while ( iterator != config->end() ) {   

            // HEADER ----->
            if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");            
            std::vector<String> tokens = tokenize( *iterator , '*' );     
            if( tokens[ 0 ] == "2025CONFIG" ){ 
                Serial.println( "[2025CONFIG] found..." );

                // ASSETS ----->
                if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                tokens = tokenize( *iterator , '*' );
                if( tokens[ 0 ] == "ASSETS" ){ 
                    Serial.println( "[ASSETS] found... load assets!" );

                    while( true ){ // read the next asset
                        ++iterator;tokens = tokenize( *iterator , '*' );
                        if( tokens[ 0 ] != "AS" ) break;

                        Serial.println( "[ASSET] found... load asset!" );
                        if( tokens.size() != 4 ) throw std::runtime_error( "Parse AS error, expecting 4 tokens" );
                        assets.push_back( assetClass( tokens[ 1 ] , tokens[ 2 ] , tokens[ 3 ]  ) );        
                        Serial.println( "Asset added!!" );
                    }

                }else{
                    throw std::runtime_error( "Parse error, expecting ASSETS" );
                }                    

                //  LAYOUTS ----->
                if( tokens[ 0 ] == "LAYOUTS" ){ 
                    Serial.println( "[LAYOUTS] found... load layout!" );
                    
                    if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                    tokens = tokenize( *iterator , '*' );                                        
                    while( true ){  // get the next  layout                        
                        if( tokens[ 0 ] != "LAY" ) break; // end layouts?                        

                        // get name
                        String layoutName; Serial.println( "Found LAY ..." );                        
                        if( tokens.size() == 2 ){
                            layoutName = tokens[ 1 ];
                        }else throw std::runtime_error( "Parse error token LAY expecting 2 tokens " );

                        // make layout step 1
                        layoutClass layout(layoutName);

                        // ZONES --->
                        if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                        tokens = tokenize( *iterator , '*' );
                        while( true ){ // read the next zone                            
                            if( tokens[ 0 ] != "LAYZONE" ) break;

                            Serial.println( "Found LayoutZone ..." );                        
                            if( tokens.size() == 3 ){
                                String zoneTag = tokens[ 1 ];
                                String zoneName = tokens[ 2 ];

                                // make layout step 2
                                layoutZoneClass zone(zoneName,zoneTag);

                                // COMPONENTS ----->
                                while( true ){ // read the next component
                                    if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                                    tokens = tokenize( *iterator , '*' );
                                    if( tokens[ 0 ] != "ZONECOMP" ) break;

                                    Serial.println( "Found Zone component & defects..." );                        
                                    if( tokens.size() >= 3 ){

                                        // make layout step 3
                                        zone.components.push_back( tokens );

                                    }else throw std::runtime_error( "Parse error token ZONECOMP expecting >=3 tokens " );                                                                        
                                }

                                layout.zones.push_back( zone );

                            }else throw std::runtime_error( "Parse error token LAYZONE expecting 3 tokens " );
                        }

                        // add layout
                        layouts.push_back( layout );

                    }                 
                }else{
                    throw std::runtime_error( "Parse error, expecting LAYOUTS" );
                }     

                //  INSPE TYPES ----->
                if( tokens[ 0 ] == "INSPTYPES" ){ 
                    Serial.println( "[INSPTYPES] found... load inpection types!" );

                    if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                    tokens = tokenize( *iterator , '*' );                                        
                    while( true ){  // get the next inspt
                        if( tokens[ 0 ] != "INSP" ) break; // end inspts
                        Serial.println( "Found INSP ..." );                        

                        // get name
                        String inspName; 
                        if( tokens.size() >= 3 ){
                            inspName = tokens[ 1 ];                            
                        }else throw std::runtime_error( "Parse error token INSP expecting >= 3 tokens " );

                        inspectionTypeClass inspType(inspName);
                        tokens.erase(tokens.begin(), tokens.begin() + 2);
                        inspType.layouts = tokens;

                        // FORM FIELDS --->                    
                        while( true ){ // read the next ff    
                            if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                            tokens = tokenize( *iterator , '*' );                        
                            if( tokens[ 0 ] != "INSPFF" ) break;

                            Serial.println( "Found FF ..." );                        
                            if( tokens.size() == 4 ){

                                tokens.erase(tokens.begin(), tokens.begin() + 1);
                                inspType.formFields.push_back( tokens );

                            }else throw std::runtime_error( "Parse error token FF expecting 4 tokens " );

                        }

                        // add inspt
                        inspectionTypes.push_back( inspType );

                    }     

                }else{
                    throw std::runtime_error( "Parse error, expecting INSPTYPES" );
                }                    

                //--

                //  USERS TYPES ----->
                if( tokens[ 0 ] == "USERS" ){ 
                    Serial.println( "[USERS] found... " );
                    
                    while( true ){  // get the next user            
                        if (++iterator == config->end()) throw std::runtime_error("Unexpected end ");
                        tokens = tokenize( *iterator , '*' );                                        
                        if( tokens[ 0 ] != "USER" ) break; // end users
                        Serial.println( "Found USER ..." );                        

                        // get data
                        String name; 
                        String username; 
                        String password; 
                        if( tokens.size() == 4 ){
                            name = tokens[ 1 ];
                            username = tokens[ 2 ];
                            password = tokens[ 3 ];
                        }else throw std::runtime_error( "Parse error token USER expecting 4 tokens " );
                        userClass nextUser( name, username, password );
                        users.push_back( nextUser );
                    }     

                }else{
                    throw std::runtime_error( "Parse error, expecting USERS" );
                }                    


                //--

                // done?
                if( tokens[ 0 ] == "END" ){ 
                    Serial.println( "Found [END]" );                
                    isLoaded = true;
                    printDebugContents();
                    return;
                }else{
                    Serial.println( "Parse error un expected token" );                
                    Serial.println( tokens[ 0 ] );                                    
                    throw std::runtime_error( "Parse error un expected token" );                    
                }

            } // header found
        } // scanning

        throw std::runtime_error( "Parse error: unexpected end of file." );   
    }
    

    void printDebugContents() {
        Serial.println("========= ASSETS =========");
        for (const assetClass& a : assets) {
            Serial.print("ID: "); Serial.print(a.ID);
            Serial.print(", Layout: "); Serial.print(a.layoutName);
            Serial.print(", Tag: "); Serial.println(a.tag);
        }

        Serial.println("========= LAYOUTS =========");
        for (const layoutClass& l : layouts) {
            Serial.print("Layout Name: "); Serial.println(l.name);
            for (const layoutZoneClass& z : l.zones) {
                Serial.print("  Zone Name: "); Serial.print(z.name);
                Serial.print(", Tag: "); Serial.println(z.tag);
                for (const auto& compGroup : z.components) {
                    Serial.print("    Component Group: ");
                    for (const String& comp : compGroup) {
                        Serial.print(comp); Serial.print(" | ");
                    }
                    Serial.println();
                }
            }
        }

        Serial.println("========= INSPECTION TYPES =========");
        for (const inspectionTypeClass& i : inspectionTypes) {
            Serial.print("Inspection Name: "); Serial.println(i.name);
            
            Serial.print("  Layouts: ");
            for (const String& layout : i.layouts) {
                Serial.print(layout); Serial.print(" | ");
            }
            Serial.println();

            Serial.println("  Form Fields:");
            for (const auto& fieldGroup : i.formFields) {
                Serial.print("    ");
                for (const String& field : fieldGroup) {
                    Serial.print(field); Serial.print(" | ");
                }
                Serial.println();
            }
        }

        Serial.println("========= USERS =========");
        for (size_t i = 0; i < users.size(); i++) {
            Serial.print("User ");
            Serial.print(i);
            Serial.print(": Name=");
            Serial.print(users[i].name);
            Serial.print(", Username=");
            Serial.print(users[i].username);
            Serial.print(", Password=");
            Serial.println( "*" );  // users[i].password
        }
        Serial.println("-------------------");

    }

    //------------------------------

    void saveConfigToKVStore(const std::vector<String>* config) {

        Serial.println("Config save to KVStore....");

        String joined = "";
        for (size_t i = 0; i < config->size(); i++) {
            joined += config->at(i);
            joined += '\n';
        }

        int ret = kv_set("/kv/config2025", joined.c_str(), joined.length(), 0);
        if (ret != MBED_SUCCESS) {
            throw std::runtime_error("Failed to save config to KVStore!");
        }

        Serial.println("Config saved to KVStore.");
    }

    void loadConfigFromKVStore() {
        Serial.println("Config load from KVStore and parsed  ....");

        size_t actual_size = 0;
        char buffer[4096]; // adjust if your config is bigger

        int ret = kv_get("/kv/config2025", buffer, sizeof(buffer), &actual_size);
        if (ret != MBED_SUCCESS || actual_size == 0) {
            throw std::runtime_error("Failed to read config from disk!\nHave you ever synced ?");
        }

        String joined = String(buffer).substring(0, actual_size);

        std::vector<String> config;
        int start = 0;
        int end = joined.indexOf('\n');
        while (end >= 0) {
            String line = joined.substring(start, end);
            line.trim();
            if (line.length() > 0) {
                config.push_back(line);
            }
            start = end + 1;
            end = joined.indexOf('\n', start);
        }

        parse(&config);

        Serial.println("Config loaded from KVStore and parsed!");
    }

//-------------------------------------------------

};

#endif 
