# Dokumentasi Payload Telemetri & Estimasi Bandwidth

Dokumen ini berisi spesifikasi payload telemetri perangkat IoT akuakultur, hasil validasi skema data, serta analisis estimasi kebutuhan bandwidth jaringan untuk operasional bulanan.

---

## 1. Contoh JSON Payload Firmware

Firmware ESP32 mengirimkan data telemetri secara berkala menggunakan format JSON standar. Berikut adalah contoh payload normal dan payload saat terjadi error sensor:

### Payload Normal (127 Byte)
```json
{"device_id":"esp32-unib-node01","seq":14,"uptime_ms":28006,"mode":"non-blocking","temp_c":24.00,"hum_pct":40.00,"alert":false}