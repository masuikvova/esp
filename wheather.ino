#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTCAPI.hpp"
#include "ArduinoJson.h"
#include "FileReading.hpp"
#include "SetupWiFi.hpp"
#include "API.hpp"

LiquidCrystal_I2C lcd(0x3F, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
bool send = false;
bool loadData = true;
bool beep = false;
int timerCount = 0;

void setup() {
  Serial.begin(9600);
  pinMode(D0, INPUT);
  attachInterrupt(digitalPinToInterrupt(D5), sendDataToFirebase, CHANGE); 
  setupWiFi();
  setupRTC();
  setupTimer();
  setupAPI();
  setupAlarm();
  lcd.init();
  lcd.setBacklight(50);
}

void setupTimer() {
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(timerTriger);
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
  interrupts();
}

void setupAlarm(){
  String alarm = fileRead(alarm_setting_file);
  if (alarm != "FILE ERROR") {
    Serial.println(alarm);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(alarm);
    if (!root.success()) {
      Serial.println("error parse alarm setting");
      isAlarmOn = false;
      return;
    }
    alarmHour = root["hour"];
    alarmMinutes =  root["minutes"];
    isAlarmOn = root["isOn"];
    Serial.println("read alarm settings ok");
  } else {
    Serial.println("read alarm settings error");
    isAlarmOn = false;
    return;
  }
}

void loop() {
  handleAPI();
  if (loadData && WiFi.status() == WL_CONNECTED) {
    getWeather();
    getExchangeRate();
    loadData = false;
  }
  
  if(beep){
    beepTone();
  }
  
  // if(send){
  //sendFirebase("Alarm", "Alarm message", "alarm");
  // TODO setup alarm when outer device will find
  //send = !send;
  // }
}

void timerTriger() {
  getTime();
  timerCount++;
  if (timerCount == 1200) {// update date each 2 minutes
    loadData = true;
    timerCount = 0;
  }
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
}

void getTime() {
  DateTime now = rtc.now();
  char datestring[20];
  snprintf_P(datestring,
             20,
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             now.month(),
             now.day(),
             now.year(),
             now.hour(),
             now.minute(),
             now.second() );
  Serial.println(datestring);

  lcd.setCursor(8, 1);
  if (now.hour() < 10) {
    lcd.print("0");
  }
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute());
  if(isAlarmOn){
    if(alarmHour == now.hour() &&  alarmMinutes == now.minute() && (now.second()==0 || now.second() ==1 )){
      beep = true;
    }
  }
}

void beepTone(){
  beep = false;
  //TODO implement beep function;
}

void sendDataToFirebase() {
  Serial.println("interruption");
  send = true;
}

void getExchangeRate() {
  WiFiClientSecure client;
  String title = "";
  String headers = "";
  String body = "";
  bool finishedHeaders = false;
  bool currentLineIsBlank = true;
  bool gotResponse = false;
  bool needToAdd = false;
  long now;
  char host[] = "api.privatbank.ua";
  if (client.connect(host, 443)) {
    Serial.println("connected");
    String URL = "/p24api/pubinfo?json&exchange&coursid=5";
    client.println("GET " + URL + " HTTP/1.1");
    client.print("Host: "); client.println(host);
    client.println("User-Agent: arduino/1.0");
    client.println("");

    now = millis();
    // checking the timeout
    while (millis() - now < 1500) {
      while (client.available()) {
        char c = client.read();
        if (finishedHeaders) {
          if (c == '[') {
            needToAdd = true;
          }
          if (needToAdd) {
            body = body + c;
          }
          if (c == ']') {
            needToAdd = false;
          }
        } else {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          }
          else {
            headers = headers + c;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
        gotResponse = true;
      }
    }
    Serial.println(body);

    if (gotResponse) {
      DynamicJsonBuffer jsonBuffer;
      JsonArray& root = jsonBuffer.parseArray(body);
      JsonObject& obj = root[0];
      double rate  =  obj["sale"];
      rate_= rate;
      char str_temp[6];
      dtostrf(rate, 4, 2, str_temp);
      lcd.setCursor(0, 1);
      lcd.print("      ");
      lcd.setCursor(0, 1);
      lcd.print("$");
      lcd.print(str_temp);
    }
  }
}

void getWeather() {
  HTTPClient http;
  http.begin("http://api.openweathermap.org/data/2.5/weather?q=kyiv,ua&appid=0e4e4b1aae84ff9f39812706b6230477");
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(response);
    if (!root.success()) {
      Serial.println("Parse weather failed :(");
      return;
    }
    String main = root["weather"][0]["main"].as<String>();
    main_= main;
    double temp  =  root["main"]["temp"];
    double tempC = temp - 273.0;
    tempC_  = tempC;
    double presure = root["main"]["pressure"];
    presure_= presure;
    // Print values.
    Serial.println(main);
    Serial.print("Temperature ");
    Serial.println(tempC);
    Serial.print("Presure  ");
    Serial.println(presure);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(main);
    lcd.print(" ");
    lcd.print(tempC);
    lcd.print("C");
  } else {
    Serial.println("Error in HTTP request (wether)");
  }
  http.end();
}


void sendFirebase(String title, String content, String topicName) {
  //more info @ https://firebase.google.com/docs/cloud-messaging/http-server-ref
  String data = "{";
  data = data + "\"to\": \"/topics/" + topicName + "\",";
  data = data + "\"notification\": {";
  data = data + "\"body\": \"" + title + "\",";
  data = data + "\"title\" : \"" + content + "\" ";
  data = data + "} }";
  HTTPClient httpTOPIK;
  httpTOPIK.begin("http://fcm.googleapis.com/fcm/send");
  httpTOPIK.addHeader("Authorization", "key=AIzaSyBBV9DqZW_wyuUY5HSOVFoHAWxC7TDWLWw");
  httpTOPIK.addHeader("Content-Type", "application/json");
  httpTOPIK.addHeader("Host", "fcm.googleapis.com");
  httpTOPIK.addHeader("Content-Length", String(content.length()));
  httpTOPIK.POST(data);
  httpTOPIK.writeToStream(&Serial);
  httpTOPIK.end();
  Serial.println();
}

