#include <Arduino.h>
#include "HardwareSerial.h"

#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_TEST_GPS true

HardwareSerial SerialAT(1); 

#define DUMP_AT_COMMANDS

const char apn[] = "internet.itelcel.com";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif


int result;
String data;
int val1 = 21, val2 = 3;
String inputCommand = "";
// put function declarations here:
int myFunction(int, int);

bool reply = false;
void modem_on() {
  int i = 40;
  Serial.print(F("\r\n# Startup #\r\n"));
  Serial.print(F("# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
  while (i) {
    SerialAT.println(F("AT"));

    // Show the User: we are doing something.
    Serial.print(F("."));
    delay(500);

    // Did the Modem send something?
    if (SerialAT.available()) {
      String r = SerialAT.readString();
      Serial.print("\r\n# Response:\r\n" + r);
      if ( r.indexOf("OK") >= 0 ) {
        reply = true;
        break;;
      } else {
        Serial.print(F("\r\n# "));
      }
    }

    // Did the User try to send something? Maybe he did not receive the first messages yet. Inform the User what is happening
    if (Serial.available() && !reply) {
      Serial.read();
      Serial.print(F("\r\n# Modem is not yet online."));
      Serial.print(F("\r\n# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
    }

    // On the 5th try: Inform the User what is happening
    if(i == 35) {
      Serial.print(F("\r\n# Modem did not yet answer. Probably Power loss?\r\n"));
      Serial.print(F("# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
    }
    delay(500);
    i--;
  }
  Serial.println(F("#\r\n"));
}
bool configModem(){
  String ret;

  ret = modem.setNetworkMode(2);
  Serial.printf("setNetworkMode: %d dBm", ret);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {//VALIDA LA CONEXION POR QUE S E QUEDA EN UN BUCLE POR EL RETURN
    Serial.println("=>Error al intentar conectar modem<=");
    delay(10000);
    return false;
  }
  return true;
}
void modem_info(){
  String ccid = modem.getSimCCID();
  Serial.printf("CCID: %d dBm", ccid);

  String imei = modem.getIMEI();
  Serial.printf("IMEI: %d dBm", imei);

  String imsi = modem.getIMSI();
  Serial.printf("IMSI: %d dBm", imsi);

  String cop = modem.getOperator();
  Serial.printf("Operator: %d dBm", cop);

  String prov = modem.getProvider();
   Serial.printf("Provider: %d dBm", prov);

  IPAddress local = modem.localIP();
  Serial.printf("Local IP: %d dBm", local);

  int csq = modem.getSignalQuality();
  Serial.printf("Signal quality: %d dBm", csq);
}
bool validateNetwork(){
  bool res = modem.isGprsConnected();
  Serial.print("GPRS status:");
  Serial.println(res ? "connected" : "not connected");
  //############################################################
  return res; 
}
void gnss_data(){
  #if TINY_GSM_TEST_GPS && defined TINY_GSM_MODEM_HAS_GPS
    DBG("Enabling GPS/GNSS/GLONASS and waiting 15s for warm-up");
  #if !defined(TINY_GSM_MODEM_SARAR5)  // not needed for this module
    modem.enableGPS();
  #endif
    delay(15000L);
    float gps_latitude  = 0;
    float gps_longitude = 0;
    float gps_speed     = 0;
    float gps_altitude  = 0;
    int   gps_vsat      = 0;
    int   gps_usat      = 0;
    float gps_accuracy  = 0;
    int   gps_year      = 0;
    int   gps_month     = 0;
    int   gps_day       = 0;
    int   gps_hour      = 0;
    int   gps_minute    = 0;
    int   gps_second    = 0;
    for (int8_t i = 15; i; i--) {
      DBG("Requesting current GPS/GNSS/GLONASS location");
      if (modem.getGPS(&gps_latitude, &gps_longitude, &gps_speed, &gps_altitude,
                      &gps_vsat, &gps_usat, &gps_accuracy, &gps_year, &gps_month,
                      &gps_day, &gps_hour, &gps_minute, &gps_second)) {
        DBG("Latitude:", String(gps_latitude, 8),
            "\tLongitude:", String(gps_longitude, 8));
        DBG("Speed:", gps_speed, "\tAltitude:", gps_altitude);
        DBG("Visible Satellites:", gps_vsat, "\tUsed Satellites:", gps_usat);
        DBG("Accuracy:", gps_accuracy);
        DBG("Year:", gps_year, "\tMonth:", gps_month, "\tDay:", gps_day);
        DBG("Hour:", gps_hour, "\tMinute:", gps_minute, "\tSecond:", gps_second);
        break;
      } else {
        DBG("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
        delay(15000L);
      }
    }
    DBG("Retrieving GPS/GNSS/GLONASS location again as a string");
    String gps_raw = modem.getGPSraw();
  #if !defined(TINY_GSM_MODEM_SARAR5)  // not available for this module
    DBG("GPS/GNSS Based Location String:", gps_raw);
    DBG("Disabling GPS");
    modem.disableGPS();
  #endif
  #endif 
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  SerialAT.begin(115200, SERIAL_8N1, 18, 17);
  delay(100);
  modem_on();
  if (reply) {
    Serial.println(F("***********************************************************"));
    Serial.println(F(" You can now send AT commands"));
    Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
    Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
    Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
    Serial.println(F(" can have undesired consiquinces..."));
    Serial.println(F("***********************************************************\n"));

    // Uncomment to read received SMS
    //SerialAT.println("AT+CMGL=\"ALL\"");
  } else {
    Serial.println(F("***********************************************************"));
    Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
    Serial.println(F("***********************************************************\n"));
  }

  //result = myFunction(val1, val2);
  }

void loop() {
  Serial.println("Initializing modem...");
  if (!modem.init()) {
    Serial.println(F("Failed to restart modem, delaying 10s and retrying"));
    return;
  }
 bool network = validateNetwork();
  if(!network){
    Serial.println("Configurando MODEM ...");
    configModem();
  }else if(network){
    Serial.println("INFO MODEM SIMCOM => ");
    //modem_info();
    gnss_data();
    delay(3000);
  }else{
    Serial.println("MODEM ERR VALID NETWORK!");
    delay(3000);
  }
  // Leer datos del módulo y mostrar en monitor
  if (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar != '\n' && inChar != '\r') {
      inputCommand += inChar;
    }
    if (inChar == '\n' || inChar == '\r') {
      if (inputCommand.length() > 0) {
        SerialAT.println(inputCommand);
        inputCommand = "";
      }
    }
  }

  /*Serial.print("SUM: ");
  data = String(val1)+" + "+String(val2);
  Serial.print(data);
  Serial.print(" = ");
  Serial.println(result);
  delay(3000);*/

}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}


