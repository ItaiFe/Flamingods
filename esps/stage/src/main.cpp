/**
 * Stage ESP32 - LED Lighting Controller
 * 
 * Controls 1 LED strip with different lighting plans triggered via HTTP endpoints.
 * Designed for Midburn art installation with local network control.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "led_plans.h"

// WiFi Configuration
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Web Server
WebServer server(80);

// LED Plans Controller
LEDPlans ledController;

// Status variables
LightingPlan currentPlan = PLAN_IDLE;
bool wifiConnected = false;
unsigned long lastStatusUpdate = 0;

// Function prototypes
void setupWiFi();
void setupServer();
void handleIdle();
void handleSkip();
void handleShow();
void handleSpecial();
void handleStatus();
void handleHealth();
void handleNotFound();

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Stage ESP32 Starting ===");
    
    // Initialize LED strip
    FastLED.addLeds<WS2812B, LED_STRIP_PIN, GRB>(leds, NUM_LEDS);
    
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    
    // Initialize LED controller
    ledController.begin();
    
    // Setup WiFi
    setupWiFi();
    
    // Setup HTTP server
    setupServer();
    
    Serial.println("Stage ESP32 initialization complete!");
}

void loop() {
    // Handle HTTP requests
    server.handleClient();
    
    // Update LED patterns
    ledController.update();
    
    // Update FastLED
    FastLED.show();
    
    // Status updates
    if (millis() - lastStatusUpdate > 5000) {
        lastStatusUpdate = millis();
        if (wifiConnected) {
            Serial.printf("Status: Plan %d, WiFi: %s, RSSI: %d\n", 
                currentPlan, WiFi.localIP().toString().c_str(), WiFi.RSSI());
        }
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

void setupServer() {
    // HTTP endpoints
    server.on("/idle", HTTP_POST, handleIdle);
    server.on("/skip", HTTP_POST, handleSkip);
    server.on("/show", HTTP_POST, handleShow);
    server.on("/special", HTTP_POST, handleSpecial);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/health", HTTP_GET, handleHealth);
    
    // 404 handler
    server.onNotFound(handleNotFound);
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void handleIdle() {
    Serial.println("POST /idle - Switching to IDLE plan");
    currentPlan = PLAN_IDLE;
    ledController.setPlan(PLAN_IDLE);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"idle\"}");
}

void handleSkip() {
    Serial.println("POST /skip - Switching to SKIP plan");
    currentPlan = PLAN_SKIP;
    ledController.setPlan(PLAN_SKIP);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"skip\"}");
}

void handleShow() {
    Serial.println("POST /show - Switching to SHOW plan");
    currentPlan = PLAN_SHOW;
    ledController.setPlan(PLAN_SHOW);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"show\"}");
}

void handleSpecial() {
    Serial.println("POST /special - Switching to SPECIAL plan");
    currentPlan = PLAN_SPECIAL;
    ledController.setPlan(PLAN_SPECIAL);
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"special\"}");
}

void handleStatus() {
    StaticJsonDocument<200> doc;
    doc["status"] = "success";
    doc["current_plan"] = currentPlan;
    doc["wifi_connected"] = wifiConnected;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
}

void handleHealth() {
    server.send(200, "text/plain", "OK");
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
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
    server.send(404, "text/plain", message);
}
