/*---------------------------------------------------
    Developer   : o.sirrik
    Email       : oktay.sirrik@gmail.com
    Date        : 5/14/2025
    Version     : 1.1
    
    Description : This module organizes the connection 
    of a 128x64 oled charcter display with i2c interface.
----------------------------------------------------*/

#include <Arduino.h>
#include "oled.h"


void OLED::begin(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println(F("Display allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}



void OLED::splash(String ver){
  display.clearDisplay();
  display.setTextSize(2);      
  display.setCursor(32, 0);    
  display.print("Baran");
  display.setCursor(4, 20);    
  display.print("Technology");
  display.setTextSize(1);      
  display.setCursor(10, 42);    
  display.print("MQTT THING");
  display.setCursor(18, 56);    
  display.print("Ver:"+ver);
  display.display();
}


void OLED::showMode(byte mode){
  String xmd = "";
  if(mode == 1){xmd = "Ethernet";}
  if(mode == 2){xmd = "Wifi";}
  display.clearDisplay();
  display.setTextSize(1);      
  display.setCursor(10, 2);    
  display.print("Change Mode");
  display.setCursor(10, 22);    
  display.print("to "+xmd);
  display.setCursor(10, 42);    
  display.print("Please wait.!");
  display.display();
}

void OLED::showError(String error){
  display.clearDisplay();
  display.setTextSize(2);      
  display.setCursor(10, 2);    
  display.print("ERROR");
  display.setTextSize(1);      
  display.setCursor(10, 42);    
  display.print(error);
  display.display();
}


void OLED::showReset(){
  display.clearDisplay();
  display.setTextSize(1);      
  display.setCursor(10, 2);    
  display.print("Factory Reset");
  display.setCursor(10, 22);    
  display.print("Please wait.!");
  display.display();
}

void OLED::showETHsrc(){
  display.clearDisplay();
  display.setTextSize(1);      
  display.setCursor(10, 2);    
  display.print("Searching Ethernet");
  display.setCursor(10, 22);    
  display.print("Please wait.!");
  display.display();
}


void OLED::showMqttConn(bool sta){
  display.clearDisplay();
  display.setTextSize(1);      
  display.setCursor(10, 2);    
  display.print("Mqtt Connection: ");
  display.setCursor(10, 22);    
  if(sta == true){
    display.print("connected..!");  
  } else {
    display.print("not connected..!");  
  }
  display.display();
}


void OLED::showTemp(float temp, bool alm, byte mode){
  String stmp = String(temp);
  String xmd = "";
  if(mode == 1){xmd = "ETH";}
  if(mode == 2){xmd = "WIFI";}
  String xsta = "Normal";
  if(alm==true){xsta = "Alarm";}
  display.clearDisplay();
  display.setTextSize(1);      
  display.setCursor(0, 0);    
  display.print(xmd);
  display.setTextSize(3);      
  display.setCursor(4, 15);    
  display.print(stmp);
  display.setCursor(110, 15);    
  display.print("C");
  display.setTextSize(2);      
  display.setCursor(26, 45);    
  display.print(xsta);
  display.display();
}