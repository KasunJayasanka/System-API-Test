#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClient.h>
#include "setting.h"

/****************** GLOBAL VERIABLES ****************************/
//StaticJsonDocument<3072> doc;
DynamicJsonDocument doc(2048);
DeserializationError error;
ESP8266WiFiMulti WiFiMulti;
HTTPClient https;
WiFiClient client;

// preper jason payload
String getSensorValues() {

  String jsonString="";
  delay(1000);

  if (Serial.available()>0) {

    jsonString = Serial.readStringUntil('\n');
    Serial.println(jsonString);

  }
  return jsonString;
}

void setup()
{
  Serial.begin(9600);
  WifiInit();
}

void loop()                                                                                
{
  // get data

      String jsonOutput = getSensorValues();
      
      // post data to server and get response
      error = deserializeJson(doc, SendDataToServer(jsonOutput));
      
      if (error)
      {
        // print error to serial monitor
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      
      // Get the root object in the document
      JsonObject root = doc.as<JsonObject>();
      String success = root["success"];
      // String massage = root["massage"];
      String massage = root["msg"];
    
      // sleep for 2s
      delay(5000);

}

/****************** HTTP  REQUEST AND WIFI INITIALIZATION ****************************/


void WifiInit() {

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  connectToWifi();
}

String SendDataToServer(String data) {
  if(WiFi.status() == WL_CONNECTED){                            // check wifi cinnection status

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
   
    client->setInsecure();

    https.begin(*client, API);                                    // Specify request destination
    
    https.addHeader("content-type", "application/json");          // Specify content-type header
    //https.addHeader("X-API-KEY", API_KEY);          // Add api key here
    int httpCode = https.POST(data);                             // send the request
    
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);
        // We have response from server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          return payload;
        }
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        return ERROR_MSG;
      }
    https.end(); // Close connectiong
    return ERROR_MSG;
  }
  else{
    connectToWifi();
    return ERROR_MSG_WIFI;
  }
}

void connectToWifi(void){

    while((WiFiMulti.run() != WL_CONNECTED)){
        WiFiMulti.addAP(SSID,PASSWORD);
        Serial.println("Waiting for connection");
        delay(5000);
    }
    Serial.println("connected");
}


