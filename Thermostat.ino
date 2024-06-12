/*
  This software started from the ideas and concepts of David Bird Copyright (c) 2020
  This modified software version ideas and concepts is Copyright Bogdan Vaduva (c) 2024
  All rights to this software are reserved.
  It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
  5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies or substantial portions of the software
     and where the software use is visible to an end-user.
  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
  FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR
  A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OR
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
Improvment will be to add at setup an MQTT url, topic, user and password where the data will be sent with a specific rate
*/
//################# LIBRARIES ################
#include <ESPmDNS.h>          // Built-in
#include "ESPAsyncWebSrv.h"   // https://github.com/me-no-dev/ESPAsyncWebServer/tree/63b5303880023f17e1bca517ac593d8a33955e94 or a fork as here
#include "AsyncTCP.h"         // https://github.com/me-no-dev/AsyncTCP
#include "FS.h"
#include <LittleFS.h>
#include <DHT.h>

#define SensorReadings 144              // maximum number of sensor readings, typically 144/day at 6-per-hour
#define NumOfSensors 2                  // we have 1 of 2 sensors: Temperature and Humidity
#define NumOfEvents 4                   // Number of events per-day, 4 is a practical limit
#define ON true                         // Define what ON means
#define OFF false                       // Define what OFF means
#define FORMAT_LITTLEFS_IF_FAILED true  // Format LittleFS
#define SIMULATE false                  // If we don't have sensors and want to test the software
#define RelayReverse true               // Set to true for Relay that requires a signal LOW for ON
#define RelayPIN 3                      // Define the Relay Control pin
#define LEDPIN 8
#define DHTPIN 2                        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11                   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

//################  VERSION  ###########################################
String version = "1.0";                 // Program version; I've reordered functions/logic
String sitetitle = "Smart Thermostat";  // Site title
String Year = "2024";                   // Year - For the footer line

//################ VARIABLES FOR Temperature and Humidity ###########################################
float Temperature = 0;                // Variable for the current temperature
float Humidity = 0;                   // Variable for the current temperature
String RelayState = "OFF";            // Current setting of the control/thermostat relay
float TargetTemp = 21;                // Default thermostat value for set temperature

struct SensorsData{                   // Sensors data (2 sensors - Temperature, Humidity)
  float Temperature = 0;
  float Humidity = 0;
  String RelayState;
};

SensorsData sensordata[SensorReadings];
int sensordataCounter = 0;             // Used for sensor data storage

struct TimerData {
  String DoW;                 // Day of Week for the programmed event
  String Start[NumOfEvents];  // Start time
  String Stop[NumOfEvents];   // End time
  String Temp[NumOfEvents];   // Required temperature during the Start-End times
};
TimerData Timer[7];                       // Timer settings, 7-days of the week

struct ThermostatSettings {
  const char* ServerName = "ESP32Thermostat";  // Connect to the server with http://hpserver.local/ e.g. if name = "myserver" use http://myserver.local/
  const char* Timezone = "GMT0BST,M3.5.0/01,M10.5.0/02";
  // Example time zones
  //const char* Timezone = "MET-1METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
  //const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
  //const char* Timezone = "EST-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
  //const char* Timezone = "EST5EDT,M3.2.0,M11.1.0";           // EST USA
  //const char* Timezone = "CST6CDT,M3.2.0,M11.1.0";           // CST USA
  //const char* Timezone = "MST7MDT,M4.1.0,M10.5.0";           // MST USA
  //const char* Timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";      // Auckland
  //const char* Timezone = "EET-2EEST,M3.5.5/0,M10.5.5/0";     // Asia
  //const char* Timezone = "ACST-9:30ACDT,M10.1.0,M4.1.0/3":   // Australia

  String DataFile = "dataFile.txt";     // Storage file name on flash

  float Hysteresis = 0.2;               // Heating Hysteresis default value
  float TargetTemp = 21;                // Default thermostat value for set temperature
  int FrostTemp = 5;                    // Default thermostat value for frost protection temperature
  int ManualOverride = 0;               // Manual override
  float ManOverrideTemp = 21;           // Manual override temperature
  float MaxTemperature = 28;            // Maximum temperature detection, switches off thermostat when reached
  int EarlyStart = 0;                   // Default thermostat value for early start of heating
  String Units = "M";                   // or Units = "I" for °F and 12:12pm time format
  int TimerCheckDuration = 15000;        // Check for timer event every 5-seconds
  int UnixTime = 0;                     // Time now (when updated) of the current time

  String Time_str, DoW_str;             // For Date and Time
  String TimerState = "OFF";            // Current setting of the timer
};

ThermostatSettings settings;

int LastTimerSwitchCheck = 0;         // Counter for last timer check
int LastReadingCheck = 0;             // Counter for last reading saved check
int LastReadingDuration = 1;          // Add sensor reading every n-mins
float LastTemperature = 0;            // Last temperature used for rogue reading detection

AsyncWebServer server(80);            // Server on IP address port 80 (web-browser default, change to your requirements, e.g. 8080
// To access server from outside of a WiFi (LAN) network e.g. on port 8080 add a rule on your Router that forwards a connection request
// to http://your_WAN_address:8080/ to http://your_LAN_address:8080 and then you can view your ESP server from anywhere.
// Example http://yourhome.ip:8080 and your ESP Server is at 192.168.0.40, then the request will be directed to http://192.168.0.40:8080  SetupSystem();  // General system setup

//#########################################################################################
void setup() {
  SetupSystem();
  if (!SetupTime()) {                     // Start NTP clock services
    Serial.println("Error setting time...");
  }
  StartLittleFS();                        // Start LittleFS file system (SPIFFS depreciated)
  SetupDeviceName(settings.ServerName);   // Set logical device name - could be saved into a file
  Initialize_Array();                     // Initialize the array for storage and set some values
  RecoverSettings();                      // Recover settings from LittleFS
  if (SetupWiFi()){
    TurnHeating(OFF);                     // Switch heating OFF at init and will see after...
    digitalWrite(RelayPIN, LOW);
    routes(sitetitle, version, Year);     // Setting up routes and pages
    RecoverSettings();                    // Recover settings
  } else {
    SaveSettings();                       // Settings are saved when we initialize WiFi
  }
  // It doesn't matter if we setup the WiFi, we start our sensors and logging
  StartSensors();                         // Start the sensors (DHT in our case)
  Serial.println("Read sensors...");
  ReadSensors();                          // Get current values for the sensors
  ControlHeating();
  Serial.println("Add sensor readings..."); // To the entire array
  for (int r = 0; r < SensorReadings; r++) {
    sensordata[r].Temperature = Temperature;
    sensordata[r].Humidity = Humidity;
    sensordata[r].RelayState = RelayState;
  }
  LastTimerSwitchCheck = millis() + settings.TimerCheckDuration;  // preload timer value with update duration
  server.begin();
}
//#########################################################################################
void loop() {
  // We light the blue LED on the ESP32 to show that is working
  digitalWrite(LEDPIN, HIGH);
  delay(1000);
  digitalWrite(LEDPIN, LOW);
  delay(1000);
  // The logic from the original program of David Bird
  if ((millis() - LastTimerSwitchCheck) > settings.TimerCheckDuration) {
    LastTimerSwitchCheck = millis();    // Reset time
    ReadSensors();                      // Get sensor readings, or get simulated values if 'simulated' is ON
    UpdateLocalTime();                  // Updates Time UnixTime to 'now'
    CheckTimerEvent();                // Check for schedules actuated
    Serial.println("Loop - read sensors");
  }
  if ((millis() - LastReadingCheck) > (LastReadingDuration * 60 * 1000)) {
    LastReadingCheck = millis();        // Update reading record every ~n-mins e.g. 60,000uS = 1-min
    AssignSensorReadingsToArray();
    Serial.println("Loop - assign read sensor values");
  }
}
//#########################################################################################
// David Bird
void CheckTimerEvent() {
  String TimeNow;
  UpdateTargetTemperature();
  TimeNow = ConvertUnixTime(settings.UnixTime);                      // Get the current time e.g. 15:35
  settings.TimerState = "OFF";                                       // Switch timer off until decided by the schedule
  if (settings.EarlyStart > 0) {                                     // If early start is enabled by a value > 0
    TimeNow = ConvertUnixTime(settings.UnixTime + settings.EarlyStart * 60);  // Advance the clock by the Early Start Duration
  }
  if (settings.ManualOverride == 1) {      // If manual override is enabled then turn the heating on
    TargetTemp = settings.ManOverrideTemp;  // Set the target temperature to the manual overide temperature
    ControlHeating();              // Control the heating as normal
  } else {
    TargetTemp = settings.TargetTemp;
    for (byte dow = 0; dow < 7; dow++) {  // Look for any valid timer events, if found turn the heating on
      for (byte p = 0; p < NumOfEvents; p++) {
        // Now check for a scheduled ON time, if so Switch the Timer ON and check the temperature against target temperature
        if (String(dow) == settings.DoW_str && (TimeNow >= Timer[dow].Start[p] && TimeNow <= Timer[dow].Stop[p] && Timer[dow].Start[p] != "")) {
          settings.TimerState = "ON";
          ControlHeating();
          settings.ManualOverride = 0;  // If it was ON turn it OFF when the timer starts a controlled period
        }
      }
    }
  }
  CheckAndSetFrostTemperature();
}
//#########################################################################################
// David Bird
void UpdateTargetTemperature() {
  String TimeNow;
  TimeNow = ConvertUnixTime(settings.UnixTime);
  for (byte dow = 0; dow < 7; dow++) {
    for (byte p = 0; p < NumOfEvents; p++) {
      if (String(dow) == settings.DoW_str && (TimeNow >= Timer[dow].Start[p] && TimeNow < Timer[dow].Stop[p])) {
        TargetTemp = Timer[dow].Temp[p].toFloat();  // Found the programmed set-point temperature from the scheduled time period
      }
    }
  }
  if (settings.ManualOverride == 1) TargetTemp = settings.ManOverrideTemp;
  Serial.println("Target Temperature = " + String(TargetTemp, 1) + "°");
}
//#########################################################################################
// David Bird
void CheckAndSetFrostTemperature() {
  if (settings.TimerState == "OFF" && settings.ManualOverride == 0) {  // Only check for frost protection when heating is off
    if (Temperature < (settings.FrostTemp - settings.Hysteresis)) {      // Check if temperature is below Frost Protection temperature and hysteresis offset
      TurnHeating(ON);                              // Switch Relay/Heating ON if so
      Serial.println("Frost protection actuated...");
    }
    if (Temperature > (settings.FrostTemp + settings.Hysteresis)) {  // Check if temerature is above Frost Protection temperature and hysteresis offset
      TurnHeating(OFF);                         // Switch Relay/Heating OFF if so
    }
  }
}
//#########################################################################################
// David Bird
void ControlHeating() {
  if (Temperature < (TargetTemp - settings.Hysteresis)) {  // Check if room temeperature is below set-point and hysteresis offset
    TurnHeating(ON);                           // Switch Relay/Heating ON if so
  }
  if (Temperature > (TargetTemp + settings.Hysteresis)) {  // Check if room temeperature is above set-point and hysteresis offset
    TurnHeating(OFF);                          // Switch Relay/Heating OFF if so
  }
  if (Temperature > settings.MaxTemperature) {  // Check for faults/over-temperature
    TurnHeating(OFF);               // Switch Relay/Heating OFF if temperature is above maximum temperature
  }
  Serial.println("Control heating executed ...");
}
//#########################################################################################
// David Bird (but this is a book example)
void TurnHeating(bool demand) {
  pinMode(RelayPIN, OUTPUT);
  if (demand) {
    RelayState = "ON";
    if (RelayReverse) {
      digitalWrite(RelayPIN, LOW);
    } else {
      digitalWrite(RelayPIN, HIGH);
    }
    Serial.println("Thermostat ON");
  } else {
    RelayState = "OFF";
    if (RelayReverse) {
      digitalWrite(RelayPIN, HIGH);
    } else {
      digitalWrite(RelayPIN, LOW);
    }
    Serial.println("Thermostat OFF");
  }
}
//#########################################################################################
// David Bird
void Initialize_Array() {
  Timer[0].DoW = "Sun";
  Timer[1].DoW = "Mon";
  Timer[2].DoW = "Tue";
  Timer[3].DoW = "Wed";
  Timer[4].DoW = "Thu";
  Timer[5].DoW = "Fri";
  Timer[6].DoW = "Sat";
  Serial.println("Initialized Timer ...");
}
//#########################################################################################
// David Bird and ...
void SetupSystem() {
  Serial.begin(115200);  // Initialise serial communications
  delay(200);
  Serial.println(__FILE__);
  Serial.println("Starting...");
  digitalWrite(LEDPIN, HIGH);
  pinMode(LEDPIN, OUTPUT);
}
//#########################################################################################
void StartLittleFS() {
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      Serial.println("LittleFS Mount Failed");
      return;
  }
  Serial.println("LittleFS Succesfully Mounted");
}
//#########################################################################################
// David Bird and ...
void SaveSettings() {
  Serial.println("Getting ready to Save settings...");
  File dataFile = LittleFS.open("/" + settings.DataFile, "w");
  if (dataFile) {
    Serial.println("Saving settings...");
    for (byte dow = 0; dow < 7; dow++) {
      Serial.println("Day of week = " + String(dow));
      for (byte p = 0; p < NumOfEvents; p++) {
        dataFile.println(Timer[dow].Temp[p]);
        dataFile.println(Timer[dow].Start[p]);
        dataFile.println(Timer[dow].Stop[p]);
        Serial.println("Period: " + String(p) + " " + Timer[dow].Temp[p] + " from: " + Timer[dow].Start[p] + " to: " + Timer[dow].Stop[p]);
      }
    }
    dataFile.println(settings.Hysteresis, 1);
    dataFile.println(settings.FrostTemp, 1);
    dataFile.println(settings.EarlyStart);
    dataFile.println(settings.ManualOverride);
    dataFile.println(settings.ManOverrideTemp);
    dataFile.println(settings.TargetTemp);
    Serial.println("Saved Hysteresis : " + String(settings.Hysteresis));
    Serial.println("Saved Frost Temp : " + String(settings.FrostTemp));
    Serial.println("Saved EarlyStart : " + String(settings.EarlyStart));
    Serial.println("Saved ManualOverride : " + String(settings.ManualOverride));
    Serial.println("Saved ManOverrideTemp : " + String(settings.ManOverrideTemp));
    Serial.println("Saved Default Temp : " + String(settings.TargetTemp));
    dataFile.close();
    Serial.println("Settings saved...");
  }
}
//#########################################################################################
// David Bird and ...
void RecoverSettings() {
  String Entry;
  Serial.println("Reading settings...");
  File dataFile = LittleFS.open("/" + settings.DataFile);
  if (dataFile) {  // if the file is available, read it
    Serial.println("Recovering settings...");
    while (dataFile.available()) {
      for (byte dow = 0; dow < 7; dow++) {
        Serial.println("Day of week = " + String(dow));
        for (byte p = 0; p < NumOfEvents; p++) {
          Timer[dow].Temp[p] = dataFile.readStringUntil('\n');
          Timer[dow].Temp[p].trim();
          Timer[dow].Start[p] = dataFile.readStringUntil('\n');
          Timer[dow].Start[p].trim();
          Timer[dow].Stop[p] = dataFile.readStringUntil('\n');
          Timer[dow].Stop[p].trim();
          Serial.println("Period: " + String(p) + " " + Timer[dow].Temp[p] + " from: " + Timer[dow].Start[p] + " to: " + Timer[dow].Stop[p]);
        }
      }
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.Hysteresis = Entry.toFloat();
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.FrostTemp = Entry.toInt();
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.EarlyStart = Entry.toInt();
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.ManualOverride = Entry.toInt();
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.ManOverrideTemp = Entry.toInt();
      Entry = dataFile.readStringUntil('\n');
      Entry.trim();
      settings.TargetTemp = Entry.toInt();
      Serial.println("Recovered Hysteresis : " + String(settings.Hysteresis));
      Serial.println("Recovered Frost Temp : " + String(settings.FrostTemp));
      Serial.println("Recovered EarlyStart : " + String(settings.EarlyStart));
      Serial.println("Recovered ManualOverride : " + String(settings.ManualOverride));
      Serial.println("Recovered ManOverrideTemp : " + String(settings.ManOverrideTemp));
      Serial.println("Recovered Default Temp : " + String(settings.TargetTemp));
      dataFile.close();
      Serial.println("Settings recovered...");
      if (settings.ManualOverride==1) TargetTemp = settings.ManOverrideTemp;
      else TargetTemp = settings.TargetTemp;
    }
  }
}
//#########################################################################################
// David Bird 
void SetupDeviceName(const char* DeviceName) {
  if (MDNS.begin(DeviceName)) {  // The name that will identify your device on the network
    Serial.println("mDNS responder started");
    Serial.print("Device name: ");
    Serial.println(DeviceName);
    MDNS.addService("n8i-mlp", "tcp", 23);  // Add service
  } else
    Serial.println("Error setting up MDNS responder");
}
//#########################################################################################
void StartSensors() {
  if (!SIMULATE) {                                // If not sensor simulating, then start the real one
    dht.begin();
    dht.readTemperature();
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    dht.readHumidity();
    Serial.println(F("Humidity Sensor"));
    Serial.println(F("------------------------------------"));
    delay(1000);                                  // Wait for sensor to start
  }
}
//#########################################################################################
void ReadSensors() {            // We can add more sensors if needed
  if (SIMULATE) {
    Temperature = 20.2 + random(-15, 15) / 10.0;  // Generate a random temperature value between 18.7° and 21.7°
    Humidity = random(45, 55);                    // Generate a random humidity value between 45% and 55%
  } else {
    float t = dht.readTemperature();
    if (isnan(t)) {
      Serial.println(F("Error reading temperature!"));
    } else {
      Temperature = t;
      if (Temperature >= 50 || Temperature < -30) Temperature = LastTemperature;  // Check and correct any errorneous readings
      LastTemperature = Temperature;
      Serial.print(F("Temperature: "));
      Serial.print(Temperature);
      Serial.println(F("C"));
    }
    // Get humidity event and print its value.
    float h = dht.readHumidity();
    if (isnan(h)) {
      Serial.println(F("Error reading humidity!"));
    } else {
      Humidity = h;
      Serial.print(F("Humidity: "));
      Serial.print(Humidity);
      Serial.println(F("%"));
    }
  }
}
//#########################################################################################
// David Bird and ...
void AssignSensorReadingsToArray() {
  byte ptr, p;
  ptr = sensordataCounter;
  sensordata[ptr].Temperature = Temperature;
  sensordata[ptr].Humidity = Humidity;
  sensordata[ptr].RelayState = RelayState;
  ptr++;
  if (ptr >= SensorReadings) {
    p = 0;
    do {
      sensordata[p].Temperature = sensordata[p + 1].Temperature;
      sensordata[p].Humidity = sensordata[p + 1].Humidity;
      sensordata[p].RelayState = sensordata[p + 1].RelayState;
      p++;
    } while (p < SensorReadings);
    ptr = SensorReadings - 1;
    sensordata[SensorReadings - 1].Temperature = Temperature;
    sensordata[SensorReadings - 1].Humidity = Humidity;
    sensordata[SensorReadings - 1].RelayState = RelayState;
  }
  sensordataCounter = ptr;
}
//#########################################################################################
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
//#########################################################################################
void routes(String sitetitle, String version, String year) {
  // Set handler for '/'
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->redirect("/home");  // Go to home page
  });
  // Set handler for '/home'
  server.on("/home", HTTP_GET, [sitetitle, version, year](AsyncWebServerRequest* request) {
    request->send(200, "text/html", CreateHomePage(sitetitle, version, year));
  });
  // Set handler for '/graphs'
  server.on("/graphs", HTTP_GET, [sitetitle, version, year](AsyncWebServerRequest* request) {
    request->send(200, "text/html", CreateGraphsPage(sitetitle, version, year));
  });
  // Set handler for '/timer'
  server.on("/schedule", HTTP_GET, [sitetitle, version, year](AsyncWebServerRequest* request) {
    request->send(200, "text/html", CreateSchedulePage(sitetitle, version, year));
  });
  // Set handler for '/setup'
  server.on("/setup", HTTP_GET, [sitetitle, version, year](AsyncWebServerRequest* request) {
    request->send(200, "text/html", CreateSetupPage(sitetitle, version, year));
  });
  // Set handler for '/help'
  server.on("/help", HTTP_GET, [sitetitle, version, year](AsyncWebServerRequest* request) {
    request->send(200, "text/html", CreateHelpPage(sitetitle, version, year));
  });
  server.onNotFound(notFound);
  // Set handler for '/handletimer' inputs
  server.on("/doschedule", HTTP_POST, [](AsyncWebServerRequest* request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < NumOfEvents; p++) {
        Timer[dow].Temp[p] = request->arg(String(dow) + "." + String(p) + ".Temp");
        Timer[dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[dow].Stop[p] = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/home");  // Go back to home page
  });
  // Set handler for '/handlesetup' inputs
  server.on("/dosetup", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        if (p->name() == "defaulttemp") {
          String numArg = p->value();
          settings.TargetTemp = numArg.toFloat();
        }
        if (p->name() == "hysteresis") {
          String numArg = p->value();
          settings.Hysteresis = numArg.toFloat();
        }
        if (p->name() == "frosttemp") {
          String numArg = p->value();
          settings.FrostTemp = numArg.toFloat();
        }
        if (p->name() == "earlystart") {
          String numArg = p->value();
          settings.EarlyStart = numArg.toInt();
        }
        if (p->name() == "manualoverride") {
          String numArg = p->value();
          settings.ManualOverride = numArg.toInt();
        }
        if (p->name() == "manualoverridetemp") {
          String numArg = p->value();
          settings.ManOverrideTemp = numArg.toFloat();
        }
      }
    }
    SaveSettings();
    if (settings.ManualOverride==1) TargetTemp = settings.ManOverrideTemp;
    else TargetTemp = settings.TargetTemp;
    request->redirect("/home");  // Go back to home page
  });
  // Set handler for '/handletimer' inputs
  server.on("/doreset", HTTP_GET, [](AsyncWebServerRequest* request) {
    ResetWiFiAndSettings();
    request->redirect("/home");  // Go back to home page
    ESP.restart();
  });
}
//#########################################################################################
// David Bird 
boolean SetupTime() {
  configTime(0, 0, "time.nist.gov");  // (gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", settings.Timezone, 1);          // setenv()adds "TZ" variable to the environment, only used if set to 1, 0 means no change
  tzset();
  delay(200);
  bool TimeStatus = UpdateLocalTime();
  return TimeStatus;
}
//#########################################################################################
// David Bird 
boolean UpdateLocalTime() {
  struct tm timeinfo;
  time_t now;
  char time_output[30];
  while (!getLocalTime(&timeinfo, 15000)) {  // Wait for up to 15-sec for time to synchronise
    return false;
  }
  time(&now);
  settings.UnixTime = now;
  //See http://www.cplusplus.com/reference/ctime/strftime/
  strftime(time_output, sizeof(time_output), "%H:%M", &timeinfo);  // Creates: '14:05'
  settings.Time_str = time_output;
  strftime(time_output, sizeof(time_output), "%w", &timeinfo);  // Creates: '0' for Sun
  settings.DoW_str = time_output;
  return true;
}
//#########################################################################################
// David Bird 
String ConvertUnixTime(int unix_time) {
  time_t tm = unix_time;
  struct tm* now_tm = localtime(&tm);
  char output[40];
  strftime(output, sizeof(output), "%H:%M", now_tm);  // Returns 21:12
  return output;
}

