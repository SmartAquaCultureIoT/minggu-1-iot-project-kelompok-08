# Laporan Praktikum Minggu 1
**Toolchain, Simulasi Wokwi, dan Inisialisasi Repositori Git**

**Mata Kuliah:** Teknologi Internet of Things (IoT), S1 Informatika, Fakultas Teknik, Universitas Bengkulu
**Proyek:** Smart Aquaculture: Sistem Pemantauan Kualitas Air dan Pemberi Pakan Ikan Otomatis Berbasis IoT pada Kolam Budidaya Air Tawar
**Kelompok: 8**  | **Tanggal: 5 November 2026** 

| Nama | NIM | Peran |
|---|---|---|
| Kenia Nurma Feblia | [G1A023004] | Project Manager & Dashboard Engineer |
| Primanda Nafissa Alfiansyah | [G1A023044] | Device & Firmware Engineer |
| Hesi Desta Lestari | [G1A023006] | Back-End & Data Engineer |
| M. Dimas Al-Shiddiq | [G1A023092] | Security & DevOps Engineer |

---

## 1. Pendahuluan

### 1.1 Latar Belakang
Pembudidaya ikan air tawar di wilayah Bengkulu (lele, mas, nila) menghadapi risiko kematian massal akibat penurunan kualitas air yang mendadak, sementara pemberian pakan manual memakan tenaga dan kurang efisien. Proyek ini membangun sistem IoT end-to-end untuk pemantauan kualitas air jarak jauh dan pemberian pakan otomatis.

### 1.2 Tujuan Minggu 1
1. Menyiapkan toolchain pengembangan (VS Code, PlatformIO, Wokwi).
2. Membangun simulasi node ESP32 pada Wokwi dengan firmware non-blocking berbasis `millis()`.
3. Menghasilkan telemetri JSON terstruktur melalui Serial Monitor.
4. Membandingkan secara kuantitatif pola blocking (`delay`) dan non-blocking (`millis`).
5. Menginisialisasi repositori Git dengan struktur 5 folder dan model feature branch.

### 1.3 Posisi pada Arsitektur
Pekerjaan Minggu 1 berada pada **lapisan perangkat (device layer)** dari arsitektur IoT lima lapis (ITU-T Y.4000 dan ISO/IEC 30141:2024). Lapisan jaringan, backend, penyimpanan, keamanan, dan aplikasi dikerjakan pada minggu-minggu berikutnya.

---

## 2. Lingkungan Pengembangan

| Komponen | Keterangan |
|---|---|
| Editor | Visual Studio Code dengan ekstensi PlatformIO IDE dan Wokwi Simulator |
| Platform | `espressif32@6.9.0` (versi dikunci), framework Arduino |
| Board | ESP32 DevKit (`esp32dev`) |
| Library | DHT sensor library 1.4.7, Adafruit Unified Sensor 1.1.15, ArduinoJson 7.4.3 |
| Simulator | Wokwi for VS Code (Community License) |
| Baud rate | 115200 |

---

## 3. Perancangan

### 3.1 Skema Rangkaian (Wokwi)

| Komponen | Pin ESP32 | Keterangan |
|---|---|---|
| DHT22 (data/SDA) | GPIO 4 | Sensor suhu dan kelembapan |
| DHT22 (VCC/GND) | 3V3 / GND | Catu daya |
| LED merah | GPIO 2 | Indikator alert, lewat resistor 220 Ω |
| Tombol darurat | GPIO 15 dan GND | `INPUT_PULLUP`, aktif LOW |

DHT22 berperan sebagai sensor validasi alur akuisisi data pada Minggu 1. Sensor utama proyek (pH 6,5 sampai 8,5 dan turbidity) serta aktuator servo pakan ditambahkan pada fase berikutnya di atas kerangka yang sama. LED dan tombol merupakan prototipe peringatan lokal yang kelak dipetakan ke buzzer dan LED alert.

![Skema Rangkaian](docs/lampiran/minggu1_rangkaian.png)

### 3.2 Rancangan Firmware
Firmware (`firmware/src/main.cpp`) dirancang sepenuhnya non-blocking:
- **Penjadwalan dengan `millis()`**: telemetri dikirim setiap 2000 ms tanpa `delay()`.
- **Tombol dengan debounce leading-edge**: perubahan status diproses segera lalu diabaikan selama 50 ms.
- **LED alert berkedip** setiap 250 ms melalui pencatat waktu `millis()`, tanpa menahan CPU.
- **Serialisasi JSON** memakai ArduinoJson 7 (`JsonDocument`), nilai desimal dibulatkan 2 digit agar ukuran payload konsisten.
- **Pengukuran latensi tombol**: interrupt pada GPIO 15 (FALLING) mencatat waktu tekan, kemudian dibandingkan dengan waktu saat tombol diproses di program utama. Interrupt hanya dipakai untuk pengukuran, logika utama tetap polling `millis()`.
- Satu penanda kompilasi `USE_BLOCKING` (0 atau 1) memilih mode non-blocking atau blocking untuk eksperimen.

### 3.3 Format Payload Telemetri
```json
{"device_id":"esp32-unib-node01","seq":14,"uptime_ms":28006,"mode":"non-blocking","temp_c":24.00,"hum_pct":40.00,"alert":false}
```

| Field | Tipe | Fungsi |
|---|---|---|
| `device_id` | string | Identitas node |
| `seq` | integer | Nomor urut pesan, untuk mendeteksi paket hilang |
| `uptime_ms` | integer | Waktu sejak boot (ms) |
| `mode` | string | `blocking` atau `non-blocking` (untuk eksperimen) |
| `temp_c` | float | Suhu (°C) |
| `hum_pct` | float | Kelembapan (%) |
| `alert` | boolean | Status peringatan (dipicu tombol darurat) |

---

## 4. Hasil dan Pengujian

### 4.1 Hasil Simulasi
- Build berhasil (`[SUCCESS]`); berkas `firmware.bin` dan `firmware.elf` terbentuk pada `.pio/build/esp32dev/`.
- Serial Monitor menampilkan JSON setiap 2000 ms (`uptime_ms` 2006, 4006, 6006, dan seterusnya) dengan `seq` naik teratur.
- Perubahan nilai DHT22 pada simulator langsung tercermin pada `temp_c` dan `hum_pct` di JSON berikutnya.
- Penekanan tombol mengubah `alert` antara `true` dan `false`, dan LED berkedip saat `alert` bernilai `true`.

![Hasil  Simulasi](docs/lampiran/minggu1-hasil-simulasi.png)

### 4.2 Eksperimen Blocking vs Non-blocking
Latensi diukur oleh firmware (selisih waktu tekan pada interrupt dan waktu pemrosesan), dalam milidetik waktu simulasi. Pengambilan data dilakukan sebanyak 10 kali tekan untuk tiap mode.

**Data mentah**

| Mode | Latensi tiap tekan (ms) |
|---|---|
| Non-blocking | 35, 0, 0, 0, 2, 0, 2, 67, 0, 1 |
| Blocking | 3436, 4024, 4019, 2005, 12081, 4023, 1735, 2009, 4023, 2005 |

![Non Blocking](docs/lampiran/minggu1-non-blocking.png)

![Blocking](docs/lampiran/minggu1-blocking.png)

**Ringkasan**

| Metrik | Non-blocking (`millis()`) | Blocking (`delay(2000)`) |
|---|---|---|
| Jumlah percobaan | 10 | 10 |
| Latensi minimum | 0 ms | 1735 ms |
| Latensi maksimum | 67 ms | 12081 ms |
| Median | ±0,5 ms | ±3728 ms |
| Rata-rata | ±10,7 ms | ±3936 ms |
| Tekanan terbaca langsung (≤ ±2014 ms) | 10 dari 10 | 4 dari 10 |
| Tekanan hilang (> 1 siklus) | 0 dari 10 | 6 dari 10 |
| Interval telemetri | 2000 ms (stabil) | ±2014 ms (bergeser) |
| Kedipan LED | halus, tiap 250 ms | tidak berkedip halus (berubah tiap ±2 detik) |
| Pengiriman data pertama | pada interval pertama | pada 497 ms |

### 4.3 Analisis
1. **Respons tombol.** Pada non-blocking, 8 dari 10 tekanan diproses dalam 0 sampai 2 ms. Dua nilai tinggi (35 dan 67 ms) diduga terjadi karena tekanan ulang yang datang dalam jendela debounce 50 ms setelah tombol dilepas, atau karena beban simulator. Median 0,5 ms mewakili perilaku tipikal.
2. **Penundaan pada blocking.** Satu siklus blocking berlangsung ±2014 ms (`delay(2000)` ditambah waktu baca dan serialisasi). Tombol hanya dibaca satu kali per siklus, sehingga latensi terbaik pun mendekati satu siklus penuh (1735 sampai 2009 ms).
3. **Tekanan yang hilang.** Pada blocking, 6 dari 10 tekanan memiliki latensi sekitar 2, 3, atau 6 siklus (misalnya 4023 ms dan 12081 ms ≈ 6 × 2014 ms). Artinya tombol yang ditekan dan dilepas di tengah `delay()` tidak terbaca sama sekali, sehingga pengguna harus menekan ulang. Untuk sistem Smart Aquaculture, hilangnya perintah darurat atau alarm kualitas air tidak dapat diterima.
4. **Drift interval.** Pada blocking, jadwal telemetri bergeser menjadi ±2014 ms karena waktu proses ditambahkan ke `delay()`. Pada non-blocking, penjadwalan memakai selisih `millis()` sehingga interval tetap 2000 ms.
5. **Catatan metodologi.** Latensi blocking bergantung pada cara menekan tombol (klik singkat atau ditahan), sehingga median dan proporsi tekanan hilang lebih informatif daripada rata-rata. Seluruh angka dalam ms waktu simulasi (tidak dipengaruhi kecepatan simulator; indikator kecepatan simulator tercatat 41% sampai 99%).

---

## 5. Estimasi Ukuran Payload dan Bandwidth

Ukuran payload JSON compact (tanpa newline), dihitung dengan skrip dan diverifikasi dari contoh Serial Monitor:

| Kondisi | Ukuran |
|---|---|
| Tipikal (seq 2 digit, uptime 5 digit, `non-blocking`, alert false) | **127 byte** |
| Awal operasi (seq 1 digit, uptime 4 digit, alert true) | 124 byte |
| Terburuk (seq dan uptime 10 digit) | 140 byte |

Bandwidth pada interval 2 detik (payload tipikal 127 byte):

| Metrik | Nilai |
|---|---|
| Pesan per hari | 43.200 |
| Laju data | 63,5 B/s (508 bit/s) |
| Volume per hari | 5.486.400 B (≈ 5,49 MB) |
| Volume per 30 hari | ≈ 164,6 MB |

Dengan overhead MQTT (topik contoh `unib/aquaculture/esp32-unib-node01/telemetry`, 44 karakter; header 2 B, panjang topik 2 B, topik 44 B), ukuran per pesan menjadi ±175 B (QoS 0) atau ±177 B (QoS 1), atau sekitar 7,56 MB per hari. Angka ini belum memasukkan overhead TCP/IP dan TLS, sehingga merupakan estimasi minimum. Setelah sensor pH dan turbidity ditambahkan, payload diperkirakan bertambah sekitar 30 sampai 40 byte dan perhitungan akan diperbarui.

**Rancangan awal topik MQTT dan skema data (untuk minggu berikutnya):**

| Topik | Fungsi | QoS |
|---|---|---|
| `unib/aquaculture/{device_id}/telemetry` | data sensor | 0 atau 1 |
| `unib/aquaculture/{device_id}/status` | online/offline (LWT, retained) | 1 |
| `unib/aquaculture/{device_id}/alert` | peringatan kritis | 1 |
| `unib/aquaculture/{device_id}/cmd/feeder` | perintah pakan | 1 |

Skema InfluxDB (usulan): measurement `telemetry`, tag `device_id`, field `temp_c`, `hum_pct`, `alert`, `seq`, `uptime_ms` (kelak `ph` dan `turbidity`), dengan timestamp diberikan server saat data diterima.

---

## 6. Repositori dan Manajemen Versi

**Struktur 5 folder:**
```
iot-smart-aquaculture/
├── firmware/        (main.cpp, platformio.ini, wokwi.toml, diagram.json)
├── edge-gateway/
├── backend-api/
├── docker/
├── docs/            (laporan-minggu-01.md)
├── .gitignore
└── README.md
```

**`.gitignore`** mengecualikan berkas yang tidak boleh ter-commit: `.pio/`, `.env`, `__pycache__/`, `*.pyc`, `.venv/`, kunci dan sertifikat (`*.pem`, `*.key`), serta data Docker. Berkas kredensial (misalnya `password.txt` untuk Mosquitto) tidak disimpan di repositori; yang disimpan hanya berkas contoh (`*.example`).

**Model feature branch:**

| Branch | Pemilik |
|---|---|
| `feature/device-firmware` | Primanda |
| `feature/backend-data` | Hesi |
| `feature/devops-security` | Dimas |
| `feature/dashboard-pm` | Kenia |

Alur kerja: setiap anggota bekerja pada branch masing-masing, lalu membuka Pull Request ke `main` dan meminta review satu anggota lain sebelum merge.

**Remote:** [URL repositori GitHub/GitLab kelompok]

---

## 7. Kendala dan Solusi

| No | Kendala | Penyebab | Solusi |
|---|---|---|---|
| 1 | Build gagal pada tahap linking (`undefined reference to strlen/strcpy/...`) | `platform = espressif32` tanpa versi, sehingga mengambil toolchain terbaru yang bermasalah | Versi platform dikunci ke `espressif32@6.9.0`, folder hasil build dibersihkan, lalu build ulang |
| 2 | Editor visual `diagram.json` di VS Code meminta upgrade plan | Editor visual tidak tersedia pada lisensi gratis | `diagram.json` diedit sebagai teks (Text Editor) atau dirakit di situs Wokwi lalu disalin |
| 3 | Build gagal di laptop lain (`CreateProcess: No such file or directory`) | Proyek dipindah dalam bentuk ZIP, membawa folder `.pio` dan toolchain tidak lengkap | Hapus `.pio`, pasang ulang toolchain, dan pindahkan proyek lewat `git clone` (bukan ZIP) |
| 4 | Respons tombol terasa tertunda dan log tampil bergeser | Debounce berbasis kestabilan menunda ≥ 50 ms; `\n` tanpa `\r` | Debounce leading-edge dan penggunaan `\r\n` pada log |

---

## 8. Ceklis Deliverable Minggu 1

| No | Deliverable | Status |
|---|---|---|
| 1 | `platformio.ini` (DHT sensor library dan ArduinoJson 7.x) | ☑ |
| 2 | `wokwi.toml` menunjuk `firmware.bin` dan `firmware.elf` | ☑ |
| 3 | `diagram.json`: ESP32, DHT22 (GPIO 4), LED (GPIO 2), tombol (GPIO 15) | ☑ |
| 4 | Firmware non-blocking `millis()` dan payload JSON via Serial | ☑ |
| 5 | Tabel analisis blocking vs non-blocking | ☑ |
| 6 | Struktur 5 folder dan `.gitignore` | ☑ |
| 7 | Remote repositori dan feature branch 4 role | ☑ |
| 8 | `docs/laporan-minggu-01.md` | ☑ (dokumen ini) |

---

## 9. Kesimpulan

1. Simulasi node ESP32 pada Wokwi berhasil menghasilkan telemetri JSON yang sesuai spesifikasi, dengan interval stabil 2000 ms.
2. Eksperimen kuantitatif menunjukkan keunggulan non-blocking: latensi tombol 0 sampai 2 ms (median ±0,5 ms) dibanding 1735 sampai 12081 ms (median ±3728 ms) pada blocking, serta 6 dari 10 tekanan hilang pada blocking.
3. Karena sistem Smart Aquaculture harus merespons kondisi darurat sambil tetap membaca sensor, pola non-blocking `millis()` ditetapkan sebagai standar seluruh firmware berikutnya.
4. Estimasi payload (±127 byte) dan bandwidth (±5,49 MB per hari tanpa overhead protokol) memberi dasar perancangan broker MQTT dan penyimpanan time-series.
5. Repositori dengan struktur 5 folder dan model feature branch menjadi dasar kolaborasi tim.

---
