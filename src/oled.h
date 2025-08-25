#ifndef OLED_h
#define OLED_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OLED {

public:
	OLED(){
		display = Adafruit_SSD1306(128, 64, &Wire, -1);	
	};
	void begin();
	void splash(String);
	void showMode(byte);
	void showETHsrc();
	void showReset();
	void showTemp(float, bool, byte);
	void showMqttConn(bool);

private:
	Adafruit_SSD1306 display;

};

#endif