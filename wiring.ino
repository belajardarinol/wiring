// ========================================
// TEMPTRON 607 A-C - Temperature Controller
// Menggunakan sensor suhu DHT11
// ========================================

#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TM1637Display.h>
#include <DHT.h>

// ========================================
// WiFi Configuration (COMMENTED FOR LATER)
// ========================================
// #include <WiFi.h>
// #include <ESPAsyncWebServer.h>
// #include "webpage.h"
// const char* ssid = "Sejahtera";
// const char* password = "presiden sekarang";
// AsyncWebServer server(80);

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
int relay_heater = 14;    // Relay untuk pemanas
int relay_cooler = 12;    // Relay untuk pendingin
int led_status1 = 13;     // LED status 1
int led_status2 = 5;      // LED status 2
int led_status3 = 23;     // LED status 3
int led_status4 = 19;     // LED status 4
int led_status5 = 18;     // LED status 5
int led_status6 = 2;      // LED status 6

// ========================================
// Global Variables
// ========================================
float currentTemp = 0;
float setpointTemp = 80.0;  // Default setpoint 80°C (range 60-100)
String inputCode = "";
bool isAuthenticated = false;
bool systemActive = false;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // Baca sensor setiap 2 detik

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
  display1.setBrightness(0x0f);
  display2.setBrightness(0x0f);
  display1.showNumberDec((int)setpointTemp, false);
  display2.showNumberDec(0, false);
  
  Serial.println("Inisialisasi selesai!");
  Serial.println("Setpoint default: 80°C (Range: 60-100°C)");
  Serial.println("Masukkan kode akses untuk memulai...");
  
  // ========================================
  // WiFi Setup (COMMENTED FOR LATER)
  // ========================================
  // Serial.println("Connecting to WiFi...");
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting...");
  // }
  // Serial.println("Connected to WiFi");
  // Serial.print("IP Address: ");
  // Serial.println(WiFi.localIP());
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

// Validasi kode akses (range 60-100)
bool validateCode(String code) {
  int codeNum = code.toInt();
  if (codeNum >= 60 && codeNum <= 100) {
    setpointTemp = (float)codeNum;
    return true;
  }
  return false;
}

// Kontrol suhu berdasarkan setpoint
void temperatureControl() {
  if (currentTemp < setpointTemp - 2) {
    // Suhu di bawah setpoint - Aktifkan Heater
    digitalWrite(relay_heater, HIGH);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(led_status1, HIGH);  // LED heater ON
    digitalWrite(led_status2, LOW);   // LED cooler OFF
    Serial.println("Status: HEATING");
  } 
  else if (currentTemp > setpointTemp + 2) {
    // Suhu di atas setpoint - Aktifkan Cooler
    digitalWrite(relay_heater, LOW);
    digitalWrite(relay_cooler, HIGH);
    digitalWrite(led_status1, LOW);   // LED heater OFF
    digitalWrite(led_status2, HIGH);  // LED cooler ON
    Serial.println("Status: COOLING");
  } 
  else {
    // Suhu optimal - Matikan semua
    digitalWrite(relay_heater, LOW);
    digitalWrite(relay_cooler, LOW);
    digitalWrite(led_status1, LOW);
    digitalWrite(led_status2, LOW);
    Serial.println("Status: OPTIMAL");
  }
}

// Update display
void updateDisplay() {
  display2.showNumberDec((int)currentTemp, false);
  if (inputCode.length() > 0) {
    display1.showNumberDec(inputCode.toInt(), false);
  } else {
    display1.showNumberDec((int)setpointTemp, false);
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
    
    // Cek apakah pengguna ingin menggunakan suhu/pengguna lain
    if (!isAuthenticated) {
      // Validasi kode input dari keypad
      if (key >= '0' && key <= '9') {
        inputCode += key;
        Serial.print("Input code: ");
        Serial.println(inputCode);
        display1.showNumberDec(inputCode.toInt(), false);
        
        // Jika sudah 2 digit, validasi
        if (inputCode.length() >= 2) {
          if (validateCode(inputCode)) {
            isAuthenticated = true;
            systemActive = true;
            Serial.println("Kode valid! Sistem aktif.");
            Serial.print("Setpoint diatur ke: ");
            Serial.print(setpointTemp);
            Serial.println("°C");
            digitalWrite(led_status3, HIGH);  // LED sistem aktif
          } else {
            Serial.println("Kode tidak valid! (Range: 60-100)");
            digitalWrite(led_status4, HIGH);  // LED error
            delay(1000);
            digitalWrite(led_status4, LOW);
          }
          inputCode = "";
        }
      }
      
      // Tombol C untuk cancel/reset
      if (key == 'C') {
        inputCode = "";
        Serial.println("Input direset");
        display1.showNumberDec((int)setpointTemp, false);
      }
    } 
    else {
      // Sistem sudah authenticated
      // Tombol untuk mengubah setpoint
      if (key >= '0' && key <= '9') {
        inputCode += key;
        display1.showNumberDec(inputCode.toInt(), false);
        if (inputCode.length() >= 2) {
          if (validateCode(inputCode)) {
            Serial.print("Setpoint diubah ke: ");
            Serial.print(setpointTemp);
            Serial.println("°C");
          } else {
            Serial.println("Setpoint tidak valid!");
          }
          inputCode = "";
        }
      }
      
      // Tombol D untuk stop sistem
      if (key == 'D') {
        systemActive = false;
        isAuthenticated = false;
        digitalWrite(relay_heater, LOW);
        digitalWrite(relay_cooler, LOW);
        digitalWrite(led_status1, LOW);
        digitalWrite(led_status2, LOW);
        digitalWrite(led_status3, LOW);
        Serial.println("Sistem dihentikan!");
      }
      
      // Tombol C untuk reset input
      if (key == 'C') {
        inputCode = "";
        display1.showNumberDec((int)setpointTemp, false);
      }
    }
  }
  
  // Ambil data suhu dari sensor (setiap 2 detik)
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    
    currentTemp = dht.readTemperature();
    
    // Cek apakah pembacaan sensor valid
    if (isnan(currentTemp)) {
      Serial.println("Error: Gagal membaca sensor DHT!");
      digitalWrite(led_status4, HIGH);  // LED error
      return;
    } else {
      digitalWrite(led_status4, LOW);
    }
    
    Serial.print("Suhu: ");
    Serial.print(currentTemp);
    Serial.print("°C | Setpoint: ");
    Serial.print(setpointTemp);
    Serial.println("°C");
    
    // Update display
    updateDisplay();
    
    // Cek apakah suhu di bawah setpoint (kontrol suhu)
    if (systemActive) {
      temperatureControl();
    }
  }
  
  delay(50);  // Small delay untuk stabilitas
}
