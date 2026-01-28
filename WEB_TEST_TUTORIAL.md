# 🌐 Panduan Uji Coba Web Dashboard & WiFi
Panduan ini untuk menguji koneksi antara ESP32 dan Dashboard Web setelah fitur WiFi diaktifkan.

---

## 📋 Prasyarat
1.  **WiFi Hotspot:**
    *   Pastikan ada Hotspot dengan nama: **`Sejahtera`**
    *   Password: **`presiden sekarang`**
    *   (Huruf harus SAMA PERSIS, besar/kecilnya).
    *   *Tip: Bisa pakai Tethering HP.*

2.  **Perangkat:** Laptop atau HP yang terhubung ke **Hotspot yang SAMA**.

---

## 🚀 Langkah Uji Koneksi

### 1. Cek Koneksi WiFi (Serial Monitor)
1.  Buka Serial Monitor (di Arduino IDE / VSCode).
2.  Reset ESP32 (Tekan tombol EN/R).
3.  Tunggu log:
    ```
    Connecting to WiFi...
    .......
    Connected to WiFi
    IP Address: 192.168.xx.xx
    Web Server started!
    ```
4.  **CATAT IP ADDRESS-nya!** (Misal: `192.168.1.105`).

### 2. Akses Dashboard Web
1.  Buka Browser (Chrome/Safari) di Laptop/HP.
2.  Ketik alamat IP tadi: `http://192.168.1.105` (sesuaikan dengan IP Anda).
3.  **Harus muncul Tampilan Temptron Dashboard** dengan warna ungu/kuning yang keren.

### 3. Tes Integrasi (2 Arah)

#### A. Dari Alat ke Web (Monitoring)
1.  Lihat suhu di layar LCD alat. (Misal `29` °C).
2.  Lihat angka di Web Dashboard (Display Kiri).
3.  Harusnya angkanya **SAMA** dan berubah jika suhu ruangan berubah.

#### B. Dari Web ke Alat (Control)
1.  Di Web, coba ubah **Required Temp (Menu 02)**.
2.  Klik angka/settingannya.
3.  (Catatan: Fitur update dari Web ke Alat via tombol mungkin butuh penyesuaian API lebih lanjut, tapi kita cek dulu apakah datanya terbaca).

#### C. Tes API (Opsional)
Coba akses URL ini di browser untuk melihat data mentah:
`http://192.168.1.105/status`
Harusnya muncul teks format JSON seperti:
```json
{
  "telemetry": {
    "temp": 29.5,
    "humidity": 60.1,
    "setpoint": 30.0
  },
  ...
}
```

---

## ❓ Troubleshooting
*   **Gak bisa connect WiFi?** -> Cek nama WiFi & Password (case sensitive). Sinyal WiFi harus 2.4GHz (ESP32 tidak dukung 5GHz).
*   **Web gak bisa dibuka?** -> Pastikan HP/Laptop di jaringan WiFi yang SAMA dengan ESP32.
*   **Angka diam aja?** -> Refresh halaman web. (Nanti bisa kita tambahkan auto-refresh).

Selamat Mencoba! 🌍✨
