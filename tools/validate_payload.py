import json

SAMPLE = '{"device_id":"esp32-unib-node01","seq":14,"uptime_ms":28006,"mode":"non-blocking","temp_c":24.00,"hum_pct":40.00,"alert":false}'
ERROR_SAMPLE = '{"error":"dht_read_failed"}'

REQUIRED = {
    "device_id": str,
    "seq": int,
    "uptime_ms": int,
    "mode": str,
    "temp_c": float,
    "hum_pct": float,
    "alert": bool,
}

def validate(raw: str):
    data = json.loads(raw)  # error di sini = JSON tidak valid
    if "error" in data:
        return False, f"payload error dari firmware: {data['error']}"
    for key, tipe in REQUIRED.items():
        if key not in data:
            return False, f"field hilang: {key}"
        v = data[key]
        if tipe is int and (isinstance(v, bool) or not isinstance(v, int)):
            return False, f"{key} harus integer"
        if tipe is float and (isinstance(v, bool) or not isinstance(v, (int, float))):
            return False, f"{key} harus angka"
        if tipe in (str, bool) and not isinstance(v, tipe):
            return False, f"{key} harus {tipe.__name__}"
    return True, "OK"

if __name__ == "__main__":
    for name, s in [("normal", SAMPLE), ("error", ERROR_SAMPLE)]:
        print(name, "->", validate(s), "| ukuran:", len(s.encode()), "byte")