/**
 * ESP32 Touch Sensor Firmware
 * 
 * Detects electrical shortage/contact using ESP32's built-in capacitive touch sensors.
 * This firmware monitors multiple touch pins and detects when they are touched,
 * indicating a potential electrical shortage or contact.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Touch sensor configuration
#define NUM_TOUCH_SENSORS 6
#define TOUCH_THRESHOLD 20  // Lower values = more sensitive
#define TOUCH_DEBOUNCE_MS 50  // Debounce time in milliseconds

// Touch sensor pins (ESP32 capacitive touch pins)
const int touchPins[NUM_TOUCH_SENSORS] = {
    T0,  // GPIO 4
    T1,  // GPIO 0
    T2,  // GPIO 2
    T3,  // GPIO 15
    T4,  // GPIO 13
    T5   // GPIO 12
};

// Touch sensor names
const char* touchNames[NUM_TOUCH_SENSORS] = {
    "Touch-0",
    "Touch-1", 
    "Touch-2",
    "Touch-3",
    "Touch-4",
    "Touch-5"
};

// Touch sensor states
bool touchStates[NUM_TOUCH_SENSORS] = {false};
bool lastTouchStates[NUM_TOUCH_SENSORS] = {false};
unsigned long lastTouchTime[NUM_TOUCH_SENSORS] = {0};
int touchValues[NUM_TOUCH_SENSORS] = {0};

// LED pins for visual feedback
#define LED_BUILTIN_PIN 2
#define LED_TOUCH_PIN 5  // GPIO 5 for touch indication

// Status variables
unsigned long lastPrint = 0;
unsigned long lastStatusUpdate = 0;
int touchEventCount = 0;
bool anyTouchActive = false;

// WiFi credentials
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Connection status
bool wifiConnected = false;

// OTA settings
const char* ota_hostname = "esp32-touch-sensor";
const char* ota_password = "touch123";

// Function prototypes
void initializeTouchSensors();
void readTouchSensors();
void updateTouchStates();
void handleTouchEvents();
void printTouchStatus();
void printSystemInfo();
void initializeWiFi();
void initializeOTA();

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(2000);  // Increased delay for better serial stability
    
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("    ESP32 Touch Sensor Firmware");
    Serial.println("========================================");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.printf("Chip ID: %08X\n", ESP.getEfuseMac());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Flash Speed: %d Hz\n", ESP.getFlashChipSpeed());
    Serial.println("========================================");
    Serial.println("Touch sensor shortage detection active!");
    Serial.println("========================================");
    
    // Debug: Print system information
    Serial.println("DEBUG: System initialization started");
    Serial.printf("DEBUG: Serial baud rate: %d\n", 115200);
    Serial.printf("DEBUG: Free heap at startup: %d bytes\n", ESP.getFreeHeap());
    
    // Initialize LEDs
    Serial.println("DEBUG: Initializing LEDs...");
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    pinMode(LED_TOUCH_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW);
    digitalWrite(LED_TOUCH_PIN, LOW);
    
    Serial.println("DEBUG: LEDs initialized successfully");
    Serial.printf("DEBUG: LED_BUILTIN_PIN = %d, LED_TOUCH_PIN = %d\n", LED_BUILTIN_PIN, LED_TOUCH_PIN);
    
    // Initialize touch sensors
    Serial.println("DEBUG: Starting touch sensor initialization...");
    initializeTouchSensors();
    
    // Initialize WiFi
    initializeWiFi();
    
    // Initialize OTA if WiFi is connected
    if (wifiConnected) {
        initializeOTA();
    }
    
    Serial.println("DEBUG: Setup complete - monitoring touch sensors...");
    Serial.println("Touch any sensor to detect electrical shortage!");
    
    // Debug: Test LED functionality
    Serial.println("DEBUG: Testing LED functionality...");
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN_PIN, HIGH);
        digitalWrite(LED_TOUCH_PIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN_PIN, LOW);
        digitalWrite(LED_TOUCH_PIN, LOW);
        delay(200);
    }
    Serial.println("DEBUG: LED test complete");
    
    // Debug: Test touch sensor readings
    Serial.println("DEBUG: Testing touch sensor readings...");
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        int testValue = touchRead(touchPins[i]);
        Serial.printf("DEBUG: Touch sensor %d (%s) test reading: %d\n", 
                     i, touchNames[i], testValue);
    }
    Serial.println("DEBUG: Touch sensor test complete");
    
    Serial.println("DEBUG: System ready for operation!");
    Serial.println();
}

void loop() {
    static unsigned long loopCount = 0;
    loopCount++;
    
    // Debug: Print loop count every 1000 iterations
    if (loopCount % 1000 == 0) {
        Serial.printf("DEBUG: Loop count: %lu, Free heap: %d bytes\n", loopCount, ESP.getFreeHeap());
    }
    
    // Handle OTA updates
    if (wifiConnected) {
        ArduinoOTA.handle();
    }
    
    // Read touch sensors
    readTouchSensors();
    
    // Update touch states and handle events
    updateTouchStates();
    handleTouchEvents();
    
    // Print status every 2 seconds
    if (millis() - lastPrint >= 2000) {
        lastPrint = millis();
        printTouchStatus();
    }
    
    // Print system info every 30 seconds
    if (millis() - lastStatusUpdate >= 30000) {
        lastStatusUpdate = millis();
        printSystemInfo();
    }
    
    // Update LED indicators
    digitalWrite(LED_BUILTIN_PIN, anyTouchActive ? HIGH : LOW);
    digitalWrite(LED_TOUCH_PIN, anyTouchActive ? HIGH : LOW);
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

void initializeTouchSensors() {
    Serial.println("DEBUG: Initializing touch sensors...");
    Serial.printf("DEBUG: Number of touch sensors: %d\n", NUM_TOUCH_SENSORS);
    Serial.printf("DEBUG: Touch threshold: %d\n", TOUCH_THRESHOLD);
    Serial.printf("DEBUG: Touch debounce: %d ms\n", TOUCH_DEBOUNCE_MS);
    
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        Serial.printf("DEBUG: Setting up touch sensor %d...\n", i);
        
        // Set touch sensor threshold
        touchSetCycles(0x1000, 0x1000);
        
        // Initialize touch sensor states
        touchStates[i] = false;
        lastTouchStates[i] = false;
        lastTouchTime[i] = 0;
        touchValues[i] = 0;
        
        // Test initial touch reading
        int initialValue = touchRead(touchPins[i]);
        touchValues[i] = initialValue;
        
        Serial.printf("DEBUG: Touch sensor %d (%s) on GPIO %d initialized\n", 
                     i, touchNames[i], touchPins[i]);
        Serial.printf("DEBUG: Initial touch value: %d\n", initialValue);
    }
    
    Serial.println("DEBUG: Touch sensors initialized successfully");
    Serial.println("DEBUG: Starting touch sensor monitoring...");
}

void readTouchSensors() {
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        // Read raw touch value
        touchValues[i] = touchRead(touchPins[i]);
    }
}

void updateTouchStates() {
    anyTouchActive = false;
    
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        // Check if touch is detected (lower values indicate touch)
        bool currentTouch = (touchValues[i] < TOUCH_THRESHOLD);
        
        // Debounce the touch detection
        if (currentTouch != lastTouchStates[i]) {
            if (millis() - lastTouchTime[i] > TOUCH_DEBOUNCE_MS) {
                touchStates[i] = currentTouch;
                lastTouchStates[i] = currentTouch;
                lastTouchTime[i] = millis();
            }
        }
        
        if (touchStates[i]) {
            anyTouchActive = true;
        }
    }
}

void handleTouchEvents() {
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        // Detect touch press (transition from not touched to touched)
        if (touchStates[i] && !lastTouchStates[i]) {
            touchEventCount++;
            Serial.printf("DEBUG: Touch press detected on sensor %d\n", i);
            Serial.printf("🔴 TOUCH DETECTED! Sensor %d (%s) - SHORTAGE ALERT!\n", 
                         i, touchNames[i]);
            Serial.printf("   Raw value: %d (threshold: %d)\n", touchValues[i], TOUCH_THRESHOLD);
            Serial.printf("   Time: %lu ms\n", millis());
            Serial.printf("   GPIO Pin: %d\n", touchPins[i]);
            Serial.println("   ⚠️  Electrical shortage or contact detected!");
        }
        
        // Detect touch release (transition from touched to not touched)
        if (!touchStates[i] && lastTouchStates[i]) {
            Serial.printf("DEBUG: Touch release detected on sensor %d\n", i);
            Serial.printf("🟢 Touch released on sensor %d (%s)\n", i, touchNames[i]);
            Serial.printf("   Raw value: %d (threshold: %d)\n", touchValues[i], TOUCH_THRESHOLD);
            Serial.printf("   GPIO Pin: %d\n", touchPins[i]);
        }
    }
}

void printTouchStatus() {
    Serial.printf("[%lu] Touch Status: ", millis() / 1000);
    
    for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
        Serial.printf("%s:%d(%d) ", touchNames[i], 
                     touchStates[i] ? 1 : 0, touchValues[i]);
    }
    
    Serial.printf("Events:%d ", touchEventCount);
    Serial.printf("Active:%s ", anyTouchActive ? "YES" : "NO");
    Serial.printf("Free:%d", ESP.getFreeHeap());
    
    if (wifiConnected) {
        Serial.printf(" WiFi:%s", WiFi.localIP().toString().c_str());
    }
    
    Serial.println();
}

void printSystemInfo() {
    Serial.println("--- System Info ---");
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Chip ID: %08X\n", ESP.getEfuseMac());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Touch events: %d\n", touchEventCount);
    Serial.printf("Active sensors: %d\n", anyTouchActive ? 1 : 0);
    
    if (wifiConnected) {
        Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("WiFi IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("OTA Hostname: %s\n", ota_hostname);
    } else {
        Serial.println("WiFi: Disconnected");
    }
    
    Serial.println("------------------");
}

void initializeWiFi() {
    Serial.println("DEBUG: Initializing WiFi...");
    Serial.printf("DEBUG: WiFi SSID: %s\n", ssid);
    Serial.printf("DEBUG: WiFi password length: %d\n", strlen(password));
    
    WiFi.mode(WIFI_STA);
    Serial.println("DEBUG: WiFi mode set to STA");
    
    WiFi.begin(ssid, password);
    Serial.println("DEBUG: WiFi.begin() called");
    
    // Wait for WiFi connection
    int wifi_attempts = 0;
    Serial.println("DEBUG: Waiting for WiFi connection...");
    while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
        delay(500);
        Serial.printf("DEBUG: WiFi attempt %d, status: %d\n", wifi_attempts + 1, WiFi.status());
        wifi_attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("DEBUG: WiFi connected successfully!");
        Serial.printf("DEBUG: IP address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("DEBUG: MAC address: %s\n", WiFi.macAddress().c_str());
        Serial.printf("DEBUG: RSSI: %d dBm\n", WiFi.RSSI());
    } else {
        Serial.println("DEBUG: WiFi connection failed");
        Serial.printf("DEBUG: Final WiFi status: %d\n", WiFi.status());
        Serial.println("DEBUG: Continuing without network");
    }
}

void initializeOTA() {
    Serial.println("Initializing OTA...");
    
    ArduinoOTA.setHostname(ota_hostname);
    ArduinoOTA.setPassword(ota_password);
    
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("OTA Start updating " + type);
        digitalWrite(LED_BUILTIN_PIN, HIGH);
        digitalWrite(LED_TOUCH_PIN, HIGH);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA End");
        digitalWrite(LED_BUILTIN_PIN, LOW);
        digitalWrite(LED_TOUCH_PIN, LOW);
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
}