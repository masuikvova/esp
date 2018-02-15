const char *ssidAP = "ESPWIFI";
const char *passwordAP = "1234567890";
String setting_file = "wifi_setting";
String ssid;
String password;
byte tries = 11;

IPAddress ip(192, 168, 0, 120);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void setupWiFi();
void startAccessPoint();
void connectToWiFi();
void saveWiFiSettings(String ssid, String pass);
void clearWifiSettingsFile();

void startAccessPoint() {
  Serial.println("");
  Serial.println("WiFi up AP");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP, passwordAP);
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, gateway);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (--tries && WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("connect fail");
  }
}

void setupWiFi() {
  // Попытка подключения к точке доступа
  String creditance = fileRead(setting_file);
  if (creditance != "FILE ERROR") {
    Serial.println(creditance);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(creditance);
    if (!root.success()) {
      Serial.println("error parse wifi settings");
      startAccessPoint();
      return;
    }
    ssid = root["ssid"].as<String>();
    password = root["password"].as<String>();
    Serial.println(ssid);
    Serial.println(password);
  } else {
    startAccessPoint();
    return;
  }
  connectToWiFi();
}

void saveWiFiSettings(String ssid, String pass) {
  String data = "{";
  data = data + "\"ssid\": \"" + ssid + "\",";
  data = data + "\"password\": \"" + pass + "\"";
  data = data + "}\n";
  Serial.println(data);
  fileWrite(setting_file, data);
}


void clearWifiSettingsFile() {
  fileRemove(setting_file);
}


