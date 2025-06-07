#include <WiFi.h>


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

  Serial.println("check wifi status....");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Fatal WL_NO_SHIELD");
    blinkerror();
    while (true);
  }else{
      Serial.println("... status OK");
  }

  // attempt to connect to Wifi network:
  while ( true ) {
    Serial.println("Connecting....");
    Serial.println("Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.println(pass);
    status = WiFi.begin(ssid, pass);
    delay(5000);

    if( status == WL_CONNECT_FAILED ){
      Serial.println("While connecting: [WL_CONNECT_FAILED]"); 
      while(true) blinkerror();
    } 

    if( status == WL_CONNECTED ){
      Serial.println("Connected !!"); 
      break;
    }
  }
  
  printWifiStatus();
}




void loop() {

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

  delay( 1000 );

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

  







