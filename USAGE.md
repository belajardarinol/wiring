# Panduan Penggunaan — TEMPTRON 607 A-C

## Ringkas
- Perangkat: ESP32 + DHT11 + 2x TM1637 + Keypad I2C + 2 Relay + LED indikator
- Setpoint: 20–100°C (2 digit). Histeresis ±2°C.
- Mode kerja: Otomatis (HEATING/COOLING/OPTIMAL) berdasarkan setpoint.
- Sistem menu: 3 menu (Setpoint, Batas Atas, Batas Bawah) dengan alarm.

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
2. Masukkan setpoint 2 digit pada keypad (20–100) untuk mengaktifkan sistem.
   - Contoh: Tekan `3` lalu `0` untuk setpoint 30°C.
3. Perangkat membaca suhu tiap 2 detik dan mengatur relay:
   - Suhu < setpoint − 2°C → HEATER ON
   - Suhu > setpoint + 2°C → COOLER ON
   - Dalam rentang ±2°C → Keduanya OFF (OPTIMAL)
4. Alarm akan berbunyi (LED Status 5) jika suhu keluar dari batas atas/bawah.

## Tampilan dan Kontrol

### Mode Normal (MENU_IDLE)
- **Display 1 (besar)**: Setpoint suhu
- **Display 2 (kecil)**: Suhu aktual dari sensor
- **Keypad**:
  - `0–9`: Quick edit setpoint (2 digit, langsung aktifkan sistem)
  - `A`: Masuk ke mode menu
  - `C`: Reset input
  - `D`: Hentikan sistem

### Mode Browse Menu (MENU_BROWSE)
- **Display 1**: Nomor menu (01, 02, 03)
- **Display 2**: Nilai setting saat ini
- **Keypad**:
  - `A`: Next menu (01 → 02 → 03 → 01)
  - `B`: Pilih menu untuk edit
  - `C`: Kembali ke mode normal

### Mode Edit Menu
- **Display 1**: Input angka baru
- **Display 2**: Nilai lama
- **Keypad**:
  - `0–9`: Input nilai baru (2 digit, otomatis simpan)
  - `C`: Cancel, kembali ke browse menu

## Daftar Menu (16 Menu)
1. **Menu 01**: Setpoint Suhu (20–100°C) - Target suhu yang diinginkan
2. **Menu 02**: Heat Temperature (20–100°C) - Suhu aktifkan heater
3. **Menu 03**: Cool Temperature (20–100°C) - Suhu aktifkan cooler
4. **Menu 04**: Fan 1 Control (0=OFF, 1=ON) - Relay tambahan untuk fan 1
5. **Menu 05**: Fan 2 Control (0=OFF, 1=ON) - Relay tambahan untuk fan 2
6. **Menu 06**: Fan 3 Control (0=OFF, 1=ON) - Relay tambahan untuk fan 3
7. **Menu 07**: Fan 4 Control (0=OFF, 1=ON) - Relay tambahan untuk fan 4
8. **Menu 08**: Timer ON (0–99 menit) - Timer untuk auto ON sistem
9. **Menu 09**: Timer OFF (0–99 menit) - Timer untuk auto OFF sistem
10. **Menu 10**: Humidity Setpoint (0–99%) - Target kelembaban (DHT11)
11. **Menu 11**: Cool On Delay (0–99 detik) - Delay sebelum cooler ON
12. **Menu 12**: Cool Off Delay (0–99 detik) - Delay sebelum cooler OFF
13. **Menu 13**: Low Alarm (20–100°C) - Batas bawah alarm (harus < batas atas)
14. **Menu 14**: High Alarm (20–100°C) - Batas atas alarm (harus > batas bawah)
15. **Menu 15**: Alarm Delay (0–99 detik) - Delay sebelum alarm bunyi
16. **Menu 16**: Display Brightness (0–15) - Kecerahan display (0=redup, 15=terang)

## LED Indikator
- LED Status 1 (pin 13): Heater ON
- LED Status 2 (pin 5): Cooler ON
- LED Status 3 (pin 23): Sistem aktif
- LED Status 4 (pin 19): Error sensor
- LED Status 5 (pin 18): **ALARM** - Suhu di luar batas
- LED Status 6 (pin 2): Cadangan

## Contoh Skenario

### Skenario 1: Quick Start (Mode Cepat)
1. Tekan `3` lalu `0` → Setpoint 30°C, sistem langsung aktif.
2. Display 1 menampilkan `30`, Display 2 menampilkan suhu aktual.
3. Ubah setpoint cepat: ketik `2` `8` → Setpoint berubah ke 28°C.
4. Hentikan sistem: tekan `D`.

### Skenario 2: Menggunakan Menu System
1. Dari mode normal, tekan `A` → Masuk mode menu.
2. Display 1 menampilkan `01`, Display 2 menampilkan setpoint saat ini (30).
3. Tekan `A` beberapa kali untuk browse menu → `02`, `03`, ... `16`, lalu kembali ke `01`.
4. Misal di menu `14` (High Alarm), tekan `B` untuk edit.
5. Ketik `4` `0` → Batas atas alarm diubah ke 40°C.
6. Sistem otomatis kembali ke browse menu.
7. Tekan `A` untuk menu `13` (Low Alarm), tekan `B`, ketik `2` `0` → Batas bawah 20°C.
8. Tekan `A` untuk menu `16` (Brightness), tekan `B`, ketik `0` `8` → Display redup.
9. Tekan `C` untuk kembali ke mode normal.
10. Jika suhu keluar dari rentang 20-40°C, LED Status 5 akan menyala (ALARM).

### Skenario 3: Setting Fan Control
1. Tekan `A` → Masuk menu, tekan `A` sampai menu `04` (Fan 1).
2. Tekan `B` untuk edit, ketik `0` `1` → Fan 1 enabled.
3. Ulangi untuk menu 05-07 jika ingin enable fan lain.
4. **Catatan**: Anda perlu tambahkan relay fisik untuk fan di pin yang tersedia.

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
