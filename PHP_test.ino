#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

int addrAP = 0;
int addrPASS = 256;

void setup() {
  EEPROM.begin(1024);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  //WiFiMulti.addAP("PA_Goscie", "superplus");
  WiFiMulti.addAP("OneCloos", "biggenerator");

  if (EEPROM.read(1023) == 1) {
    Serial.println("reading AP from EEPROM...");
    String sAP;
    EEPROM.get(addrAP, sAP);
    char eeAP[sAP.length()];
    sAP.toCharArray(eeAP, sAP.length());
    Serial.println(sAP);

    String sPASS;
    EEPROM.get(addrPASS, sPASS);
    char eePASS[sPASS.length()];
    sPASS.toCharArray(eePASS, sPASS.length());
    Serial.println(sPASS);

    WiFiMulti.addAP(eeAP, eePASS);
  }
}
int oldmil = 0;
void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;


    if (millis() - oldmil > 5000) {
      Serial.print("[HTTP] begin...\n");
      if (http.begin(client, String("http://clooske.y0.pl/connector.php?name=ArduinoNodeMCU&val=" + WiFi.localIP().toString()))) {  // HTTP


        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
      oldmil = millis();
    }

  }

  if (stringComplete) {
    if (inputString.substring(0, 5) == "WIFI;") {

      String app = inputString.substring(5, inputString.indexOf(';', 6)+1);
      String pp = inputString.substring(inputString.indexOf(';', 6) + 1, inputString.indexOf(';', inputString.indexOf(';', 6) + 1));
      char ap[app.length()];
      app.toCharArray(ap, app.length());
      char p[pp.length()];
      pp.toCharArray(p, pp.length());
      Serial.println(String("Adding AP:\n" + String(ap) + "\np:\n" + String(p)));
      WiFiMulti.addAP(ap, p);
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.write(1023, 1);
      EEPROM.put(addrAP, app);
      EEPROM.put(addrPASS, pp);
      EEPROM.commit();
      WiFi.printDiag(Serial);
    }

    if (inputString.substring(0,5).equals("clear")) {
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      Serial.println("Clearing EEPROM...");
    }
    
    inputString = "";
    stringComplete = false;

  }

  // SERIAL EVENT
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char) Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      Serial.println(inputString);
    }
  }
}
