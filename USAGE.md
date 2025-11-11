# Panduan Penggunaan — TEMPTRON 607 A-C

## Ringkas
- Perangkat: ESP32 + DHT11 + 2x TM1637 + Keypad I2C + 2 Relay + LED indikator
- Setpoint: 60–100°C (2 digit). Histeresis ±2°C.
- Mode kerja: Otomatis (HEATING/COOLING/OPTIMAL) berdasarkan setpoint.

## Persiapan
- Pastikan komponen terpasang sesuai WIRING_DIAGRAM.md
- Instal library (Arduino Library Manager):
  - Keypad_I2C
  - Keypad
  - TM1637Display
  - DHT sensor library
- Board: ESP32 Dev Module

## Upload Firmware
1. Buka `wiring.ino` di Arduino IDE.
2. Pilih board: ESP32 Dev Module dan port yang sesuai.
3. Upload.
4. Buka Serial Monitor pada 115200 baud untuk melihat status.

## Operasional Dasar
1. Nyalakan perangkat.
2. Masukkan kode akses 2 digit pada keypad (60–100) untuk mengaktifkan sistem.
   - Kode sekaligus menjadi setpoint suhu.
3. Perangkat membaca suhu tiap 2 detik dan mengatur relay:
   - Suhu < setpoint − 2°C → HEATER ON
   - Suhu > setpoint + 2°C → COOLER ON
   - Dalam rentang ±2°C → Keduanya OFF (OPTIMAL)

## Tampilan dan Kontrol
- Display 1: Setpoint (atau input sementara saat mengetik kode)
- Display 2: Suhu aktual
- Keypad:
  - Mode belum authenticated:
    - 0–9: Input kode (2 digit)
    - C: Reset/cancel input
  - Mode sudah authenticated:
    - 0–9: Ubah setpoint (2 digit)
    - C: Reset input
    - D: Hentikan sistem dan logout

## LED Indikator
- LED Status 1 (pin 13): Heater ON
- LED Status 2 (pin 5): Cooler ON
- LED Status 3 (pin 23): Sistem aktif
- LED Status 4 (pin 19): Error sensor
- LED Status 5 (pin 18): Cadangan
- LED Status 6 (pin 2): Cadangan

## Contoh Skenario
1. Tekan `8` lalu `0` → Setpoint 80°C, sistem aktif.
2. Amati Display 2 untuk suhu aktual, Display 1 untuk setpoint.
3. Ubah setpoint: ketik dua digit baru (mis. `7` `5`).
4. Hentikan sistem: tekan `D`.

## Troubleshooting Singkat
- DHT11 tidak terbaca: cek kabel, tegangan, perhatikan LED Status 4.
- Display kosong/aneh: cek pin CLK/DIO, suplai, dan brightness di kode.
- Keypad tidak respons: cek alamat I2C (default 0x20) dan koneksi SDA/SCL.
- Relay tidak aktif: cek pin, suplai eksternal relay, dan logika HIGH/LOW.

## Web Interface (opsional, nanti)
- Fitur WiFi/HTTP sudah disiapkan dan dikomentari di `wiring.ino`.
- Untuk mengaktifkan, ikuti bagian "Konfigurasi Web Interface" di `CONFIGURATION.md`.

## Referensi
- Instalasi dan informasi lengkap: `README.md`
- Pengaturan/penyesuaian: `CONFIGURATION.md`
- Diagram koneksi: `WIRING_DIAGRAM.md`
