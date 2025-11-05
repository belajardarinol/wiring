# TEMPTRON 607 A-C - Temperature Controller

## Deskripsi
Sistem kontrol suhu otomatis yang meniru Temptron 607 A-C menggunakan ESP32, sensor DHT11, dan kontrol relay untuk pemanas/pendingin. Sistem ini mengikuti flowchart kontrol suhu dengan autentikasi kode akses.

## Fitur Utama
- ✅ **Kontrol Suhu Otomatis** - Mengatur pemanas/pendingin berdasarkan setpoint
- ✅ **Sensor DHT11** - Monitoring suhu real-time
- ✅ **Autentikasi Kode** - Kode akses 60-100 (sekaligus setpoint suhu)
- ✅ **Display 7-Segment** - Menampilkan suhu aktual dan setpoint
- ✅ **Keypad 4x4** - Input kode dan kontrol sistem
- ✅ **Relay Control** - Kontrol heater dan cooler
- ✅ **LED Indikator** - Status sistem (heating/cooling/optimal/error)
- 🔜 **Web Interface** - Monitoring via browser (akan diaktifkan nanti)

## Hardware yang Dibutuhkan
- ESP32 Development Board
- DHT11 Temperature Sensor (pin 27)
- 2x TM1637 7-Segment Display
  - Display 1 (CLK: 26, DIO: 25) - Suhu Aktual
  - Display 2 (CLK: 17, DIO: 16) - Setpoint
- Keypad I2C 4x4 (alamat 0x20)
- 2x Relay Module
  - Relay Heater (pin 14)
  - Relay Cooler (pin 12)
- 6x LED Indikator
  - LED Status 1-6 (pin 13, 5, 23, 19, 18, 2)

## Library yang Diperlukan
Install library berikut melalui Arduino Library Manager:
1. **Keypad_I2C** - Untuk keypad I2C
2. **Keypad** - Library keypad standar
3. **Wire** - I2C communication (built-in)
4. **TM1637Display** - Untuk 7-segment display
5. **DHT sensor library** - Untuk sensor DHT11

### Library untuk Web Interface (Nanti):
6. **WiFi** - Built-in ESP32
7. **ESPAsyncWebServer** - https://github.com/me-no-dev/ESPAsyncWebServer
8. **AsyncTCP** - https://github.com/me-no-dev/AsyncTCP

## Cara Instalasi

### 1. Install Library
```
Arduino IDE -> Sketch -> Include Library -> Manage Libraries
```
Cari dan install library 1-5 yang diperlukan di atas.

### 2. Upload ke ESP32
1. Buka file `wiring.ino` di Arduino IDE
2. Pilih board: **ESP32 Dev Module**
3. Pilih port yang sesuai
4. Klik **Upload**

### 3. Monitoring via Serial Monitor
1. Buka Serial Monitor (115200 baud)
2. Sistem akan menampilkan status inisialisasi
3. Monitor suhu dan status kontrol real-time

### 4. Web Interface (Untuk Nanti)
Fitur web interface sudah dikomentar dalam kode. Untuk mengaktifkan:
1. Uncomment bagian WiFi di `wiring.ino`
2. Ubah kredensial WiFi sesuai jaringan Anda
3. Upload ulang ke ESP32

## Cara Penggunaan

### Flowchart Sistem
Sistem mengikuti flowchart Temptron 607 A-C:

1. **Mulai/Nyala Ulang** → Sistem inisialisasi
2. **Input Kode Akses** → Masukkan 2 digit (60-100)
3. **Validasi Kode** → Jika valid, sistem aktif dengan setpoint sesuai kode
4. **Monitoring Suhu** → Sensor membaca suhu setiap 2 detik
5. **Kontrol Otomatis**:
   - Jika suhu < setpoint-2°C → Aktifkan HEATER
   - Jika suhu > setpoint+2°C → Aktifkan COOLER
   - Jika suhu optimal → Matikan semua relay

### Fungsi Keypad

#### Mode Belum Authenticated:
- **0-9**: Input kode akses (2 digit, range 60-100)
- **C**: Reset/cancel input

#### Mode Sudah Authenticated:
- **0-9**: Ubah setpoint (2 digit, range 60-100)
- **C**: Reset input
- **D**: Stop sistem dan logout

### Contoh Penggunaan:
1. Nyalakan sistem
2. Tekan `8` `0` → Setpoint 80°C, sistem aktif
3. Display 1 menampilkan suhu aktual
4. Display 2 menampilkan setpoint (80)
5. LED indikator menunjukkan status:
   - LED 1: Heater aktif
   - LED 2: Cooler aktif
   - LED 3: Sistem aktif
   - LED 4: Error sensor
6. Untuk ubah setpoint, tekan 2 digit baru (misal: `7` `5` untuk 75°C)
7. Untuk stop, tekan `D`

## LED Indikator
- **LED Status 1 (pin 13)**: Heater ON
- **LED Status 2 (pin 5)**: Cooler ON
- **LED Status 3 (pin 23)**: Sistem Aktif
- **LED Status 4 (pin 19)**: Error Sensor
- **LED Status 5 (pin 18)**: Reserved
- **LED Status 6 (pin 2)**: Reserved

## Troubleshooting

### Sensor DHT11 tidak terbaca
- Pastikan koneksi pin 27 ke DHT11 benar
- Cek power supply DHT11 (3.3V atau 5V)
- LED Status 4 akan menyala jika ada error sensor

### Display tidak menampilkan angka
- Cek koneksi CLK dan DIO ke display
- Pastikan display mendapat power yang cukup
- Coba adjust brightness di kode

### Keypad tidak merespon
- Cek alamat I2C (default 0x20)
- Pastikan koneksi SDA dan SCL benar
- Test dengan I2C scanner

### Relay tidak aktif
- Cek koneksi relay ke pin yang benar
- Pastikan relay module mendapat power eksternal jika diperlukan
- Cek logika relay (HIGH/LOW aktif)

### Sistem tidak masuk mode authenticated
- Pastikan input kode dalam range 60-100
- Cek Serial Monitor untuk debug message
- Tekan C untuk reset input jika salah

## Spesifikasi Teknis
- **Range Setpoint**: 60°C - 100°C
- **Hysteresis**: ±2°C
- **Update Rate**: 2 detik
- **Sensor**: DHT11 (akurasi ±2°C)
- **Komunikasi**: I2C (Keypad), Serial (Debug)
- **Power**: 5V via USB atau eksternal

## Pengembangan Selanjutnya
- [ ] Aktivasi web interface untuk monitoring remote
- [ ] Tambah sensor suhu lebih akurat (DS18B20, MAX6675)
- [ ] Data logging ke SD card
- [ ] Alarm buzzer untuk suhu kritis
- [ ] PID control untuk kontrol lebih presisi
- [ ] Multi-zone temperature control

## Lisensi
Open source - bebas digunakan dan dimodifikasi

## Catatan
Kode ini dibuat mengikuti flowchart Temptron 607 A-C dengan fokus pada kontrol suhu menggunakan sensor DHT11. Fitur web interface sudah disiapkan namun dikomentar untuk implementasi nanti.
