#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncWebServer_ESP32_W5500.h>
#include <Arduino_JSON.h>
#include <SPIFFS.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "mainh.h"
#include "constants.h"
#include "xeprom.h"
#include "oled.h"


#define MSG_BUFFER_SIZE 256


unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];

//eeprom class
XEPROM ee;

//oled class
OLED oled;

//DS18B20 Temperature Sensor
OneWire  oneWire(TSENSOR); 
DallasTemperature sensor(&oneWire);


WiFiClientSecure client;
PubSubClient mqtt_client(client); 

//web server
AsyncWebServer    server(80);
AsyncEventSource events("/events");

//json variable
JSONVar jdata;


//setup
void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, 0);
  
  ee.begin();
  oled.begin();
  sensor.begin();

  if (!SPIFFS.begin(true)) {
    delay(100);
  }

  oled.splash(firmware);
  delay(3000); 
  initDefaults(false);

  bool ief = initEth();
  
  if(ief){
    mode = 1;
    mqtt_init();
  }else{
    mode = 0;
    initWifi();
  }
  ee.setMode(mode);
  oled.showMode(mode);
  initWeb();
  checkServer();  //is mqtt network available?

  t0 = millis();
  t1 = millis();
  
}


//loop
void loop() {  
  
  if((mode == 1) && ( connected == true)){
    if (!mqtt_client.connected()){
      mqtt_reconnect();
      delay(1000);
    } else {
      mqtt_client.loop();
      if(mqinterval>0){
        if( (millis()-t1) > mqinterval*1000) {
          t1 = millis();
          mqtt_publish();
        }
      }
    }
  }

  if( (millis()-t0) > refresh_timer){
    t0 = millis();
    temp = getTemp();
    oled.showTemp(temp, alrm, mode);
    updateMain();
  }


  //switch mode if button pressed more than 3 second
  if (state == true) {
    unsigned int tb = millis();
    
    while (digitalRead(BUTTON) == 1) { delay(10);}
    int dt = millis() - tb;
 
    if (dt<2000){ 
      state = false;
      return;
    }

    //switch mode wifi
    if (dt<3000){
      ee.setMode(mode);
    } else {
      //factory reset
      initDefaults(true);
    }

    oled.showReset();
    delay(3000);
    ESP.restart();
  }
}



// **********************************************************************************
//                              functions
// **********************************************************************************
void IRAM_ATTR  ISR_sett_mode() {
  state = true;
}

float getTemp(){
  status = false;
  sensor.requestTemperatures(); 
  temp = sensor.getTempCByIndex(0);

  if ( temp == -127 ) { //@reading error
    delay(100);
    sensor.requestTemperatures(); 
    temp = sensor.getTempCByIndex(0);
  }

  if ( temp == -127 ) { //@reading error
    status = true;
    temp = 0;           //not display -127
    return temp;
  }

  alrm = false;
  if (temp > limit) {
    alrm = true;
  }
  return temp;
}


void onRequest(AsyncWebServerRequest *request) {} 
void onFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {}
void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    jdata = JSON.parse((const char*)data);  
    request->send(200);

    String cmd = jdata["cmd"];

    if (cmd == "sel"){
      updateUI(jdata["val"]);
    }

    if (cmd == "save"){
      String menu = jdata["menu"];
      
      if (menu == "mqtt"){
        String _server = jdata["server"];
        String _user = jdata["user"];
        String _pass = jdata["pass"];
        String _ptop = jdata["ptop"];
        String _stop = jdata["stop"];
        JSONVar  _int = jdata["interval"];
        ee.setServer(_server);
        ee.setUser (_user);
        ee.setPass(_pass);
        ee.setPTopic(_ptop);
        ee.setSTopic(_stop);
        ee.setInterval(_int);
      }
     
      if (menu == "sensor"){
        JSONVar _limit = jdata["limit"];
        JSONVar _id = jdata["id"];
        String _name = jdata["name"];
        String _loc = jdata["loc"];
        float flimit = (float)((double)_limit);
        ee.setLimit(flimit);
        ee.setID(_id);
        ee.setName(_name);
        ee.setLoc(_loc);
      }

      if (menu == "network"){
        JSONVar _ip1 = jdata["ip1"];
        JSONVar _ip2 = jdata["ip2"];
        JSONVar _ip3 = jdata["ip3"];
        JSONVar _ip4 = jdata["ip4"];

        JSONVar _gw1 = jdata["gw1"];
        JSONVar _gw2 = jdata["gw2"];
        JSONVar _gw3 = jdata["gw3"];
        JSONVar _gw4 = jdata["gw4"];

        JSONVar _sn1 = jdata["sn1"];
        JSONVar _sn2 = jdata["sn2"];
        JSONVar _sn3 = jdata["sn3"];
        JSONVar _sn4 = jdata["sn4"];
        ee.setIPAddress(_ip1, _ip2, _ip3, _ip4);
        ee.setGWAddress(_gw1, _gw2, _gw3, _gw4);
        ee.setSNAddress(_sn1, _sn2, _sn3, _sn4);
      }
      
      ESP.restart();      
    }
}



void initWeb(){
    server.addHandler(&events);
    server.serveStatic("/", SPIFFS, "/");  
    server.begin();

    // Handle Web Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/", HTTP_POST, onRequest, onFileUpload, onBody);
    server.on("/comm", HTTP_POST, onRequest, onFileUpload, onBody);
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, handleUpload);

    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
      client->send("ok", NULL, millis(), 10000);
    });
}

void initWifi(){
  WiFi.softAP(ssid, pswd);
  WiFi.softAPConfig(ipw, gww, subnet);
}


bool initEth(){
  oled.showETHsrc();
  ip1 = ee.readByte(AD_IP);
  ip2 = ee.readByte(AD_IP+1);
  ip3 = ee.readByte(AD_IP+2);
  ip4 = ee.readByte(AD_IP+3);
  IPAddress ipe(ip1, ip2, ip3, ip4);

  gw1 = ee.readByte(AD_GW);
  gw2 = ee.readByte(AD_GW+1);
  gw3 = ee.readByte(AD_GW+2);
  gw4 = ee.readByte(AD_GW+3);
  IPAddress gwe(gw1, gw2, gw3, gw4);

  sn1 = ee.readByte(AD_SN);
  sn2 = ee.readByte(AD_SN+1);
  sn3 = ee.readByte(AD_SN+2);
  sn4 = ee.readByte(AD_SN+3);
  IPAddress sne(sn1, sn2, sn3, sn4);

  ESP32_W5500_onEvent();
  ETH.begin( MISO, MOSI, SCK, CS, INT, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);
  ETH.config(ipe, gwe, sne, xdns);

  bool isconnect = true;
  unsigned long te = millis();
  // ESP32_W5500_waitForConnect();
  while (ESP32_W5500_isConnected() == false) {
    delay(100);
    if (millis() - te > eth_timeout) {
      isconnect = false;
      break;
    }
  }
  return isconnect;
}



void initDefaults(bool fdf){
  byte db; 
  db = ee.getMode();
  if(db>2){
    mode = 0;
    ee.setMode(mode);
  }else{
    mode = db;
  }
  
  if(fdf == true){
      mode = 0;
      ee.setMode(mode);
      ee.setVersion(major, minor);

      ee.setIPAddress(ip1, ip2, ip3, ip4);
      ee.setGWAddress(gw1, gw2, gw3, gw4);
      ee.setSNAddress(sn1, sn2, sn3, sn4);
      
      ee.setLimit(limit);
      ee.setID(sid);
      ee.setName(String(sname));
      ee.setLoc(String(sloc));

      ee.setInterval(mqinterval);
      ee.setUser(mqtt_user);
      ee.setPass(mqtt_pass);
      ee.setPTopic(mqtt_ptopic);
      ee.setSTopic(mqtt_stopic);
      ee.setServer(mqtt_server);
  } else{
      db = ee.readByte(AD_MAJOR);
      if(db==255){ee.setVersion(major, minor);}

      db = ee.readByte(AD_LIMIT);
      if(db==255){ee.setLimit(limit);}

      db = ee.readByte(AD_IP);
      if(db==255){
        ee.setIPAddress(ip1, ip2, ip3, ip4);
      }

      db = ee.readByte(AD_GW);
      if(db==255){
        ee.setGWAddress(gw1, gw2, gw3, gw4);
      }

      db = ee.readByte(AD_SN+3);
      if(db==255){
        ee.setSNAddress(sn1, sn2, sn3, sn4);
      }

      db = ee.readByte(AD_ID);
      if(db==255){ee.setID(sid);}

      db = ee.readByte(AD_NAME);
      if(db==255){ee.setName(String(sname));}

      db = ee.readByte(AD_LOC);
      if(db==255){ee.setLoc(String(sloc));}

      db = ee.readByte(AD_INTERVAL);
      if(db==255){ee.setInterval(mqinterval);}

      db = ee.readByte(AD_USER);
      if(db==255){ee.setUser(mqtt_user);}

      db = ee.readByte(AD_PASS);
      if(db==255){ee.setPass(mqtt_pass);}

      db = ee.readByte(AD_PTOPIC);
      if(db==255){ee.setPTopic(mqtt_ptopic);}

      db = ee.readByte(AD_STOPIC);
      if(db==255){ee.setSTopic(mqtt_stopic);}

      db = ee.readByte(AD_SERVER);
      if(db==255){ee.setServer(mqtt_server);}
  }
  //read vars from ee
  limit = ee.getLimit();
  mqinterval = ee.getInterval();
  firmware = ee.getVersion();
  ipaddr = ee.getIPAddress();
  gwaddr = ee.getGWAddress();
  snaddr = ee.getSNAddress();
  seid = ee.getID();
  sename = ee.getName();
  seloc = ee.getLoc();
  mquser = ee.getUser();
  mqpass = ee.getPass();
  mqstopic = ee.getSTopic();
  mqptopic = ee.getPTopic();
  mqserver = ee.getServer();
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}


bool checkCertificate() {
  String cert;
  bool fcert=false;
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  while (foundfile) {
    cert = String(foundfile.name());
    if(cert=="ssl.crt"){fcert=true;}
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();
  return fcert;
}



// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->redirect("/");
  }
}


//UPDATE UI
void updateMain(){
  String stotal =  humanReadableSize(SPIFFS.totalBytes());
  String sused = humanReadableSize(SPIFFS.usedBytes());
  String sfree = humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
  String suptime = getUptime();
  String edata =  String(temp)+","+ String(alrm)+","+ String(status)+","+suptime + "," + stotal + "," + sused + "," + sfree + "," + firmware;
  events.send(edata.c_str(), "uimain", millis());
}

void updateMqtt(){
  String sslc = "none";
  if(checkCertificate()){sslc="ssl.crt";}
  String edata =  mqserver + ","+ mquser + "," +mqpass + ","+ mqptopic + "," + mqstopic + ","+ String(mqinterval) + "," + sslc;
  events.send(edata.c_str(), "uimqtt", millis());
}

void updateSensor(){
  String edata =  String(limit) + "," + seid + "," + sename + "," + seloc;
  events.send(edata.c_str(), "uisensor", millis());
}

void updateNetwork(){
  String edata =  ipaddr+","+gwaddr+","+snaddr+","+String(ssid)+","+String(pswd);
  events.send(edata.c_str(), "uinetwork", millis());
}


void updateUI(String menu){
  if(menu == "main"){ updateMain();}
  if(menu == "mqtt"){ updateMqtt();}
  if(menu == "sensor"){ updateSensor();}
  if(menu == "network"){ updateNetwork();}
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




//MQTT Fuctions
void checkServer(){
  IPAddress gwe(gw1, gw2, gw3, gw4);
  connected = false;
  if (Ping.ping(gwe, 3)) { connected = true;}
}



void mqtt_init(){
    File file = SPIFFS.open("/ssl.crt","r");
    if(!file){
      return;
    }
  
    size_t fileSize = file.size();
    char* buffer = (char*)malloc(fileSize + 1); // +1 for null terminator
    
    if (buffer == NULL) {
      file.close();
      return;
    }
    
    file.readBytes(buffer, fileSize);
    buffer[fileSize] = '\0'; // Null-terminate the string
    file.close();
    // memcpy(CA_cert, buffer, sizeof(fileSize));
    //Set up the certificates and keys
    client.setCACert(buffer);          //Root CA certificate
    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_client.setCallback(mqtt_callback);

    //connection
    if (mqtt_client.connect("BaranMQTT", mqtt_user , mqtt_pass)) {
      oled.showMqttConn(true);
    } else {
      oled.showMqttConn(false);
      char lastError[100];
      client.lastError(lastError,100);  //Get the last error for WiFiClientSecure
    }
    mqtt_client.subscribe(mqtt_stopic);
    free(buffer); 
    delay(500);
  }


void mqtt_reconnect(){
    if (mqtt_client.connect("BaranMQTT", mqtt_user , mqtt_pass)) {
      oled.showMqttConn(true);
    } else {
      oled.showMqttConn(false);
      char lastError[100];
      client.lastError(lastError,100); 
    }
    delay(500);
  }



/***** Call back Method for Receiving MQTT messages and Switching LED ****/
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String inmsg = "";
    for (int i = 0; i < length; i++) inmsg += (char)payload[i];
    
    if( strcmp(topic, mqtt_stopic) == 0){ //is topic matched?
     if (inmsg.equals("1")) {
      digitalWrite(RELAY, 1);
     }else{
      digitalWrite(RELAY, 0);
     }
  }
}


/**** Method for Publishing MQTT Messages **********/
void mqtt_publish(){
  JSONVar pubmsg;  
  pubmsg["temp"] = temp;
  pubmsg["alarm"] = alrm;
  pubmsg["status"] = status;
  pubmsg["limit"] = limit;
  pubmsg["id"] = seid;
  pubmsg["location"] = seloc;
  pubmsg["name"] = sename;
  pubmsg["uptime"] = getUptime();
  String strmsg = JSON.stringify(pubmsg);
  mqtt_client.publish(mqtt_ptopic, strmsg.c_str());
}

