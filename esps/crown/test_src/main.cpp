#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Crown ESP32 Test ===");
    Serial.println("Hardware test - minimal firmware");
    Serial.println("If you see this, the ESP32 is working!");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Chip ID: %08X\n", ESP.getEfuseMac());
}

void loop() {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000) {
        Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
        lastPrint = millis();
    }
    delay(100);
}
