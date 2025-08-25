#include <Arduino.h>

//General Purpose IO
#define BUTTON 	25
#define RELAY 	33
#define TSENSOR	32


//WIZ5500 ETH Chip Interface
#define MOSI    23
#define MISO    19
#define CS      5
#define SCK     18
#define RST     16
#define INT     17


// global variables
unsigned long t0;
unsigned long t1;
unsigned int refresh_timer = 3000;      //3000 mS (3 second)
unsigned int eth_timeout = 10000;       //10,000 mS (10 second)

bool connected = false;     // is connected to mqtt network
bool state = false;         //button pressed?
bool alrm = false;          //0:no alrm ,  1: over temperature
bool status = false;        //0:sensor ok, 1: sensor error
float temp = 0.0;

const int mqtt_port        = 8883;     


//WIFI SSID/PASS
const char* ssid = "MQTT_THING";
const char* pswd = "11223344";


//WIFI DEFAULT IP ADDRESS
IPAddress ipw(11, 0, 0, 1);
IPAddress gww(11, 0, 0, 1);

//ETH mac address
byte mac[] = { 0x42, 0x52, 0x4E, 0x00, 0x00, 0x01};   //can increment mac addr on the next chip
IPAddress subnet(255, 255, 255,0);
IPAddress xdns(8, 8, 8, 8);

//EEPROM MAP
byte mode = 0;              // EE[0]    1    0:No eth or wifi  1:eth  2:wifi 
byte major = 1;             // EE[1]    1    firmware version
byte minor = 0;             // EE[2]    1    

//ETH IP/Gateway/Subnet address
byte ip1 = 10;              // EE[3]   1
byte ip2 = 0;               // EE[4]   1
byte ip3 = 0;               // EE[5]   1
byte ip4 = 1;               // EE[6]   1   

byte gw1 = 10;              // EE[7]   1
byte gw2 = 0;               // EE[8]   1
byte gw3 = 0;               // EE[9]   1
byte gw4 = 1;               // EE[10]   1   

byte sn1 = 255;              // EE[11]   1
byte sn2 = 255;              // EE[12]   1
byte sn3 = 255;              // EE[13]   1
byte sn4 = 0;                // EE[14]   1   


//SENSOR RELATED VARIABLES
float limit         = 22.00;                // EE [15]   4
int sid             = 3400;                 // EE [19]   4
const char *sname   = "BTS_0001";           // EE [23]   16
const char *sloc    = "Istanbul-Umraniye";  // EE [39]   32


//MQTT RELATED VARIABLES
byte mqinterval           = 10;                                         // EE [71]   1
const char* mqtt_user     = "barantech";                                // EE [72]   16
const char* mqtt_pass     = "Aa123456*";                                // EE [88]   16
const char* mqtt_ptopic   = "baran_sensor";                             // EE [104]   16
const char* mqtt_stopic   = "baran_relay";                              // EE [120]   16
const char* mqtt_server   = "y8a4811d.ala.eu-central-1.emqxsl.com";     // EE [136]   64   - 198 end address



//STRINGS 
String firmware = "1.0";
String ipaddr = "10.0.0.1";
String gwaddr = "10.0.0.1";
String snaddr = "255.255.255.0";

String seid = String(sid);
String sename = String(sname);
String seloc = String(sloc);

String mquser = String(mqtt_user);
String mqpass = String(mqtt_pass);
String mqptopic = String(mqtt_ptopic);
String mqstopic = String(mqtt_stopic);
String mqserver = String(mqtt_server);
