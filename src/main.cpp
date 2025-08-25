#include <Arduino.h>
#include <AsyncTCP.h>
#include <AsyncWebServer_ESP32_W5500.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#include "xeprom.h"
#include "oled.h"
#include "xsnmp.h"
#include "constants.h"
#include "mainh.h"

AsyncWebServer    server(80);
AsyncEventSource events("/events");

//DS18B20 Temperature Sensor
OneWire  oneWire(TSENSOR); 
DallasTemperature sensor(&oneWire);


JSONVar jdata;

XEPROM ee;
OLED oled;


// **********************************************************************************
//                              setup
// **********************************************************************************

void setup(){
  //setup pins
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  Serial.begin(115200);
  while (!Serial && millis() < 5000);
  delay(500);
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  
  ee.begin();
  oled.begin();
  oled.splash();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // server.on("/", HTTP_POST, onRequest, onFileUpload, onBody);
  server.on("/sett", HTTP_POST, onRequest, onFileUpload, onBody);


  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    client->send("ok", NULL, millis(), 10000);
  });

  server.addHandler(&events);
  server.serveStatic("/", SPIFFS, "/");  
  server.begin();


  Serial.println("ready..!");
}


// **********************************************************************************
//                              loop
// **********************************************************************************

void loop(){
    if(Serial.available() > 0){
      char rx = Serial.read();

      if(rx == 'x'){ 
        updateUI();
      }

      if(rx == '0'){ 
        oled.showTemp(20,false);
        ee.writeByte(0,0);
        ESP.restart();
      }


      if(rx == '1'){ 
        oled.showTemp(21,true);
        ee.writeByte(0,1);
        ESP.restart();
      }


    }
}



// **********************************************************************************
//                              functions
// **********************************************************************************
void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    Serial.println("onbody request");
    jdata = JSON.parse((const char*)data);  
    Serial.println(jdata);
    request->send(200, "text/plain", "ok");
}


void initEth(){
  Serial.println("ETH Init");
  IPAddress ipe(ip1, ip2, ip3, ip4);
  IPAddress gwe(ip1, ip2, ip3, 1);
  digitalWrite(LED,0);
  ESP32_W5500_onEvent();
  ETH.begin( MISO, MOSI, SCK, CS, INT, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);
  ETH.config(ipe, gwe, subnet, xdns);
  ESP32_W5500_waitForConnect();
}

void initWifi(){
  Serial.println("Wifi Init");
  digitalWrite(LED,1);
  WiFi.softAP(ssid, pswd);
  WiFi.softAPConfig(ipw, gww, subnet);
}

void updateUI(bool fdf){
  temp = getTemp();
  String edata = String(fdf)+","+String(temp) + ","+ String(alrm) + "," + getUptime();
  if (fdf){
    float xlimit = ee.readFloat(1);
    String xid = ee.getID();
    String xname = ee.getName();
    String xloc = ee.getLoc();
    edata += ","+String(xlimit) +"," + getIpAddr() + "," + xid + "," + xname + "," + xloc;
  }
  events.send(edata.c_str(), "tmps", millis());
}


float getTemp(){
  alrm = false;
  status = false;
  sensor.requestTemperatures(); 
  temp = sensor.getTempCByIndex(0);
  if ( temp == -127 ) { //@reading error
    status = true;
    return btemp;
  }
  btemp = temp;
  if (temp > limit) {alrm = true;}
  return temp;
}


String getUptime(){
  unsigned long seconds = millis()/1000;
  if (seconds>86400){ESP.restart();}   //daily restart 86400
  unsigned long minutes = seconds / 60;
  unsigned long sec = seconds % 60;
  unsigned long hrs = minutes / 60;
  unsigned long mnt = minutes % 60;
  String upts = String(hrs)+" hr "+String(mnt)+" min "+String(sec)+" sec";
  return upts;
}
