# 🎮 Panduan Operasi & Uji Coba Temptron 607 (Clone)

Panduan ini menjelaskan cara kerja logika Temptron 607 A-C yang telah diprogram ke dalam ESP32, serta instruksi langkah demi langkah untuk menguji keypad dan display.

---

## 🎹 1. Pemetaan Keypad (Mapping)

Karena kita menggunakan Keypad 4x4 standar (A-D), tombol-tombolnya dipetakan agar sesuai dengan fungsi asli Temptron:

| Tombol Fisik (4x4) | Fungsi Temptron Asli | Keterangan |
| :---: | :---: | :--- |
| **A** | **DATA** | Pindah ke menu/fungsi berikutnya (Next) |
| **B** | **PROG** | Masuk/Keluar Mode Edit (Program) |
| **C** | **CLEAR** | Batal / Reset input / Kembali ke Home (Idle) |
| **D** | **ENTER** | Simpan nilai setting (Confirm) |
| **0-9** | **Angka** | Input angka & Shortcut Navigasi |

---

## 📺 2. Perilaku Display (Layar)

Sistem menggunakan 2 layar 7-Segment (TM1637):

### A. Mode IDLE (Standby)
Kondisi normal saat tidak menekan tombol apa-apa.
*   **Layar Kiri (Besar):** Menampilkan **SUHU AKTUAL** (Contoh: `28`).
*   **Layar Kanan (Kecil):** **MATI / KOSONG** (Ini tanda sistem standby).

### B. Mode MENU (Browse)
Saat tombol **A (DATA)** ditekan.
*   **Layar Kiri (Besar):** Menampilkan **NILAI SETTING** saat ini.
*   **Layar Kanan (Kecil):** Menampilkan **NOMOR FUNGSI** (01, 02, dst).

### C. Mode EDIT (Program)
Saat tombol **B (PROG)** ditekan di dalam menu.
*   **Layar Kiri (Besar):** Menampilkan **ANGKA YANG DIKETIK** (akan berubah saat Anda ketik).
*   **Layar Kanan (Kecil):** Tetap menampilkan **NOMOR FUNGSI**.

---

## 🧪 3. Skenario Uji Coba

Ikuti langkah-langkah ini untuk memastikan sistem berjalan persis seperti Temptron asli.

### 🟢 Uji 1: Navigasi Menu (Tombol DATA)
1.  Pastikan layar dalam posisi IDLE (Kiri: Suhu, Kanan: Mati).
2.  Tekan **A** (DATA).
    *   *Hasil:* Layar Kanan muncul `01`. Layar kiri muncul nilai setpoint (misal `30`).
3.  Tekan **A** lagi berkali-kali.
    *   *Hasil:* Layar Kanan berubah `02` -> `03` -> ... -> `16` -> `01`.
4.  Tekan **C** (CLEAR).
    *   *Hasil:* Kembali ke Mode IDLE (Kanan mati).

---

### 🟢 Uji 2: Mengubah Setpoint (Tombol PROG & ENTER)
Kita akan mengubah target suhu (Menu 01) dari 30°C menjadi 25°C.

1.  Tekan **A** satu kali (Masuk Menu 01).
    *   *Display:* `30` | `01`
2.  Tekan **B** (PROG).
    *   *Display:* Layar Kiri jadi `0` (Siap ketik). Led indikator mungkin berkedip (opsional).
3.  Ketik angka **2** lalu **5**.
    *   *Display:* `25` | `01`
4.  Tekan **D** (ENTER).
    *   *Hasil:* Angka tersimpan. Layar berkedip sebentar sebagai konfirmasi.
5.  Tekan **C** (CLEAR) untuk kembali ke awal.
    *   *Cek:* Sekarang Layar Kiri (Suhu Aktual) mungkin memicu Relay Heater/Cooler karena setpoint berubah.

---

### 🟢 Uji 3: Shortcut Navigasi (Tombol Angka di Menu Awal)
Fitur khas Temptron: Loncat menu dengan cepat tanpa tekan DATA berkali-kali.

1.  Posisi IDLE.
2.  Tekan angka **0** lalu **8** (shortcut ke Timer ON).
3.  *Hasil:* Sistem langsung masuk ke Menu 08.
    *   *Display:* (Nilai Timer) | `08`
4.  Tekan **C** untuk kembali.

---

### 🟢 Uji 4: Fitur Batal (Tombol CLEAR/PROG)
Menguji jika kita salah ketik.

1.  Masuk Menu 01 (Tekan A).
2.  Tekan **B** (PROG/Edit).
3.  Ketik angka ngawur (misal `99`).
4.  JANGAN tekan ENTER.
5.  Tekan **B** lagi (atau C).
    *   *Hasil:* Batal simpan. Nilai kembali ke `25` (nilai lama).

---

## 📋 4. Daftar Fungsi (Menu Map)

Berikut daftar fungsi yang tersedia di kode `wiring.ino` Anda:

| No | Fungsi | Range | Deskripsi |
| :--: | :--- | :--- | :--- |
| **01** | Required Temp | 20-100 | Target suhu yang diinginkan (Setpoint) |
| **02** | Heat Temp | 20-100 | Batas suhu nyalahin Heater |
| **03** | Cool Temp | 20-100 | Batas suhu nyalahin Cooler utama |
| **04** | Fan 1 Mode | 0/1 | Enable/Disable logic Kipas 1 |
| **05** | Fan 2 Mode | 0/1 | Enable/Disable logic Kipas 2 |
| ... | ... | ... | (Kipas 3-4 dst) |
| **08** | Timer ON | 0-99 | Durasi Timer Nyala (menit) |
| **09** | Timer OFF | 0-99 | Durasi Timer Mati (menit) |
| **10** | Humidity Set | 0-99 | Target Kelembaban |
| **11** | Cool On Time | 0-99 | Delay Cooler Nyala (detik) |
| **12** | Cool Off Time | 0-99 | Delay Cooler Mati (detik) |
| **13** | Low Alarm | < High | Batas Alarm Bawah |
| **14** | High Alarm | > Low | Batas Alarm Atas |
| **15** | Alarm Delay | 0-99 | Waktu tunggu sebelum alarm bunyi |
| **16** | Brightness | 0-15 | Kecerahan Layar Display |
