/*---------------------------------------------------
    Developer   : o.sirrik
    Email       : oktay.sirrik@gmail.com
    Date        : 7/29/2025
    Version     : 1.0
    
    Description : This class handles write and read operations 
    to eprom. Developers should use the corresponding functions 
    according to the data types. The ESP32 does not have an 
    EEPROM area; instead, data is stored in a flash partition. 
    Thus, it should reserve an area at the beginning.
----------------------------------------------------*/

#include <Arduino.h>
#include <String.h>
#include "xeprom.h"

XEPROM::XEPROM() {}

void XEPROM::begin(){
    EEPROM.begin(EE_SIZE);
}


void XEPROM::writeByte(byte adr, byte data){
    EEPROM.write(adr, data);
    EEPROM.commit();
}

byte XEPROM::readByte(byte adr){
    return EEPROM.read(adr);
}

void XEPROM::writeInt(byte adr, int data){
    EEPROM.put(adr, data);
    EEPROM.commit();
}

int XEPROM::readInt(byte adr){
    int rd;
    EEPROM.get(adr, rd);
    return rd;
}

void XEPROM::writeFloat(byte adr, float data){
    EEPROM.put(adr, data);
    EEPROM.commit();
}

float XEPROM::readFloat(byte adr){
    float rd;
    EEPROM.get(adr, rd);
    return rd;
}

void XEPROM::writeString(byte adr, char *buff) {
    size_t x = strlen(buff);
    for (char i = 0; i < x; i++)  {
        EEPROM.write(i+adr, buff[i]);
    }
    EEPROM.write(adr+x, '\0');
    EEPROM.commit();
}


String XEPROM::readString(byte adr, byte size) {
    char buff[size];
    for (char i = 0; i < size; i++)  {
        buff[i] = EEPROM.read(i+adr);
    }
    return String(buff);
}


/*---------------------------------------
            CONVENIENT FUNCTIONS
----------------------------------------*/
byte XEPROM::getMode() {
    return readByte(AD_MODE);
}

void XEPROM::setMode(byte mode) {
    writeByte(AD_MODE, mode);
}

String XEPROM::getVersion() {
    byte _major = readByte(AD_MAJOR);
    byte _minor = readByte(AD_MINOR);
    return String(_major)+"."+String(_minor);
}

void XEPROM::setVersion(byte _major, byte _minor) {
    writeByte(AD_MAJOR, _major);
    writeByte(AD_MINOR, _minor);
}


float XEPROM::getLimit() {
    return readFloat(AD_LIMIT);
}

void XEPROM::setLimit(float lmt) {
    writeFloat(AD_LIMIT, lmt);
}

String XEPROM::getIPAddress() {
    byte ip1 = readByte(AD_IP);
    byte ip2 = readByte(AD_IP+1);
    byte ip3 = readByte(AD_IP+2);
    byte ip4 = readByte(AD_IP+3);
    String ipa = String(ip1) + "."+ String(ip2) + "." + String(ip3) +"." + String(ip4);
    return ipa;
}

String XEPROM::getGWAddress() {
    byte ip1 = readByte(AD_GW);
    byte ip2 = readByte(AD_GW+1);
    byte ip3 = readByte(AD_GW+2);
    byte ip4 = readByte(AD_GW+3);
    String ipa = String(ip1) + "."+ String(ip2) + "." + String(ip3) +"." + String(ip4);
    return ipa;
}

String XEPROM::getSNAddress() {
    byte ip1 = readByte(AD_SN);
    byte ip2 = readByte(AD_SN+1);
    byte ip3 = readByte(AD_SN+2);
    byte ip4 = readByte(AD_SN+3);
    String ipa = String(ip1) + "."+ String(ip2) + "." + String(ip3) +"." + String(ip4);
    return ipa;
}


void XEPROM::setIPAddress(byte ip1, byte ip2, byte ip3, byte ip4) {
    writeByte(AD_IP, ip1);
    writeByte(AD_IP+1, ip2);
    writeByte(AD_IP+2, ip3);
    writeByte(AD_IP+3, ip4);
}

void XEPROM::setGWAddress(byte ip1, byte ip2, byte ip3, byte ip4) {
    writeByte(AD_GW, ip1);
    writeByte(AD_GW+1, ip2);
    writeByte(AD_GW+2, ip3);
    writeByte(AD_GW+3, ip4);
}

void XEPROM::setSNAddress(byte ip1, byte ip2, byte ip3, byte ip4) {
    writeByte(AD_SN, ip1);
    writeByte(AD_SN+1, ip2);
    writeByte(AD_SN+2, ip3);
    writeByte(AD_SN+3, ip4);
}


int XEPROM::getID() {
    return readInt(AD_ID);
}

void XEPROM::setID(int sid) {
    writeInt(AD_ID, sid);
}


String XEPROM::getName() {
    return readString(AD_NAME,15);
}

void XEPROM::setName(String name) {
    char *cname = (char *)name.c_str();
    writeString(AD_NAME, cname);
}


String XEPROM::getLoc() {
    return readString(AD_LOC,31);
}


void XEPROM::setLoc(String loc) {
    char *cloc = (char *)loc.c_str();
    writeString(AD_LOC, cloc);
}


String XEPROM::getServer() {
    return readString(AD_SERVER, 64);
}

void XEPROM::setServer(String val) {
    char *cval = (char *)val.c_str();
    writeString(AD_SERVER, cval);
}

String XEPROM::getUser() {
    return readString(AD_USER, 16);
}

void XEPROM::setUser(String val) {
    char *cval = (char *)val.c_str();
    writeString(AD_USER, cval);
}


String XEPROM::getPass() {
    return readString(AD_PASS, 16);
}

void XEPROM::setPass(String val) {
    char *cval = (char *)val.c_str();
    writeString(AD_PASS, cval);
}

String XEPROM::getPTopic() {
    return readString(AD_PTOPIC, 16);
}

void XEPROM::setPTopic(String val) {
    char *cval = (char *)val.c_str();
    writeString(AD_PTOPIC, cval);
}

String XEPROM::getSTopic() {
    return readString(AD_STOPIC, 16);
}

void XEPROM::setSTopic(String val) {
    char *cval = (char *)val.c_str();
    writeString(AD_STOPIC, cval);
}

byte XEPROM::getInterval() {
    return readByte(AD_INTERVAL);
}

void XEPROM::setInterval(byte val) {
    writeByte(AD_INTERVAL, val);
}
