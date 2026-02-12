// ========================================
// TEMPTRON 607 A-C - Temperature Controller
// Menggunakan sensor suhu DHT11
// ========================================

#include <DHT.h>
#include <EEPROM.h>
// TEMPORARY: Disabled for LCD testing without ESP32 package
#include <HTTPClient.h>
#include <I2CKeyPad.h>
#include <Keypad.h>
#include <TM1637Display.h>
#include <Wire.h>

// ========================================
// WiFi Configuration - DISABLED FOR TESTING
// ========================================
#include "webpage.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

// TEMPORARY: WiFi disabled for LCD testing
const char *ssid = "Sejahtera";
const char *password = "presiden sekarang";
AsyncWebServer server(80);
const char *apiTelemetry = "https://malik.kukode.com/api/telemetry.php";
const char *apiConfig = "https://malik.kukode.com/api/config.php";
const char *apiManual = "https://malik.kukode.com/api/manual.php";

// ========================================
// Pin Definitions
// ========================================
#define CLK1 26
#define DIO1 25
#define CLK2 17
#define DIO2 16
#define DHTPIN 27 // Sesuai WIRING User (Pin 27)
#define DHTTYPE DHT11

// ========================================
// RELAY ACTIVE LOW/HIGH SETTING
// Ubah ke true jika relay module Anda Active LOW (umum pada relay module biru)
// ========================================
#define RELAY_ACTIVE_LOW                                                       \
  false // false = relay ON saat GPIO HIGH (relay module Anda)

// Helper macros for relay control
#if RELAY_ACTIVE_LOW
#define RELAY_ON LOW
#define RELAY_OFF HIGH
#else
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#endif

// ========================================
// Relay/LED pins untuk kontrol
// Urutan sesuai wiring: 18, 19, 23, 5, 13, 12, 14, 27, 2
// ========================================
int led_status1 = 18;  // Kipas 1 (aktif saat suhu +1°C dari setpoint) - Relay 1
int led_status2 = 19;  // Kipas 2 (aktif saat suhu +2°C dari setpoint) - Relay 2
int led_status3 = 23;  // Kipas 3 (aktif saat suhu +3°C dari setpoint) - Relay 3
int led_status4 = 5;   // Kipas 4 (aktif saat suhu +4°C dari setpoint) - Relay 4
int led_status5 = 13;  // Kipas 5 (aktif saat suhu +5°C dari setpoint) - Relay 5
int relay_cooler = 12; // Cooling (aktif saat suhu tinggi) - Relay 6
int relay_heater = 14; // Heater (aktif saat suhu rendah) - Relay 7
int led_status6 = 4; // Kipas 6 (dipindah ke Pin 4 karena Pin 27 dipakai Sensor)
int led_status6_extra = 2; // Kipas 6 tambahan - dikontrol bersama led_status6

// Helper function untuk mengontrol Kipas 6 (pin 27 dan pin 2 bersamaan)
void setFan6(int state) {
  digitalWrite(led_status6, state);
  digitalWrite(led_status6_extra, state);
}

// ========================================
// Global Variables
// ========================================
// Global Variables
// ========================================
float currentTemp = 0;
float currentHumidity = 0;
bool sensorValid = false;
String inputCode = "";
bool systemActive = true;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // Baca sensor setiap 2 detik
unsigned long lastPostTime = 0;
const unsigned long postInterval = 10000;
unsigned long lastConfigFetchTime = 0;
const unsigned long configFetchInterval = 30000;
unsigned long lastManualFetchTime = 0;
const unsigned long manualFetchInterval = 1000;

const int EEPROM_SIZE = 1024;
const uint16_t CONFIG_MAGIC = 0x607C; // Changed to invalidate old EEPROM layout

// Temperature Reduction Group (for functions 22-30)
struct TempReductionGroup {
  int days;        // Number of days for this group
  float reduction; // Temperature reduction in °C for this group
};

struct ConfigData {
  uint16_t magic;
  float setpointTemp;
  float heatTemp; // Differential BELOW setpoint for heater ON
  float coolTemp; // Absolute temperature for cooling system ON
  // Fan differentials (°C ABOVE setpoint for each fan group)
  float fan1_diff;
  float fan2_diff;
  float fan3_diff;
  float fan4_diff;
  float fan5_diff;
  int timerOn;  // Fan On Time (minutes) for minimum ventilation
  int timerOff; // Fan Off Time (minutes) for minimum ventilation
  float
      humiditySetpoint; // Humidity % that triggers next fan group / cooling off
  int coolOnTime;       // Cool ON time (seconds) for cycling
  int coolOffTime;      // Cool OFF time (seconds) for cycling
  float lowerAlarmDiff; // Low alarm differential BELOW setpoint
  float upperAlarmDiff; // High alarm differential ABOVE setpoint
  int alarmDelay;       // Alarm delay in seconds
  int displayBrightness;
  // Menu 17-21
  float waterClock;
  int feedMult;
  float dailyFeed;
  float totalFeed;
  float day1Temp;
  // Menu 22-30: Temperature Reduction Table (9 groups)
  TempReductionGroup reductionTable[9];
  // Menu 31-32
  int growDay;
  float resetTime;
  // Menu 38: Fan Mode (0=normal, 1=minimum ventilation rotation)
  int fanMode;
  // Menu 39: Alarm Type (0=disabled, 1=low only, 2=high only, 3=both)
  int alarmType;
  bool systemActive;
};

ConfigData config;

void saveConfig();
bool loadConfig();

// ========================================
// Menu Parameters - Sesuai Manual Temptron 607
// ========================================
// Func 01: Current Time (placeholder - needs RTC)
float currentTime_placeholder = 0;

// Func 02: Required Temperature (Setpoint)
float setpointTemp = 30.0; // Default 30°C (range 0-99)

// Func 03: Heat - Differential BELOW setpoint to activate heater
// Manual: "Heat set point is the temperature differential below the requested
//          room temperature that the heating system will turn on"
float heatTemp = 1.0; // Default 1.0°C differential

// Func 04-08: Fan 1-5 Differentials
// Manual: "Fan X set point is the temperature differential above the required
//          temperature at which time fan X will operate nonstop"
float fan1_diff = 1.0; // Default +1.0°C above setpoint
float fan2_diff = 2.0; // Default +2.0°C above setpoint
float fan3_diff = 3.0; // Default +3.0°C above setpoint
float fan4_diff = 4.0; // Default +4.0°C above setpoint
float fan5_diff = 5.0; // Default +5.0°C above setpoint

// Func 09: Fan On Time (minutes, 0 = disabled)
int timerOn = 1;  // Default 1 minute
int timerOff = 2; // Default 2 minutes
unsigned long timerStartTime = 0;
bool timerActive = false; // Internal: Current state of intermittent

// Func 10: Fan Off Time (see timerOff above)

// Func 11: Humidity Setpoint
// Manual: "If humidity rises to this level, bring next fan group into
// operation" Manual: "Also used for cooling - if humidity rises to this level,
// cooling turns off"
float humiditySetpoint = 60.0; // Default 60% (range 0-99)

// Func 12: Cool Temperature (absolute temperature to activate cooling)
float coolTemp = 32.0; // Default 32°C

// Func 13: Cool On Time (minutes:seconds for cooling cycle ON period)
int coolOnTime = 120; // Default 2 minutes (120 seconds)

// Func 14: Cool Off Time (minutes:seconds for cooling cycle OFF period)
int coolOffTime = 300; // Default 5 minutes (300 seconds)

// Func 15: Low Alarm - Differential BELOW setpoint
// Manual: "Low alarm set point is the temperature differential below the
//          requested room temperature"
float lowerAlarmDiff = 5.0; // Default 5.0°C below setpoint

// Func 16: High Alarm - Differential ABOVE setpoint
// Manual: "High alarm set point is the temperature differential above the
//          requested room temperature"
float upperAlarmDiff = 5.0; // Default 5.0°C above setpoint

// Alarm Delay (internal, can be set via hidden menu)
int alarmDelay = 5; // Default 5 seconds (0-99)
unsigned long alarmTriggerTime = 0;

// Display Brightness (Func 16 secondary or separate)
int displayBrightness = 15; // Default 15 (0x0f), range 0-15

// ========================================
// MENU TAMBAHAN (17-32)
// ========================================
// Func 17: Water Clock (liters consumed)
float waterClock = 0.0;

// Func 18: Feed Multiply (Kg per minute of auger motor)
int feedMult = 25; // Default 25 kg per minute

// Func 19: Daily Feed Consumption (display only, calculated)
float dailyFeed = 0.0;

// Func 20: Total Feed Consumption (display only, calculated)
float totalFeed = 0.0;

// Func 21: Temperature Day 1 (required temp for day 1 of flock)
float day1Temp = 32.0;

// Func 22-30: Automatic Temperature Reduction Table (9 groups)
// Each group has: days (how many days), reduction (°C to reduce over those
// days)
TempReductionGroup reductionTable[9] = {
    {7, 2.0}, // Group 1: 7 days, reduce 2.0°C
    {3, 1.5}, // Group 2: 3 days, reduce 1.5°C
    {1, 0.0}, // Group 3: placeholder
    {1, 0.0}, // Group 4: placeholder
    {1, 0.0}, // Group 5: placeholder
    {1, 0.0}, // Group 6: placeholder
    {1, 0.0}, // Group 7: placeholder
    {1, 0.0}, // Group 8: placeholder
    {1, 0.0}  // Group 9: placeholder
};

// Func 31: Grow Day (current day of flock)
int growDay = 1;

// Func 32: Reset Time
float resetTime = 0.0;

// ========================================
// Hidden Functions (33-39)
// ========================================
// Func 38: Fan Mode
// 0 = Normal (all enabled fans run when above their differential)
// 1 = Minimum ventilation rotation mode
int fanMode = 0;

// Func 39: Alarm Type
// 0 = Disabled, 1 = Low alarm only, 2 = High alarm only, 3 = Both
int alarmType = 3; // Default: both alarms active

// Alarm state
bool alarmTriggered = false;
unsigned long alarmTimerStart = 0; // Timer untuk delay alarm

// Cooling state - now supports cycling ON/OFF
bool coolingSystemActive = false; // Whether cooling system should be running
bool coolingCycleState =
    true; // Current state in cooling cycle (true=ON, false=OFF)
unsigned long coolingCycleTimer = 0; // Timer for cooling cycle

// ========================================
// Manual Control State
// ========================================
bool manualMode = false; // true = kontrol dari web override otomatis
int manual_fan1 = 0;
int manual_fan2 = 0;
int manual_fan3 = 0;
int manual_fan4 = 0;
int manual_fan5 = 0;
int manual_fan6 = 0;
int manual_heater7 = 0;
int manual_cooling = 0;

// ========================================
// Menu System Variables
// ========================================
enum MenuState {
  MENU_IDLE,   // Mode normal - tampilkan suhu
  MENU_BROWSE, // Mode browse menu
  MENU_EDIT    // Mode edit parameter
};

MenuState currentMenuState = MENU_IDLE;
int currentMenuNumber = 1; // Menu yang sedang dipilih (1-39)

// Helper: Calculate effective setpoint based on grow day and reduction table
float getEffectiveSetpoint() {
  if (growDay <= 1)
    return day1Temp;

  float effectiveTemp = day1Temp;
  int dayCount = 1;

  for (int g = 0; g < 9; g++) {
    int groupDays = reductionTable[g].days;
    float groupReduction = reductionTable[g].reduction;

    if (groupDays <= 0)
      continue;

    // Calculate daily reduction for this group
    float dailyReduction =
        (groupDays > 0) ? (groupReduction / (float)groupDays) : 0;

    for (int d = 0; d < groupDays; d++) {
      dayCount++;
      effectiveTemp -= dailyReduction;
      if (dayCount >= growDay)
        return effectiveTemp;
    }
  }

  return effectiveTemp;
}

// ========================================
// Objects Initialization
// ========================================
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display1(CLK1, DIO1); // Display untuk suhu aktual
TM1637Display display2(CLK2, DIO2); // Display untuk setpoint

const int i2caddress = 0x20;
I2CKeyPad kpd(i2caddress);
char keymap[] = "123A456B789C*0#D"; // Index 16 is \0 (NO_KEY), Index 17 (FAIL)

// ========================================
// Setup Function - Inisialisasi Sistem
// ========================================
void setup() {
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("TEMPTRON 607 A-C - Temperature Controller");
  Serial.println("========================================");

  // Inisialisasi I2C dan Keypad
  Wire.begin();
  if (kpd.begin()) {
    kpd.loadKeyMap(keymap);
  } else {
    Serial.println("Keypad not found at 0x20");
  }

  // Inisialisasi DHT Sensor
  dht.begin();
  EEPROM.begin(EEPROM_SIZE);
  loadConfig();

  // Setup pin relay dan LED sebagai OUTPUT
  pinMode(relay_heater, OUTPUT);
  pinMode(relay_cooler, OUTPUT);
  pinMode(led_status1, OUTPUT);
  pinMode(led_status2, OUTPUT);
  pinMode(led_status3, OUTPUT);
  pinMode(led_status4, OUTPUT);
  pinMode(led_status5, OUTPUT);
  pinMode(led_status6, OUTPUT);
  pinMode(led_status6_extra, OUTPUT); // Pin 2 tambahan untuk Kipas 6

  // Matikan semua relay dan LED saat start
  digitalWrite(relay_heater, RELAY_OFF);
  digitalWrite(relay_cooler, RELAY_OFF);
  digitalWrite(led_status1, RELAY_OFF);
  digitalWrite(led_status2, RELAY_OFF);
  digitalWrite(led_status3, RELAY_OFF);
  digitalWrite(led_status4, RELAY_OFF);
  digitalWrite(led_status5, RELAY_OFF);
  digitalWrite(led_status6, RELAY_OFF);
  digitalWrite(led_status6_extra, RELAY_OFF); // Pin 2 tambahan

  // Inisialisasi Display
  display1.setBrightness(displayBrightness);
  display2.setBrightness(displayBrightness);
  display1.showNumberDec((int)setpointTemp, false);
  display2.showNumberDec(0, false);

  Serial.println("Inisialisasi selesai!");
  Serial.print("Setpoint default: ");
  Serial.print(setpointTemp);
  Serial.println("°C (Range: 20-100°C)");
  Serial.println("Masukkan kode akses untuk memulai...");

  // ========================================
  // WiFi Setup - DISABLED FOR TESTING
  // ========================================
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", index_html);
    });

    // API: Status (Telemetry)
    // Returns: { "telemetry": { ... }, "config": { ... }, "state": { ... } }
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
      String json = "{";

      // Telemetry
      json += "\"telemetry\":{";
      json += "\"temp\":" + String(currentTemp, 1) + ",";
      json += "\"humidity\":" + String(currentHumidity, 1) + ",";
      json += "\"setpoint\":" + String(setpointTemp, 1) + ",";
      json += "\"device_id\":\"" + WiFi.macAddress() + "\"";
      json += "},";

      // Config - report computed alarm limits for dashboard
      json += "\"config\":{";
      json += "\"setpoint\":" + String(setpointTemp, 1) + ",";
      json += "\"heatDiff\":" + String(heatTemp, 1) + ",";
      json += "\"coolTemp\":" + String(coolTemp, 1) + ",";
      json +=
          "\"lowerLimit\":" + String(setpointTemp - lowerAlarmDiff, 1) + ",";
      json +=
          "\"upperLimit\":" + String(setpointTemp + upperAlarmDiff, 1) + ",";
      json += "\"lowerAlarmDiff\":" + String(lowerAlarmDiff, 1) + ",";
      json += "\"upperAlarmDiff\":" + String(upperAlarmDiff, 1) + ",";
      json += "\"humiditySetpoint\":" + String(humiditySetpoint, 1) + ",";
      json += "\"alarmType\":" + String(alarmType) + ",";
      json += "\"fan1_diff\":" + String(fan1_diff, 1) + ",";
      json += "\"fan2_diff\":" + String(fan2_diff, 1) + ",";
      json += "\"fan3_diff\":" + String(fan3_diff, 1) + ",";
      json += "\"fan4_diff\":" + String(fan4_diff, 1) + ",";
      json += "\"fan5_diff\":" + String(fan5_diff, 1);
      json += "},";

      // State (Visual feedback for LEDs)
      json += "\"state\":{";
      json += "\"manual\":" + String(manualMode ? 1 : 0) + ",";
      json += "\"fan1\":" + String(digitalRead(led_status1)) + ",";
      json += "\"fan2\":" + String(digitalRead(led_status2)) + ",";
      json += "\"fan3\":" + String(digitalRead(led_status3)) + ",";
      json += "\"fan4\":" + String(digitalRead(led_status4)) + ",";
      json += "\"fan5\":" + String(digitalRead(led_status5)) + ",";
      json += "\"fan6\":" + String(digitalRead(led_status6)) + ",";
      json += "\"heater7\":" + String(digitalRead(relay_heater)) + ",";
      json += "\"cooling\":" + String(digitalRead(relay_cooler));
      json += "}";

      json += "}";
      // Authentication for status? Optional, but good for security.
      // if(!request->authenticate("admin", "semangat22")) return
      // request->requestAuthentication();
      request->send(200, "application/json", json);
    });

    // API: Config (Update Setpoint)
    // Expects JSON: { "setpoint": 30.5 }

    // API: Config (Authenticated)
    server.on(
        "/config", HTTP_POST,
        [](AsyncWebServerRequest *request) {
          if (!request->authenticate("", "semangat22") &&
              !request->authenticate("admin", "semangat22"))
            return request->requestAuthentication();
          request->send(200);
        },
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
           size_t index, size_t total) {
          // Body parser logic same as before...
          String body = "";
          for (size_t i = 0; i < len; i++)
            body += (char)data[i];

          int idx = body.indexOf("\"setpoint\"");
          if (idx >= 0) {
            int start = body.indexOf(":", idx) + 1;
            float val = body.substring(start).toFloat();
            if (val >= 20 && val <= 100) {
              setpointTemp = val;
              saveConfig();
            }
          }
        });

    // API: Manual Control
    // Expects JSON: { "manual": 1, "fan1": 1, ... }
    server.on(
        "/manual", HTTP_POST,
        [](AsyncWebServerRequest *request) {
          if (!request->authenticate("", "semangat22") &&
              !request->authenticate("admin", "semangat22"))
            return request->requestAuthentication();
          request->send(200, "application/json", "{\"ok\":true}");
        },
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
           size_t index, size_t total) {
          String body = "";
          for (size_t i = 0; i < len; i++)
            body += (char)data[i];

          // Use existing helper or custom logic
          // We need to parse 'manual' first
          if (body.indexOf("\"manual\"") >= 0) {
            // Simple boolean/int check
            if (body.indexOf("\"manual\":1") > 0 ||
                body.indexOf("\"manual\": 1") > 0 ||
                body.indexOf("\"manual\":true") > 0) {
              manualMode = true;
            } else {
              manualMode = false;
            }
          }

          if (manualMode) {
            if (body.indexOf("\"fan1\":1") > 0)
              manual_fan1 = 1;
            else if (body.indexOf("\"fan1\":0") > 0)
              manual_fan1 = 0;
            if (body.indexOf("\"fan2\":1") > 0)
              manual_fan2 = 1;
            else if (body.indexOf("\"fan2\":0") > 0)
              manual_fan2 = 0;
            if (body.indexOf("\"fan3\":1") > 0)
              manual_fan3 = 1;
            else if (body.indexOf("\"fan3\":0") > 0)
              manual_fan3 = 0;
            if (body.indexOf("\"fan4\":1") > 0)
              manual_fan4 = 1;
            else if (body.indexOf("\"fan4\":0") > 0)
              manual_fan4 = 0;
            if (body.indexOf("\"fan5\":1") > 0)
              manual_fan5 = 1;
            else if (body.indexOf("\"fan5\":0") > 0)
              manual_fan5 = 0;
            if (body.indexOf("\"fan6\":1") > 0)
              manual_fan6 = 1;
            else if (body.indexOf("\"fan6\":0") > 0)
              manual_fan6 = 0;
            if (body.indexOf("\"heater7\":1") > 0)
              manual_heater7 = 1;
            else if (body.indexOf("\"heater7\":0") > 0)
              manual_heater7 = 0;
            if (body.indexOf("\"cooling\":1") > 0)
              manual_cooling = 1;
            else if (body.indexOf("\"cooling\":0") > 0)
              manual_cooling = 0;
          }
        });

    server.begin();
    Serial.println("Web Server started!");
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

// ========================================
// Helper Functions
// ========================================

// Validasi dan simpan input berdasarkan menu yang aktif
// Input format: For most values, enter the number directly.
// For reduction table (22-30): Enter as DDTT where DD=days, TT=reduction*10
// Example: "720" = 7 days, 2.0°C reduction
bool validateAndSaveInput(String code, int menuNum) {
  int codeNum = code.toInt();
  bool valid = false;

  switch (menuNum) {

  case 1: // Func 01: Current Time (placeholder - needs RTC)
    valid = true;
    break;

  case 2: // Func 02: Required Temperature (Setpoint) (0-99)
    if (codeNum >= 0 && codeNum <= 99) {
      setpointTemp = (float)codeNum;
      valid = true;
    }
    break;

  case 3: // Func 03: Heat Differential (0-99, differential BELOW setpoint)
    // Manual: "Heat set point is the temperature differential below the
    //          requested room temperature"
    if (codeNum >= 0 && codeNum <= 99) {
      heatTemp = (float)codeNum / 10.0; // Input 10 = 1.0°C
      valid = true;
    }
    break;

  case 4: // Func 04: Fan 1 Differential (0-99, differential ABOVE setpoint)
    if (codeNum >= 0 && codeNum <= 99) {
      fan1_diff = (float)codeNum / 10.0; // Input 10 = 1.0°C
      valid = true;
    }
    break;

  case 5: // Func 05: Fan 2 Differential
    if (codeNum >= 0 && codeNum <= 99) {
      fan2_diff = (float)codeNum / 10.0;
      valid = true;
    }
    break;

  case 6: // Func 06: Fan 3 Differential
    if (codeNum >= 0 && codeNum <= 99) {
      fan3_diff = (float)codeNum / 10.0;
      valid = true;
    }
    break;

  case 7: // Func 07: Fan 4 Differential
    if (codeNum >= 0 && codeNum <= 99) {
      fan4_diff = (float)codeNum / 10.0;
      valid = true;
    }
    break;

  case 8: // Func 08: Fan 5 Differential
    if (codeNum >= 0 && codeNum <= 99) {
      fan5_diff = (float)codeNum / 10.0;
      valid = true;
    }
    break;

  case 9: // Func 09: Fan On Time (0-99 minutes)
    if (codeNum >= 0 && codeNum <= 99) {
      timerOn = codeNum;
      valid = true;
    }
    break;

  case 10: // Func 10: Fan Off Time (0-99 minutes)
    if (codeNum >= 0 && codeNum <= 99) {
      timerOff = codeNum;
      valid = true;
    }
    break;

  case 11: // Func 11: Humidity Setpoint (0-99%)
    if (codeNum >= 0 && codeNum <= 99) {
      humiditySetpoint = (float)codeNum;
      valid = true;
    }
    break;

  case 12: // Func 12: Cool Temperature (0-99°C, absolute)
    if (codeNum >= 0 && codeNum <= 99) {
      coolTemp = (float)codeNum;
      valid = true;
    }
    break;

  case 13: // Func 13: Cool On Time (mm:ss format, input as MMSS)
    // Example: 200 = 02:00 (2 minutes), 130 = 01:30 (1 min 30 sec)
    {
      int minutes = codeNum / 100;
      int seconds = codeNum % 100;
      if (minutes >= 0 && minutes <= 99 && seconds >= 0 && seconds <= 59) {
        coolOnTime = minutes * 60 + seconds;
        valid = true;
      }
    }
    break;

  case 14: // Func 14: Cool Off Time (mm:ss format, input as MMSS)
  {
    int minutes = codeNum / 100;
    int seconds = codeNum % 100;
    if (minutes >= 0 && minutes <= 99 && seconds >= 0 && seconds <= 59) {
      coolOffTime = minutes * 60 + seconds;
      valid = true;
    }
  } break;

  case 15: // Func 15: Low Alarm Differential (below setpoint)
    if (codeNum >= 0 && codeNum <= 99) {
      lowerAlarmDiff = (float)codeNum / 10.0; // Input 50 = 5.0°C
      valid = true;
    }
    break;

  case 16: // Func 16: High Alarm Differential (above setpoint)
    if (codeNum >= 0 && codeNum <= 99) {
      upperAlarmDiff = (float)codeNum / 10.0; // Input 50 = 5.0°C
      valid = true;
    }
    break;

  case 17: // Func 17: Water Clock (liters)
    waterClock = (float)codeNum;
    valid = true;
    break;

  case 18: // Func 18: Feed Multiply (Kg per minute)
    if (codeNum >= 0 && codeNum <= 999) {
      feedMult = codeNum;
      valid = true;
    }
    break;

  case 19: // Func 19: Daily Feed (display/reset)
    dailyFeed = (float)codeNum;
    valid = true;
    break;

  case 20: // Func 20: Total Feed (display/reset)
    totalFeed = (float)codeNum;
    valid = true;
    break;

  case 21: // Func 21: Temperature Day 1
    if (codeNum >= 0 && codeNum <= 99) {
      day1Temp = (float)codeNum;
      valid = true;
    }
    break;

  // Func 22-30: Temperature Reduction Table Groups 1-9
  // Input format: DDTT where DD = days (1-9), TT = reduction * 10
  // Example: 720 = 7 days, 2.0°C; 315 = 3 days, 1.5°C
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30: {
    int groupIdx = menuNum - 22; // 0-8
    if (code.length() >= 2) {
      // Parse: first digit(s) = days, last 2 digits = reduction * 10
      int days, reductionX10;
      if (code.length() <= 2) {
        days = 1;
        reductionX10 = codeNum;
      } else {
        // Last 2 chars = reduction * 10, leading chars = days
        String daysStr = code.substring(0, code.length() - 2);
        String reducStr = code.substring(code.length() - 2);
        days = daysStr.toInt();
        reductionX10 = reducStr.toInt();
      }
      if (days >= 1 && days <= 9 && reductionX10 >= 0 && reductionX10 <= 99) {
        reductionTable[groupIdx].days = days;
        reductionTable[groupIdx].reduction = (float)reductionX10 / 10.0;
        valid = true;
      }
    }
    break;
  }

  case 31: // Func 31: Grow Day
    if (codeNum >= 1 && codeNum <= 999) {
      growDay = codeNum;
      // Update setpoint based on grow day and reduction table
      setpointTemp = getEffectiveSetpoint();
      valid = true;
    }
    break;

  case 32: // Func 32: Reset Time
    resetTime = (float)codeNum;
    valid = true;
    break;

  case 38: // Func 38: Fan Mode (0=normal, 1=min vent rotation)
    if (codeNum >= 0 && codeNum <= 1) {
      fanMode = codeNum;
      valid = true;
    }
    break;

  case 39: // Func 39: Alarm Type (0=off, 1=low, 2=high, 3=both)
    if (codeNum >= 0 && codeNum <= 3) {
      alarmType = codeNum;
      valid = true;
    }
    break;
  }

  if (valid) {
    saveConfig();
  }
  return valid;
}

void postTelemetry(float temp, float humidity, float setpoint) {
  if (WiFi.status() != WL_CONNECTED)
    return;
  HTTPClient http;
  http.begin(apiTelemetry);
  http.addHeader("Content-Type", "application/json");
  String mac = WiFi.macAddress();
  String payload = String("{\"device_id\":\"") + mac +
                   "\",\"temp\":" + String(temp, 1) +
                   ",\"humidity\":" + String(humidity, 1) +
                   ",\"setpoint\":" + String(setpoint, 1) + "}";
  http.POST(payload);
  http.end();
}

void fetchConfig() {
  if (WiFi.status() != WL_CONNECTED)
    return;
  HTTPClient http;
  http.begin(apiConfig);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    int idx = body.indexOf("\"setpoint\"");
    if (idx >= 0) {
      int colon = body.indexOf(':', idx);
      if (colon >= 0) {
        int j = colon + 1;
        while (j < (int)body.length() && body[j] == ' ')
          j++;
        int k = j;
        while (k < (int)body.length()) {
          char c = body[k];
          if ((c >= '0' && c <= '9') || c == '.' || c == '-') {
            k++;
          } else {
            break;
          }
        }
        if (k > j) {
          float val = body.substring(j, k).toFloat();
          if (val >= 20 && val <= 100) {
            setpointTemp = val;
          }
        }
      }
    }
  }
  http.end();
}

int parseJsonInt(String body, const char *key, int defaultVal) {
  int idx = body.indexOf(key);
  if (idx < 0)
    return defaultVal;
  int colon = body.indexOf(':', idx);
  if (colon < 0)
    return defaultVal;
  int j = colon + 1;
  while (j < (int)body.length() && (body[j] == ' ' || body[j] == '\t'))
    j++;
  if (j >= (int)body.length())
    return defaultVal;
  char c = body[j];
  if (c == '1')
    return 1;
  if (c == '0')
    return 0;
  return defaultVal;
}

void fetchManualControl() {
  if (WiFi.status() != WL_CONNECTED)
    return;
  HTTPClient http;
  http.begin(apiManual);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    bool prevManual = manualMode;
    int m = parseJsonInt(body, "\"manual\"", manualMode ? 1 : 0);
    manualMode = (m == 1);
    if (manualMode != prevManual) {
      Serial.print("Manual mode changed to: ");
      Serial.println(manualMode ? "ON" : "OFF");
    }

    manual_fan1 = parseJsonInt(body, "\"fan1\"", manual_fan1);
    manual_fan2 = parseJsonInt(body, "\"fan2\"", manual_fan2);
    manual_fan3 = parseJsonInt(body, "\"fan3\"", manual_fan3);
    manual_fan4 = parseJsonInt(body, "\"fan4\"", manual_fan4);
    manual_fan5 = parseJsonInt(body, "\"fan5\"", manual_fan5);
    manual_fan6 = parseJsonInt(body, "\"fan6\"", manual_fan6);
    manual_heater7 = parseJsonInt(body, "\"heater7\"", manual_heater7);
    manual_cooling = parseJsonInt(body, "\"cooling\"", manual_cooling);
  }
  http.end();
}

void saveConfig() {
  config.magic = CONFIG_MAGIC;
  config.setpointTemp = setpointTemp;
  config.heatTemp = heatTemp;
  config.coolTemp = coolTemp;
  config.fan1_diff = fan1_diff;
  config.fan2_diff = fan2_diff;
  config.fan3_diff = fan3_diff;
  config.fan4_diff = fan4_diff;
  config.fan5_diff = fan5_diff;
  config.timerOn = timerOn;
  config.timerOff = timerOff;
  config.humiditySetpoint = humiditySetpoint;
  config.coolOnTime = coolOnTime;
  config.coolOffTime = coolOffTime;
  config.lowerAlarmDiff = lowerAlarmDiff;
  config.upperAlarmDiff = upperAlarmDiff;
  config.alarmDelay = alarmDelay;
  config.displayBrightness = displayBrightness;
  config.waterClock = waterClock;
  config.feedMult = feedMult;
  config.dailyFeed = dailyFeed;
  config.totalFeed = totalFeed;
  config.day1Temp = day1Temp;
  // Save reduction table
  for (int i = 0; i < 9; i++) {
    config.reductionTable[i] = reductionTable[i];
  }
  config.growDay = growDay;
  config.resetTime = resetTime;
  config.fanMode = fanMode;
  config.alarmType = alarmType;
  config.systemActive = systemActive;
  EEPROM.put(0, config);
  EEPROM.commit();
}

bool loadConfig() {
  EEPROM.get(0, config);
  if (config.magic == CONFIG_MAGIC) {
    setpointTemp = config.setpointTemp;
    heatTemp = config.heatTemp;
    coolTemp = config.coolTemp;
    fan1_diff = config.fan1_diff;
    fan2_diff = config.fan2_diff;
    fan3_diff = config.fan3_diff;
    fan4_diff = config.fan4_diff;
    fan5_diff = config.fan5_diff;
    timerOn = config.timerOn;
    timerOff = config.timerOff;
    humiditySetpoint = config.humiditySetpoint;
    coolOnTime = config.coolOnTime;
    coolOffTime = config.coolOffTime;
    lowerAlarmDiff = config.lowerAlarmDiff;
    upperAlarmDiff = config.upperAlarmDiff;
    alarmDelay = config.alarmDelay;
    displayBrightness = config.displayBrightness;
    waterClock = config.waterClock;
    feedMult = config.feedMult;
    dailyFeed = config.dailyFeed;
    totalFeed = config.totalFeed;
    day1Temp = config.day1Temp;
    // Load reduction table
    for (int i = 0; i < 9; i++) {
      reductionTable[i] = config.reductionTable[i];
    }
    growDay = config.growDay;
    resetTime = config.resetTime;
    fanMode = config.fanMode;
    alarmType = config.alarmType;
    systemActive = config.systemActive;
    display1.setBrightness(displayBrightness);
    display2.setBrightness(displayBrightness);
    return true;
  } else {
    saveConfig(); // Save defaults if magic number mismatch
    return false;
  }
}

// Cek alarm suhu dengan differential dari setpoint (Func 15, 16, 39)
// Manual: Alarm differentials are relative to the requested temperature
// (setpoint)
void checkAlarm() {
  // Check alarm type - if disabled, do nothing
  if (alarmType == 0) {
    if (alarmTriggered) {
      alarmTriggered = false;
      Serial.println("Alarm disabled by alarm type setting");
    }
    return;
  }

  // Calculate alarm boundaries as differentials from setpoint
  float lowAlarmTemp = setpointTemp - lowerAlarmDiff;
  float highAlarmTemp = setpointTemp + upperAlarmDiff;

  // Determine if alarm condition exists based on alarm type
  bool alarmCondition = false;
  if (alarmType == 1) {
    // Low alarm only
    alarmCondition = (currentTemp < lowAlarmTemp);
  } else if (alarmType == 2) {
    // High alarm only
    alarmCondition = (currentTemp > highAlarmTemp);
  } else if (alarmType == 3) {
    // Both alarms
    alarmCondition =
        (currentTemp > highAlarmTemp || currentTemp < lowAlarmTemp);
  }

  if (alarmCondition) {
    // Alarm condition detected
    if (!alarmTriggered) {
      // Check delay timer
      if (alarmTimerStart == 0) {
        alarmTimerStart = millis();
      } else {
        if (millis() - alarmTimerStart >= (unsigned long)alarmDelay * 1000UL) {
          // Delay fulfilled, TRIGGER ALARM
          alarmTriggered = true;
          // All LEDs ON during alarm
          digitalWrite(led_status1, RELAY_ON);
          digitalWrite(led_status2, RELAY_ON);
          digitalWrite(led_status3, RELAY_ON);
          digitalWrite(led_status4, RELAY_ON);
          digitalWrite(led_status5, RELAY_ON);
          setFan6(RELAY_ON);
          Serial.print("ALARM: Temp ");
          Serial.print(currentTemp, 1);
          Serial.print("°C outside bounds [");
          Serial.print(lowAlarmTemp, 1);
          Serial.print(" - ");
          Serial.print(highAlarmTemp, 1);
          Serial.println("°C]");
        }
      }
    }
  } else {
    // Temperature normal
    alarmTimerStart = 0; // Reset timer
    if (alarmTriggered) {
      alarmTriggered = false;
      // All LEDs OFF when alarm clears
      digitalWrite(led_status1, RELAY_OFF);
      digitalWrite(led_status2, RELAY_OFF);
      digitalWrite(led_status3, RELAY_OFF);
      digitalWrite(led_status4, RELAY_OFF);
      digitalWrite(led_status5, RELAY_OFF);
      setFan6(RELAY_OFF);
      Serial.println("Alarm cleared: Temperature back to normal");
    }
  }
}

// Terapkan kontrol manual ke relay dan LED
void applyManualControl() {
  // Heater & Cooling
  digitalWrite(relay_heater, manual_heater7 ? RELAY_ON : RELAY_OFF);
  digitalWrite(relay_cooler, manual_cooling ? RELAY_ON : RELAY_OFF);

  // Kipas 1-6
  digitalWrite(led_status1, manual_fan1 ? RELAY_ON : RELAY_OFF);
  digitalWrite(led_status2, manual_fan2 ? RELAY_ON : RELAY_OFF);
  digitalWrite(led_status3, manual_fan3 ? RELAY_ON : RELAY_OFF);
  digitalWrite(led_status4, manual_fan4 ? RELAY_ON : RELAY_OFF);
  digitalWrite(led_status5, manual_fan5 ? RELAY_ON : RELAY_OFF);
  setFan6(manual_fan6 ? RELAY_ON : RELAY_OFF);

  Serial.print("MANUAL CONTROL -> Heater:");
  Serial.print(manual_heater7);
  Serial.print(" Cooling:");
  Serial.print(manual_cooling);
  Serial.print(" | F1:");
  Serial.print(manual_fan1);
  Serial.print(" F2:");
  Serial.print(manual_fan2);
  Serial.print(" F3:");
  Serial.print(manual_fan3);
  Serial.print(" F4:");
  Serial.print(manual_fan4);
  Serial.print(" F5:");
  Serial.print(manual_fan5);
  Serial.print(" F6:");
  Serial.println(manual_fan6);
}

// Cooling System Control (Func 12 + 13 + 14)
// Manual: When temp >= coolTemp, cooling system cycles ON/OFF
//         Cool On Time (Func 13) = ON period of cycle
//         Cool Off Time (Func 14) = OFF period of cycle
// Humidity integration: If humidity >= humiditySetpoint, cooling turns OFF
void checkCooling() {
  // Check if cooling should be active (temp >= cool temp)
  if (currentTemp >= coolTemp) {
    if (!coolingSystemActive) {
      // Activate cooling system, start with ON state
      coolingSystemActive = true;
      coolingCycleState = true;
      coolingCycleTimer = millis();
      Serial.println("Cooling system ACTIVATED");
    }
  } else {
    // Temp dropped below coolTemp, deactivate cooling entirely
    if (coolingSystemActive) {
      coolingSystemActive = false;
      coolingCycleState = false;
      coolingCycleTimer = 0;
      Serial.println("Cooling system DEACTIVATED (temp below coolTemp)");
    }
  }

  // Humidity override: if humidity >= humiditySetpoint, force cooling OFF
  // Manual: "If the cooling system is running and the humidity reading rises
  //          to this level, the cooling system will turn off"
  if (coolingSystemActive && currentHumidity >= humiditySetpoint &&
      humiditySetpoint > 0) {
    coolingCycleState = false;
    Serial.println("Cooling OFF: Humidity exceeded setpoint");
    return;
  }

  // Handle cycling ON/OFF when cooling system is active
  if (coolingSystemActive && coolOnTime > 0 && coolOffTime > 0) {
    unsigned long elapsed = millis() - coolingCycleTimer;

    if (coolingCycleState) {
      // Currently ON, check if ON time has elapsed
      if (elapsed >= (unsigned long)coolOnTime * 1000UL) {
        coolingCycleState = false;
        coolingCycleTimer = millis();
        Serial.println("Cooling cycle: Switch to OFF");
      }
    } else {
      // Currently OFF, check if OFF time has elapsed
      if (elapsed >= (unsigned long)coolOffTime * 1000UL) {
        coolingCycleState = true;
        coolingCycleTimer = millis();
        Serial.println("Cooling cycle: Switch to ON");
      }
    }
  } else if (coolingSystemActive) {
    // If no cycling configured, just keep ON
    coolingCycleState = true;
  }
}

// Logic Intermittent (Timer ON/OFF) untuk Kipas 1
void updateIntermittent() {
  // Jika fitur disabled (salah satu 0), paksa OFF
  if (timerOn == 0 || timerOff == 0) {
    timerActive = false;
    return;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - timerStartTime;

  if (timerActive) {
    // Sedang ON, cek durasi ON (menit -> ms)
    if (elapsed >= (unsigned long)timerOn * 60000UL) {
      timerActive = false;
      timerStartTime = now;
      Serial.println("Intermittent Fan 1: Switch to OFF");
    }
  } else {
    // Sedang OFF, cek durasi OFF
    if (elapsed >= (unsigned long)timerOff * 60000UL) {
      timerActive = true;
      timerStartTime = now;
      Serial.println("Intermittent Fan 1: Switch to ON");
    }
  }
}

// Temperature Control - Using configurable differentials per manual
// Fan differentials are relative to setpoint (Func 04-08)
// Heat differential is relative to setpoint (Func 03)
// Humidity integration: If humidity >= humiditySetpoint, activate next fan
// group
void temperatureControl() {
  updateIntermittent(); // Update timer state for minimum ventilation
  checkCooling();       // Update cooling state (Func 12 + 13 + 14)

  float diff = currentTemp - setpointTemp;

  // ========================================
  // FAN CONTROL - Using configurable differentials (Func 04-08)
  // Each fan activates when diff >= its configured differential
  // Fan 6 = extra tier (fan5_diff + 1.0°C)
  // ========================================

  // Reset all fans to OFF, then activate based on differentials
  bool f1 = false;
  bool f2 = false;
  bool f3 = false;
  bool f4 = false;
  bool f5 = false;
  bool f6 = false;

  // Fan 6: Extra tier at fan5_diff + 1.0°C (no separate menu,
  // hardware-specific)
  float fan6_threshold = fan5_diff + 1.0;

  // Activate each fan if diff exceeds its differential (0 = disabled)
  if (fan1_diff > 0 && diff >= fan1_diff)
    f1 = true;
  if (fan2_diff > 0 && diff >= fan2_diff)
    f2 = true;
  if (fan3_diff > 0 && diff >= fan3_diff)
    f3 = true;
  if (fan4_diff > 0 && diff >= fan4_diff)
    f4 = true;
  if (fan5_diff > 0 && diff >= fan5_diff)
    f5 = true;
  if (diff >= fan6_threshold)
    f6 = true;

  // ========================================
  // HUMIDITY INTEGRATION (Func 11)
  // Manual: "If the humidity reading in the house rises to this level,
  //          the unit will automatically bring into operation the next fan
  //          group"
  // ========================================
  if (humiditySetpoint > 0 && currentHumidity >= humiditySetpoint) {
    // Activate the next inactive fan group
    if (!f1) {
      f1 = true;
    } else if (!f2) {
      f2 = true;
    } else if (!f3) {
      f3 = true;
    } else if (!f4) {
      f4 = true;
    } else if (!f5) {
      f5 = true;
    } else if (!f6) {
      f6 = true;
    }
    Serial.println("Humidity override: Next fan group activated");
  }

  // If diff < lowest configured fan differential, use intermittent (minimum
  // ventilation)
  if (diff < fan1_diff || fan1_diff == 0) {
    // Below minimum fan threshold - use intermittent timer for minimum
    // ventilation
    if (timerOn > 0 && timerOff > 0 && timerActive) {
      f1 = true; // Minimum ventilation fan
    }
    Serial.println("Status: MINIMUM VENTILATION / INTERMITTENT");
  } else {
    // Log active fan level
    int level = (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6;
    Serial.print("Status: FANS LEVEL ");
    Serial.println(level);
  }

  // ========================================
  // HEATING CONTROL (Func 03)
  // Manual: "Heat set point is the temperature differential BELOW the
  //          requested room temperature that the heating system will turn on"
  // ========================================
  bool heaterState = false;
  if (heatTemp > 0 && currentTemp <= (setpointTemp - heatTemp)) {
    heaterState = true;
    Serial.print("Status: HEATER ON (temp ");
    Serial.print(currentTemp, 1);
    Serial.print(" <= setpoint ");
    Serial.print(setpointTemp, 1);
    Serial.print(" - heat_diff ");
    Serial.print(heatTemp, 1);
    Serial.print(" = ");
    Serial.print(setpointTemp - heatTemp, 1);
    Serial.println(")");
  }

  // ========================================
  // APPLY TO RELAY OUTPUTS
  // Fan differential of 0 = disabled (fan won't activate)
  // ========================================

  // Fan 1 (Func 04)
  if (fan1_diff > 0)
    digitalWrite(led_status1, f1 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status1, RELAY_OFF);

  // Fan 2 (Func 05)
  if (fan2_diff > 0)
    digitalWrite(led_status2, f2 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status2, RELAY_OFF);

  // Fan 3 (Func 06)
  if (fan3_diff > 0)
    digitalWrite(led_status3, f3 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status3, RELAY_OFF);

  // Fan 4 (Func 07)
  if (fan4_diff > 0)
    digitalWrite(led_status4, f4 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status4, RELAY_OFF);

  // Fan 5 (Func 08)
  if (fan5_diff > 0)
    digitalWrite(led_status5, f5 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status5, RELAY_OFF);

  // Fan 6 (Hardware extra, always enabled if threshold met)
  setFan6(f6 ? RELAY_ON : RELAY_OFF);

  // Heater (Func 03)
  digitalWrite(relay_heater, heaterState ? RELAY_ON : RELAY_OFF);

  // Cooler (Func 12 + Cycling from Func 13/14)
  // Uses coolingCycleState which accounts for cycling and humidity override
  digitalWrite(relay_cooler, coolingCycleState ? RELAY_ON : RELAY_OFF);
  if (coolingCycleState)
    Serial.println("Status: COOLER ON");
}

// Tampilkan menu di display (Authentic Version)
void showMenu() {
  // Display2 (Small/Right) displays the Function Number
  // Format: 0X (leading zero)
  display2.showNumberDecEx(currentMenuNumber, 0, true, 2,
                           0); // Show 2 digits, leading zero, position 0

  // Display1 (Main/Left) displays the Value
  float valToShow = 0;

  switch (currentMenuNumber) {
  case 1:          // Func 01: Clock
    valToShow = 0; // Placeholder - needs RTC
    break;
  case 2: // Func 02: Required Temp (setpoint)
    valToShow = setpointTemp;
    break;
  case 3:                      // Func 03: Heat Differential (show as x10)
    valToShow = heatTemp * 10; // Display 10 = 1.0°C
    break;
  case 4: // Func 04: Fan 1 Differential (show as x10)
    valToShow = fan1_diff * 10;
    break;
  case 5: // Func 05: Fan 2 Differential
    valToShow = fan2_diff * 10;
    break;
  case 6: // Func 06: Fan 3 Differential
    valToShow = fan3_diff * 10;
    break;
  case 7: // Func 07: Fan 4 Differential
    valToShow = fan4_diff * 10;
    break;
  case 8: // Func 08: Fan 5 Differential
    valToShow = fan5_diff * 10;
    break;
  case 9: // Func 09: Fan On Time (minutes)
    valToShow = timerOn;
    break;
  case 10: // Func 10: Fan Off Time (minutes)
    valToShow = timerOff;
    break;
  case 11: // Func 11: Humidity Setpoint (%)
    valToShow = humiditySetpoint;
    break;
  case 12: // Func 12: Cool Temperature (absolute)
    valToShow = coolTemp;
    break;
  case 13: // Func 13: Cool On Time (show as MM:SS encoded)
    valToShow = (coolOnTime / 60) * 100 + (coolOnTime % 60); // e.g., 200 = 2:00
    break;
  case 14: // Func 14: Cool Off Time (show as MM:SS encoded)
    valToShow = (coolOffTime / 60) * 100 + (coolOffTime % 60);
    break;
  case 15: // Func 15: Low Alarm Differential (show as x10)
    valToShow = lowerAlarmDiff * 10;
    break;
  case 16: // Func 16: High Alarm Differential (show as x10)
    valToShow = upperAlarmDiff * 10;
    break;
  case 17: // Func 17: Water Clock
    valToShow = waterClock;
    break;
  case 18: // Func 18: Feed Multiply
    valToShow = feedMult;
    break;
  case 19: // Func 19: Daily Feed
    valToShow = dailyFeed;
    break;
  case 20: // Func 20: Total Feed
    valToShow = totalFeed;
    break;
  case 21: // Func 21: Temperature Day 1
    valToShow = day1Temp;
    break;
  // Func 22-30: Temperature Reduction Table
  // Display as encoded: Days * 100 + Reduction * 10
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30: {
    int idx = currentMenuNumber - 22;
    valToShow = reductionTable[idx].days * 100 +
                (int)(reductionTable[idx].reduction * 10);
    break;
  }
  case 31: // Func 31: Grow Day
    valToShow = growDay;
    break;
  case 32: // Func 32: Reset Time
    valToShow = resetTime;
    break;
  case 38: // Func 38: Fan Mode
    valToShow = fanMode;
    break;
  case 39: // Func 39: Alarm Type
    valToShow = alarmType;
    break;
  }

  // If we are inputting a code (Edit Mode), show that instead of stored value
  if (currentMenuState == MENU_EDIT && inputCode.length() > 0) {
    display1.showNumberDec(inputCode.toInt(), false);
  }
  // Otherwise show the stored value
  else {
    display1.showNumberDec((int)valToShow, false);
  }
}

// Update display berdasarkan state
void updateDisplay() {
  if (currentMenuState == MENU_IDLE) {
    // Mode IDLE (Function 0)
    // Display 1 (Main): Show Current Temperature
    // Display 1 (Main): Show Current Temperature
    if (sensorValid) {
      display1.showNumberDec((int)currentTemp, false);
    } else {
      // Show "Err" if sensor invalid
      // Segments for "Err ": E, r, r, space
      const uint8_t SEG_ERR[] = {
          0x79, // E
          0x50, // r
          0x50, // r
          0x00  // space
      };
      display1.setSegments(SEG_ERR);
    }

    // Display 2 (Small): Show Current Humidity in IDLE
    if (sensorValid) {
      display2.showNumberDec((int)currentHumidity, false);
    } else {
      display2.clear();
    }
  } else {
    // Mode BROWSE or EDIT
    showMenu();
  }
}

// ========================================
// Main Loop - Mengikuti Flowchart
// ========================================
void loop() {
  // Ambil input dari keyboard
  char key = kpd.getChar();

  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);

    // ========================================
    // LOGIKA KEYPAD (Temptron Style)
    // Mapping:
    // A = DATA (Next Function)
    // B = PROG (Edit / Program)
    // C = CLEAR (Cancel / Reset Input)
    // D = ENTER (Confirm Value)
    // ========================================

    // 1. DATA KEY (Tombol A) - Navigate to next function
    if (key == 'A') {
      if (currentMenuState == MENU_EDIT) {
        // If editing, DATA cancels edit (matches original behavior)
        currentMenuState = MENU_BROWSE;
        inputCode = "";
        updateDisplay();
      } else {
        // Move to next menu function
        currentMenuState = MENU_BROWSE;
        currentMenuNumber++;
        // Skip undefined functions (33-37 are not defined)
        if (currentMenuNumber > 32 && currentMenuNumber < 38)
          currentMenuNumber = 38;
        if (currentMenuNumber > 39)
          currentMenuNumber = 1;
        Serial.print("Function: ");
        Serial.println(currentMenuNumber);
        showMenu();
      }
    }

    // 2. PROG KEY (Tombol B) - Masuk/Keluar Mode Edit
    else if (key == 'B') {
      if (currentMenuState == MENU_EDIT) {
        // Keluar mode edit tanpa simpan
        currentMenuState = MENU_BROWSE;
        inputCode = "";
        updateDisplay();
      } else {
        // Masuk mode edit
        currentMenuState = MENU_EDIT;
        inputCode = "";
        Serial.println("Mode EDIT Aktif");
        // Tampilkan indikator edit (misal kedip atau kosong dulu)
        display1.showNumberDec(0, true); // 00
      }
    }

    // 3. ENTER KEY (Tombol D) - Simpan Nilai / Konfirmasi
    else if (key == 'D') {
      if (currentMenuState == MENU_EDIT) {
        if (inputCode.length() > 0) {
          bool valid = validateAndSaveInput(inputCode, currentMenuNumber);
          if (valid) {
            Serial.println("Data Tersimpan!");
            // Sukses flash?
            for (int i = 0; i < 3; i++) {
              display1.setBrightness(0);
              delay(100);
              display1.setBrightness(displayBrightness);
              delay(100);
            }
          } else {
            Serial.println("Data Invalid!");
          }
          // Kembali ke mode browse setelah save
          currentMenuState = MENU_BROWSE;
          inputCode = "";
          showMenu();
        }
      } else {
        // In browse/idle, ENTER can be used for FUNC jump shortcut
        if (inputCode.length() > 0 && currentMenuState == MENU_BROWSE) {
          // Jump to menu number (supports 1-39)
          int target = inputCode.toInt();
          if ((target >= 1 && target <= 32) || target == 38 || target == 39) {
            currentMenuNumber = target;
            showMenu();
          }
          inputCode = "";
        }
      }
    }

    // 4. CLEAR/CANCEL (Tombol C)
    else if (key == 'C') {
      inputCode = "";
      if (currentMenuState == MENU_EDIT) {
        display1.showNumberDec(0, false);
      } else {
        currentMenuState = MENU_IDLE; // Kembali ke tampilan suhu
        updateDisplay();
      }
    }

    // 5. NUMERIC KEYS (0-9)
    else if (key >= '0' && key <= '9') {
      if (currentMenuState == MENU_EDIT) {
        // Input setting value
        if (inputCode.length() < 4) { // Max 4 digit
          inputCode += key;
          display1.showNumberDec(inputCode.toInt(), false);
        }
      } else {
        // Mode Idle/Browse: Press '0' to start FUNC jump sequence
        // Temptron manual: Press 0, display shows "FUNC", then enter number

        if (key == '0' && inputCode.length() == 0) {
          // Start FUNC jump sequence
          inputCode += key;
          currentMenuState = MENU_BROWSE;
          // Show "Fn" indicator on display
          const uint8_t SEG_FN[] = {
              0x71, // F
              0x54, // n
              0x00, // space
              0x00  // space
          };
          display1.setSegments(SEG_FN);
          display2.clear();
        } else if (inputCode.length() > 0) {
          inputCode += key;
          // Display the entered number
          display1.showNumberDec(inputCode.toInt(), false);

          // Auto-jump when we have enough digits (2 digits for most menus)
          if (inputCode.length() >= 2) {
            int target = inputCode.toInt();
            if ((target >= 1 && target <= 32) || target == 38 || target == 39) {
              currentMenuNumber = target;
              currentMenuState = MENU_BROWSE;
              inputCode = "";
              showMenu();
            } else {
              // Invalid menu, reset
              inputCode = "";
              updateDisplay();
            }
          }
        }
      }
    }
  }

  // ========================================
  // Baca Sensor & Update Display
  // ========================================
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();

    currentTemp = dht.readTemperature();
    currentHumidity = dht.readHumidity();

    // Cek apakah pembacaan sensor valid
    if (isnan(currentTemp) || isnan(currentHumidity)) {
      Serial.println("Error: Gagal membaca sensor DHT!");
      sensorValid = false;
    } else {
      sensorValid = true;
    }

    Serial.print("Temp: ");
    Serial.print(currentTemp);
    Serial.print("°C | Humidity: ");
    Serial.print(currentHumidity);
    Serial.print("% | Setpoint: ");
    Serial.print(setpointTemp);
    Serial.print("°C | Alarm: [");
    Serial.print(setpointTemp - lowerAlarmDiff);
    Serial.print(" - ");
    Serial.print(setpointTemp + upperAlarmDiff);
    Serial.println("°C]");

    // Update display
    updateDisplay();

    // Kontrol: Prioritas Manual Mode, lalu System Active
    if (manualMode) {
      applyManualControl();
    } else if (systemActive && sensorValid) {
      // Hanya jalankan kontrol otomatis jika sensor valid
      temperatureControl();
      checkAlarm();
    } else if (systemActive && !sensorValid) {
      // Safety: Matikan semua jika sensor error dan auto mode
      // Opsional: Nyalakan Fan minimal untuk safety?
      Serial.println("Safety Mode: Sensor Error - Menunggu perbaikan");
    }
  }
  unsigned long now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (now - lastPostTime >= postInterval) {
      postTelemetry(currentTemp, currentHumidity, setpointTemp);
      lastPostTime = now;
    }
    if (now - lastConfigFetchTime >= configFetchInterval) {
      fetchConfig();
      lastConfigFetchTime = now;
    }
    if (now - lastManualFetchTime >= manualFetchInterval) {
      fetchManualControl();
      lastManualFetchTime = now;
    }
  }

  delay(50); // Small delay untuk stabilitas
}
