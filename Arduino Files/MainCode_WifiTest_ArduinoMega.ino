#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

// Libraries for DS18B20 Temperature Sensor 
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SoftwareSerial.h> //Library for setup required Digital Pins for Serial Communications
#include<Servo.h>  //Library for Servo Motor
#include <TinyGPSPlus.h> //Library for NEO-6M-0-001 GPS Module

#include <RTClib.h> //Library for DS3231 RTC Module

//To Temperature Sensor
#define ONE_WIRE_BUS 28

OneWire oneWire(ONE_WIRE_BUS);
// Setup a oneWire instance to communicate with any OneWire devices
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//LCD Display
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); 

float lpGasThreshVal=100.0;
float lpGasReadingVal;

int count = 0;
//int temp = 0;

//Buzzer_____________
// Pin connected to the positive (long leg) of the buzzer
const int buzzerPin = 8;
// Pin connected to the sensor input
const int sensorPin = A0;

//GSM MODULE____________
SoftwareSerial mySerial (10,9);
bool alertSentMessage=false;

//Flame Sensor
const int flameSensorPin = 3;

//servo motor
Servo servo;
int servoPosition = 0;
int buttonState = 0;
int prevButtonState = 0;
const int servoPin = 7 ; //The pin connected to the servo motor

//Boolean values about the Flame Sensor and Gas Sensor
bool gasLeakageDetected = false;
bool flameDetected = false;

//push button
const int buttonPin = 4 ; //The pin connected to the push button

//magnetic sensor and window alert buzzer
const int magSensorBuzzer = 5; 
const int magneticSensor = 6;

//RTC Module
RTC_DS3231 rtc;
char t[32];
String date,time,dateTime;

//Magnetic Sensor
int window1Status;
int window2Status=1;

//Flame Sensor
int flameSensor;

//Temperature Sensor
float temperatureValue=10.5;

//Weight Sensor
float gasWeight=5;

void setup() {

  lcd.init();
  lcd.backlight();

  //Setup GAS Sensor Analog Input Pin
  pinMode(A0,INPUT);
  
  //For Temperature Sensor
  sensors.begin();

  //FOR GSM======
  Serial.begin(9600);
  mySerial.begin(9600);
  
  //FOR BUZZER======
  // Initialize the buzzer pin as an output
  pinMode(buzzerPin, OUTPUT);
  // Set the sensor pin as an input
  pinMode(sensorPin, INPUT);

  //LED Blink system
  pinMode(11, OUTPUT);  

  //disconect current with relay module
    
  pinMode(2,OUTPUT);

  //Flame sensor
  pinMode(flameSensorPin,INPUT);

  //servo motor

  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);   // Configure the button pin as INPUT_PULLUP
  servo.attach(servoPin);              // Attach the servo to its pin
  servo.write(servoPosition);          // Initialize the servo position to 0 degrees

  //magnetic sensor
  pinMode(magneticSensor,INPUT_PULLUP);
  pinMode(magSensorBuzzer,OUTPUT);

  //GPS Module
  Serial2.begin(9600);

  //RTC Module
  Wire.begin();
  rtc.begin();

  //  if (! rtc.begin()) 
  // {
  //   Serial.println(" RTC Module not Present");
  //   while (1);
  // }

  if (rtc.lostPower())
  {
    //Serial.println("RTC power failure, reset the time!");
    rtc.adjust(DateTime(2023, 6, 6, 12, 0, 0));
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


}


void loop() {

  //Getting Current Date and Time 
  String dateTimeString=getDateTime();
  date = dateTimeString.substring(dateTimeString.indexOf(":") + 1, dateTimeString.indexOf(","));
  dateTimeString = dateTimeString.substring(dateTimeString.indexOf(",") + 1); // Remove the first sensor value
  time = dateTimeString.substring(dateTimeString.indexOf(":") + 1, dateTimeString.indexOf(","));
  dateTime=date+" "+time;

  //Getting Temperature
  temperatureValue=sensors.getTempCByIndex(0);

  // Serial.println("Date:");
  // Serial.println(date);
  // Serial.println(time);

  lpGasReadingVal = analogRead(A0);
  //Serial.println(lpGasReadingVal);

  buttonState = digitalRead(buttonPin);      // Read the state of the button

  //magnetic sensor
  window1Status=digitalRead(magneticSensor);

  // Serial.print("Window State = ");
  // Serial.println(window1Status);

  serialCommunicateData(dateTime,date,time,gasLeakageDetected,flameDetected,temperatureValue,window1Status,window2Status,gasWeight);

  if(window1Status==LOW)
  {
    tone(magSensorBuzzer,700,1000);
    delay(1000);
  }
  else
  {
    noTone(magSensorBuzzer);
  }
  

  if(lpGasReadingVal>=lpGasThreshVal){
  //display gas leak

    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Gas Leakage!"); 

  //LED Blink system ON
    digitalWrite(11, HIGH);   // turn the LED on (HIGH is the voltage level) 

  //Turn off the Electricity and Turn on the Exhaust Fan

    digitalWrite(2,HIGH); 
    delay(1000);

  //Rotating Servo Motor   

    if (!gasLeakageDetected) 
    {
      if (servoPosition == 0) {
        servoPosition = 90;     // Set servo position to 90 degrees
      }
        gasLeakageDetected = true;
    }      
    delay(1000);



  
  //Display Temperature on LCD Display
    sensors.requestTemperatures(); 
    lcd.setCursor(0, 1);
    lcd.print("Temp ");
    temperatureValue=sensors.getTempCByIndex(0);
    lcd.print(temperatureValue);
  //delay(1000);

    serialCommunicateData(dateTime,date,time,gasLeakageDetected,flameDetected,temperatureValue,window1Status,window2Status,gasWeight);

  //Ringing Buzzer to alert about Gas Leakage
    tone(buzzerPin, 1000, 1000);
    delay(500);
    tone(buzzerPin, 700, 2000);
    delay(500);
    tone(buzzerPin, 1000, 1000);
    delay(500);
    tone(buzzerPin, 700);

  //Send Message to User

  if (alertSentMessage==false)
  {
  //Send message by gsm module
    SendMessage("+94775840180", "ALERT!...Gas Leakage Detected!");
   alertSentMessage = true;
  }
  
  //turn off led
    digitalWrite(11, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);             
  }
  
  else{

    //Display Gas Leakage Status on LCD Display
    lcd.setCursor(0, 0); 
    lcd.print("No Gas Leakage!");

    //Turn off Buzzer which ring when Gas Leakage
    noTone(buzzerPin); 

    gasLeakageDetected = false;  //boolean value of gas leakage
    alertSentMessage = false;    //boolean value indicating whether a message has been already sent

    //Display Temperature Value on LCD Display
    sensors.requestTemperatures(); 
    lcd.setCursor(0, 1);
    lcd.print("Temp ");
    temperatureValue=sensors.getTempCByIndex(0);
    lcd.print(temperatureValue);
    delay(1000);

    //Turn on the Electricity and Turn off Exhaust Fan
    digitalWrite(2,LOW);
    delay(1000) ;       
  }

  //Flame Sensor
  flameSensor = digitalRead(flameSensorPin);
  if(flameSensor==0)//check if flame is detected
  {
      
    //Display Fire Status & Temperature Value on LCD
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Fire Detected!");
    sensors.requestTemperatures(); 
    lcd.setCursor(0, 1);
    lcd.print("Temp ");
    temperatureValue=sensors.getTempCByIndex(0);
    lcd.print(temperatureValue);

    //Servo Motor Rotate if fire exists  
    if (!flameDetected) 
    {
      if (servoPosition == 0) {
        servoPosition = 90;     // Set servo position to 90 degrees
      }
        flameDetected = true;
    }      
    delay(1000);

    serialCommunicateData(dateTime,date,time,gasLeakageDetected,flameDetected,temperatureValue,window1Status,window2Status,gasWeight);

    //ring buzzer 
    tone(buzzerPin,1200);

    //Send Message to User and Fire Authority
    if (alertSentMessage==false)
    {
      //message to owner 
      SendMessage("+94775840180",("ALERT!...Fire Detected!"));
      if (mySerial.available()>0)
      Serial.write(mySerial.read());
      delay(2000);
      
      //message to fire authority
      SendMessage("+94712288226",FireAlertMessage());
      delay(3000);
     alertSentMessage = true;
    }

  }
  else
  {
      flameDetected = false;  
      alertSentMessage = false;
      if(gasLeakageDetected==false) noTone(buzzerPin);
  }

  /************************************************************************************************************/
  
  //Manually Control the Servo Motor Using Push Button
  if (buttonState != prevButtonState) {
    if (buttonState == LOW) {    // If the button is pressed (LOW state)
      if (servoPosition == 0) {
        servoPosition = 90;     // Set servo position to 90 degrees
      }
      else {
        servoPosition = 0;      // Set servo position to 0 degrees
      }
    }
  }

  servo.write(servoPosition);   // Move the servo to the desired position
  prevButtonState = buttonState;   // Save the button state for the next iteration

  /**********************************END of The Loop***************************************/
  
}

/***************************************************Custom Functions ************************************/

/*==========================Functions to Send Message Through GSM Module=======================*/

/*==========================Sending Message to User=======================*/
void SendMessage(const char* recipient, const String message){

  mySerial.println("AT+CMGF=1");
  delay(2000);
  mySerial.print("AT+CMGS=\"");
  mySerial.print(recipient);  // Set the recipient phone number
  mySerial.println("\"");
  delay(1000);
  mySerial.print(message);
  delay(1000);

  mySerial.write(26);  // Send the Ctrl+Z character to indicate the end of the message
  delay(1000);
}


/*==========================Sending Message to Fire Authority=======================*/

//creates the message string to be sent to the fire authority using GPS location coordinates

String FireAlertMessage()//creates the message string to be sent to the fire authority using GPS location coordinates
{
  TinyGPSPlus gps;
  String date, time, url, message;
  //Serial.println(Serial2.available());
  int count = 0;
  while ((!gps.location.isValid()||!gps.time.isValid())&&count<=400)
  {
    gps.encode(Serial2.read()); 
    count++;   
  }
  if (gps.location.isValid())
  { 
    url = "\nLocation: www.google.com/maps/search/?api=1&query="+String(gps.location.lat(), 7)+"%2C"+String(gps.location.lng(),7);
  }
 
  if (gps.date.isValid())
  {
    date = "\nDate: "+String(gps.date.day())+"/"+String(gps.date.month())+"/"+String(gps.date.year());
  }
  else
  {
    date = " ";
  }

  Serial.print(" ");
  if (gps.time.isValid())
  {
    //converting UTC to local time (+0530)
    int localHour, localMinute, localSecond;
    String timeOfDay = "AM"; //AM, PM
    localHour = (gps.time.hour()+5);
    if(localHour>=12) timeOfDay = "PM";
    if(localHour>12) localHour -=12;
    localMinute = (gps.time.minute()+30);
    if(localMinute>=60) {localMinute-=60; localHour+=1;}
    localSecond = gps.time.second();

    time = "\nTime: ";
    if(localHour<10) time+= "0";
    time = time + String(localHour)+":";
    if(localMinute<10) time +="0";
    time = time + String(localMinute)+":";
    if(localSecond<10) time +="0";
    time = time +String(localSecond)+" ";
    time += timeOfDay;
  }

  message = "ALERT! Fire Detected."+ url + date + time;
  // Serial.print("\n");
  // Serial.println(message);
  
  return message;
}


/*==========================Getting Date & Time using RTC Module=======================*/

String getDateTime(){

  DateTime now = rtc.now();

  String time=String(now.hour())+":"+String(now.minute())+":"+String(now.second());
  String date=String(now.year())+"-"+String(now.month())+"-"+String(now.day());
  // sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  
  // Serial.print(F("Date/Time: "));
  // Serial.println(t);
  // Serial.print("");
  // Serial.println("Date "+Date);
  // Serial.println("Time "+Time);

  String dateTime="Date:"+date+","+"Time:"+time;

  return dateTime;
  
  delay(1000);

}

/*==========================Serial Communicating to WiFi Sheild=======================*/

void serialCommunicateData(String dateTime,String date,String time,bool gasLeakageDetected,bool flameDetected,float temperatureValue,int window1Status,int window2Status,float gasWeight){

    String serialComStr;
    serialComStr="dateTime:"+dateTime+",";
    serialComStr+="date:"+date+",";
    serialComStr+="time:"+time+",";
    serialComStr+="gasLeakageDetected:"+String(gasLeakageDetected)+",";
    serialComStr+="flameDetected:"+String(flameDetected)+",";
    serialComStr+="temperatureValue:"+String(temperatureValue)+",";
    serialComStr+="window1Status:"+String(window1Status)+",";
    serialComStr+="window2Status:"+String(window2Status)+",";
    serialComStr+="gasWeight:"+String(gasWeight)+",";

    Serial.println(serialComStr);
    delay(1000);

}
