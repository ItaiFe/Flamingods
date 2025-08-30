/**
 * Station ESP32 - Button Controller for Flamingo Server
 * 
 * Features:
 * - 5 physical buttons (Red, Green, Blue, Yellow, White)
 * - HTTP communication with Flamingo server
 * - Color mixing when multiple buttons are pressed
 * - 4 stations, each connected to flamingo server
 * 
 * Includes OTA (Over-The-Air) update capability
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "station_config.h"

// WiFi Configuration
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Flamingo Server Configuration
const char* flamingoServer = "http://192.168.1.200";  // Crown ESP IP (flamingo server)
const int flamingoPort = 80;



// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Button Configuration
#define BUTTON_RED_PIN     2
#define BUTTON_GREEN_PIN   4
#define BUTTON_BLUE_PIN    5
#define BUTTON_YELLOW_PIN  18
#define BUTTON_WHITE_PIN   19

// Button debounce time
#define DEBOUNCE_TIME     50

// Button states
struct ButtonState {
    bool pressed;
    bool lastState;
    unsigned long lastDebounceTime;
    bool active;
};

ButtonState buttons[5] = {0};  // Red, Green, Blue, Yellow, White

// Color definitions for buttons
const char* buttonColors[5] = {"red", "green", "blue", "yellow", "white"};

// HTTP client
HTTPClient http;

// Status variables
bool wifiConnected = false;
unsigned long lastStatusUpdate = 0;
unsigned long lastButtonCheck = 0;

// OTA variables
bool otaInProgress = false;
unsigned long otaStartTime = 0;
int otaProgress = 0;

// Function prototypes
void setupWiFi();
void setupButtons();
void setupOTA();
void checkButtons();
void sendColorToFlamingo(const char* color);
void sendMixedColorToFlamingo();

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Station ESP32 Starting ===");
    Serial.printf("Station ID: %d (%s)\n", STATION_ID, STATION_NAME);
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    
    // Setup buttons
    setupButtons();
    
    // Setup WiFi
    setupWiFi();
    
    // Setup OTA
    setupOTA();
    
    Serial.println("Station ESP32 initialization complete!");
}

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();
    
    // Check buttons every 10ms
    if (millis() - lastButtonCheck > 10) {
        checkButtons();
        lastButtonCheck = millis();
    }
    
    // Status updates
    if (millis() - lastStatusUpdate > 5000) {
        lastStatusUpdate = millis();
        if (wifiConnected) {
            Serial.printf("Status: WiFi: %s, RSSI: %d, OTA: %s\n", 
                WiFi.localIP().toString().c_str(), WiFi.RSSI(),
                otaInProgress ? "In Progress" : "Idle");
        }
    }
    
    // Small delay for stability
    delay(10);
}

void setupButtons() {
    // Configure button pins as inputs with internal pull-up resistors
    pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
    pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);
    pinMode(BUTTON_WHITE_PIN, INPUT_PULLUP);
    
    // Initialize button states
    for (int i = 0; i < 5; i++) {
        buttons[i].pressed = false;
        buttons[i].lastState = HIGH;
        buttons[i].lastDebounceTime = 0;
        buttons[i].active = false;
    }
    
    Serial.println("Buttons initialized");
}

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
        wifiConnected = false;
    }
}

void setupOTA() {
    // Configure OTA
    ArduinoOTA.setHostname(STATION_NAME);
    ArduinoOTA.setPassword("flamingods2024");
    
    // OTA callbacks
    ArduinoOTA.onStart([]() {
        otaInProgress = true;
        otaStartTime = millis();
        otaProgress = 0;
        Serial.println("OTA Update Started");
    });
    
    ArduinoOTA.onEnd([]() {
        otaInProgress = false;
        Serial.println("OTA Update Completed");
        Serial.println("Rebooting in 3 seconds...");
        delay(3000);
        ESP.restart();
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        otaProgress = (progress * 100) / total;
        Serial.printf("OTA Progress: %u%%\r", otaProgress);
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        otaInProgress = false;
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA initialized");
}

void checkButtons() {
    // Check each button
    int buttonPins[5] = {BUTTON_RED_PIN, BUTTON_GREEN_PIN, BUTTON_BLUE_PIN, BUTTON_YELLOW_PIN, BUTTON_WHITE_PIN};
    
    for (int i = 0; i < 5; i++) {
        int reading = digitalRead(buttonPins[i]);
        
        // If the switch changed, due to noise or pressing
        if (reading != buttons[i].lastState) {
            buttons[i].lastDebounceTime = millis();
        }
        
        // If enough time has passed since the last change
        if ((millis() - buttons[i].lastDebounceTime) > DEBOUNCE_TIME) {
            // If the button state has changed
            if (reading != buttons[i].pressed) {
                buttons[i].pressed = reading;
                
                if (buttons[i].pressed == LOW) {  // Button pressed (LOW due to pull-up)
                    buttons[i].active = true;
                    Serial.printf("Button %s pressed\n", buttonColors[i]);
                } else {  // Button released
                    buttons[i].active = false;
                    Serial.printf("Button %s released\n", buttonColors[i]);
                }
            }
        }
        
        buttons[i].lastState = reading;
    }
    
    // Check if any buttons are active
    int activeButtons = 0;
    for (int i = 0; i < 5; i++) {
        if (buttons[i].active) activeButtons++;
    }
    
    // Send appropriate command to flamingo server
    if (activeButtons == 1) {
        // Single button pressed - send single color
        for (int i = 0; i < 5; i++) {
            if (buttons[i].active) {
                sendColorToFlamingo(buttonColors[i]);
                break;
            }
        }
    } else if (activeButtons > 1) {
        // Multiple buttons pressed - send mixed color
        sendMixedColorToFlamingo();
    }
}

void sendColorToFlamingo(const char* color) {
    if (!wifiConnected) {
        Serial.println("WiFi not connected, cannot send to flamingo server");
        return;
    }
    
    Serial.printf("Sending color %s to flamingo server\n", color);
    
    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["station_id"] = STATION_ID;
    doc["station_name"] = STATION_NAME;
    doc["action"] = "color";
    doc["color"] = color;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Send HTTP POST to flamingo server
    String url = String(flamingoServer) + "/station-color";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.printf("Response: %s\n", response.c_str());
    } else {
        Serial.printf("HTTP Error code: %d\n", httpResponseCode);
    }
    
    http.end();
}

void sendMixedColorToFlamingo() {
    if (!wifiConnected) {
        Serial.println("WiFi not connected, cannot send to flamingo server");
        return;
    }
    
    Serial.println("Sending mixed color to flamingo server");
    
    // Create JSON payload with all active colors
    StaticJsonDocument<400> doc;
    doc["station_id"] = STATION_ID;
    doc["station_name"] = STATION_NAME;
    doc["action"] = "mixed-color";
    doc["timestamp"] = millis();
    
    JsonArray colors = doc.createNestedArray("colors");
    for (int i = 0; i < 5; i++) {
        if (buttons[i].active) {
            colors.add(buttonColors[i]);
        }
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Send HTTP POST to flamingo server
    String url = String(flamingoServer) + "/station-mixed-color";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.printf("Response: %s\n", response.c_str());
    } else {
        Serial.printf("HTTP Error code: %d\n", httpResponseCode);
    }
    
    http.end();
}
