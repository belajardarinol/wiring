# Diagram Koneksi Hardware - TEMPTRON 607 A-C

## Koneksi ESP32

### Sensor DHT11
```
DHT11          ESP32
-----          -----
VCC    →       3.3V atau 5V
GND    →       GND
DATA   →       GPIO 27
```

### Display 7-Segment TM1637 #1 (Suhu Aktual)
```
Display 1      ESP32
---------      -----
VCC    →       5V
GND    →       GND
CLK    →       GPIO 26
DIO    →       GPIO 25
```

### Display 7-Segment TM1637 #2 (Setpoint)
```
Display 2      ESP32
---------      -----
VCC    →       5V
GND    →       GND
CLK    →       GPIO 17
DIO    →       GPIO 16
```

### Keypad I2C 4x4
```
Keypad I2C     ESP32
----------     -----
VCC    →       5V
GND    →       GND
SDA    →       GPIO 21 (default I2C SDA)
SCL    →       GPIO 22 (default I2C SCL)

I2C Address: 0x20
```

### Relay Module
```
Relay Heater   ESP32
------------   -----
VCC    →       5V (atau power eksternal)
GND    →       GND
IN     →       GPIO 14

Relay Cooler   ESP32
------------   -----
VCC    →       5V (atau power eksternal)
GND    →       GND
IN     →       GPIO 12
```

### LED Indikator
```
LED            ESP32         Fungsi
---            -----         ------
LED 1  →       GPIO 13       Heater ON
LED 2  →       GPIO 5        Cooler ON
LED 3  →       GPIO 23       Sistem Aktif
LED 4  →       GPIO 19       Error Sensor
LED 5  →       GPIO 18       Reserved
LED 6  →       GPIO 2        Reserved

Catatan: Gunakan resistor 220Ω - 330Ω untuk setiap LED
```

## Diagram Blok Sistem

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32                                │
│                                                               │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐              │
│  │  Keypad  │───▶│  Input   │───▶│  Logic   │              │
│  │  I2C     │    │  Handler │    │  Control │              │
│  └──────────┘    └──────────┘    └─────┬────┘              │
│                                         │                    │
│  ┌──────────┐    ┌──────────┐         │                    │
│  │  DHT11   │───▶│  Temp    │─────────┤                    │
│  │  Sensor  │    │  Reader  │         │                    │
│  └──────────┘    └──────────┘         │                    │
│                                         │                    │
│                                    ┌────▼────┐              │
│                                    │  Temp   │              │
│                                    │ Control │              │
│                                    └────┬────┘              │
│                                         │                    │
│  ┌──────────┐    ┌──────────┐    ┌────▼────┐              │
│  │ Display  │◀───│  Display │◀───│ Output  │              │
│  │ TM1637   │    │  Handler │    │ Handler │              │
│  └──────────┘    └──────────┘    └────┬────┘              │
│                                         │                    │
│  ┌──────────┐                          │                    │
│  │  Relay   │◀─────────────────────────┤                    │
│  │  Module  │                          │                    │
│  └──────────┘                          │                    │
│                                         │                    │
│  ┌──────────┐                          │                    │
│  │   LED    │◀─────────────────────────┘                    │
│  │ Indicator│                                               │
│  └──────────┘                                               │
└─────────────────────────────────────────────────────────────┘
```

## Catatan Penting

### Power Supply
- **ESP32**: 5V via USB atau Vin pin
- **DHT11**: 3.3V atau 5V (lebih stabil di 5V)
- **Display TM1637**: 5V
- **Keypad I2C**: 5V
- **Relay Module**: 5V (atau power eksternal untuk beban besar)
- **LED**: 3.3V dengan resistor pembatas

### Relay Module
⚠️ **PERHATIAN**: 
- Jika mengontrol beban AC (heater/cooler), gunakan relay yang sesuai rating
- Pastikan isolasi yang baik antara sisi kontrol (ESP32) dan sisi beban (AC)
- Gunakan power supply terpisah untuk relay jika beban besar
- Relay module biasanya aktif LOW (relay ON saat pin LOW)

### I2C Address
- Keypad I2C default: `0x20`
- Jika ada konflik, cek dengan I2C scanner
- Pastikan pull-up resistor ada di SDA dan SCL (biasanya sudah built-in di module)

### Grounding
- Pastikan semua GND terhubung ke common ground
- Gunakan kabel ground yang cukup tebal untuk relay

## Testing Koneksi

### Test I2C Scanner
```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  
  delay(5000);
}
```

### Test DHT11
```cpp
#include <DHT.h>

#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temp);
  delay(2000);
}
```
