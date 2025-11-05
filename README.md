# TempTron 607 A-C Controller - ESP32 Web Interface

## Deskripsi
Proyek ini menggabungkan kontrol hardware ESP32 dengan interface web untuk monitoring dan kontrol sistem TempTron 607 A-C Controller.

## Fitur
- Monitoring suhu dan kelembaban real-time via DHT11
- Display 7-segment TM1637 untuk menampilkan data
- Keypad I2C untuk input
- Kontrol 8 LED
- Web interface yang dapat diakses melalui browser
- Update data sensor otomatis setiap 2 detik di web interface

## Hardware yang Dibutuhkan
- ESP32
- DHT11 sensor (pin 27)
- 2x TM1637 Display
- Keypad I2C (alamat 0x20)
- 8x LED (pin 14, 12, 13, 5, 23, 19, 18, 2)

## Library yang Diperlukan
Install library berikut melalui Arduino Library Manager:
1. **WiFi** (built-in ESP32)
2. **ESPAsyncWebServer** - https://github.com/me-no-dev/ESPAsyncWebServer
3. **AsyncTCP** - https://github.com/me-no-dev/AsyncTCP
4. **Keypad_I2C**
5. **Keypad**
6. **Wire** (built-in)
7. **TM1637Display**
8. **DHT sensor library**

## Cara Instalasi

### 1. Install Library
```
Arduino IDE -> Sketch -> Include Library -> Manage Libraries
```
Cari dan install semua library yang diperlukan di atas.

### 2. Konfigurasi WiFi
Edit file `wiring.ino` dan ubah kredensial WiFi:
```cpp
const char* ssid = "YOUR_WIFI_SSID";        // Ganti dengan nama WiFi Anda
const char* password = "YOUR_WIFI_PASSWORD"; // Ganti dengan password WiFi Anda
```

### 3. Upload ke ESP32
1. Pilih board ESP32 di Arduino IDE
2. Pilih port yang sesuai
3. Klik Upload

### 4. Akses Web Interface
1. Buka Serial Monitor (115200 baud)
2. Tunggu hingga ESP32 terhubung ke WiFi
3. Catat IP Address yang ditampilkan
4. Buka browser dan akses: `http://[IP_ADDRESS]`
   Contoh: `http://192.168.1.100`

## Struktur File
```
wiring/
├── wiring.ino      # File utama Arduino
├── webpage.h       # File HTML interface (embedded)
└── README.md       # Dokumentasi ini
```

## API Endpoints

### GET /
Menampilkan web interface utama

### GET /data
Mengembalikan data sensor dalam format JSON:
```json
{
  "temp": 28.4,
  "humy": 65
}
```

## Fungsi Keypad
- **1-4**: Kontrol LED 1-4 dan set num1
- **5-8**: Kontrol LED 5-8 dan set num2
- **9-0**: Set num2
- **A**: Matikan semua LED

## Troubleshooting

### ESP32 tidak terhubung ke WiFi
- Pastikan SSID dan password benar
- Pastikan WiFi dalam jangkauan
- Cek Serial Monitor untuk pesan error

### Web interface tidak muncul
- Pastikan ESP32 sudah terhubung ke WiFi
- Cek IP address di Serial Monitor
- Pastikan komputer/HP dalam jaringan WiFi yang sama

### Library tidak ditemukan
- Install semua library yang diperlukan
- Restart Arduino IDE setelah instalasi

## Lisensi
Open source - bebas digunakan dan dimodifikasi

## Kontak
Untuk pertanyaan dan support, silakan buat issue di repository ini.
