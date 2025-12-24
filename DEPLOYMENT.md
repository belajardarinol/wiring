# 🚀 Deployment Guide - TempTron 607 A-C

## 📋 Struktur File

```
wiring/
├── wiring.ino              # Kode ESP32 (Firmware Utama)
├── webpage.h               # Embedded Web Page (HTML/JS/CSS dalam header file)
├── index.php               # Dashboard monitoring web remote (Opsional)
├── index2.php              # Interface ala controller fisik (Opsional)
└── ...
```

---

## 📡 Setup ESP32 (Web Server Lokal)

Fitur Web Interface lokal sekarang **sudah diaktifkan** langsung di dalam ESP32. Anda bisa mengakses dashboard monitoring dan kontrol langsung dari IP Address ESP32 tanpa perlu hosting/server terpisah.

### 1. Persiapan Library

Di Arduino IDE, buka **Sketch -> Include Library -> Manage Libraries** dan install:

1.  **ESPAsyncWebServer** - https://github.com/me-no-dev/ESPAsyncWebServer
    *   *Note: Mungkin perlu download .zip dari GitHub jika tidak ada di Library Manager.*
2.  **AsyncTCP** - https://github.com/me-no-dev/AsyncTCP
    *   *Note: Diperlukan oleh ESPAsyncWebServer.*
3.  **DHT sensor library** by Adafruit
4.  **TM1637** by Avishay Orpaz
5.  **Keypad_I2C** by Joe Young

### 2. Konfigurasi WiFi

Pastikan kredensial WiFi di file `wiring.ino` sudah benar:

```cpp
// wiring.ino baris 23-24
const char *ssid = "Sejahtera";              // Ganti dengan nama WiFi Anda
const char *password = "presiden sekarang";  // Ganti dengan password WiFi Anda
```

### 3. Upload Firmware

1.  Buka `wiring.ino` dengan Arduino IDE.
2.  Pilih Board: **ESP32 Dev Module**.
3.  Klik tombol **Upload**.
4.  Setelah selesai, buka **Serial Monitor** (Baud rate: 115200).
5.  Tekan tombol Reset (EN) di board ESP32.
6.  Tunggu hingga muncul pesan:
    ```
    Connected to WiFi
    IP Address: 192.168.1.X
    Web Server started!
    ```

### 4. Akses Web Interface

1.  Catat **IP Address** yang muncul di Serial Monitor (misal: `192.168.1.15`).
2.  Buka browser di HP atau Laptop yang terhubung ke WiFi yang sama.
3.  Ketik alamat IP tersebut di address bar (contoh: `http://192.168.1.15`).
4.  Anda akan melihat **Dashboard TempTron 607** modern.

---

## 🎮 Kontrol Manual (Testing)

Di dalam Web Interface, Anda dapat menggunakan tombol-tombol di bagian **Manual Control** untuk menyalakan/mematikan Relay secara langsung.

*   Klik tombol **Fan 1** - **Fan 6** atau **Heater** / **Cooling**.
*   Tombol akan berubah warna menjadi terang jika aktif.
*   Cek respon fisik di modul Relay.

---

## 🌐 API Endpoints (Untuk Developer)

ESP32 menyediakan API JSON sederhana untuk integrasi lain:

*   **GET /status**
    *   Returns JSON: Telemetry suhu, config setpoint, dan status real-time semua relay.
*   **GET /update_config?setpoint=30**
    *   Mengubah setpoint suhu target.
*   **POST /manual**
    *   Mengirim perintah kontrol manual (JSON body).

Selamat mencoba! 🚀
