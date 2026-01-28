# 🛠️ Tutorial Uji Manual (Manual Output Test)
Panduan ini digunakan untuk menguji apakah **Relay, Kipas, Heater, dan Cooling** berfungsi dengan benar dengan cara memanipulasi settingan suhu secara manual lewat Keypad.

---

## 📋 Persiapan
1.  Pastikan **ESP32 menyala** dan **Layar Kiri** menampilkan suhu ruangan (Misal: **28°C**).
2.  Pastikan tidak ada error ("Err") di layar.
3.  Siapkan alat tulis untuk mencatat hasil (Relay Nyala/Mati).

---

## 🔥 Skenario 1: Tes HEATER (Pemanas)
Tujuan: Memastikan Relay Heater (Pin 14) menyala saat suhu dingin.

1.  **Cek Suhu Ruangan:** Misal suhu saat ini **28°C**.
2.  **Masuk Menu 02 (Heat Temp):**
    *   Tekan **A** dua kali sampai layar kanan muncul `02`.
    *   Tekan **B** (Edit).
    *   Ketik angka **35** (Diatas suhu ruangan 28°C).
    *   Tekan **D** (Simpan).
3.  **Kembali ke Menu Utama:** Tekan **C**.
4.  **Cek Hasil:**
    *   Karena Suhu (28) < Batas Heat (35), maka **Relay Heater HARUS MENYALA**.
    *   *Jika ada lampu indikator di modul relay, pastikan menyala.*

---

## ❄️ Skenario 2: Tes Kipas Bertingkat (Fan 1 - 6)
Tujuan: Memastikan Kipas menyala bertahap sesuai selisih suhu.

**Logika:** Kipas 1 nyala jika beda 1°C, Kipas 2 beda 2°C, dst.

1.  **Cek Suhu Ruangan:** Misal suhu saat ini **28°C**.
2.  **Masuk Menu 01 (Setpoint):**
    *   Tekan **A** satu kali (Menu `01`).
    *   Tekan **B** (Edit).

3.  **Tahap A (Fan 1 Nyala):**
    *   Ubah Setpoint jadi **27°C** (Suhu 28 - 27 = Beda 1°C).
    *   Tekan **D**, lalu **C**.
    *   **Hasil:** **Fan 1 (Relay Pin 18)** HARUS NYALA. (Fan lain mati).

4.  **Tahap B (Fan 1 & 2 Nyala):**
    *   Ubah Setpoint jadi **26°C** (Suhu 28 - 26 = Beda 2°C).
    *   **Hasil:** **Fan 1 & Fan 2** HARUS NYALA.

5.  **Tahap C (Fan 1 - 4 Nyala):**
    *   Ubah Setpoint jadi **24°C** (Suhu 28 - 24 = Beda 4°C).
    *   **Hasil:** **Fan 1, 2, 3, 4** HARUS NYALA.

6.  **Tahap D (SEMUA KIPAS Nyala - Full Power):**
    *   Ubah Setpoint jadi **20°C** (Suhu 28 - 20 = Beda 8°C -> Maksimal).
    *   **Hasil:** **Semua Fan (1, 2, 3, 4, 5, 6)** HARUS NYALA.

---

## 🧊 Skenario 3: Tes COOLING (Pendingin)
Tujuan: Memastikan Relay Cooling/Pompa menyala saat panas ekstrem.

1.  **Cek Suhu Ruangan:** Misal suhu saat ini **28°C**.
2.  **Masuk Menu 03 (Cool Temp):**
    *   Tekan **A** tiga kali (Menu `03`).
    *   Ubah nilai ke **25°C** (Dibawah suhu ruangan).
    *   Tekan **D**, lalu **C**.
3.  **Cek Hasil:**
    *   Tunggu beberapa detik (karena ada *Cool On Delay* di Menu 11, default 0 detik).
    *   **Relay Cooling (Pin 12)** HARUS MENYALA.

---

## 🔄 Skenario 4: Tes INTERMITTENT (Timer Kipas 1)
Tujuan: Tes kipas nyala-mati sendiri saat suhu sudah tercapai (Suhu Pas).

1.  **Kembalikan Suhu:** Set Menu 01 (Setpoint) ke suhu ruangan (misal 28°C).
    *   Saat suhu = setpoint, semua kipas harusnya MATI.
2.  **Setting Timer:**
    *   **Menu 08 (Timer ON):** Set ke **1** (1 Menit).
    *   **Menu 09 (Timer OFF):** Set ke **1** (1 Menit).
3.  **Cek Hasil:**
    *   Perhatikan **Fan 1**.
    *   Dia akan **Nyala 1 menit**, lalu **Mati 1 menit**, berulang-ulang.

---

## ✅ Checklist Hasil Tes

| Output | Pin ESP32 | Hasil Tes (OK/Fail) |
| :--- | :--- | :--- |
| **Heater** | 14 | |
| **Fan 1** | 18 | |
| **Fan 2** | 19 | |
| **Fan 3** | 23 | |
| **Fan 4** | 5 | |
| **Fan 5** | 13 | |
| **Fan 6** | 4 | |
| **Cooling**| 12 | |
