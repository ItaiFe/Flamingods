/**
 * Crown ESP32 - LED Lighting Controller
 * 
 * Controls 1 LED strip with different lighting plans:
 * - Idle: Soft yellow pulsating halo mode
 * - Button: Crazy colors party mode (triggered via HTTP)
 * - WiFi Fallback: Halo + running colorful pixels
 * 
 * Includes OTA (Over-The-Air) update capability
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "led_plans.h"

// WiFi Configuration
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Web Server
WebServer server(80);

// LED Plans Controller
LEDPlans ledController;

// Status variables
LightingPlan currentPlan = PLAN_IDLE;
bool wifiConnected = false;
unsigned long lastWiFiCheck = 0;
unsigned long lastStatusUpdate = 0;

// OTA variables
bool otaInProgress = false;
unsigned long otaStartTime = 0;
int otaProgress = 0;

// Function prototypes
void setupWiFi();
void setupServer();
void setupOTA();
void checkWiFiConnection();
void handleIdle();
void handleButton();
void handleStatus();
void handleHealth();
void handleVersion();
void handleOTA();
void handleOTAStatus();
void handleNotFound();

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Crown ESP32 Starting ===");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    

    
    // Initialize LED strip
    FastLED.addLeds<WS2811, LED_STRIP_PIN, RBG>(leds, NUM_LEDS);
    
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    
    // Initialize LED controller
    ledController.begin();
    
    // Setup WiFi
    setupWiFi();
    
    // Setup OTA
    setupOTA();
    
    // Setup HTTP server
    setupServer();
    
    // Set initial plan based on WiFi status
    if (wifiConnected) {
        currentPlan = PLAN_IDLE;
        ledController.setPlan(PLAN_IDLE);
    } else {
        currentPlan = PLAN_WIFI_FALLBACK;
        ledController.setPlan(PLAN_WIFI_FALLBACK);
    }
    
    Serial.println("Crown ESP32 initialization complete!");
}

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();
    
    // Handle HTTP requests
    server.handleClient();
    
    // Check WiFi connection periodically
    if (millis() - lastWiFiCheck > 10000) {  // Check every 10 seconds
        checkWiFiConnection();
        lastWiFiCheck = millis();
    }
    
    // Update LED patterns
    ledController.update();
    
    // Update FastLED
    FastLED.show();
    
    // Status updates
    if (millis() - lastStatusUpdate > 5000) {
        lastStatusUpdate = millis();
        Serial.printf("Status: Plan %d, WiFi: %s, OTA: %s\n", 
            currentPlan, 
            wifiConnected ? "Connected" : "Disconnected",
            otaInProgress ? "In Progress" : "Idle");
    }
    
    // Small delay for stability
    delay(20);
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
    ArduinoOTA.setHostname("crown-esp32");
    ArduinoOTA.setPassword("flamingods2024");
    
    // OTA callbacks
    ArduinoOTA.onStart([]() {
        otaInProgress = true;
        otaStartTime = millis();
        otaProgress = 0;
        Serial.println("OTA Update Started");
        
        // Switch to WiFi fallback mode during OTA
        currentPlan = PLAN_WIFI_FALLBACK;
        ledController.setPlan(PLAN_WIFI_FALLBACK);
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
        
        // Return to previous plan
        if (wifiConnected) {
            currentPlan = PLAN_IDLE;
            ledController.setPlan(PLAN_IDLE);
        }
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA initialized");
}

void setupServer() {
    // HTTP endpoints
    server.on("/idle", HTTP_POST, handleIdle);
    server.on("/button", HTTP_POST, handleButton);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/health", HTTP_GET, handleHealth);
    server.on("/version", HTTP_GET, handleVersion);
    server.on("/ota", HTTP_POST, handleOTA);
    server.on("/ota-status", HTTP_GET, handleOTAStatus);
    
    // 404 handler
    server.onNotFound(handleNotFound);
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void checkWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiConnected) {
            // WiFi just reconnected
            wifiConnected = true;
            Serial.println("WiFi reconnected!");
            
            // Switch to idle mode if not in button mode and OTA not in progress
            if (currentPlan != PLAN_BUTTON && !otaInProgress) {
                currentPlan = PLAN_IDLE;
                ledController.setPlan(PLAN_IDLE);
                Serial.println("Switched to IDLE mode");
            }
        }
    } else {
        if (wifiConnected) {
            // WiFi just disconnected
            wifiConnected = false;
            Serial.println("WiFi disconnected!");
            
            // Switch to WiFi fallback mode if not in button mode and OTA not in progress
            if (currentPlan != PLAN_BUTTON && !otaInProgress) {
                currentPlan = PLAN_WIFI_FALLBACK;
                ledController.setPlan(PLAN_WIFI_FALLBACK);
                Serial.println("Switched to WiFi FALLBACK mode");
            }
        }
        
        // Try to reconnect
        WiFi.reconnect();
    }
}

void handleIdle() {
    Serial.println("POST /idle - Switching to IDLE plan");
    currentPlan = PLAN_IDLE;
    ledController.setPlan(PLAN_IDLE);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"idle\"}");
}

void handleButton() {
    Serial.println("POST /button - Switching to BUTTON plan");
    currentPlan = PLAN_BUTTON;
    ledController.setPlan(PLAN_BUTTON);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"button\"}");
}

void handleStatus() {
    StaticJsonDocument<300> doc;
    doc["status"] = "success";
    doc["current_plan"] = currentPlan;
    doc["wifi_connected"] = wifiConnected;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["device"] = "crown-esp32";
    doc["ota_in_progress"] = otaInProgress;
    doc["ota_progress"] = otaProgress;
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

void handleHealth() {
    server.send(200, "text/plain", "OK");
}

void handleVersion() {
    StaticJsonDocument<100> doc;
    doc["status"] = "success";
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["device"] = "crown-esp32";
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

void handleOTA() {
    if (otaInProgress) {
        server.send(409, "application/json", "{\"status\":\"error\",\"message\":\"OTA already in progress\"}");
        return;
    }
    
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"OTA update ready. Use Arduino IDE or esptool to upload firmware.\"}");
}

void handleOTAStatus() {
    StaticJsonDocument<150> doc;
    doc["status"] = "success";
    doc["ota_in_progress"] = otaInProgress;
    doc["ota_progress"] = otaProgress;
    doc["uptime"] = millis() / 1000;
    
    if (otaInProgress) {
        doc["ota_duration"] = (millis() - otaStartTime) / 1000;
    }
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i);
    }
    
    server.send(404, "text/plain", message);
}
