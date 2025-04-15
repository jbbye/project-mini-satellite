#include <LoRa.h>

// LoRa Pins
#define LORA_CS    10    // Chip Select
#define LORA_RESET 9     // Reset
#define LORA_IRQ   2     // Interrupt

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize LoRa
  LoRa.setPins(LORA_CS, LORA_RESET, LORA_IRQ);
  if (!LoRa.begin(433E6)) {  // Adjust frequency for your region
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa Receiver Initialized");
}

void loop() {
  // Check for incoming packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Read the incoming message
    String receivedMessage = "";
    while (LoRa.available()) {
      receivedMessage += (char)LoRa.read();
    }

    // Print the received message
    Serial.println("Received: " + receivedMessage);

    // Print RSSI (Signal Strength)
    Serial.print("Signal Strength (RSSI): ");
    Serial.println(LoRa.packetRssi());
  }

  // Small delay to prevent flooding
  delay(1000);
}
