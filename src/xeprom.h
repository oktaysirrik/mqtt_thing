#ifndef XEPROM_h
#define XEPROM_h

#include <Arduino.h>
#include <EEPROM.h>

#define EE_SIZE 256

#define AD_MODE 0
#define AD_MAJOR 1
#define AD_MINOR 2

#define AD_IP 3
#define AD_GW 7
#define AD_SN 11

#define AD_LIMIT 15
#define AD_ID 19
#define AD_NAME 23
#define AD_LOC 39

#define AD_INTERVAL 71
#define AD_USER 72
#define AD_PASS 88
#define AD_PTOPIC 104
#define AD_STOPIC 120
#define AD_SERVER 136


class XEPROM {

public:
	XEPROM();
	void begin();
	
	byte readByte(byte);
	byte getMode();
	void setMode(byte);
	String getVersion();
	void setVersion(byte, byte);

	String getIPAddress();
	void setIPAddress(byte, byte, byte, byte);
	String getGWAddress();
	void setGWAddress(byte, byte, byte, byte);
	String getSNAddress();
	void setSNAddress(byte, byte, byte, byte);

	float getLimit();
	void setLimit(float);
	int getID();
	void setID(int);
	String getName();
	void setName(String);
	String getLoc();
	void setLoc(String);

	byte getInterval();
	void setInterval(byte);
	String getUser();
	void setUser(String);
	String getPass();
	void setPass(String);
	String getPTopic();
	void setPTopic(String);
	String getSTopic();
	void setSTopic(String);
	String getServer();
	void setServer(String);


private:
	void writeByte(byte, byte);
	void writeInt(byte, int);
	int readInt(byte);
	void writeFloat(byte, float);
	float readFloat(byte);
	void writeString(byte , char *);
	String readString(byte, byte);

};

#endif