#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TM1637Display.h>
#include <DHT.h>
#include "webpage.h"

// WiFi credentials
const char* ssid = "Sejahtera";
const char* password = "presiden sekarang";

// Create AsyncWebServer object on port 80
// AsyncWebServer server(80);

#define CLK1 26
#define DIO1 25
#define CLK2 17
#define DIO2 16

float temp;
float humy;
int num1;
int num2;

 int led1 = 14;
 int led2 = 12;
 int led3 = 13;
 int led4 = 5;
 int led5 = 23;
 int led6 = 19;
 int led7 = 18;
 int led8 =2;

#define DHTPIN 27   
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const uint8_t derajat[] = {
  SEG_A | SEG_F | SEG_B | SEG_G,           // d
  SEG_A | SEG_F | SEG_E | SEG_D  // O
  };

TM1637Display display1(CLK1, DIO1);
TM1637Display display2(CLK2, DIO2);
const byte ROWS = 4; // empat rows
const byte COLS = 4; // empat columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; 
byte colPins[COLS] = {4, 5, 6, 7};
int i2caddress = 0x20; // alamat PCF8574, semua pin A0 - A2 ke Ground.
Keypad_I2C kpd = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, i2caddress );

void setup(){
  Serial.begin(115200);
  Wire.begin();
  kpd.begin();
  dht.begin();
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Route to get sensor data
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temp\":" + String(temp, 1) + ",\"humy\":" + String(humy, 0) + "}";
    request->send(200, "application/json", json);
  });

  // Start server
  server.begin();
}
  
void loop(){
  temp = dht.readTemperature();
  humy = dht.readHumidity();
  display1.setBrightness(0x0f);
  display2.setBrightness(0x0f);
  display1.showNumberDec(temp, false, 2,0);
  display1.showNumberDec(num1, false, 2,2);
  display2.showNumberDec(humy, false, 2,0);
  display2.showNumberDec(num2, false, 2,2);
  
  char key = kpd.getKey();
  if (key == '1'){
      num1=1;
      digitalWrite(led1, HIGH);
    }
  if (key == '2'){
      num1=2;
      digitalWrite(led2, HIGH);
    }
  if (key == '3'){
      num1=3;
      digitalWrite(led3, HIGH);
    }
  if (key == '4'){
      num1=4;
      digitalWrite(led4, HIGH);
    }
  if (key == '5'){
      num2=5;
      digitalWrite(led5, HIGH);
    }
  if (key == '6'){
      num2=6;
      digitalWrite(led6, HIGH);
    }
  if (key == '7'){
      num2=7;
      digitalWrite(led7, HIGH);
    }
  if (key == '8'){
      num2=8;
      digitalWrite(led8, HIGH);
    }
  if (key == '9'){
      num2=9;
    }
  if (key == '0'){
      num2=0;
    }
  if (key == 'A'){
     digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
     digitalWrite(led5, LOW);
      digitalWrite(led6, LOW);
      digitalWrite(led7, LOW);
      digitalWrite(led8, LOW);
    }
}
