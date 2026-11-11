payload = 127           # byte, tipikal
interval_s = 2
topic = "unib/aquaculture/esp32-unib-node01/telemetry"

msg_per_day = 86400 // interval_s
overhead_mqtt = 2 + 2 + len(topic)   # fixed header + panjang topik + topik

for nama, ukuran in [("JSON saja", payload), ("MQTT QoS0", payload + overhead_mqtt)]:
    per_hari = ukuran * msg_per_day
    print(f"{nama}: {ukuran} B/pesan | {ukuran/interval_s:.1f} B/s | "
          f"{per_hari/1e6:.2f} MB/hari | {per_hari*30/1e6:.1f} MB/30 hari")