# 🚀 Deployment Guide - TempTron 607 A-C

## 📋 Struktur File

```
wiring/
├── wiring.ino              # Kode ESP32
├── index.php               # Dashboard web (NEW!)
├── index.html              # UI alternatif (static)
├── webpage.h               # Embedded web page untuk ESP32
├── api/
│   ├── telemetry.php       # Endpoint untuk terima data dari ESP32
│   ├── config.php          # Endpoint untuk update/get konfigurasi
│   └── status.php          # Endpoint untuk dashboard (gabungan telemetry + config)
└── data/                   # Folder untuk simpan data (auto-created)
    ├── telemetry.json      # Data terakhir dari sensor
    ├── config.json         # Konfigurasi setpoint, dll
    └── telemetry-log.jsonl # Log history (opsional)
```

---

## 🌐 Setup Web Server (PHP)

### Opsi 1: Hosting Lokal (XAMPP/MAMP/WAMP)

1. **Install XAMPP/MAMP** (sudah include Apache + PHP + MySQL)
2. **Copy folder `wiring`** ke:
   - Windows: `C:\xampp\htdocs\wiring`
   - Mac: `/Applications/MAMP/htdocs/wiring`
   - Linux: `/var/www/html/wiring`

3. **Buat folder `data`** (kalau belum ada):
   ```bash
   mkdir data
   chmod 775 data
   ```

4. **Start Apache** dari XAMPP/MAMP control panel

5. **Akses dashboard**:
   - Browser: `http://localhost/wiring/index.php`
   - Atau: `http://192.168.x.x/wiring/index.php` (dari device lain di jaringan lokal)

### Opsi 2: Hosting Online (VPS/Shared Hosting)

1. **Upload semua file** via FTP/cPanel File Manager ke folder `public_html/`

2. **Set permission folder `data`**:
   ```bash
   chmod 775 data
   ```

3. **Update URL di ESP32** (`wiring.ino` line 23-24):
   ```cpp
   const char* apiTelemetry = "http://your-domain.com/api/telemetry.php";
   const char* apiConfig = "http://your-domain.com/api/config.php";
   ```

4. **Akses dashboard**:
   - `http://your-domain.com/index.php`

---

## 📡 Setup ESP32

### 1. Update WiFi Credentials

Edit `wiring.ino` line 20-21:

```cpp
const char* ssid = "Sejahtera";              // ← Ganti dengan SSID WiFi kamu
const char* password = "presiden sekarang";  // ← Ganti dengan password WiFi
```

### 2. Update API Endpoints

Edit `wiring.ino` line 23-24:

```cpp
// Untuk hosting lokal:
const char* apiTelemetry = "http://192.168.1.100/wiring/api/telemetry.php";
const char* apiConfig = "http://192.168.1.100/wiring/api/config.php";

// Untuk hosting online:
const char* apiTelemetry = "http://your-domain.com/api/telemetry.php";
const char* apiConfig = "http://your-domain.com/api/config.php";
```

**Catatan:** Ganti `192.168.1.100` dengan IP address komputer yang menjalankan web server.

### 3. Upload ke ESP32

1. Install library yang diperlukan di Arduino IDE:
   - `DHT sensor library` by Adafruit
   - `Adafruit Unified Sensor`
   - `TM1637` by Avishay Orpaz
   - `Keypad` by Mark Stanley
   - `Keypad_I2C` by Joe Young

2. Pilih board: **ESP32 Dev Module**

3. Upload kode `wiring.ino`

4. Buka Serial Monitor (115200 baud) untuk lihat:
   - Status koneksi WiFi
   - IP address ESP32
   - Data suhu real-time
   - Status heating/cooling

---

## 🎯 Cara Pakai Dashboard

### Akses Dashboard

1. Buka browser: `http://localhost/wiring/index.php` atau `http://your-domain.com/index.php`

2. Dashboard akan menampilkan:
   - **Current Status**: Suhu, humidity, setpoint real-time
   - **Control Panel**: Form untuk update setpoint dari web
   - **Cooling Status**: Indikator LED kipas 1-6 (bertingkat)
   - **Heating Status**: Indikator LED heater 7-8 (bertingkat)
   - **Alarm Status**: Indikator alarm + range batas suhu
   - **System Info**: Status koneksi device

### Update Setpoint dari Web

1. Masukkan nilai setpoint baru (20-100°C) di form **Control Panel**
2. Klik **Update Setpoint**
3. ESP32 akan otomatis fetch config baru dalam 30 detik (atau restart ESP32 untuk langsung apply)

---

## 🔧 Troubleshooting

### ESP32 tidak bisa konek WiFi

- Cek SSID & password di kode
- Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
- Cek jarak ESP32 ke router

### Dashboard tidak muncul data

1. **Cek folder `data` sudah dibuat** dan punya permission 775
2. **Test API manual**:
   ```bash
   # Test telemetry POST
   curl -X POST http://localhost/wiring/api/telemetry.php \
     -H "Content-Type: application/json" \
     -d '{"device_id":"test","temp":28.5,"humidity":65,"setpoint":30}'
   
   # Test status GET
   curl http://localhost/wiring/api/status.php
   ```

3. **Cek Serial Monitor ESP32**:
   - Apakah ada pesan "Connected to WiFi"?
   - Apakah ada error saat POST telemetry?

### LED tidak nyala sesuai logika

- Cek wiring fisik relay/LED ke pin yang benar
- Cek Serial Monitor untuk lihat status "COOLING LEVEL X" atau "HEATING LEVEL X"
- Pastikan `systemActive = true` (tekan angka 2 digit di keypad untuk aktivasi)

### Alarm tidak bunyi

- Logika alarm sekarang hanya nyalakan LED 1-6 bersamaan
- Kalau mau buzzer, tambahkan pin buzzer dan update fungsi `checkAlarm()`

---

## 📊 Monitoring & Logging

### Lihat Log History

File `data/telemetry-log.jsonl` menyimpan semua data yang masuk (format JSON Lines).

Contoh baca log:

```bash
tail -f data/telemetry-log.jsonl
```

Atau buat script PHP untuk visualisasi grafik (opsional).

---

## 🔐 Security Tips (Untuk Production)

1. **Tambahkan autentikasi** di dashboard (login user/password)
2. **Gunakan HTTPS** (SSL certificate) untuk hosting online
3. **Batasi akses API** dengan API key atau IP whitelist
4. **Backup data** secara berkala

---

## 📞 Support

Kalau ada masalah, cek:

1. **Serial Monitor ESP32** untuk debug koneksi & sensor
2. **Browser Console** (F12) untuk debug JavaScript
3. **Apache error log** untuk debug PHP:
   - XAMPP: `C:\xampp\apache\logs\error.log`
   - Linux: `/var/log/apache2/error.log`

---

## 🎉 Selesai!

Sistem sudah siap dipakai:

- ✅ ESP32 baca sensor DHT11 setiap 2 detik
- ✅ Kontrol heater/cooler bertingkat otomatis
- ✅ Kirim data ke server setiap 10 detik
- ✅ Dashboard web real-time monitoring
- ✅ Remote control setpoint dari web
- ✅ Alarm visual (LED kedip) kalau suhu keluar batas

**Enjoy your smart temperature controller! 🌡️🔥❄️**
