/**
 * ESP32 FTDI Test Firmware
 * 
 * Minimal test firmware to verify FTDI flashing and basic ESP32 functionality.
 * This firmware is designed to be as simple as possible to help diagnose
 * boot loop issues with the ESP32 devices.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <ArduinoOTA.h>

// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Test variables
unsigned long lastPrint = 0;
int counter = 0;
bool ledState = false;

// LED pin (GPIO 2 is commonly used for built-in LED on ESP32)
#define LED_PIN 2

// WiFi credentials
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// ESP32-ETH01V1.4 built-in Ethernet settings
// LAN8720A PHY configuration for ESP32-ETH01V1.4

// Network settings
IPAddress ip(192, 168, 1, 100);  // Static IP for Ethernet
IPAddress dns_server(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Connection status
bool wifiConnected = false;
bool ethernetConnected = false;

// OTA settings
const char* ota_hostname = "test-esp32";
const char* ota_password = "test123";

// Ethernet event handler
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            ETH.setHostname("esp32-eth01");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("ETH Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("ETH MAC: ");
            Serial.print(ETH.macAddress());
            Serial.print(", IPv4: ");
            Serial.print(ETH.localIP());
            if (ETH.fullDuplex()) {
                Serial.print(", FULL_DUPLEX");
            }
            Serial.print(", ");
            Serial.print(ETH.linkSpeed());
            Serial.println("Mbps");
            ethernetConnected = true;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH Disconnected");
            ethernetConnected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("ETH Stopped");
            ethernetConnected = false;
            break;
        default:
            break;
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);  // Give time for serial to initialize
    
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("    ESP32 FTDI Test Firmware");
    Serial.println("========================================");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.printf("Chip ID: %08X\n", ESP.getEfuseMac());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.println("========================================");
    Serial.println("If you see this message, the ESP32 is working!");
    Serial.println("========================================");
    
    // Initialize LED (GPIO 2)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("Built-in LED initialized");
    
    // Initialize built-in Ethernet (ESP32-ETH01V1.4)
    Serial.println("Initializing built-in Ethernet...");
    
    // Register event handler
    WiFi.onEvent(WiFiEvent);
    
    // Manual power control for ESP32-ETH01V1.4
    Serial.println("Setting up power pin GPIO 5...");
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);  // Power on the PHY
    delay(500);  // Give more time for power to stabilize
    
    // Initialize Ethernet with most common ESP32-ETH01V1.4 configuration
    // PHY address 0, GPIO 5 power, GPIO0_IN clock
    Serial.println("Initializing Ethernet with PHY address 0...");
    Serial.println("Configuration: PHY_ADDR=1, POWER=5, MDC=23, MDIO=18, TYPE=LAN8720, CLK=GPIO0_IN");
    
    bool eth_init_result = ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
    
    if (eth_init_result) {
        Serial.println("ETH.begin() succeeded - waiting for connection...");
    } else {
        Serial.println("ETH.begin() failed - check hardware connections");
    }
    
    // Wait for Ethernet connection
    Serial.println("Waiting for Ethernet connection...");
    int ethernet_attempts = 0;
    while (!ethernetConnected && ethernet_attempts < 20) {
        delay(500);
        Serial.print(".");
        ethernet_attempts++;
    }
    
    if (ethernetConnected) {
        Serial.println();
        Serial.println("Built-in Ethernet connected!");
        Serial.printf("IP address: %s\n", ETH.localIP().toString().c_str());
    } else {
        Serial.println();
        Serial.println("Built-in Ethernet connection failed, trying WiFi...");
    }
    
    if (!ethernetConnected) {
        
        // Initialize WiFi as fallback
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        // Wait for WiFi connection
        int wifi_attempts = 0;
        while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
            delay(500);
            Serial.print(".");
            wifi_attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println();
            Serial.println("WiFi connected!");
            Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.println();
            Serial.println("Both Ethernet and WiFi connection failed!");
        }
    }
    
    // Initialize OTA if either connection is successful
    if (ethernetConnected || wifiConnected) {
        
        // Initialize OTA
        Serial.println("Initializing OTA...");
        ArduinoOTA.setHostname(ota_hostname);
        ArduinoOTA.setPassword(ota_password);
        
        ArduinoOTA.onStart([]() {
            String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
            Serial.println("OTA Start updating " + type);
            digitalWrite(LED_PIN, HIGH);  // Turn on LED during OTA
        });
        
        ArduinoOTA.onEnd([]() {
            Serial.println("\nOTA End");
            digitalWrite(LED_PIN, LOW);  // Turn off LED after OTA
        });
        
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
        });
        
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("OTA Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
        
        ArduinoOTA.begin();
        Serial.println("OTA ready");
    } else {
        Serial.println();
        Serial.println("No network connection - continuing without OTA");
    }
    
    Serial.println("Starting main loop...");
    Serial.println();
}

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();
    
    // Built-in Ethernet doesn't need maintain() call
    
    // Print status every 2 seconds
    if (millis() - lastPrint >= 2000) {
        lastPrint = millis();
        counter++;
        
        // Toggle LED
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        
        // Print status
        Serial.printf("[%lu] Counter: %d, LED: %s, Free heap: %d bytes", 
                     millis() / 1000, counter, ledState ? "ON" : "OFF", ESP.getFreeHeap());
        
        // Add connection status
        if (ethernetConnected) {
            Serial.printf(", Ethernet: %s", ETH.localIP().toString().c_str());
        } else if (wifiConnected) {
            Serial.printf(", WiFi: %s", WiFi.localIP().toString().c_str());
        } else {
            Serial.print(", Network: Disconnected");
        }
        Serial.println();
        
        // Print some system info every 10 iterations
        if (counter % 10 == 0) {
            Serial.println("--- System Info ---");
            Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
            Serial.printf("Chip ID: %08X\n", ESP.getEfuseMac());
            Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
            Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
            Serial.printf("Flash Speed: %d Hz\n", ESP.getFlashChipSpeed());
            if (ethernetConnected) {
                Serial.printf("Ethernet IP: %s\n", ETH.localIP().toString().c_str());
                Serial.printf("Ethernet MAC: %s\n", ETH.macAddress().c_str());
                Serial.printf("Ethernet Link: %s\n", ETH.linkUp() ? "UP" : "DOWN");
                Serial.printf("Ethernet Speed: %d Mbps\n", ETH.linkSpeed());
                Serial.printf("Ethernet Duplex: %s\n", ETH.fullDuplex() ? "FULL" : "HALF");
                Serial.printf("OTA Hostname: %s\n", ota_hostname);
            } else if (wifiConnected) {
                Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
                Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
                Serial.printf("OTA Hostname: %s\n", ota_hostname);
            }
            Serial.println("------------------");
        }
    }
    
    // Small delay to prevent overwhelming the serial output
    delay(10);
}
