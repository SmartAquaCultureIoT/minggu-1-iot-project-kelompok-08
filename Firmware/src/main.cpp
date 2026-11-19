#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHT_PIN   4
#define LED_PIN   2
#define BTN_PIN   15
#define DHT_TYPE  DHT22

#define USE_BLOCKING 0

const char* DEVICE_ID = "esp32-unib-node01";
const unsigned long SENSOR_INTERVAL_MS = 2000;
const unsigned long BLINK_INTERVAL_MS  = 250;
const unsigned long DEBOUNCE_MS        = 50;

DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastSensorMs   = 0;
unsigned long lastBlinkMs    = 0;
unsigned long lastDebounceMs = 0;
uint32_t seq = 0;

bool alertActive = false;
bool ledState    = false;
int  btnStable   = HIGH;

volatile unsigned long pressIsrMs   = 0;
volatile bool           pressPending = false;

void IRAM_ATTR onBtnPress() {
  if (!pressPending) {
    pressIsrMs = millis();
    pressPending = true;
  }
}

void publishTelemetry() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("{\"error\":\"dht_read_failed\"}");
    return;
  }

  JsonDocument doc;
  doc["device_id"] = DEVICE_ID;
  doc["seq"]       = ++seq;
  doc["uptime_ms"] = millis();
  doc["mode"]      = USE_BLOCKING ? "blocking" : "non-blocking";
  doc["temp_c"]    = serialized(String(t, 2));
  doc["hum_pct"]   = serialized(String(h, 2));
  doc["alert"]     = alertActive;

  serializeJson(doc, Serial);
  Serial.println();
}

void handleButton() {
  int reading = digitalRead(BTN_PIN);

  if (reading != btnStable && (millis() - lastDebounceMs) > DEBOUNCE_MS) {
    btnStable = reading;
    lastDebounceMs = millis();

    if (btnStable == LOW) {
      alertActive = !alertActive;
      unsigned long latency = pressPending ? (millis() - pressIsrMs) : 0;
      Serial.printf("[BTN] alert=%s | latensi=%lu ms\r\n",
                    alertActive ? "true" : "false", latency);
      pressPending = false;
    } else {
      pressPending = false;
    }
  }
}

void updateLed() {
  if (!alertActive) {
    ledState = false;
  } else if (millis() - lastBlinkMs >= BLINK_INTERVAL_MS) {
    lastBlinkMs = millis();
    ledState = !ledState;
  }
  digitalWrite(LED_PIN, ledState);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), onBtnPress, FALLING);
  dht.begin();

  Serial.printf("Boot: %s | mode=%s\r\n", DEVICE_ID,
                USE_BLOCKING ? "blocking" : "non-blocking");
}

void loop() {
#if USE_BLOCKING
  handleButton();
  updateLed();
  publishTelemetry();
  delay(SENSOR_INTERVAL_MS);
#else
  handleButton();
  updateLed();

  unsigned long now = millis();
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    lastSensorMs = now;
    publishTelemetry();
  }
#endif
}