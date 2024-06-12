#include "ESPAsyncWebSrv.h"   // https://github.com/me-no-dev/ESPAsyncWebServer/tree/63b5303880023f17e1bca517ac593d8a33955e94
#include <WiFi.h>             // Built-in

//################ VARIABLES ################
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";

const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";

//#########################################################################################
// Read File from LittleFS
// David Bird / book examples
String readFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}
//#########################################################################################
// Write file to LittleFS
// David Bird / book examples
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}
//#########################################################################################
// delete file
// book examples
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}
//#########################################################################################
// Initialize WiFi
bool initWiFi(String ssid, String pass) {
  IPAddress dns(8, 8, 8, 8);  // Use Google as DNS
  if (ssid == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
  unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}
//#########################################################################################
// Get WiFi Signal Strength
// David Bird / book examples
String WiFiSignal() {
  float Signal = WiFi.RSSI();
  Signal = 90 / 40.0 * Signal + 212.5;  // From Signal = 100% @ -50dBm and Signal = 10% @ -90dBm and y = mx + c
  if (Signal > 100) Signal = 100;
  return " " + String(Signal, 0) + "%";
}
//#########################################################################################
// WiFi Manager page
// book examples
String WiFiPage(String sitetitle){
  String webpage;
  webpage = "<!DOCTYPE html>";
  webpage += "<html>";
  webpage += "<head>";
  webpage += "  <title>ESP Wi-Fi Manager - " + sitetitle + "</title>";
  webpage += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  webpage += "  <link rel=\"icon\" href=\"data:,\">";
  webpage += "  <style>";
  webpage += CreateHtmlCss();
  webpage += "  </style>";
  webpage += "</head>";
  webpage += "<body>";
  webpage += "  <div class=\"topnav\">";
  webpage += "    <h1>ESP Wi-Fi Manager</h1>";
  webpage += "  </div>";
  webpage += "  <div class=\"content\">";
  webpage += "    <div class=\"card-grid\">";
  webpage += "      <div class=\"card\">";
  webpage += "        <form action=\"/\" method=\"POST\">";
  webpage += "          <p>";
  webpage += "            <label for=\"ssid\">SSID</label>";
  webpage += "            <input type=\"text\" id =\"ssid\" name=\"ssid\"><br>";
  webpage += "            <label for=\"pass\">Password</label>";
  webpage += "            <input type=\"text\" id =\"pass\" name=\"pass\"><br>";
  webpage += "            <input type=\"submit\" value=\"Submit\">";
  webpage += "          </p>";
  webpage += "        </form>";
  webpage += "      </div>";
  webpage += "    </div>";
  webpage += "  </div>";
  webpage += "</body>";
  webpage += "</html>";
  return webpage;
}
//#########################################################################################
// Reset WiFi
void ResetWiFiAndSettings(){
  Serial.println("Delete files / format LittleFS ...");
  LittleFS.format();
  delay(200);
}
//#########################################################################################
// Setup WiFi
// It needs the AsyncWebServer server(80);
boolean SetupWiFi(){
  String ssid;
  String pass;
  // Load values saved in SPIFFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  Serial.println(ssid);
  Serial.println(pass);

  if (initWiFi(ssid, pass)) {
    Serial.println("Connected to " + ssid);
    return true;
  } else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [&sitetitle](AsyncWebServerRequest* request) {
      request->send(200, "text/html", WiFiPage(sitetitle));
    });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            String p1 = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(p1);
            // Write file to save value
            writeFile(LittleFS, ssidPath, p1.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            String p2 = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(p2);
            // Write file to save value
            writeFile(LittleFS, passPath, p2.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router!");
      delay(3000);
      ESP.restart();
    });
    return false;
 }
}
