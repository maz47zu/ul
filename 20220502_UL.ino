#include "Hive.h"
#include "ClientWrapper.h"
#include "HX711.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>
#include <UrlEncode.h>
#include <Arduino.h>

#include <ArduinoHttpClient.h>

#define calibration_factor -22350.0 //wsp. kalibracji
#define DOUT  32
#define CLK  33
#define transistor 13

#define TINY_GSM_MODEM_SIM800
#define SIM800C_AXP192_VERSION_20200609
#define SerialMon Serial
#define SerialAT  Serial1
 #define DUMP_AT_COMMANDS
#define TINY_GSM_DEBUG SerialMon
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define GSM_PIN ""
#include "utilities.h"
#include <TinyGsmClient.h>
// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

const char apn[]      = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

float humidity = 0;
float tempC = 0;
float weight;

Weather sensor;
HX711 scale;

const String ID = "e62b5b3d-aaaa-aaaa-885c-ce6ad2e9750a";

ClientWrapper client;

void setupModem()
{
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);
}
bool Bearing_set();
bool Https_get();
bool Close_serve();

void setup() {
  SerialMon.begin(115200);
  randomSeed(33);

  client.init("", "", "http://51.68.141.235:8088/");
  
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  setupModem();
  delay(6000);
  modem.restart();
  
  pinMode(transistor,OUTPUT);
  digitalWrite(transistor, HIGH);
    
  sensor.begin();
    
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  #if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) { modem.simUnlock(GSM_PIN); }
  #endif

}

void loop() {
  #if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
  #endif

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
  SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }
  
  #if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) { SerialMon.println("GPRS connected"); }
  #endif

  SerialMon.print(F("Performing HTTPS GET request... "));

  if(Bearing_set()==false) SerialMon.println("Bearing set fall");
  
  humidity = sensor.getRH();

  tempC = sensor.getTemp();
  
  weight = scale.get_units(), 2; //kilograms [kg]
  Serial.println(weight);
  
  Hive h1 = Hive(10, ID, tempC, humidity, weight);
  if(Https_get(h1)==false) {SerialMon.println("https get fall");}

  Close_serve();

  #if TINY_GSM_USE_WIFI
    modem.networkDisconnect();
    SerialMon.println(F("WiFi disconnected"));
  #endif
  #if TINY_GSM_USE_GPRS
    modem.gprsDisconnect();
    SerialMon.println(F("GPRS disconnected"));
  #endif



  delay(30000);
  //delay(5000);
}

float randomFloat(int a, int b) {
  return random(a * 100, b * 100) / 100.0;
}
//Bearing set
bool Bearing_set(){

    modem.sendAT(GF("+HTTPTERM"));//Configuring Bearer Scenarios
    if(modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPTERM"));
        //return false;
    }

    
    modem.sendAT(GF("+SAPBR=0,1"));//Configuring Bearer Scenarios
    if(modem.waitResponse(10000L) != 1) {
       DBG(GF("+SAPBR=0,1"));
        return false;
    }
   delay(1000);

    modem.sendAT(GF("+SAPBR=3,1,\"Contype\",\"GPRS\""));//Configuring Bearer Scenarios
    if(modem.waitResponse(10000L) != 1) {
       DBG(GF("+SAPBR=3,1,\"Contype\",\"GPRS\""));
        return false;
    }

    modem.sendAT(GF("+SAPBR=1,1"));//Activate a GPRS context
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+SAPBR=1,1"));
        //return false;
    }

    modem.sendAT(GF("+SAPBR=2,1"));//Query the GPRS context
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+SAPBR=2,1"));
        //return false;
    }
    delay(2000);
}

bool Https_get(Hive h1){
  
    modem.sendAT(GF("+HTTPINIT"));// Initialize the HTTP service
    if (modem.waitResponse(10000L) != 1) {
      DBG(GF("+HTTPINIT"));
        return false;
    }
    modem.sendAT(GF("+HTTPPARA=\"CID\",1"));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPPARA=\"CID\",1"));
        return false;
    }
        
    String hiveJSON = h1.asJson();
    String encoded = urlEncode(hiveJSON);
    SerialMon.println(encoded);

    String toSend = "51.68.141.235:8088/hives/update?updatehive="+encoded;
    modem.sendAT(GF("+HTTPPARA=\"URL\",\""+toSend+"\""));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPPARA=\"URL\",\"www.baidu.com\""));
        return false;
    }
        modem.sendAT(GF("+HTTPPARA=\"REDIR\",1"));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPPARA=\"REDIR\",1"));
        return false;
    }
    modem.sendAT(GF("+HTTPSSL=0"));//Enabling the HTTPS function
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPSSL=1"));
        return false;
    }

    modem.sendAT(GF("+HTTPACTION=0"));//Get
    if (modem.waitResponse(60000L) != 1) {
       DBG(GF("+HTTPACTION=0"));
        return false;
    }
    delay(10000);

    modem.sendAT(GF("+HTTPREAD"));//Read data from the HTTP server
    if (modem.waitResponse(60000L) != 1) {
       DBG(GF("+HTTPREAD"));
        return false;
    }

}

bool Close_serve(){

    modem.sendAT(GF("+HTTPTERM"));//close https
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+HTTPTERM"));
        return false;
    }

    modem.sendAT(GF("+SAPBR=0,1"));//close GPRS
    if (modem.waitResponse(10000L) != 1) {
       DBG(GF("+SAPBR=0,1"));
        return false;
    }
}
