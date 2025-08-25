// **********************************************************************************
//                              prototype functions
// **********************************************************************************
void ISR_sett_mode();
void initWeb();
void initWifi();
bool initEth();
void initDefaults(bool);
void updateMain();
void updateMqtt();
void updateSensor();
void updateNetwork();
void updateUI(String);
void checkServer();
float getTemp();
String getUptime();
bool checkCertificate();
String humanReadableSize(const size_t);


//web
void onRequest(AsyncWebServerRequest *);
void onBody(AsyncWebServerRequest *, uint8_t *, size_t, size_t, size_t);
void onFileUpload(AsyncWebServerRequest *, const String& , size_t , uint8_t *, size_t , bool );
void handleUpload(AsyncWebServerRequest *, String, size_t, uint8_t *, size_t, bool);

//mqtt
void mqtt_init();
void mqtt_reconnect();
void mqtt_callback(char*, byte*, unsigned int);
void mqtt_publish(String);


