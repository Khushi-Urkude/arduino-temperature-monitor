// Smart Temperature & Humidity Monitor (Arduino Uno, C++)
// DHT22 sensor -> moving-average filter -> LED + buzzer alert -> CSV serial log
// Libraries: "DHT sensor library" by Adafruit (+ Adafruit Unified Sensor)

#include <DHT.h>

#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LED_PIN 8
#define BUZZER_PIN 9

const float TEMP_LIMIT_C = 35.0;               // alert threshold
const unsigned long READ_INTERVAL_MS = 2000;   // DHT22 needs >= 2 s between reads
const int WINDOW = 5;                          // moving-average window

DHT dht(DHT_PIN, DHT_TYPE);
float samples[WINDOW];
int sampleIndex = 0;
int sampleCount = 0;
unsigned long lastRead = 0;

// Smooths sensor noise so a single bad reading does not trigger the alarm
float movingAverage(float value) {
  samples[sampleIndex] = value;
  sampleIndex = (sampleIndex + 1) % WINDOW;
  if (sampleCount < WINDOW) sampleCount++;
  float sum = 0;
  for (int i = 0; i < sampleCount; i++) sum += samples[i];
  return sum / sampleCount;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin();
  Serial.println("time_ms,temp_c,avg_c,humidity_pct,alert");
}

void loop() {
  // Non-blocking timing with millis() instead of delay()
  unsigned long now = millis();
  if (now - lastRead < READ_INTERVAL_MS) return;
  lastRead = now;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Error handling: skip invalid readings (disconnected or faulty sensor)
  if (isnan(temp) || isnan(hum)) {
    Serial.println("ERROR,sensor_read_failed");
    return;
  }

  float avg = movingAverage(temp);
  bool alert = avg > TEMP_LIMIT_C;

  digitalWrite(LED_PIN, alert ? HIGH : LOW);
  if (alert) tone(BUZZER_PIN, 1000);
  else noTone(BUZZER_PIN);

  Serial.print(now);  Serial.print(",");
  Serial.print(temp); Serial.print(",");
  Serial.print(avg);  Serial.print(",");
  Serial.print(hum);  Serial.print(",");
  Serial.println(alert ? 1 : 0);
}
