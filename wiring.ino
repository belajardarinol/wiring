// ========================================
// TEMPTRON 607 A-C - Temperature Controller
// Menggunakan sensor suhu DHT11
// ========================================

#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <EEPROM.h>

// ========================================
// WiFi Configuration (COMMENTED FOR LATER)
// ========================================
#include <WiFi.h>
// #include <ESPAsyncWebServer.h>
// #include "webpage.h"
const char* ssid = "Sejahtera";
const char* password = "presiden sekarang";
// AsyncWebServer server(80);
const char* apiTelemetry = "https://malik.ifailamir.my.id/api/telemetry.php";
const char* apiConfig = "https://malik.ifailamir.my.id/api/config.php";
const char* apiManual   = "https://malik.ifailamir.my.id/api/manual.php";

// ========================================
// Pin Definitions
// ========================================
#define CLK1 26
#define DIO1 25
#define CLK2 17
#define DIO2 16
#define DHTPIN 27   
#define DHTTYPE DHT11

// Relay/LED pins untuk kontrol
int relay_heater = 14;    // Heater 7 (aktif saat suhu -2°C dari setpoint)
int relay_cooler = 12;    // Heater 8 (aktif saat suhu -4°C dari setpoint)
int led_status1 = 13;     // Kipas 1 (aktif saat suhu +1°C dari setpoint)
int led_status2 = 5;      // Kipas 2 (aktif saat suhu +2°C dari setpoint)
int led_status3 = 23;     // Kipas 3 (aktif saat suhu +3°C dari setpoint)
int led_status4 = 19;     // Kipas 4 (aktif saat suhu +4°C dari setpoint)
int led_status5 = 18;     // Kipas 5 (aktif saat suhu +5°C dari setpoint)
int led_status6 = 2;      // Kipas 6 (aktif saat suhu +6°C dari setpoint)

// ========================================
// Global Variables
// ========================================
float currentTemp = 0;
float currentHumidity = 0;
String inputCode = "";
bool systemActive = false;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // Baca sensor setiap 2 detik
unsigned long lastPostTime = 0;
const unsigned long postInterval = 10000;
unsigned long lastConfigFetchTime = 0;
const unsigned long configFetchInterval = 30000;
unsigned long lastManualFetchTime = 0;
const unsigned long manualFetchInterval = 1000;

const int EEPROM_SIZE = 512;
const uint16_t CONFIG_MAGIC = 0x607A;

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
};

ConfigData config;

void saveConfig();
bool loadConfig();

// ========================================
// Menu Parameters (16 Menu)
// ========================================
// Menu 01: Setpoint Suhu
float setpointTemp = 30.0;  // Default 30°C (range 20-100)

// Menu 02: Heat Temperature (suhu aktifkan heater)
float heatTemp = 28.0;      // Default 28°C

// Menu 03: Cool Temperature (suhu aktifkan cooler)
float coolTemp = 32.0;      // Default 32°C

// Menu 04-07: Fan Control (placeholder untuk relay tambahan)
bool fan1_enabled = false;
bool fan2_enabled = false;
bool fan3_enabled = false;
bool fan4_enabled = false;

// Menu 08-09: Timer ON/OFF (dalam menit, 0 = disabled)
int timerOn = 0;            // Timer ON (0-99 menit)
int timerOff = 0;           // Timer OFF (0-99 menit)
unsigned long timerStartTime = 0;
bool timerActive = false;

// Menu 10: Humidity Setpoint
float humiditySetpoint = 60.0;  // Default 60% (range 0-99)

// Menu 11: Cool On Time (delay sebelum cooler ON, dalam detik)
int coolOnDelay = 0;        // Default 0 detik (0-99)

// Menu 12: Cool Off Time (delay sebelum cooler OFF, dalam detik)
int coolOffDelay = 0;       // Default 0 detik (0-99)

// Menu 13: Low Alarm (batas bawah)
float lowerLimit = 25.0;    // Default 25°C

// Menu 14: High Alarm (batas atas)
float upperLimit = 35.0;    // Default 35°C

// Menu 15: Alarm Delay (delay sebelum alarm bunyi, dalam detik)
int alarmDelay = 5;         // Default 5 detik (0-99)
unsigned long alarmTriggerTime = 0;

// Menu 16: Display Brightness
int displayBrightness = 15; // Default 15 (0x0f), range 0-15

// Alarm state
bool alarmTriggered = false;

// ========================================
// Manual Control State
// ========================================
bool manualMode = false;  // true = kontrol dari web override otomatis
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
  MENU_IDLE,              // Mode normal - tampilkan suhu
  MENU_BROWSE,            // Mode browse menu
  MENU_EDIT               // Mode edit parameter
};

MenuState currentMenuState = MENU_IDLE;
int currentMenuNumber = 1;  // Menu yang sedang dipilih (1-16)

// ========================================
// Objects Initialization
// ========================================
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display1(CLK1, DIO1);  // Display untuk suhu aktual
TM1637Display display2(CLK2, DIO2);  // Display untuk setpoint

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; 
byte colPins[COLS] = {4, 5, 6, 7};
int i2caddress = 0x20;
Keypad_I2C kpd = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, i2caddress );

// ========================================
// Setup Function - Inisialisasi Sistem
// ========================================
void setup(){
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("TEMPTRON 607 A-C - Temperature Controller");
  Serial.println("========================================");
  
  // Inisialisasi I2C dan Keypad
  Wire.begin();
  kpd.begin();
  
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
  
  // Matikan semua relay dan LED saat start
  digitalWrite(relay_heater, LOW);
  digitalWrite(relay_cooler, LOW);
  digitalWrite(led_status1, LOW);
  digitalWrite(led_status2, LOW);
  digitalWrite(led_status3, LOW);
  digitalWrite(led_status4, LOW);
  digitalWrite(led_status5, LOW);
  digitalWrite(led_status6, LOW);
  
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
  // WiFi Setup (COMMENTED FOR LATER)
  // ========================================
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  //
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send_P(200, "text/html", index_html);
  // });
  //
  // server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
  //   String json = "{\"temp\":" + String(currentTemp, 1) + ",\"setpoint\":" + String(setpointTemp, 1) + "}";
  //   request->send(200, "application/json", json);
  // });
  //
  // server.begin();
}
  
// ========================================
// Helper Functions
// ========================================

// Validasi dan simpan input berdasarkan menu yang aktif
bool validateAndSaveInput(String code, int menuNum) {
  int codeNum = code.toInt();
  bool valid = false;
  
  switch(menuNum) {
    case 1:  // Setpoint Suhu (20-100)
      if (codeNum >= 20 && codeNum <= 100) {
        setpointTemp = (float)codeNum;
        valid = true;
      }
      break;
      
    case 2:  // Heat Temperature (20-100)
      if (codeNum >= 20 && codeNum <= 100) {
        heatTemp = (float)codeNum;
        valid = true;
      }
      break;
      
    case 3:  // Cool Temperature (20-100)
      if (codeNum >= 20 && codeNum <= 100) {
        coolTemp = (float)codeNum;
        valid = true;
      }
      break;
      
    case 4:  // Fan 1 (0=OFF, 1=ON)
      if (codeNum == 0 || codeNum == 1) {
        fan1_enabled = (codeNum == 1);
        valid = true;
      }
      break;
      
    case 5:  // Fan 2 (0=OFF, 1=ON)
      if (codeNum == 0 || codeNum == 1) {
        fan2_enabled = (codeNum == 1);
        valid = true;
      }
      break;
      
    case 6:  // Fan 3 (0=OFF, 1=ON)
      if (codeNum == 0 || codeNum == 1) {
        fan3_enabled = (codeNum == 1);
        valid = true;
      }
      break;
      
    case 7:  // Fan 4 (0=OFF, 1=ON)
      if (codeNum == 0 || codeNum == 1) {
        fan4_enabled = (codeNum == 1);
        valid = true;
      }
      break;
      
    case 8:  // Timer ON (0-99 menit)
      if (codeNum >= 0 && codeNum <= 99) {
        timerOn = codeNum;
        valid = true;
      }
      break;
      
    case 9:  // Timer OFF (0-99 menit)
      if (codeNum >= 0 && codeNum <= 99) {
        timerOff = codeNum;
        valid = true;
      }
      break;
      
    case 10:  // Humidity Setpoint (0-99%)
      if (codeNum >= 0 && codeNum <= 99) {
        humiditySetpoint = (float)codeNum;
        valid = true;
      }
      break;
      
    case 11:  // Cool On Delay (0-99 detik)
      if (codeNum >= 0 && codeNum <= 99) {
        coolOnDelay = codeNum;
        valid = true;
      }
      break;
      
    case 12:  // Cool Off Delay (0-99 detik)
      if (codeNum >= 0 && codeNum <= 99) {
        coolOffDelay = codeNum;
        valid = true;
      }
      break;
      
    case 13:  // Low Alarm (20-100, harus < upperLimit)
      if (codeNum >= 20 && codeNum <= 100 && codeNum < upperLimit) {
        lowerLimit = (float)codeNum;
        valid = true;
      }
      break;
      
    case 14:  // High Alarm (20-100, harus > lowerLimit)
      if (codeNum >= 20 && codeNum <= 100 && codeNum > lowerLimit) {
        upperLimit = (float)codeNum;
        valid = true;
      }
      break;
      
    case 15:  // Alarm Delay (0-99 detik)
      if (codeNum >= 0 && codeNum <= 99) {
        alarmDelay = codeNum;
        valid = true;
      }
      break;
      
    case 16:  // Display Brightness (0-15)
      if (codeNum >= 0 && codeNum <= 15) {
        displayBrightness = codeNum;
        display1.setBrightness(displayBrightness);
        display2.setBrightness(displayBrightness);
        valid = true;
      }
      break;
  }
  
  if (valid) { saveConfig(); }
  return valid;
}

void postTelemetry(float temp, float humidity, float setpoint) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(apiTelemetry);
  http.addHeader("Content-Type", "application/json");
  String mac = WiFi.macAddress();
  String payload = String("{\"device_id\":\"") + mac + "\",\"temp\":" + String(temp, 1) + ",\"humidity\":" + String(humidity, 1) + ",\"setpoint\":" + String(setpoint, 1) + "}";
  http.POST(payload);
  http.end();
}

void fetchConfig() {
  if (WiFi.status() != WL_CONNECTED) return;
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
        while (j < (int)body.length() && body[j] == ' ') j++;
        int k = j;
        while (k < (int)body.length()) {
          char c = body[k];
          if ((c >= '0' && c <= '9') || c == '.' || c == '-') { k++; } else { break; }
        }
        if (k > j) {
          float val = body.substring(j, k).toFloat();
          if (val >= 20 && val <= 100) { setpointTemp = val; }
        }
      }
    }
  }
  http.end();
}

int parseJsonInt(String body, const char* key, int defaultVal) {
  int idx = body.indexOf(key);
  if (idx < 0) return defaultVal;
  int colon = body.indexOf(':', idx);
  if (colon < 0) return defaultVal;
  int j = colon + 1;
  while (j < (int)body.length() && (body[j] == ' ' || body[j] == '\t')) j++;
  if (j >= (int)body.length()) return defaultVal;
  char c = body[j];
  if (c == '1') return 1;
  if (c == '0') return 0;
  return defaultVal;
}

void fetchManualControl() {
  if (WiFi.status() != WL_CONNECTED) return;
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

// Tampilkan menu di display
void showMenu() {
  // Display1 (besar) menampilkan nomor menu dengan leading zero
  display1.showNumberDecEx(currentMenuNumber, 0b01000000, true);  // Format: 0X (X = nomor menu)
  
  // Display2 (kecil) menampilkan nilai setting saat ini
  switch(currentMenuNumber) {
    case 1:
      display2.showNumberDec((int)setpointTemp, false);
      break;
    case 2:
      display2.showNumberDec((int)heatTemp, false);
      break;
    case 3:
      display2.showNumberDec((int)coolTemp, false);
      break;
    case 4:
      display2.showNumberDec(fan1_enabled ? 1 : 0, false);
      break;
    case 5:
      display2.showNumberDec(fan2_enabled ? 1 : 0, false);
      break;
    case 6:
      display2.showNumberDec(fan3_enabled ? 1 : 0, false);
      break;
    case 7:
      display2.showNumberDec(fan4_enabled ? 1 : 0, false);
      break;
    case 8:
      display2.showNumberDec(timerOn, false);
      break;
    case 9:
      display2.showNumberDec(timerOff, false);
      break;
    case 10:
      display2.showNumberDec((int)humiditySetpoint, false);
      break;
    case 11:
      display2.showNumberDec(coolOnDelay, false);
      break;
    case 12:
      display2.showNumberDec(coolOffDelay, false);
      break;
    case 13:
      display2.showNumberDec((int)lowerLimit, false);
      break;
    case 14:
      display2.showNumberDec((int)upperLimit, false);
      break;
    case 15:
      display2.showNumberDec(alarmDelay, false);
      break;
    case 16:
      display2.showNumberDec(displayBrightness, false);
      break;
  }
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
     display1.setBrightness(displayBrightness);
     display2.setBrightness(displayBrightness);
     return true;
   } else {
     saveConfig();
     return false;
   }
 }

// Cek alarm batas suhu
void checkAlarm() {
  if (currentTemp > upperLimit || currentTemp < lowerLimit) {
    if (!alarmTriggered) {
      alarmTriggered = true;
      // Semua LED nyala bersamaan saat alarm
      digitalWrite(led_status1, HIGH);
      digitalWrite(led_status2, HIGH);
      digitalWrite(led_status3, HIGH);
      digitalWrite(led_status4, HIGH);
      digitalWrite(led_status5, HIGH);
      digitalWrite(led_status6, HIGH);
      Serial.println("ALARM: Suhu di luar batas!");
      Serial.print("Batas: ");
      Serial.print(lowerLimit);
      Serial.print("°C - ");
      Serial.print(upperLimit);
      Serial.println("°C");
    }
  } else {
    if (alarmTriggered) {
      alarmTriggered = false;
      // Semua LED mati bersamaan saat alarm clear
      digitalWrite(led_status1, LOW);
      digitalWrite(led_status2, LOW);
      digitalWrite(led_status3, LOW);
      digitalWrite(led_status4, LOW);
      digitalWrite(led_status5, LOW);
      digitalWrite(led_status6, LOW);
      Serial.println("Alarm cleared: Suhu kembali normal");
    }
  }
}

// Terapkan kontrol manual ke relay dan LED
void applyManualControl() {
  // Heater & Cooling
  digitalWrite(relay_heater, manual_heater7 ? HIGH : LOW);
  digitalWrite(relay_cooler, manual_cooling ? HIGH : LOW);

  // Kipas 1-6
  digitalWrite(led_status1, manual_fan1 ? HIGH : LOW);
  digitalWrite(led_status2, manual_fan2 ? HIGH : LOW);
  digitalWrite(led_status3, manual_fan3 ? HIGH : LOW);
  digitalWrite(led_status4, manual_fan4 ? HIGH : LOW);
  digitalWrite(led_status5, manual_fan5 ? HIGH : LOW);
  digitalWrite(led_status6, manual_fan6 ? HIGH : LOW);

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

// Kontrol suhu berdasarkan setpoint dengan logika bertingkat
void temperatureControl() {
  float diff = currentTemp - setpointTemp;
  
  // ========================================
  // COOLING: Kipas 1-6 bertingkat + Relay Cooling
  // ========================================
  if (diff >= 7.0) {
    // +7°C atau lebih: Semua kipas ON + Relay Cooling ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, HIGH);
    digitalWrite(led_status4, HIGH);
    digitalWrite(led_status5, HIGH);
    digitalWrite(led_status6, HIGH);
    digitalWrite(relay_cooler, HIGH);  // Main Cooling ON
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 7 (All fans + Main Cooling ON)");
  }
  else if (diff >= 6.0) {
    // +6°C: Semua kipas ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, HIGH);
    digitalWrite(led_status4, HIGH);
    digitalWrite(led_status5, HIGH);
    digitalWrite(led_status6, HIGH);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 6 (All fans ON)");
  }
  else if (diff >= 5.0) {
    // +5°C: Kipas 1-5 ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, HIGH);
    digitalWrite(led_status4, HIGH);
    digitalWrite(led_status5, HIGH);
    digitalWrite(led_status6, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 5");
  }
  else if (diff >= 4.0) {
    // +4°C: Kipas 1-4 ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, HIGH);
    digitalWrite(led_status4, HIGH);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 4");
  }
  else if (diff >= 3.0) {
    // +3°C: Kipas 1-3 ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, HIGH);
    digitalWrite(led_status4, LOW);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 3");
  }
  else if (diff >= 2.0) {
    // +2°C: Kipas 1-2 ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, HIGH);
    digitalWrite(led_status3, LOW);
    digitalWrite(led_status4, LOW);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 2");
  }
  else if (diff >= 1.0) {
    // +1°C: Kipas 1 ON
    digitalWrite(led_status1, HIGH);
    digitalWrite(led_status2, LOW);
    digitalWrite(led_status3, LOW);
    digitalWrite(led_status4, LOW);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(relay_heater, LOW);
    Serial.println("Status: COOLING LEVEL 1");
  }
  
  // ========================================
  // HEATING: Heater bertingkat
  // ========================================
  else if (diff <= -2.0) {
    // -2°C atau lebih dingin: Heater ON
    digitalWrite(relay_heater, HIGH);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(led_status1, LOW);
    digitalWrite(led_status2, LOW);
    digitalWrite(led_status3, LOW);
    digitalWrite(led_status4, LOW);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    Serial.println("Status: HEATING (Heater ON)");
  }
  
  // ========================================
  // OPTIMAL: Semua OFF
  // ========================================
  else {
    // Suhu dalam range optimal (-2 sampai +1)
    digitalWrite(relay_heater, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(led_status1, LOW);
    digitalWrite(led_status2, LOW);
    digitalWrite(led_status3, LOW);
    digitalWrite(led_status4, LOW);
    digitalWrite(led_status5, LOW);
    digitalWrite(led_status6, LOW);
    Serial.println("Status: OPTIMAL (All OFF)");
  }
}

// Update display berdasarkan state
void updateDisplay() {
  if (currentMenuState == MENU_IDLE) {
    // Mode normal: Display1 = setpoint, Display2 = suhu aktual
    if (inputCode.length() > 0) {
      display1.showNumberDec(inputCode.toInt(), false);
    } else {
      display1.showNumberDec((int)setpointTemp, false);
    }
    display2.showNumberDec((int)currentTemp, false);
  } else if (currentMenuState == MENU_BROWSE) {
    // Mode browse: tampilkan menu
    showMenu();
  } else if (currentMenuState == MENU_EDIT) {
    // Mode edit: Display1 = input, Display2 = nilai lama dari menu
    if (inputCode.length() > 0) {
      display1.showNumberDec(inputCode.toInt(), false);
    } else {
      display1.showNumberDec(0, false);
    }
    // Tampilkan nilai lama di display2 (sama dengan showMenu)
    showMenu();
  }
}

// ========================================
// Main Loop - Mengikuti Flowchart
// ========================================
void loop(){
  // Ambil input dari keyboard
  char key = kpd.getKey();
  
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    
    // ========================================
    // MENU IDLE - Mode Normal
    // ========================================
    if (currentMenuState == MENU_IDLE) {
      // Tombol A: Masuk ke mode browse menu
      if (key == 'A') {
        currentMenuState = MENU_BROWSE;
        currentMenuNumber = 1;
        Serial.println("Masuk mode menu");
        showMenu();
      }
      
      // Tombol angka: Quick edit setpoint (backward compatibility)
      else if (key >= '0' && key <= '9') {
        inputCode += key;
        display1.showNumberDec(inputCode.toInt(), false);
        if (inputCode.length() >= 2) {
          if (validateAndSaveInput(inputCode, 1)) {  // Menu 1 = Setpoint
            Serial.print("Setpoint diubah ke: ");
            Serial.print(setpointTemp);
            Serial.println("°C");
            if (!systemActive) {
              systemActive = true;
              digitalWrite(led_status3, HIGH);
              Serial.println("Sistem aktif!");
            }
          } else {
            Serial.println("Setpoint tidak valid! (Range: 20-100)");
          }
          inputCode = "";
        }
      }
      
      // Tombol C: Reset input
      else if (key == 'C') {
        inputCode = "";
        updateDisplay();
      }
      
      // Tombol D: Stop sistem
      else if (key == 'D') {
        systemActive = false;
        digitalWrite(relay_heater, LOW);
        digitalWrite(relay_cooler, LOW);
        digitalWrite(led_status1, LOW);
        digitalWrite(led_status2, LOW);
        digitalWrite(led_status3, LOW);
        Serial.println("Sistem dihentikan!");
      }
    }
    
    // ========================================
    // MENU BROWSE - Navigasi Menu
    // ========================================
    else if (currentMenuState == MENU_BROWSE) {
      // Tombol A: Next menu (1 -> 2 -> ... -> 16 -> 1)
      if (key == 'A') {
        currentMenuNumber++;
        if (currentMenuNumber > 16) currentMenuNumber = 1;
        Serial.print("Menu: ");
        Serial.println(currentMenuNumber);
        showMenu();
      }
      
      // Tombol B: Pilih menu untuk edit
      else if (key == 'B') {
        currentMenuState = MENU_EDIT;
        Serial.print("Edit Menu ");
        if (currentMenuNumber < 10) Serial.print("0");
        Serial.println(currentMenuNumber);
        inputCode = "";
        updateDisplay();
      }
      
      // Tombol C: Kembali ke mode normal
      else if (key == 'C') {
        currentMenuState = MENU_IDLE;
        Serial.println("Kembali ke mode normal");
        updateDisplay();
      }
    }
    
    // ========================================
    // MENU EDIT - Edit Parameter
    // ========================================
    else if (currentMenuState == MENU_EDIT) {
      
      // Tombol angka: Input nilai baru
      if (key >= '0' && key <= '9') {
        inputCode += key;
        display1.showNumberDec(inputCode.toInt(), false);
        
        // Setelah 2 digit, validasi dan simpan
        if (inputCode.length() >= 2) {
          bool valid = validateAndSaveInput(inputCode, currentMenuNumber);
          
          if (valid) {
            Serial.print("Menu ");
            if (currentMenuNumber < 10) Serial.print("0");
            Serial.print(currentMenuNumber);
            Serial.print(" diubah ke: ");
            Serial.println(inputCode);
          } else {
            Serial.println("Input tidak valid!");
            digitalWrite(led_status4, HIGH);
            delay(500);
            digitalWrite(led_status4, LOW);
          }
          
          inputCode = "";
          // Kembali ke browse menu setelah edit
          currentMenuState = MENU_BROWSE;
          showMenu();
        }
      }
      
      // Tombol C: Cancel edit, kembali ke browse
      else if (key == 'C') {
        inputCode = "";
        currentMenuState = MENU_BROWSE;
        Serial.println("Edit dibatalkan");
        showMenu();
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
      digitalWrite(led_status4, HIGH);
      return;
    } else {
      digitalWrite(led_status4, LOW);
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
    
    // Kontrol jika sistem aktif
    if (systemActive) {
      if (manualMode) {
        applyManualControl();
      } else {
        temperatureControl();
        checkAlarm();  // Cek alarm batas suhu
      }
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
  
  delay(50);  // Small delay untuk stabilitas
}
