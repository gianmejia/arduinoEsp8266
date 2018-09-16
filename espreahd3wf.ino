/*
This example will open a configuration portal when no WiFi configuration has been previously
entered or when a button is pushed. It is the easiest scenario for configuration but 
requires a pin and a button on the ESP8266 device. The Flash button is convenient 
for this on NodeMCU devices.
*/

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/kentaylor/WiFiManager
// select wich pin will trigger the configuraton portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared 
// with the bootloader, so always set them HIGH at power-up
// Onboard LED I/O pin on NodeMCU board

const int PIN_LED = 2; // D4 on NodeMCU and WeMos. Controls the onboard LED.
/*Trigger for inititating config mode is Pin D3 and also flash button on NodeMCU
 * Flash button is convenient to use but if it is pressed it will stuff up the serial port device driver 
 * until the computer is rebooted on windows machines.
 */
const int TRIGGER_PIN = 0; // D3 on NodeMCU and WeMos.
/*
 * Alternative trigger pin. Needs to be connected to a button to use this pin. It must be a momentary connection
 * not connected permanently to ground. Either trigger pin will work.
 */
const int TRIGGER_PIN2 = 13; // D7 on NodeMCU and WeMos.

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;

//********************************************************************++
#include "HTTPSRedirect.h"
#include "DebugMacros.h"              //para progr
const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "AKfycby95_phfEWdZNRzD_vsMlF9d-10BgGwH8J538RLwUqjVNabUtAQ";

const int httpsPort = 443;

// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
const char* fingerprint = "";

// Write to Google Spreadsheet
String url = String("/macros/s/") + GScriptId + "/exec?value=Hello";
// Fetch Google Calendar events for 1 week ahead
String url2 = String("/macros/s/") + GScriptId + "/exec?cal";
// Read from Google Spreadsheet
String url3 = String("/macros/s/") + GScriptId + "/exec?read";

String payload_base =  "{\"command\": \"appendRow\", \
                    \"sheet_name\": \"Sheet1\", \
                    \"values\": ";
String payload = "";
int i=0;
HTTPSRedirect* client = nullptr;
// used to store the values of free stack and heap
// before the HTTPSRedirect object is instantiated
// so that they can be written to Google sheets
// upon instantiation
unsigned int free_heap_before = 0;
unsigned int free_stack_before = 0;
static int error_count = 0;
static int connect_count = 0;
const unsigned int MAX_CONNECT = 20;
static bool flag = false;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 60000;  
boolean manda=false;
boolean conectado=false;
const int PIN_SENSOR = 12;
const int PIN_INDICA = 16;
boolean bloq1=true;
//********************************************************************+




void setup() {
  // put your setup code here, to run once:
  // initialize the LED digital pin as an output.
  pinMode(PIN_LED, OUTPUT);
 pinMode(PIN_INDICA, OUTPUT);
//  digitalWrite(PIN_INDICA,LOW); 
  Serial.begin(115200);
  digitalWrite(PIN_INDICA,LOW); 
  Serial.println("\n Starting");
  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  if (WiFi.SSID()==""){
    Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  }
  else{
    digitalWrite(PIN_LED, HIGH); // Turn led off as we are not in configuration mode.
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  }
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);
  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
 //*****************************************************
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
  
  if (client->verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }

 

  
  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
  pinMode(PIN_SENSOR, INPUT_PULLUP);

 //***************************************************** 
  }
}


void loop() {
  // is configuration portal requested?
  if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW) || (initialConfig)) {
  // if ((digitalRead(TRIGGER_PIN) == LOW) || (initialConfig)) {
     Serial.println("Configuration portal requested.");
     digitalWrite(PIN_LED, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //wifiManager.setConfigPortalTimeout(600);

    //it starts an access point 
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.startConfigPortal()) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }
    digitalWrite(PIN_LED, HIGH); // Turn led off as we are not in configuration mode.
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }

  // put your main code here, to run repeatedly:
//*****************************************************************
if (WiFi.status() != WL_CONNECTED){
  WiFi.begin();
   digitalWrite(PIN_INDICA,HIGH); 
  // ESP.reset();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  flag=false;
  }
  
  
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    DPRINTLN("Error creating client object!");
    error_count = 5;
  }
  
 
//*******************************************************
conectado=WiFi.status();
if(manda & conectado){
 manda=false;
    i++;
 Serial.println("CONECTANDOO-----"); 
String url = String("/macros/s/") + GScriptId + "/exec?value="+i;
if (client->GET(url, host)){
    ++connect_count;
    String str = client->getResponseBody();
      str.trim();
     Serial.print("aqui ( "); 
     Serial.print(str);
     Serial.println(")"); 
   
  
  }
  else{
    ++error_count;
    DPRINT("Error-count while connecting: ");
    DPRINTLN(error_count);
  }

if (connect_count > MAX_CONNECT){
    //error_count = 5;
    connect_count = 0;
    flag = false;
    delete client;
   client = nullptr;
   Serial.println("maxiconeciones."); 
    
  }
  
  if (error_count > 15){
    Serial.println("Halting processor..."); 
    delete client;
    client = nullptr;
    Serial.printf("Final free heap: %u\n", ESP.getFreeHeap());
    //Serial.printf("Final unmodified stack   = %4d\n", cont_get_free_stack(&g_cont));
    Serial.flush();
    ESP.deepSleep(0);
  }
  
}
  
  
  
  // In my testing on a ESP-01, a delay of less than 1500 resulted 
  // in a crash and reboot after about 50 loop runs.

  
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
  
    manda=true;
  startMillis=millis(); 
  }
 ESP.wdtFeed();

if ((digitalRead(PIN_SENSOR) == LOW)){
 delay(20);
if ((digitalRead(PIN_SENSOR) == LOW)){

 if(bloq1){
 bloq1=false;
 Serial.println(F("pulsooo")); 
 manda=true;
 }
}
} else {
   bloq1=true;
 
}
  
  // delay(0);
  //delay(2000);
 // yield();
  // manda=true;   





}
