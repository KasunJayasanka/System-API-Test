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
String getSensorValues(String dateTime,String date,String time,String gasLeakageDetected,String flameDetected,String temperatureValue,String window1Status,String window2Status,String gasWeight) {

  delay(1000);

  String jsonString = "";
  JsonObject data = doc.to<JsonObject>(); // create json object

  // set X-API-KEY
  data["X-API-KEY"] = API_KEY;

  data["dateTime"] = dateTime;
  data["date"] = date;
  data["time"] = time;
  data["gasLeakageDetected"] = gasLeakageDetected;
  data["flameDetected"] = flameDetected;
  data["temperatureValue"] = temperatureValue;
  data["window1Status"] = window1Status;
  data["window2Status"] = window2Status;
  data["gasWeight"] = gasWeight;

  serializeJson(data, jsonString);

  Serial.println(jsonString); // print to serilal monitoro for debug purpuses

  return jsonString;

}


String gettingValuesFromArduino(){
    String jsonOutput;
    
    if (Serial.available()>0) {
    
      String dataString = Serial.readStringUntil('\n');
      
      dataString.trim();

      String dateTime = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the first sensor value
      String date = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the second sensor value
      String time = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the third sensor value
      String gasLeakageDetected = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the fourth sensor value
      String flameDetected = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the fifth sensor value
      String temperatureValue = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the sixth sensor value
      String window1Status = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the seventh sensor value
      String window2Status = dataString.substring(dataString.indexOf(":") + 1, dataString.indexOf(","));
      dataString = dataString.substring(dataString.indexOf(",") + 1); // Remove the eighth sensor value
      String gasWeight = dataString.substring(dataString.indexOf(":") + 1,dataString.indexOf(","));
      
      //printReceivedData(dateTime,date,time,gasLeakageDetected,flameDetected,temperatureValue,window1Status,window2Status,gasWeight);

      jsonOutput= getSensorValues(dateTime,date,time,gasLeakageDetected,flameDetected,temperatureValue,window1Status,window2Status,gasWeight);
      }
      else
      {
      jsonOutput=getSensorValues("","","","","","","","","");
      }
      return jsonOutput;
}

void setup()
{
  Serial.begin(9600);
  WifiInit();
}

void loop()                                                                                
{
  // get data

      String jsonOutput = gettingValuesFromArduino();
      
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

void printReceivedData(String dateTime,String date,String time,String gasLeakageDetected,String flameDetected,String temperatureValue,String window1Status,String window2Status,String gasWeight){

    String serialComStr;
    serialComStr="dateTime:"+dateTime+",";
    serialComStr+="date:"+date+",";
    serialComStr+="time:"+time+",";
    serialComStr+="gasLeakageDetected:"+gasLeakageDetected+",";
    serialComStr+="flameDetected:"+flameDetected+",";
    serialComStr+="temperatureValue:"+temperatureValue+",";
    serialComStr+="window1Status:"+window1Status+",";
    serialComStr+="window2Status:"+window2Status+",";
    serialComStr+="gasWeight:"+gasWeight;

    Serial.println(serialComStr);
    delay(1000);

}
