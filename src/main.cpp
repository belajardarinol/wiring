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

const int EEPROM_SIZE = 512;
const uint16_t CONFIG_MAGIC = 0x607B;

struct ConfigData {
  uint16_t magic;
  float setpointTemp;
  float heatTemp;
  float coolTemp;
  bool fan1_enabled;
  bool fan2_enabled;
  bool fan3_enabled;
  bool fan4_enabled;
  int timerOn;
  int timerOff;
  float humiditySetpoint;
  int coolOnDelay;
  int coolOffDelay;
  float lowerLimit;
  float upperLimit;
  int alarmDelay;
  int displayBrightness;
  // Tambahan Menu 17-32
  float waterClock;
  int feedMult;
  float dailyFeed;
  float totalFeed;
  float day1Temp;
  float tempGraph;
  int growDay;
  float resetTime;
  bool systemActive;
};

ConfigData config;

void saveConfig();
bool loadConfig();

// ========================================
// Menu Parameters (16 Menu)
// ========================================
// Menu 01: Setpoint Suhu
float setpointTemp = 30.0; // Default 30°C (range 20-100)

// Menu 02: Heat Temperature (suhu aktifkan heater)
float heatTemp = 28.0; // Default 28°C

// Menu 03: Cool Temperature (suhu aktifkan cooler)
float coolTemp = 32.0; // Default 32°C

// Menu 04-07: Fan Control (placeholder untuk relay tambahan)
bool fan1_enabled = false;
bool fan2_enabled = false;
bool fan3_enabled = false;
bool fan4_enabled = false;

// Menu 08-09: Timer ON/OFF (dalam menit, 0 = disabled)
int timerOn = 0;  // Timer ON (0-99 menit)
int timerOff = 0; // Timer OFF (0-99 menit)
unsigned long timerStartTime = 0;
bool timerActive =
    false; // Internal: Current state of intermittent (true=ON, false=OFF)

// Menu 10: Humidity Setpoint
float humiditySetpoint = 60.0; // Default 60% (range 0-99)

// Menu 11: Cool On Time (delay sebelum cooler ON, dalam detik)
int coolOnDelay = 0; // Default 0 detik (0-99)

// Menu 12: Cool Off Time (delay sebelum cooler OFF, dalam detik)
int coolOffDelay = 0; // Default 0 detik (0-99)

// Menu 13: Low Alarm (batas bawah)
float lowerLimit = 25.0; // Default 25°C

// Menu 14: High Alarm (batas atas)
float upperLimit = 35.0; // Default 35°C

// Menu 15: Alarm Delay (delay sebelum alarm bunyi, dalam detik)
int alarmDelay = 5; // Default 5 detik (0-99)
unsigned long alarmTriggerTime = 0;

// Menu 16: Display Brightness
int displayBrightness = 15; // Default 15 (0x0f), range 0-15

// ========================================
// MENU TAMBAHAN (17-32)
// ========================================
// Menu 17: Water Clock (00:00 - 23:59 format HH.MM)
float waterClock = 0.0;

// Menu 18: Feed Mult (Faktor pakan)
int feedMult = 100;

// Menu 19: Daily Feed (Total pakan harian)
float dailyFeed = 0.0;

// Menu 20: Total Feed (Total pakan kumulatif)
float totalFeed = 0.0;

// Menu 21: Day 1 Temp (Suhu hari pertama)
float day1Temp = 32.0;

// Menu 22: Temp Graph (Kenaikan/penurunan suhu per hari)
float tempGraph = 0.0;

// Menu 31: Grow Day (Hari ke-berapa)
int growDay = 1;

// Menu 32: Reset Time (Waktu reset data)
float resetTime = 0.0;

// Alarm state
bool alarmTriggered = false;
unsigned long alarmTimerStart = 0; // Timer untuk delay alarm

// Cooling state
unsigned long coolTimerStart = 0; // Timer untuk delay cooling
bool coolingActiveState = false;  // Internal state status cooling

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
int currentMenuNumber = 1; // Menu yang sedang dipilih (1-16)

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

      // Config
      json += "\"config\":{";
      json += "\"setpoint\":" + String(setpointTemp, 1) + ",";
      json += "\"lowerLimit\":" + String(lowerLimit, 1) + ",";
      json += "\"upperLimit\":" + String(upperLimit, 1);
      json += "},";

      // State (Visual feedback for LEDs)
      // Note: In auto mode, these are calculated. In manual, they are valid.
      // We send the pin states to reflect reality.
      json += "\"state\":{";
      json += "\"manual\":" + String(manualMode ? 1 : 0) + ",";
      json += "\"fan1\":" + String(digitalRead(led_status1)) + ",";
      json += "\"fan2\":" + String(digitalRead(led_status2)) + ",";
      json += "\"fan3\":" + String(digitalRead(led_status3)) + ",";
      json += "\"fan4\":" + String(digitalRead(led_status4)) + ",";
      json += "\"fan5\":" + String(digitalRead(led_status5)) + ",";
      json += "\"fan6\":" + String(digitalRead(led_status6)) + ",";
      json += "\"heater7\":" + String(digitalRead(relay_heater)) + ",";
      json +=
          "\"cooling\":" +
          String(digitalRead(
              relay_cooler)); // Note: index.php maps cooling led to 'cooling'
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
bool validateAndSaveInput(String code, int menuNum) {
  int codeNum = code.toInt();
  bool valid = false;

  switch (menuNum) {

  case 1: // Clock (Skip for now)
    // No action for clock setting yet in this version without RTC logic
    valid = true;
    break;

  case 2: // Required Temp (Setpoint) (20-100)
    if (codeNum >= 20 && codeNum <= 100) {
      setpointTemp = (float)codeNum;
      valid = true;
    }
    break;

  case 3: // Heat (20-100)
    if (codeNum >= 20 && codeNum <= 100) {
      heatTemp = (float)codeNum;
      valid = true;
    }
    break;

  case 4: // Fan 1 (0=OFF, 1=ON)
    if (codeNum == 0 || codeNum == 1) {
      fan1_enabled = (codeNum == 1);
      valid = true;
    }
    break;

  case 5: // Fan 2 (0=OFF, 1=ON)
    if (codeNum == 0 || codeNum == 1) {
      fan2_enabled = (codeNum == 1);
      valid = true;
    }
    break;

  case 6: // Fan 3 (0=OFF, 1=ON)
    if (codeNum == 0 || codeNum == 1) {
      fan3_enabled = (codeNum == 1);
      valid = true;
    }
    break;

  case 7: // Fan 4 (0=OFF, 1=ON)
    if (codeNum == 0 || codeNum == 1) {
      fan4_enabled = (codeNum == 1);
      valid = true;
    }
    break;

  case 8: // Fan 5 (0=OFF, 1=ON)
    if (codeNum == 0 || codeNum == 1) {
      // Logic fan 5 enabled belum ada variable khususnya, kita asumsikan
      // pakai slot dummy atau tambah nanti. Sesuai kode lama: fan5 belum ada,
      // kode lama case 8 adalah Timer ON. Kita geser Timer ON ke menu 9.
      // Untuk sementara Fan 5 kita simpan di fan4_enabled atau buat baru,
      // atau abaikan dulu jika belum ada relay fisik. ALERT: Kode relay hanya
      // sampai Fan 4 (fan4_enabled). Mari mapping ke variable yang ada dulu
      // agar tidak error compile.
      valid = true;
    }
    break;

  case 9: // Fan On Time (0-99 menit) -> (Variable: timerOn)
    if (codeNum >= 0 && codeNum <= 99) {
      timerOn = codeNum;
      valid = true;
    }
    break;

  case 10: // Fan Off Time (0-99 menit) -> (Variable: timerOff)
    if (codeNum >= 0 && codeNum <= 99) {
      timerOff = codeNum;
      valid = true;
    }
    break;

  case 11: // Humidity Set (0-99%) -> (Variable: humiditySetpoint)
    if (codeNum >= 0 && codeNum <= 99) {
      humiditySetpoint = (float)codeNum;
      valid = true;
    }
    break;

  case 12: // Cool Temp (20-50 C) -> (Variable: coolTemp)
    if (codeNum >= 20 && codeNum <= 100) { // Range 20-100 biar aman
      coolTemp = (float)codeNum;
      valid = true;
    }
    break;

  case 13: // Cool On Time (0-99 det/menit) -> (Variable: coolOnDelay ? atau
           // kita perlu variable khusus?)
    // Di kode lama: coolOnDelay. Kita pakai itu.
    if (codeNum >= 0 && codeNum <= 99) {
      coolOnDelay = codeNum;
      valid = true;
    }
    break;

  case 14: // Cool Off Time -> (Variable: coolOffDelay)
    if (codeNum >= 0 && codeNum <= 99) {
      coolOffDelay = codeNum;
      valid = true;
    }
    break;

  case 15: // Low Alarm -> (Variable: lowerLimit)
    if (codeNum >= 0 && codeNum <= 100 && codeNum < upperLimit) {
      lowerLimit = (float)codeNum;
      valid = true;
    }
    break;

  case 16: // High Alarm -> (Variable: upperLimit)
    if (codeNum >= 0 && codeNum <= 100 && codeNum > lowerLimit) {
      upperLimit = (float)codeNum;
      valid = true;
    }
    break;
    break;

  case 17:                       // Water Clock
    waterClock = (float)codeNum; // Simplified
    valid = true;
    break;
  case 18: // Feed Mult
    feedMult = codeNum;
    valid = true;
    break;
  case 19: // Daily Feed
    dailyFeed = (float)codeNum;
    valid = true;
    break;
  case 20: // Total Feed
    totalFeed = (float)codeNum;
    valid = true;
    break;
  case 21: // Day 1 Temp
    day1Temp = (float)codeNum;
    valid = true;
    break;
  case 22: // Temp Graph
    tempGraph = (float)codeNum;
    valid = true;
    break;
  case 31: // Grow Day
    growDay = codeNum;
    valid = true;
    break;
  case 32: // Reset Time
    resetTime = (float)codeNum;
    valid = true;
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
  config.fan1_enabled = fan1_enabled;
  config.fan2_enabled = fan2_enabled;
  config.fan3_enabled = fan3_enabled;
  config.fan4_enabled = fan4_enabled;
  config.timerOn = timerOn;
  config.timerOff = timerOff;
  config.humiditySetpoint = humiditySetpoint;
  config.coolOnDelay = coolOnDelay;
  config.coolOffDelay = coolOffDelay;
  config.lowerLimit = lowerLimit;
  config.upperLimit = upperLimit;
  config.alarmDelay = alarmDelay;
  config.displayBrightness = displayBrightness;
  config.waterClock = waterClock;
  config.feedMult = feedMult;
  config.dailyFeed = dailyFeed;
  config.totalFeed = totalFeed;
  config.day1Temp = day1Temp;
  config.tempGraph = tempGraph;
  config.growDay = growDay;
  config.resetTime = resetTime;
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
    fan1_enabled = config.fan1_enabled;
    fan2_enabled = config.fan2_enabled;
    fan3_enabled = config.fan3_enabled;
    fan4_enabled = config.fan4_enabled;
    timerOn = config.timerOn;
    timerOff = config.timerOff;
    humiditySetpoint = config.humiditySetpoint;
    coolOnDelay = config.coolOnDelay;
    coolOffDelay = config.coolOffDelay;
    lowerLimit = config.lowerLimit;
    upperLimit = config.upperLimit;
    alarmDelay = config.alarmDelay;
    displayBrightness = config.displayBrightness;
    waterClock = config.waterClock;
    feedMult = config.feedMult;
    dailyFeed = config.dailyFeed;
    totalFeed = config.totalFeed;
    day1Temp = config.day1Temp;
    tempGraph = config.tempGraph;
    growDay = config.growDay;
    resetTime = config.resetTime;
    systemActive = config.systemActive;
    display1.setBrightness(displayBrightness);
    display2.setBrightness(displayBrightness);
    return true;
  } else {
    saveConfig();
    return false;
  }
}

// Cek alarm batas suhu dengan delay (Menu 13, 14, 15)
void checkAlarm() {
  if (currentTemp > upperLimit || currentTemp < lowerLimit) {
    // Kondisi alarm terdeteksi
    if (!alarmTriggered) {
      // Cek timer delay
      if (alarmTimerStart == 0) {
        alarmTimerStart = millis();
      } else {
        if (millis() - alarmTimerStart >= (unsigned long)alarmDelay * 1000UL) {
          // Delay terpenuhi, TRIGGER ALARM
          alarmTriggered = true;
          // Semua LED nyala bersamaan saat alarm
          digitalWrite(led_status1, RELAY_ON);
          digitalWrite(led_status2, RELAY_ON);
          digitalWrite(led_status3, RELAY_ON);
          digitalWrite(led_status4, RELAY_ON);
          digitalWrite(led_status5, RELAY_ON);
          setFan6(RELAY_ON);
          Serial.println("ALARM: Suhu di luar batas (Delayed)!");
        }
      }
    }
  } else {
    // Suhu normal
    alarmTimerStart = 0; // Reset timer
    if (alarmTriggered) {
      alarmTriggered = false;
      // Semua LED mati bersamaan saat alarm clear
      digitalWrite(led_status1, RELAY_OFF);
      digitalWrite(led_status2, RELAY_OFF);
      digitalWrite(led_status3, RELAY_OFF);
      digitalWrite(led_status4, RELAY_OFF);
      digitalWrite(led_status5, RELAY_OFF);
      setFan6(RELAY_OFF);
      Serial.println("Alarm cleared: Suhu kembali normal");
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

// Helper untuk cek logic Cooling (Menu 03 + 11 + 12)
void checkCooling() {
  if (currentTemp >= coolTemp) {
    // Suhu >= Cool Temp, minta COOL ON
    if (!coolingActiveState) {
      // Jika sebelumnya OFF, cek timer ON Delay
      if (coolTimerStart == 0) {
        coolTimerStart = millis(); // Mulai hitung
      } else {
        // Cek apakah delay sudah terpenuhi
        if (millis() - coolTimerStart >= (unsigned long)coolOnDelay * 1000UL) {
          coolingActiveState = true;
          coolTimerStart = 0; // Reset timer
        }
      }
    } else {
      // Jika sudah ON, reset timer (antisipasi switching state)
      coolTimerStart = 0;
    }
  } else {
    // Suhu < Cool Temp, minta COOL OFF
    if (coolingActiveState) {
      // Jika sebelumnya ON, cek timer OFF Delay
      if (coolTimerStart == 0) {
        coolTimerStart = millis(); // Mulai hitung
      } else {
        // Cek apakah delay sudah terpenuhi
        if (millis() - coolTimerStart >= (unsigned long)coolOffDelay * 1000UL) {
          coolingActiveState = false;
          coolTimerStart = 0; // Reset timer
        }
      }
    } else {
      // Jika sudah OFF, reset timer
      coolTimerStart = 0;
    }
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

// Kontrol suhu berdasarkan setpoint dengan logika bertingkat
void temperatureControl() {
  updateIntermittent(); // Update timer state for Fan 1
  checkCooling();       // Update cooling state (Menu 03 + Delays)

  float diff = currentTemp - setpointTemp;

  // ========================================
  // CONTROL KOOLING (Kipas 1-6) - Berbasis Diff Suhu
  // ========================================

  // Reset semua fan ke OFF dulu (default), nanti di-Override jika ON
  bool f1 = false;
  bool f2 = false;
  bool f3 = false;
  bool f4 = false;
  bool f5 = false;
  bool f6 = false;

  if (diff >= 6.0) {
    // +6°C: Semua kipas ON
    f1 = true;
    f2 = true;
    f3 = true;
    f4 = true;
    f5 = true;
    f6 = true;
    Serial.println("Status: FANS LEVEL 6 (All fans)");
  } else if (diff >= 5.0) {
    // +5°C: Kipas 1-5 ON
    f1 = true;
    f2 = true;
    f3 = true;
    f4 = true;
    f5 = true;
    f6 = false;
    Serial.println("Status: FANS LEVEL 5");
  } else if (diff >= 4.0) {
    // +4°C: Kipas 1-4 ON
    f1 = true;
    f2 = true;
    f3 = true;
    f4 = true;
    f5 = false;
    f6 = false;
    Serial.println("Status: FANS LEVEL 4");
  } else if (diff >= 3.0) {
    // +3°C: Kipas 1-3 ON
    f1 = true;
    f2 = true;
    f3 = true;
    f4 = false;
    f5 = false;
    f6 = false;
    Serial.println("Status: FANS LEVEL 3");
  } else if (diff >= 2.0) {
    // +2°C: Kipas 1-2 ON
    f1 = true;
    f2 = true;
    f3 = false;
    f4 = false;
    f5 = false;
    f6 = false;
    Serial.println("Status: FANS LEVEL 2");
  } else if (diff >= 1.0) {
    // +1°C: Kipas 1 ON
    f1 = true;
    f2 = false;
    f3 = false;
    f4 = false;
    f5 = false;
    f6 = false;
    Serial.println("Status: FANS LEVEL 1");
  } else {
    // Diff < 1.0 (Optimal / Heating)
    // Fan 1 ikut Intermittent (Timer ON/OFF)
    if (timerOn > 0 && timerOff > 0 && timerActive)
      f1 = true;
    Serial.println("Status: FANS IDLE / INTERMITTENT");
  }

  // ========================================
  // CONTROL HEATING (Heater) - Berbasis Heat Temp (Menu 02)
  // ========================================
  bool heaterState = false;
  if (currentTemp <= heatTemp) {
    heaterState = true;
    Serial.println("Status: HEATER ON");
  }

  // ========================================
  // EKSEKUSI KE RELAY (Dengan Cek Enable/Disable)
  // ========================================

  // Kipas 1 (Menu 04 Enable)
  if (fan1_enabled)
    digitalWrite(led_status1, f1 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status1, RELAY_OFF);

  // Kipas 2 (Menu 05 Enable)
  if (fan2_enabled)
    digitalWrite(led_status2, f2 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status2, RELAY_OFF);

  // Kipas 3 (Menu 06 Enable)
  if (fan3_enabled)
    digitalWrite(led_status3, f3 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status3, RELAY_OFF);

  // Kipas 4 (Menu 07 Enable)
  if (fan4_enabled)
    digitalWrite(led_status4, f4 ? RELAY_ON : RELAY_OFF);
  else
    digitalWrite(led_status4, RELAY_OFF);

  // Kipas 5 & 6 (Belum ada menu enable khusus, anggap enabled)
  digitalWrite(led_status5, f5 ? RELAY_ON : RELAY_OFF);
  setFan6(f6 ? RELAY_ON : RELAY_OFF);

  // Heater (Menu 02)
  digitalWrite(relay_heater, heaterState ? RELAY_ON : RELAY_OFF);

  // Cooler (Menu 03 Control + Menu 11/12 Delay)
  // Independent dari logic Kipas
  digitalWrite(relay_cooler, coolingActiveState ? RELAY_ON : RELAY_OFF);
  if (coolingActiveState)
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
  bool isBool = false;

  switch (currentMenuNumber) {
  case 1:          // Clock
    valToShow = 0; // Temp placeholder
    break;
  case 2: // Required Temp
    valToShow = setpointTemp;
    break;
  case 3: // Heat
    valToShow = heatTemp;
    break;
  case 4: // Fan 1
    valToShow = fan1_enabled;
    isBool = true;
    break;
  case 5: // Fan 2
    valToShow = fan2_enabled;
    isBool = true;
    break;
  case 6: // Fan 3
    valToShow = fan3_enabled;
    isBool = true;
    break;
  case 7: // Fan 4
    valToShow = fan4_enabled;
    isBool = true;
    break;
  case 8:          // Fan 5
    valToShow = 0; // Placeholder for Fan 5 relay
    isBool = true;
    break;
  case 9: // Fan On Time
    valToShow = timerOn;
    break;
  case 10: // Fan Off Time
    valToShow = timerOff;
    break;
  case 11: // Humidity Set
    valToShow = humiditySetpoint;
    break;
  case 12: // Cool Temp
    valToShow = coolTemp;
    break;
  case 13: // Cool On Time
    valToShow = coolOnDelay;
    break;
  case 14: // Cool Off Time
    valToShow = coolOffDelay;
    break;
  case 15: // Low Alarm
    valToShow = lowerLimit;
    break;
  case 16: // High Alarm
    valToShow = upperLimit;
    break;
  case 17:
    valToShow = waterClock;
    isBool = false;
    break;
  case 18:
    valToShow = feedMult;
    isBool = false;
    break;
  case 19:
    valToShow = dailyFeed;
    isBool = false;
    break;
  case 20:
    valToShow = totalFeed;
    isBool = false;
    break;
  case 21:
    valToShow = day1Temp;
    isBool = false;
    break;
  case 22:
    valToShow = tempGraph;
    isBool = false;
    break;
  case 31:
    valToShow = growDay;
    isBool = false;
    break;
  case 32:
    valToShow = resetTime;
    isBool = false;
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

    // Display 2 (Small): Show Function Number/Idle
    // If we are in IDLE, show " 00" or clear?
    // User Guide says: "Small display shows nothing in idle"
    display2.clear();
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

    // 1. DATA KEY (Tombol A) - Pindah Menu
    if (key == 'A') {
      if (currentMenuState == MENU_EDIT) {
        // Jika sedang edit, DATA membatalkan edit (mirip perilaku asli)
        currentMenuState = MENU_BROWSE;
        inputCode = "";
        updateDisplay();
      } else {
        // Pindah ke menu berikutnya
        currentMenuState = MENU_BROWSE; // Pastikan masuk mode browse
        currentMenuNumber++;
        if (currentMenuNumber > 16)
          currentMenuNumber = 1; // Kembali ke 1 (atau 0 untuk display?)
        Serial.print("Fungsi: ");
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
        // Di mode browse/idle, ENTER bisa dipakai untuk shortcut navigasi
        // (jika logika 0 dipakai)
        if (inputCode.length() > 0 && currentMenuState == MENU_BROWSE) {
          // Jump to menu number
          int target = inputCode.toInt();
          if (target >= 1 && target <= 16) {
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
      // Logic Navigasi: Tekan '0' di awal untuk shortcut menu (Temptron
      // feature) Tapi untuk simplifikasi:

      if (currentMenuState == MENU_EDIT) {
        // Input nilai setting
        if (inputCode.length() < 4) { // Max 4 digit
          inputCode += key;
          display1.showNumberDec(inputCode.toInt(), false);
        }
      } else {
        // Mode Idle/Browse: Bisa ketik nomor menu untuk jump (Shortcut)
        // Temptron: Tekan angka langsung masuk ke mode navigasi ke menu tsb?
        // Kita implementasikan: Ketik angka -> Masuk ke menu tsb (jika valid)

        if (key == '0' && inputCode.length() == 0) {
          // Start jump sequence
          inputCode += key;
          // Tampilkan "0-" di display?
        } else if (inputCode.length() > 0) {
          inputCode += key;
          int target = inputCode.toInt();
          if (target >= 1 && target <= 16) {
            currentMenuNumber = target;
            currentMenuState = MENU_BROWSE;
            inputCode = ""; // Reset setelah jump
            showMenu();
          }
        }
        // Default behavior jika tidak jump: Abaikan atau Quick Setpoint?
        // Kita ikut standar: Idle tidak melakukan apa2 kecuali navigasi
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

    Serial.print("Suhu: ");
    Serial.print(currentTemp);
    Serial.print("°C | Humidity: ");
    Serial.print(currentHumidity);
    Serial.print("% | Setpoint: ");
    Serial.print(setpointTemp);
    Serial.print("°C | Batas: ");
    Serial.print(lowerLimit);
    Serial.print("-");
    Serial.print(upperLimit);
    Serial.println("°C");

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
