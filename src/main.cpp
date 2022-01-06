//Dont forget to build the HTML and other supporting files before uploading this!!!!

//when it connects to the modem nothing will show in the terminal or on the modem (192.168.0.1)
//get your phone and look at wifi connections and select the ESP to 'connect'

//open a browser ad enter the URL: 192.168.4.1 and enter the requested credentials (it times out in a minute or two so reset if not connected.)

//!!!!!! note user should enter 192.168.0.7 when using optus, the last value should change depending what is already connected to the modem. 

//to read another file onboarded to the ESP it must be saved in a new folder called 'data' with the file named anything that you want.
// the said file must first be onboarded to the ESP by:
//1.  using VSCode, go the PIO icon, > Platform > select 'Build Filesystem Image' followed by selecting 'Upload FileSystem Image'


#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

void readAnotherFileOnESP(void); 
void joinWifiNetwork(void);
void lights(void);

 
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 10); // hardcoded

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 2;
// Stores LED state

String ledState;


//for reading and writing the wifi credentials. 

    // Initialize SPIFFS
    void initSPIFFS() {
      Serial.print("initSpiffs");
      if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
      }
      Serial.println("SPIFFS mounted successfully");
    }

    // Read File from SPIFFS
    String readFile(fs::FS &fs, const char * path){
      Serial.printf("Reading file: %s\r\n", path);

      File file = fs.open(path);
      if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return String();
      }
      
      String fileContent;
      while(file.available()){
        fileContent = file.readStringUntil('\n');
        break;     
      }
      return fileContent;
    }

    // Write file to SPIFFS
    void writeFile(fs::FS &fs, const char * path, const char * message){
      Serial.printf("Writing file: %s\r\n", path);

      File file = fs.open(path, FILE_WRITE);
      if(!file){
        Serial.println("- failed to open file for writing");
        return;
      }
      if(file.print(message)){
        Serial.println("- file written");
      } else {
        Serial.println("- frite failed");
      }
    }

    // Initialize WiFi
    bool initWiFi() {
      Serial.println("init wifi");
      if(ssid=="" || ip==""){
        Serial.println("Undefined SSID or IP address.");
        return false;
      }

      WiFi.mode(WIFI_STA);
      localIP.fromString(ip.c_str());

      if (!WiFi.config(localIP, gateway, subnet)){
        Serial.println("STA Failed to configure");
        return false;
      }
      WiFi.begin(ssid.c_str(), pass.c_str());
      Serial.println("Connecting to WiFi...");

      unsigned long currentMillis = millis();
      previousMillis = currentMillis;

      while(WiFi.status() != WL_CONNECTED) {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
          Serial.println("Failed to connect.");
          return false;
        }
      }

      Serial.println(WiFi.localIP());
      return true;
    }

    // Replaces placeholder with LED state value
    String processor(const String& var) {
      Serial.println("led controller");
      if(var == "STATE") {
        if(digitalRead(ledPin)) {
          ledState = "ON";
        }
        else {
          ledState = "OFF";
        }
        return ledState;
      }
      return String();
    }
    ///end of wifi credential routines. 



 

void setup(){

  Serial.begin(115200);   ///!!!! you may need to change the platfom.ini to match or change this to 115200
  Serial.println("Hello");

//wifi set up
      Serial.println("joinWifiNetwork");

      initSPIFFS();

      // Set GPIO 2 as an OUTPUT
      pinMode(ledPin, OUTPUT);
      digitalWrite(ledPin, LOW);
      
      // Load values saved in SPIFFS
      ssid = readFile(SPIFFS, ssidPath);
      pass = readFile(SPIFFS, passPath);
      ip = readFile(SPIFFS, ipPath);
      Serial.println(ssid);
      Serial.println(pass);
      Serial.println(ip);

      if(initWiFi()) {
        // Route for root / web page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });
        server.serveStatic("/", SPIFFS, "/");
        
        // Route to set GPIO state to HIGH
        server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
          digitalWrite(ledPin, HIGH);
          request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });

        // Route to set GPIO state to LOW
        server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
          digitalWrite(ledPin, LOW);
          request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });

      

        server.begin();
      }
      else {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
        // NULL sets an open Access Point
        WiFi.softAP("ESP-WIFI-MANAGER", NULL);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 

        // Web Server Root URL
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(SPIFFS, "/wifimanager.html", "text/html");
        });
        
        server.serveStatic("/", SPIFFS, "/");
        
        server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
          int params = request->params();
          for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
              // HTTP POST ssid value
              if (p->name() == PARAM_INPUT_1) {
                ssid = p->value().c_str();
                Serial.print("SSID set to: ");
                Serial.println(ssid);
                // Write file to save value
                writeFile(SPIFFS, ssidPath, ssid.c_str());
              }
              // HTTP POST pass value
              if (p->name() == PARAM_INPUT_2) {
                pass = p->value().c_str();
                Serial.print("Password set to: ");
                Serial.println(pass);
                // Write file to save value
                writeFile(SPIFFS, passPath, pass.c_str());
              }
              // HTTP POST ip value
              if (p->name() == PARAM_INPUT_3) {
                ip = p->value().c_str();
                Serial.print("IP Address set to: ");
                Serial.println(ip);
                // Write file to save value
                writeFile(SPIFFS, ipPath, ip.c_str());
              }
              //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
          }
          request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
          delay(3000);
          ESP.restart();
        });
        server.begin();
      }
      //wifi set up ends

    
  
  //chcek the file isa actually there before you try to use it!  
//  readAnotherFileOnESP();   //be sure to change the file name to match that in data folder!!!!!

  //check for wifi cerdentials and request from user if required. 

}

void loop() {

//Serial.println("in loop");
 /*  for(byte i=0; i<5; i++){
            digitalWrite(ledPin, HIGH);
            delay(5);
            digitalWrite(ledPin, LOW);
            Serial.println("on/off");
            //digitalWrite(ledPin, HIGH)
          } */
//delay(10000);

}

/* 
void readAnotherFileOnESP()  //code to actually read a file when testing if it is there. 
{

   delay(2000);
  Serial.println("your file: ");
   if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/text.txt");    //adjust folder name as required, dont forget / for the folder path
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();


} */
