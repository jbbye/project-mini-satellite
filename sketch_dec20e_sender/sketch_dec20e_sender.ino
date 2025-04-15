#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Sensor & fan pins
#define MQ135_AIR_PIN 34
#define MQ135_SOIL_PIN 32
#define FAN_RELAY_PIN 2

// LoRa module pins
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS    15
#define LORA_RST   14
#define LORA_DIO0  26

// DHT22 sensor pin
#define DHT_PIN 12
DHT dht(DHT_PIN, DHT22); // Create DHT object

unsigned long lastStatusPrint = 0;
bool fanState = false;

void setup() {
  Serial.begin(115200);

  pinMode(MQ135_AIR_PIN, INPUT);
  pinMode(MQ135_SOIL_PIN, INPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, HIGH); // Fan OFF for active-low relay

  // Start LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true);
  }

  // Start DHT22 sensor
  dht.begin();

  Serial.println("LoRa init success. Ready to send data...");
}

void loop() {
  int airVal = analogRead(MQ135_AIR_PIN);
  float airVolt = airVal * (3.3 / 4095.0);

  int soilVal = analogRead(MQ135_SOIL_PIN);
  float soilVolt = soilVal * (3.3 / 4095.0);

  // Read temperature and humidity from DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Prepare message
  String loraMsg = "============ Air Sensor ============\n";
  loraMsg += "Analog: " + String(airVal) + " | Voltage: " + String(airVolt, 2) + "\n";

  bool airDanger =
    airVal > 3000 ||  // Ammonia
    airVal > 2800 ||  // CO
    airVal > 2700 ||  // Smoke
    airVal > 2300;    // CO₂

  loraMsg += "[" + String(airVal > 3000 ? "✔" : "✘") + "] Ammonia (NH3)\n";
  loraMsg += "[" + String(airVal > 2800 ? "✔" : "✘") + "] Carbon Monoxide (CO)\n";
  loraMsg += "[" + String(airVal > 2700 ? "✔" : "✘") + "] Smoke\n";
  loraMsg += "[" + String(airVal > 2300 ? "✔" : "✘") + "] CO2 (estimated)\n";

  if (airDanger) {
    if (!fanState) {
      loraMsg += "⚠️ Harmful gases detected — Turning ON ventilation fan!\n";
      digitalWrite(FAN_RELAY_PIN, LOW);  // ON (active-low)
      fanState = true;
    }
  } else {
    if (fanState) {
      loraMsg += "✅ Air quality is good — Turning OFF fan.\n";
      digitalWrite(FAN_RELAY_PIN, HIGH); // OFF (active-low)
      fanState = false;
    }
  }

  loraMsg += "============ Soil Sensor ============\n";
  loraMsg += "Analog: " + String(soilVal) + " | Voltage: " + String(soilVolt, 2) + "\n";
  loraMsg += "[" + String(soilVal > 2950 ? "✔" : "✘") + "] Ammonia (NH3)\n";
  loraMsg += "[" + String(soilVal > 2800 ? "✔" : "✘") + "] Nitrogen Oxides (NOx)\n";
  loraMsg += "[" + String(soilVal > 2500 ? "✔" : "✘") + "] VOCs\n";
  loraMsg += "====================================\n";

  // Include temperature and humidity data
  loraMsg += "=========== Temperature & Humidity ===========\n";
  if (isnan(temperature) || isnan(humidity)) {
    loraMsg += "⚠️ Failed to read from DHT22 sensor!\n";
  } else {
    loraMsg += "Temperature: " + String(temperature, 1) + " °C\n";
    loraMsg += "Humidity: " + String(humidity, 1) + " %\n";
  }

  // Status every 10s
  if (millis() - lastStatusPrint >= 10000) {
    loraMsg += "[Status] Gas ";
    loraMsg += airDanger ? "Detected 🚨 — Fan ON\n" : "Not Detected ✅ — Fan OFF\n";
    lastStatusPrint = millis();
  }

  // Print message to Serial Monitor
  Serial.print(loraMsg);

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print(loraMsg);
  LoRa.endPacket();

  delay(2000);
}
