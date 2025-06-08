#include <WiFi.h>
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"

// display variables
Arduino_H7_Video          Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch  TouchDetector;
bool screenIsUp = false;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    blinkerror();
  }

  Serial.println("2025 starting....");

  setupScreen();
}


void loop() {

  
  Serial.println("2025 looping .. ");
  delay(200);
  blinkerror();

  // refresh screen
  if( screenIsUp ){
    Serial.println("Painting ... ");
    lv_timer_handler();
    
  }

  Serial.println("2025 looping .. DONE");
}


//================================================== TEST


static void btn_event_cb(lv_event_t * e) {
  lv_obj_t* btn = (lv_obj_t*) lv_event_get_target(e);
  lv_obj_t* label = (lv_obj_t*) lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "Clicked!");
}


void setupScreen(){

  Serial.println("Initializing screen....");
  delay(3000);
  Display.begin();
  TouchDetector.begin();
  Serial.println("Initializing screen.... done");

  Serial.println("Begin screen test .... ");

    Serial.println("Get handlers and configure screena  .... ");
    lv_obj_t * screen = lv_obj_create( lv_scr_act() );
    lv_obj_set_size(screen, Display.width(), Display.height());
    Serial.println("Get handlers and configure screena  .... done");

    Serial.println("Configure layout  .... ");    
    // get handler to grid
    lv_obj_t * grid = lv_obj_create(lv_scr_act());
    // set grid
    static lv_coord_t col_dsc[] = {370, 370, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {215, 215, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_size(grid, Display.width(), Display.height());
    Serial.println("Configure layout  .... done");    


  Serial.println("Create cells  .... ");    

    //top left
    
    lv_obj_t*  obj1 = lv_obj_create(grid);
    lv_obj_set_grid_cell(obj1, LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_STRETCH, 0, 1);      //row
    //bottom left
    lv_obj_t* obj2 = lv_obj_create(grid);
    lv_obj_set_grid_cell(obj2, LV_GRID_ALIGN_STRETCH, 0, 1,  LV_GRID_ALIGN_STRETCH, 1, 1);      //row
    //top right
    lv_obj_t* obj3 = lv_obj_create(grid);
    lv_obj_set_grid_cell(obj3, LV_GRID_ALIGN_STRETCH, 1, 1,  LV_GRID_ALIGN_STRETCH, 0, 1);      //row
    //bottom right
    lv_obj_t* obj4 = lv_obj_create(grid);
    lv_obj_set_grid_cell(obj4, LV_GRID_ALIGN_STRETCH, 1, 1,  LV_GRID_ALIGN_STRETCH, 1, 1);      //row

  Serial.println("Create cells  .... done.");    


  Serial.println("Label test  .... ");    

    lv_obj_t * label;
    lv_obj_t * obj;

    lv_obj_t * btn = lv_btn_create(obj1);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Click me!");
    lv_obj_center(label);

  Serial.println("Label test  .... done");    

  screenIsUp = true;
}
















//==========================================================================================================


char ssid[] = "irazu2G";
char pass[] = "casiocasio";
int keyIndex = 0;  
int status = WL_IDLE_STATUS;
WiFiClient client;


void blinkerror(){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(100);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(100);                      // wait for a second  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectWifi(){

  Serial.println("check wifi hardware....");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Fatal WL_NO_SHIELD");
    blinkerror();
    while (true);
  }else{
      Serial.println("... status OK");
  }

  Serial.println("Connecting....");
  while ( true ) {

    Serial.println("Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.println(pass);
    status = WiFi.begin(ssid, pass);
    delay(5000);

    if( status == WL_CONNECT_FAILED ){
      Serial.println("While connecting: [WL_CONNECT_FAILED]"); 
      blinkerror();
    } 

    if( status == WL_CONNECTED ){
      Serial.println("Connected !!"); 
      break;
    }
  }
  
  printWifiStatus();

}

void loadConfig(){

  Serial.println("Starting connection to server...");
  IPAddress server(10,0,0,141);
  while(!client.connect( server, 8080 ) ){
    Serial.println("Starting connection to server... FAILED!");
    Serial.println("Starting connection to server... retry....");
    blinkerror();
    delay( 3000 );  
  }

  Serial.println("GET /server2025/config");
  client.println("GET /server2025/config");
  client.println("Accept: */*");
  client.println("\r\n");

  delay( 1000 ); // <-- fix this - you load until timeo or end of trx marker - for all trxs

  Serial.println("READ ----------------------");
  Serial.print( "Available: " ); Serial.println( client.available() );

  while ( client.available() ) {
    char c = client.read();
    Serial.write(c);
  }

  Serial.println("---------------------- READ");

  Serial.println("Disconnect!");
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
}




  







