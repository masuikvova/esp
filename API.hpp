#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

void getWiFiInfo();
void clearWifiSetting();
void handleNotFound();

void setupAPI(){
  server.begin();
  server.on("/wifi_setup", getWiFiInfo);
  server.on("/clear_wifi_setting", clearWifiSetting);
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

