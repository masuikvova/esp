#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
double rate_ = 0;
double tempC_ = 0;
double presure_ = 0;
String main_ = "";

void setupAPI();
void handleAPI();
void getWiFiInfo();
void clearWifiSetting();
void setTime();
void handleNotFound();
void welcomPage();

void setupAPI() {
  server.begin();
  server.on("/wifi_setup", getWiFiInfo);
  server.on("/clear_wifi_setting", clearWifiSetting);
  server.on("/set_time", setTime);
  server.on("/", welcomPage);
  server.on("/home", welcomPage);
  server.onNotFound(handleNotFound);
}

void handleAPI() {
  server.handleClient();
}

void getWiFiInfo() {
  String ssid = server.arg("ssid");
  String pass = server.arg("password");
  saveWiFiSettings(ssid, pass);
  server.send(200, "text/plain", "OK\r\n");
  Serial.println("Save wifi settings");
  delay(2000);
  ESP.restart();
}

void clearWifiSetting() {
  clearWifiSettingsFile();
  delay(1000);
  ESP.restart();
}

void welcomPage() {
  char temp[400];
  DateTime now = rtc.now();
  snprintf ( temp, 400,
             "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Clock</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Clock</h1>\
    <p>Time: %02d:%02d:%02d</p>\
    <p>Exchange rate $: %2.2f</p>\
    <p>Weather : %s</p>\
    <p>Temperature : %2.2f C</p>\
    <p>Presure : %2.2f</p>\
  </body>\
</html>",

             now.hour(), now.minute(), now.second(), rate_, main_.c_str(), tempC_, presure_);
  server.send ( 200, "text/html", temp );
}

void setTime() {
  String _year = server.arg("year");
  String _month = server.arg("month");
  String _day = server.arg("day");
  String _hour = server.arg("hour");
  String _minutes = server.arg("minutes");
  setupDataTime(_year.toInt(), _month.toInt(), _day.toInt(), _hour.toInt(), _minutes.toInt());
  server.send(200, "text/plain", "OK\r\n");
  Serial.println("set time");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

