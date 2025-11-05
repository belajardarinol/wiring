# Konfigurasi dan Penyesuaian - TEMPTRON 607 A-C

## Konfigurasi Dasar

### 1. Setpoint Default
Ubah setpoint default di file `wiring.ino`:
```cpp
float setpointTemp = 80.0;  // Default setpoint 80°C (range 60-100)
```

### 2. Range Kode Akses
Untuk mengubah range kode akses, edit fungsi `validateCode()`:
```cpp
bool validateCode(String code) {
  int codeNum = code.toInt();
  if (codeNum >= 60 && codeNum <= 100) {  // Ubah range di sini
    setpointTemp = (float)codeNum;
    return true;
  }
  return false;
}
```

### 3. Hysteresis Suhu
Ubah nilai hysteresis untuk kontrol suhu di fungsi `temperatureControl()`:
```cpp
void temperatureControl() {
  if (currentTemp < setpointTemp - 2) {  // Ubah nilai 2 untuk hysteresis bawah
    // Aktifkan heater
  } 
  else if (currentTemp > setpointTemp + 2) {  // Ubah nilai 2 untuk hysteresis atas
    // Aktifkan cooler
  }
}
```

### 4. Interval Pembacaan Sensor
Ubah interval pembacaan sensor (dalam milidetik):
```cpp
const unsigned long readInterval = 2000; // 2000 = 2 detik
```

### 5. Pin Assignment
Jika perlu mengubah pin, edit bagian Pin Definitions:
```cpp
// Display
#define CLK1 26
#define DIO1 25
#define CLK2 17
#define DIO2 16

// Sensor
#define DHTPIN 27

// Relay dan LED
int relay_heater = 14;
int relay_cooler = 12;
int led_status1 = 13;
// dst...
```

## Konfigurasi Lanjutan

### 1. Logika Relay
Beberapa relay module aktif LOW, beberapa aktif HIGH. Sesuaikan jika perlu:

**Untuk relay aktif LOW:**
```cpp
// Heater ON
digitalWrite(relay_heater, LOW);   // Ubah HIGH ke LOW
// Heater OFF
digitalWrite(relay_heater, HIGH);  // Ubah LOW ke HIGH
```

### 2. Brightness Display
Ubah brightness display (0x00 - 0x0f):
```cpp
display1.setBrightness(0x0f);  // 0x0f = maksimal
display2.setBrightness(0x0f);
```

### 3. Format Display
Untuk menampilkan dengan leading zero atau decimal:
```cpp
// Tanpa leading zero
display1.showNumberDec((int)currentTemp, false);

// Dengan leading zero
display1.showNumberDec((int)currentTemp, true);

// Dengan decimal point (untuk display yang support)
display1.showNumberDecEx((int)(currentTemp * 10), 0b01000000, false);
```

### 4. Alamat I2C Keypad
Jika keypad menggunakan alamat I2C berbeda:
```cpp
int i2caddress = 0x20;  // Ubah sesuai alamat keypad Anda
```

Untuk mencari alamat I2C, gunakan I2C Scanner (lihat WIRING_DIAGRAM.md).

## Konfigurasi Web Interface (Untuk Nanti)

Ketika siap mengaktifkan web interface:

### 1. Uncomment bagian WiFi
Di bagian atas file:
```cpp
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "webpage.h"
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
AsyncWebServer server(80);
```

### 2. Uncomment setup WiFi
Di fungsi `setup()`:
```cpp
Serial.println("Connecting to WiFi...");
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connecting...");
}
Serial.println("Connected to WiFi");
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", index_html);
});

server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
  String json = "{\"temp\":" + String(currentTemp, 1) + ",\"setpoint\":" + String(setpointTemp, 1) + "}";
  request->send(200, "application/json", json);
});

server.begin();
```

### 3. Install library tambahan
```
- ESPAsyncWebServer
- AsyncTCP
```

## Optimasi Performa

### 1. Reduce Serial Print
Untuk performa lebih baik, kurangi Serial.print di loop:
```cpp
// Hanya print saat ada perubahan signifikan
if (abs(currentTemp - lastPrintedTemp) > 0.5) {
  Serial.print("Suhu: ");
  Serial.println(currentTemp);
  lastPrintedTemp = currentTemp;
}
```

### 2. Debouncing Keypad
Jika keypad terlalu sensitif, tambahkan debouncing:
```cpp
char key = kpd.getKey();
if (key) {
  delay(50);  // Debounce delay
  // Process key
}
```

### 3. Watchdog Timer
Untuk sistem yang harus reliable 24/7:
```cpp
#include <esp_task_wdt.h>

void setup() {
  // Enable watchdog timer (10 detik)
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);
  // ...
}

void loop() {
  // Reset watchdog
  esp_task_wdt_reset();
  // ...
}
```

## Kalibrasi Sensor

### DHT11 Offset
Jika sensor DHT11 tidak akurat, tambahkan offset:
```cpp
float currentTemp = dht.readTemperature();
currentTemp = currentTemp + 2.0;  // Tambah offset +2°C jika sensor terlalu rendah
```

### Sensor Alternatif
Untuk akurasi lebih baik, ganti dengan sensor lain:

**DS18B20 (lebih akurat):**
```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(DHTPIN);
DallasTemperature sensors(&oneWire);

void setup() {
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
}
```

**MAX6675 (untuk suhu tinggi):**
```cpp
#include <max6675.h>

int thermoDO = 19;
int thermoCS = 23;
int thermoCLK = 5;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void loop() {
  currentTemp = thermocouple.readCelsius();
}
```

## Safety Features

### 1. Temperature Limit
Tambahkan batas suhu maksimal untuk safety:
```cpp
#define MAX_SAFE_TEMP 110.0

void temperatureControl() {
  // Emergency shutdown jika suhu terlalu tinggi
  if (currentTemp > MAX_SAFE_TEMP) {
    digitalWrite(relay_heater, LOW);
    digitalWrite(relay_cooler, LOW);
    Serial.println("EMERGENCY: Temperature too high!");
    // Aktifkan alarm
    return;
  }
  
  // Normal control
  // ...
}
```

### 2. Sensor Failure Detection
```cpp
int sensorFailCount = 0;
#define MAX_FAIL_COUNT 5

void loop() {
  currentTemp = dht.readTemperature();
  
  if (isnan(currentTemp)) {
    sensorFailCount++;
    if (sensorFailCount >= MAX_FAIL_COUNT) {
      // Matikan semua relay untuk safety
      digitalWrite(relay_heater, LOW);
      digitalWrite(relay_cooler, LOW);
      systemActive = false;
      Serial.println("SENSOR FAILURE - System shutdown");
    }
  } else {
    sensorFailCount = 0;  // Reset counter
  }
}
```

### 3. Timeout Protection
```cpp
unsigned long lastActivityTime = 0;
#define TIMEOUT_DURATION 3600000  // 1 jam

void loop() {
  if (systemActive && (millis() - lastActivityTime > TIMEOUT_DURATION)) {
    // Auto shutdown setelah timeout
    systemActive = false;
    digitalWrite(relay_heater, LOW);
    digitalWrite(relay_cooler, LOW);
    Serial.println("System timeout - auto shutdown");
  }
}
```

## Tips dan Trik

### 1. Debug Mode
Tambahkan mode debug untuk troubleshooting:
```cpp
#define DEBUG_MODE true

#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif
```

### 2. EEPROM untuk Menyimpan Setpoint
```cpp
#include <EEPROM.h>

void saveSetpoint() {
  EEPROM.write(0, (int)setpointTemp);
  EEPROM.commit();
}

void loadSetpoint() {
  int saved = EEPROM.read(0);
  if (saved >= 60 && saved <= 100) {
    setpointTemp = (float)saved;
  }
}
```

### 3. Multi-User Code
```cpp
struct User {
  String code;
  float setpoint;
  String name;
};

User users[] = {
  {"60", 60.0, "User1"},
  {"75", 75.0, "User2"},
  {"80", 80.0, "User3"},
  {"100", 100.0, "Admin"}
};
```

## Referensi Cepat

| Parameter | Default | Range | Satuan |
|-----------|---------|-------|--------|
| Setpoint | 80 | 60-100 | °C |
| Hysteresis | ±2 | 1-5 | °C |
| Read Interval | 2000 | 1000-10000 | ms |
| I2C Address | 0x20 | 0x20-0x27 | hex |
| Brightness | 0x0f | 0x00-0x0f | - |

## Troubleshooting Konfigurasi

Jika sistem tidak bekerja setelah konfigurasi:
1. Cek Serial Monitor untuk error message
2. Verifikasi semua pin assignment
3. Test komponen satu per satu
4. Restore ke konfigurasi default
5. Cek power supply semua komponen
