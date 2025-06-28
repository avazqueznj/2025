#ifndef DOMAIN_H
#define DOMAIN_H

#include"comms.hpp"


#include "KVStore.h"
#include "kvstore_global_api.h"

//-------------------------------------------------

// D O M A I N 

//-------------------------------------------------

class layoutZoneClass{
public:
    String name;
    String tag;
    std::vector<  std::vector<String>  > components;    
    layoutZoneClass(String nameParam, String tagParam):name(nameParam),tag(tagParam){}
    virtual ~layoutZoneClass(){}
};

class layoutClass{
public:
    String name;
    std::vector<  layoutZoneClass  > zones;    
    layoutClass(String nameParam):name(nameParam){}
    virtual ~layoutClass(){}    
};

class assetClass{
public:
    String ID;
    String layoutName;
    String tag;
    String buttonName;
    assetClass( const String IDparam, const String typeParam, const String tagParam ):
        ID(IDparam),layoutName(typeParam),tag(tagParam) {    
            buttonName += ID;
            buttonName += ": ";
            buttonName += layoutName;
        }
    virtual ~assetClass(){}            
};

class inspectionTypeClass{
public:
    String name;
    std::vector<  String  > layouts;    
    std::vector<  std::vector<String>  > formFields;    
    std::vector<  String  > formFieldValues;    
    inspectionTypeClass(String nameParam):name(nameParam){}
    virtual ~inspectionTypeClass(){}    
};

//----------------------
//----------------------
//----------------------


class defectClass {
public:
    std::string zoneName;
    std::string componentName;
    std::string defectType;
    int severity;
    std::string notes;

    defectClass(const std::string& zoneName,
                const std::string& componentName,
                const std::string& defectType,
                int severity,
                const std::string& notes)
        : zoneName(zoneName),
          componentName(componentName),
          defectType(defectType),
          severity(severity),
          notes(notes)
    {}
};


class inspectionClass {
public:
    inspectionTypeClass* type = NULL;
    std::vector<assetClass> assets;    
    std::vector<defectClass> defects;

    inspectionClass(){}

    void clear() {
        type = NULL;
        assets.clear();
        defects.clear();
    }

    std::string toString() const {
        std::string result = "INSPECTION\n";

        // --- Inspection Type ---
        if (type != NULL) {
            result += "Type: ";
            result += type->name.c_str();
            result += "\n";

            result += "Layouts:\n";
            for (const auto& layout : type->layouts) {
                result += " - ";
                result += layout.c_str();
                result += "\n";
            }

            result += "Form Fields:\n";
            size_t rowIndex = 0;
            for (const auto& row : type->formFields) {
                result += "  Row ";
                result += std::to_string(rowIndex++);
                result += ": ";
                for (size_t i = 0; i < row.size(); ++i) {
                    result += row[i].c_str();
                    if (i < row.size() - 1) {
                        result += ", ";
                    }
                }
                result += "\n";
            }

        } else {
            result += "Type: NULL\n";
        }

        // --- Assets ---
        result += "Assets:\n";
        for (const auto& asset : assets) {
            result += " - ID: ";
            result += asset.ID.c_str();
            result += ", Layout: ";
            result += asset.layoutName.c_str();
            result += ", Tag: ";
            result += asset.tag.c_str();
            result += ", ButtonName: ";
            result += asset.buttonName.c_str();
            result += "\n";
        }

        // --- Defects ---
        result += "Defects:\n";
        for (const auto& defect : defects) {
            result += " - Zone: " + defect.zoneName;
            result += ", Component: " + defect.componentName;
            result += ", Type: " + defect.defectType;
            result += ", Severity: " + std::to_string(defect.severity);
            result += ", Notes: " + defect.notes;
            result += "\n";
        }

        return result;
    }
    
};

//-------------------------------------------------

class domainManagerClass {
public:
    // has
    bool isLoaded = false;
    commsClass* comms = NULL;

    // database
    std::vector<assetClass> assets;
    std::vector<layoutClass> layouts;
    std::vector<inspectionTypeClass> inspectionTypes;

    // inspection
    inspectionClass currentInspection;

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
        assets.clear();
        layouts.clear();
        inspectionTypes.clear();
    }

    void sync(){
        
        spinnerStart();

        try{

            comms->up();
            std::vector<String> config = comms->getContent();
            emptyAll();
            parse( &config );
            saveConfigToKVStore( &config );
            comms->down();

        }catch( const std::runtime_error& error ){
            spinnerEnd();
            String chainedError = String( "ERROR: Could not sync: " ) + error.what();            
            throw std::runtime_error( chainedError.c_str() );
        }

        spinnerEnd();        
    }


    void parse( std::vector<String>* config ){        
        Serial.println( "Parsing ..." );

        currentInspection.clear();

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

                // done?
                if( tokens[ 0 ] == "END" ){ 
                    Serial.println( "Found [END]" );                
                    isLoaded = true;
                    printDebugContents();
                    return;
                };

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

    assets.clear();
    layouts.clear();
    inspectionTypes.clear();

    parse(&config);

    Serial.println("Config loaded from KVStore and parsed!");
}

//-------------------------------------------------

};

#endif 
