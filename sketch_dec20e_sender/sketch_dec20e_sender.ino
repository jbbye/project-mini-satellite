#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LoRa.h>

// LoRa Pins
#define LORA_CS    10
#define LORA_RESET 9
#define LORA_IRQ   2

#define BMP280_ADDR 0x76
#define ADXL345_ADDR 0x53

Adafruit_BMP280 bmp;

float accelX, accelY, accelZ;

void setup() {
  Serial.begin(9600);
  LoRa.setPins(LORA_CS, LORA_RESET, LORA_IRQ);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa initialized!");
  if (!bmp.begin(BMP280_ADDR)) {
    Serial.println("BMP280 initialization failed!");
    while (1);
  }
  Serial.println("BMP280 initialized!");
  Wire.begin();
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();
  Serial.println("ADXL345 initialized!");
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDR, 6, true);
  accelX = (Wire.read() | (Wire.read() << 8)) / 256.0;
  accelY = (Wire.read() | (Wire.read() << 8)) / 256.0;
  accelZ = (Wire.read() | (Wire.read() << 8)) / 256.0;
  String message = String(temperature, 2) + "C," + String(pressure, 2) + "hPa," +
                   "X=" + String(accelX, 2) + ",Y=" + String(accelY, 2) + ",Z=" + String(accelZ, 2);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket(true);
  Serial.println("Sent: " + message);
  delay(2000);  // Avoid sending packets too frequently
}

