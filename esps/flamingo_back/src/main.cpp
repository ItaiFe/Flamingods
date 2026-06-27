/**
 * Flamingo ESP32 - LED Lighting Controller
 * 
 * Controls 4 LED strips with different lighting plans triggered via HTTP endpoints.
 * Designed for Midburn art installation with local network control.
 * 
 * Includes OTA (Over-The-Air) update capability
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "led_plans.h"

// Network Configuration
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Ethernet Configuration (ESP32-ETH01V1.4)
// LAN8720A PHY configuration
#define ETH_PHY_ADDR 1
#define ETH_PHY_POWER 16  // POWER parameter for ETH.begin()
#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_PHY_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN

// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Web Server
WebServer server(80);

// Lighting plan enum
enum LightingPlan {
    PLAN_IDLE = 0,
    PLAN_SKIP = 1,
    PLAN_SHOW = 2,
    PLAN_SPECIAL = 3,
    PLAN_MIXED_COLORS = 4,  // Shows blended colors from stations
    PLAN_PARTY = 5           // Crazy party mode (10 seconds)
};

// Color tracking for multi-station color mixing
#define MAX_ACTIVE_COLORS 10
#define COLOR_TIMEOUT_MS 100  // Colors expire after 100ms (requires continuous updates)

struct ActiveColor {
    CRGB color;
    int stationId;
    unsigned long timestamp;
    bool active;
};

ActiveColor activeColors[MAX_ACTIVE_COLORS];

// Status variables
LightingPlan currentPlan = PLAN_IDLE;
bool ethernetConnected = false;
bool wifiConnected = false;
unsigned long lastStatusUpdate = 0;

// Station tracking (for multiple concurrent stations)
int lastStationId = 0;
String lastStationName = "";
unsigned long lastRequestTimestamp = 0;
unsigned long lastRequestReceived = 0;
unsigned long totalRequests = 0;

// Party mode tracking
#define PARTY_MODE_DURATION_MS 10000  // 10 seconds
unsigned long partyModeStartTime = 0;
LightingPlan previousPlan = PLAN_IDLE;

// OTA variables
bool otaInProgress = false;
unsigned long otaStartTime = 0;
int otaProgress = 0;

// Function prototypes
void WiFiEvent(WiFiEvent_t event);
void setupEthernet();
void setupWiFi();
void setupServer();
void setupOTA();
void handleRoot();
void handleActiveColors();
void handleIdle();
void handleSkip();
void handleShow();
void handleSpecial();
void handleStationColor();
void handleStationMixedColor();
void handleStationParty();
void handleStatus();
void handleHealth();
void handleVersion();
void handleOTA();
void handleOTAStatus();
void handleNotFound();
void playPartyMode();

// Color mixing functions
CRGB colorNameToRGB(const char* colorName);
void addActiveColor(CRGB color, int stationId);
void cleanupExpiredColors();
CRGB getBlendedColor();
void playMixedColorsPattern();

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Flamingo ESP32 Starting ===");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);

    // Initialize active colors array
    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        activeColors[i].active = false;
        activeColors[i].color = CRGB::Black;
        activeColors[i].stationId = 0;
        activeColors[i].timestamp = 0;
    }

    // Initialize LED strips (3 strips on GPIO 2, 4, 5 - all display same pattern)
    FastLED.addLeds<WS2812B, LED_STRIP_1_PIN, GRB>(leds_strip_1, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_2_PIN, GRB>(leds_strip_2, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_3_PIN, GRB>(leds_strip_3, NUM_LEDS_PER_STRIP);

    FastLED.setBrightness(BRIGHTNESS);
    clearAllLeds();
    
    // Setup Ethernet (primary connection)
    setupEthernet();
    
    // Setup WiFi (fallback connection)
    if (!ethernetConnected) {
        setupWiFi();
    }
    
    // Setup OTA
    setupOTA();
    
    // Setup HTTP server
    setupServer();
    
    Serial.println("Flamingo ESP32 initialization complete!");
}

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();

    // Handle HTTP requests
    server.handleClient();

    // Check if party mode timeout
    if (currentPlan == PLAN_PARTY) {
        if (millis() - partyModeStartTime >= PARTY_MODE_DURATION_MS) {
            Serial.println("Party mode ended - returning to previous plan");
            currentPlan = previousPlan;
        }
    }

    // Clean up expired colors
    cleanupExpiredColors();

    // Update LED patterns based on current plan
    switch (currentPlan) {
        case PLAN_IDLE:
            playIdleAnimation();
            break;
        case PLAN_SKIP:
        case PLAN_SHOW:
        case PLAN_SPECIAL:
            playMovingPattern();
            break;
        case PLAN_MIXED_COLORS:
            playMixedColorsPattern();
            break;
        case PLAN_PARTY:
            playPartyMode();
            break;
    }
    
    // Status updates
    if (millis() - lastStatusUpdate > 5000) {
        lastStatusUpdate = millis();
        if (ethernetConnected) {
            Serial.printf("Status: Plan %d, Ethernet: %s, OTA: %s\n", 
                currentPlan, ETH.localIP().toString().c_str(),
                otaInProgress ? "In Progress" : "Idle");
        } else if (wifiConnected) {
            Serial.printf("Status: Plan %d, WiFi: %s, RSSI: %d, OTA: %s\n", 
                currentPlan, WiFi.localIP().toString().c_str(), WiFi.RSSI(),
                otaInProgress ? "In Progress" : "Idle");
        } else {
            Serial.printf("Status: Plan %d, Network: Disconnected, OTA: %s\n", 
                currentPlan, otaInProgress ? "In Progress" : "Idle");
        }
    }
    
    // Small delay for stability
    delay(20);
}

// Ethernet event handler
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            ETH.setHostname("flamingo-esp32.local");
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

void setupEthernet() {
    Serial.println("Initializing Ethernet...");
    
    // Register event handler
    WiFi.onEvent(WiFiEvent);
    
    // Manual power control for ESP32-ETH01V1.4
    pinMode(ETH_PHY_POWER, OUTPUT);
    digitalWrite(ETH_PHY_POWER, HIGH);  // Power on the PHY
    delay(500);  // Give time for power to stabilize
    
    // Initialize Ethernet with LAN8720A PHY
    bool eth_init_result = ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
    
    if (eth_init_result) {
        Serial.println("Ethernet initialization successful - waiting for connection...");
    } else {
        Serial.println("Ethernet initialization failed - will try WiFi fallback");
    }
    
    // Wait for Ethernet connection
    int ethernet_attempts = 0;
    while (!ethernetConnected && ethernet_attempts < 20) {
        delay(500);
        Serial.print(".");
        ethernet_attempts++;
    }
    
    if (ethernetConnected) {
        Serial.println();
        Serial.println("Ethernet connected!");
        Serial.printf("IP address: %s\n", ETH.localIP().toString().c_str());
    } else {
        Serial.println();
        Serial.println("Ethernet connection failed - will try WiFi fallback");
    }
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
    // Only setup OTA if we have network connectivity
    if (!ethernetConnected && !wifiConnected) {
        Serial.println("No network connection - skipping OTA setup");
        return;
    }
    
    // Configure OTA
    ArduinoOTA.setHostname("flamingo-esp32");
    ArduinoOTA.setPassword("flamingods2024");
    
    // OTA callbacks
    ArduinoOTA.onStart([]() {
        otaInProgress = true;
        otaStartTime = millis();
        otaProgress = 0;
        Serial.println("OTA Update Started");
        
        // Switch to idle mode during OTA
        currentPlan = PLAN_IDLE;
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
        currentPlan = PLAN_IDLE;
    });
    
    ArduinoOTA.begin();
    if (ethernetConnected) {
        Serial.println("OTA initialized (Ethernet)");
    } else if (wifiConnected) {
        Serial.println("OTA initialized (WiFi)");
    }
}

void handleRoot() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Flamingo ESP32 - Station Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #1a1a1a;
            color: #fff;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            text-align: center;
            color: #4CAF50;
        }
        .status-card {
            background: #2a2a2a;
            border-radius: 8px;
            padding: 20px;
            margin: 15px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.3);
        }
        .color-preview {
            width: 100%;
            height: 100px;
            border-radius: 8px;
            margin: 15px 0;
            border: 2px solid #444;
            transition: background-color 0.5s ease;
        }
        .station {
            background: #333;
            padding: 10px;
            margin: 8px 0;
            border-radius: 4px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .station.expired {
            opacity: 0.4;
        }
        .color-dot {
            width: 30px;
            height: 30px;
            border-radius: 50%;
            border: 2px solid #666;
        }
        .stat {
            display: inline-block;
            margin: 10px 20px 10px 0;
        }
        .stat-label {
            color: #888;
            font-size: 12px;
        }
        .stat-value {
            font-size: 24px;
            font-weight: bold;
            color: #4CAF50;
        }
        .plan-badge {
            display: inline-block;
            padding: 5px 15px;
            background: #4CAF50;
            border-radius: 20px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🦩 Flamingo ESP32</h1>

        <div class="status-card">
            <h2>Current State</h2>
            <div>
                <span class="stat-label">Plan:</span>
                <span class="plan-badge" id="plan">LOADING...</span>
            </div>
            <div id="colorPreview" class="color-preview"></div>
            <div style="text-align: center; color: #888;" id="colorInfo">Blended Color</div>
        </div>

        <div class="status-card">
            <h2>Active Stations</h2>
            <div id="stations">Loading...</div>
        </div>

        <div class="status-card">
            <h2>Statistics</h2>
            <div class="stat">
                <div class="stat-label">Total Requests</div>
                <div class="stat-value" id="totalRequests">0</div>
            </div>
            <div class="stat">
                <div class="stat-label">Last Station</div>
                <div class="stat-value" id="lastStation">-</div>
            </div>
            <div class="stat">
                <div class="stat-label">Uptime</div>
                <div class="stat-value" id="uptime">0s</div>
            </div>
        </div>
    </div>

    <script>
        const planNames = ['IDLE', 'SKIP', 'SHOW', 'SPECIAL', 'MIXED_COLORS'];

        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    // Update plan
                    document.getElementById('plan').textContent = planNames[data.current_plan] || 'UNKNOWN';

                    // Update stats
                    document.getElementById('totalRequests').textContent = data.total_requests || 0;
                    document.getElementById('lastStation').textContent =
                        data.last_station_name || '-';
                    document.getElementById('uptime').textContent =
                        Math.floor(data.uptime / 60) + 'm ' + (data.uptime % 60) + 's';

                    // Update color preview (request active colors)
                    fetch('/active-colors')
                        .then(r => r.json())
                        .then(colorData => {
                            updateColorPreview(colorData);
                            updateStations(colorData);
                        });
                })
                .catch(e => console.error('Error:', e));
        }

        function updateColorPreview(colorData) {
            const preview = document.getElementById('colorPreview');
            if (colorData.blended) {
                const c = colorData.blended;
                preview.style.backgroundColor = `rgb(${c.r}, ${c.g}, ${c.b})`;
                document.getElementById('colorInfo').textContent =
                    `RGB(${c.r}, ${c.g}, ${c.b})`;
            } else {
                preview.style.backgroundColor = '#000';
                document.getElementById('colorInfo').textContent = 'No active colors';
            }
        }

        function updateStations(colorData) {
            const container = document.getElementById('stations');
            if (!colorData.active || colorData.active.length === 0) {
                container.innerHTML = '<div style="color: #888; text-align: center;">No active stations</div>';
                return;
            }

            let html = '';
            colorData.active.forEach(station => {
                const age = Math.floor((Date.now() - station.timestamp) / 1000);
                const expired = age > 5;
                html += `
                    <div class="station ${expired ? 'expired' : ''}">
                        <div>
                            <strong>Station ${station.station_id}</strong>
                            <span style="color: #888; margin-left: 10px;">${age}s ago</span>
                        </div>
                        <div class="color-dot" style="background: rgb(${station.color.r}, ${station.color.g}, ${station.color.b});"></div>
                    </div>
                `;
            });
            container.innerHTML = html;
        }

        // Update every 1 second
        updateStatus();
        setInterval(updateStatus, 1000);
    </script>
</body>
</html>
)";
    server.send(200, "text/html", html);
}

void handleActiveColors() {
    StaticJsonDocument<1024> doc;

    // Get blended color
    CRGB blended = getBlendedColor();
    JsonObject blendedObj = doc.createNestedObject("blended");
    blendedObj["r"] = blended.r;
    blendedObj["g"] = blended.g;
    blendedObj["b"] = blended.b;

    // Add active colors array
    JsonArray active = doc.createNestedArray("active");
    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        if (activeColors[i].active) {
            JsonObject station = active.createNestedObject();
            station["station_id"] = activeColors[i].stationId;
            station["timestamp"] = activeColors[i].timestamp;

            JsonObject color = station.createNestedObject("color");
            color["r"] = activeColors[i].color.r;
            color["g"] = activeColors[i].color.g;
            color["b"] = activeColors[i].color.b;
        }
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void setupServer() {
    // HTTP endpoints
    server.on("/", HTTP_GET, handleRoot);
    server.on("/active-colors", HTTP_GET, handleActiveColors);
    server.on("/idle", HTTP_POST, handleIdle);
    server.on("/skip", HTTP_POST, handleSkip);
    server.on("/show", HTTP_POST, handleShow);
    server.on("/special", HTTP_POST, handleSpecial);

    // Station endpoints (for button controller integration)
    server.on("/station-color", HTTP_POST, handleStationColor);
    server.on("/station-mixed-color", HTTP_POST, handleStationMixedColor);
    server.on("/station-party", HTTP_POST, handleStationParty);

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

void handleIdle() {
    Serial.println("POST /idle - Switching to IDLE plan");
    currentPlan = PLAN_IDLE;
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"idle\"}");
}

void handleSkip() {
    Serial.println("POST /skip - Switching to SKIP plan");
    currentPlan = PLAN_SKIP;
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"skip\"}");
}

void handleShow() {
    Serial.println("POST /show - Switching to SHOW plan");
    currentPlan = PLAN_SHOW;
    
    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"show\"}");
}

void handleSpecial() {
    Serial.println("POST /special - Switching to SPECIAL plan");
    currentPlan = PLAN_SPECIAL;

    server.send(200, "application/json", "{\"status\":\"success\",\"plan\":\"special\"}");
}

void handleStationColor() {
    // Track total requests
    totalRequests++;

    // Parse JSON payload from station
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error) {
        Serial.printf("POST /station-color - JSON parse error: %s\n", error.c_str());
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    // Extract data
    int stationId = doc["station_id"] | 0;
    const char* stationName = doc["station_name"] | "unknown";
    const char* color = doc["color"] | "unknown";
    unsigned long timestamp = doc["timestamp"] | millis();

    // Check if this is a newer request than what we have
    // This prevents race conditions where old requests override newer ones
    if (timestamp < lastRequestTimestamp && (lastRequestTimestamp - timestamp) < 60000) {
        Serial.printf("POST /station-color - Ignoring old request from Station %d (timestamp: %lu vs %lu)\n",
            stationId, timestamp, lastRequestTimestamp);
        // Still send success to not confuse the station
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Request acknowledged but superseded\"}");
        return;
    }

    // Update tracking
    lastStationId = stationId;
    lastStationName = String(stationName);
    lastRequestTimestamp = timestamp;
    lastRequestReceived = millis();

    Serial.printf("POST /station-color - Station %d (%s) pressed %s (total requests: %lu)\n",
        stationId, stationName, color, totalRequests);

    // Add color to active colors for mixing
    CRGB colorValue = colorNameToRGB(color);
    addActiveColor(colorValue, stationId);

    // Switch to mixed colors plan
    currentPlan = PLAN_MIXED_COLORS;

    // Send success response immediately (don't wait for LED update)
    StaticJsonDocument<128> response;
    response["status"] = "success";
    response["color"] = color;
    response["station_id"] = stationId;
    response["total_requests"] = totalRequests;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStationMixedColor() {
    // Track total requests
    totalRequests++;

    // Parse JSON payload from station
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error) {
        Serial.printf("POST /station-mixed-color - JSON parse error: %s\n", error.c_str());
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    // Extract data
    int stationId = doc["station_id"] | 0;
    const char* stationName = doc["station_name"] | "unknown";
    JsonArray colors = doc["colors"];
    unsigned long timestamp = doc["timestamp"] | millis();

    // Check if this is a newer request than what we have
    // This prevents race conditions where old requests override newer ones
    if (timestamp < lastRequestTimestamp && (lastRequestTimestamp - timestamp) < 60000) {
        Serial.printf("POST /station-mixed-color - Ignoring old request from Station %d (timestamp: %lu vs %lu)\n",
            stationId, timestamp, lastRequestTimestamp);
        // Still send success to not confuse the station
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Request acknowledged but superseded\"}");
        return;
    }

    // Update tracking
    lastStationId = stationId;
    lastStationName = String(stationName);
    lastRequestTimestamp = timestamp;
    lastRequestReceived = millis();

    Serial.printf("POST /station-mixed-color - Station %d (%s) pressed multiple buttons (total requests: %lu): ",
        stationId, stationName, totalRequests);
    for (JsonVariant color : colors) {
        Serial.printf("%s ", color.as<const char*>());
    }
    Serial.println();

    // Add all colors to active colors for mixing
    // Use unique virtual station IDs to allow multiple colors from same station
    int colorIndex = 0;
    for (JsonVariant color : colors) {
        CRGB colorValue = colorNameToRGB(color.as<const char*>());
        // Use station ID * 10 + color index to create unique ID for each color
        int virtualStationId = (stationId * 10) + colorIndex;
        addActiveColor(colorValue, virtualStationId);
        colorIndex++;
    }

    // Switch to mixed colors plan
    currentPlan = PLAN_MIXED_COLORS;

    // Send success response immediately (don't wait for LED update)
    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["station_id"] = stationId;
    response["total_requests"] = totalRequests;
    JsonArray responseColors = response.createNestedArray("colors");
    for (JsonVariant color : colors) {
        responseColors.add(color);
    }

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStationParty() {
    // Track total requests
    totalRequests++;

    // Parse JSON payload from station
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error) {
        Serial.printf("POST /station-party - JSON parse error: %s\n", error.c_str());
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    // Extract data
    int stationId = doc["station_id"] | 0;
    const char* stationName = doc["station_name"] | "unknown";
    unsigned long timestamp = doc["timestamp"] | millis();

    // Check if this is a newer request than what we have
    if (timestamp < lastRequestTimestamp && (lastRequestTimestamp - timestamp) < 60000) {
        Serial.printf("POST /station-party - Ignoring old request from Station %d (timestamp: %lu vs %lu)\n",
            stationId, timestamp, lastRequestTimestamp);
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Request acknowledged but superseded\"}");
        return;
    }

    // Update tracking
    lastStationId = stationId;
    lastStationName = String(stationName);
    lastRequestTimestamp = timestamp;
    lastRequestReceived = millis();

    Serial.printf("POST /station-party - Station %d (%s) pressed ALL BUTTONS - PARTY MODE! (total requests: %lu)\n",
        stationId, stationName, totalRequests);

    // Save current plan to return to later
    if (currentPlan != PLAN_PARTY) {
        previousPlan = currentPlan;
    }

    // Activate party mode for 10 seconds
    currentPlan = PLAN_PARTY;
    partyModeStartTime = millis();

    // Send success response immediately
    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["station_id"] = stationId;
    response["action"] = "party";
    response["duration_seconds"] = PARTY_MODE_DURATION_MS / 1000;
    response["total_requests"] = totalRequests;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStatus() {
    StaticJsonDocument<512> doc;
    doc["status"] = "success";
    doc["current_plan"] = currentPlan;
    doc["ethernet_connected"] = ethernetConnected;
    doc["wifi_connected"] = wifiConnected;
    doc["uptime"] = millis() / 1000;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["device"] = "flamingo-esp32.local";
    doc["ota_in_progress"] = otaInProgress;
    doc["ota_progress"] = otaProgress;

    // Station tracking (for monitoring concurrent stations)
    doc["total_requests"] = totalRequests;
    doc["last_station_id"] = lastStationId;
    doc["last_station_name"] = lastStationName;
    if (lastRequestReceived > 0) {
        doc["last_request_age_ms"] = millis() - lastRequestReceived;
    }

    // Network information
    if (ethernetConnected) {
        doc["ip_address"] = ETH.localIP().toString();
        doc["mac_address"] = ETH.macAddress();
        doc["connection_type"] = "ethernet";
        doc["link_speed"] = ETH.linkSpeed();
        doc["full_duplex"] = ETH.fullDuplex();
    } else if (wifiConnected) {
        doc["ip_address"] = WiFi.localIP().toString();
        doc["mac_address"] = WiFi.macAddress();
        doc["connection_type"] = "wifi";
        doc["rssi"] = WiFi.RSSI();
        doc["ssid"] = WiFi.SSID();
    } else {
        doc["connection_type"] = "disconnected";
    }
    
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
    doc["device"] = "flamingo-esp32.local";
    
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

// ============================================================================
// Color Mixing Implementation
// ============================================================================

/**
 * Convert color name string to CRGB color
 */
CRGB colorNameToRGB(const char* colorName) {
    String color = String(colorName);
    color.toLowerCase();

    if (color == "red") return CRGB::Red;
    if (color == "green") return CRGB::Green;
    if (color == "blue") return CRGB::Blue;
    if (color == "yellow") return CRGB::Yellow;
    if (color == "white") return CRGB::White;
    if (color == "orange") return CRGB::Orange;
    if (color == "purple") return CRGB::Purple;
    if (color == "pink") return CRGB::Pink;
    if (color == "cyan") return CRGB::Cyan;

    // Default to white for unknown colors
    return CRGB::White;
}

/**
 * Add a color to the active colors list
 * If the station already has a color, update it
 * Otherwise find an empty slot
 */
void addActiveColor(CRGB color, int stationId) {
    unsigned long now = millis();

    // First check if this station already has an active color - update it
    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        if (activeColors[i].active && activeColors[i].stationId == stationId) {
            activeColors[i].color = color;
            activeColors[i].timestamp = now;
            Serial.printf("Updated color for station %d\n", stationId);
            return;
        }
    }

    // Find an empty slot
    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        if (!activeColors[i].active) {
            activeColors[i].active = true;
            activeColors[i].color = color;
            activeColors[i].stationId = stationId;
            activeColors[i].timestamp = now;
            Serial.printf("Added color for station %d at slot %d\n", stationId, i);
            return;
        }
    }

    // If no empty slot, replace the oldest one
    int oldestIndex = 0;
    unsigned long oldestTime = activeColors[0].timestamp;
    for (int i = 1; i < MAX_ACTIVE_COLORS; i++) {
        if (activeColors[i].timestamp < oldestTime) {
            oldestTime = activeColors[i].timestamp;
            oldestIndex = i;
        }
    }

    activeColors[oldestIndex].active = true;
    activeColors[oldestIndex].color = color;
    activeColors[oldestIndex].stationId = stationId;
    activeColors[oldestIndex].timestamp = now;
    Serial.printf("Replaced oldest color (station %d) with station %d\n",
        activeColors[oldestIndex].stationId, stationId);
}

/**
 * Remove expired colors from the active list
 */
void cleanupExpiredColors() {
    unsigned long now = millis();
    bool anyExpired = false;

    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        if (activeColors[i].active) {
            if (now - activeColors[i].timestamp > COLOR_TIMEOUT_MS) {
                Serial.printf("Expired color from station %d\n", activeColors[i].stationId);
                activeColors[i].active = false;
                anyExpired = true;
            }
        }
    }

    // If all colors expired, return to idle
    if (anyExpired) {
        bool anyActive = false;
        for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
            if (activeColors[i].active) {
                anyActive = true;
                break;
            }
        }

        if (!anyActive && currentPlan == PLAN_MIXED_COLORS) {
            Serial.println("All colors expired - returning to IDLE");
            currentPlan = PLAN_IDLE;
        }
    }
}

/**
 * Blend all active colors together
 * This averages the RGB values of all active colors
 */
CRGB getBlendedColor() {
    int activeCount = 0;
    long totalR = 0, totalG = 0, totalB = 0;

    // Sum up all active colors
    for (int i = 0; i < MAX_ACTIVE_COLORS; i++) {
        if (activeColors[i].active) {
            totalR += activeColors[i].color.r;
            totalG += activeColors[i].color.g;
            totalB += activeColors[i].color.b;
            activeCount++;
        }
    }

    // If no active colors, return black
    if (activeCount == 0) {
        return CRGB::Black;
    }

    // Average the colors
    CRGB blended;
    blended.r = totalR / activeCount;
    blended.g = totalG / activeCount;
    blended.b = totalB / activeCount;

    return blended;
}

/**
 * Display the blended color on all LED strips
 * Creates a solid color fill with slight breathing effect
 */
void playMixedColorsPattern() {
    static unsigned long lastDebug = 0;

    // Get the blended color from all active button presses
    CRGB blendedColor = getBlendedColor();

    // Debug output every 500ms
    if (millis() - lastDebug > 500) {
        lastDebug = millis();
        Serial.printf("Mixed colors: RGB(%d,%d,%d)\n", blendedColor.r, blendedColor.g, blendedColor.b);
    }

    // Fill all 3 strips with the blended color at full brightness
    fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, blendedColor);
    fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, blendedColor);
    fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, blendedColor);

    FastLED.show();
}

/**
 * Party mode - Crazy fast rainbow strobe effect
 * Activated when all buttons are pressed simultaneously
 * Runs for 10 seconds then returns to previous plan
 */
void playPartyMode() {
    static unsigned long lastUpdate = 0;
    static uint8_t hue = 0;
    static uint8_t patternIndex = 0;

    // Update at 60 FPS for crazy fast effect
    if (millis() - lastUpdate < 16) {
        return;
    }
    lastUpdate = millis();

    // Rapid cycling through different patterns
    patternIndex = (millis() / 200) % 4;  // Change pattern every 200ms

    switch (patternIndex) {
        case 0:  // Rapid rainbow chase
            for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                CRGB color = CHSV(hue + (i * 10), 255, 255);
                leds_strip_1[i] = color;
                leds_strip_2[i] = color;
                leds_strip_3[i] = color;
            }
            hue += 8;  // Fast hue change
            break;

        case 1:  // Strobe effect with random colors
            {
                CRGB strobeColor = CHSV(hue, 255, 255);
                fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, strobeColor);
                fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, strobeColor);
                fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, strobeColor);
                hue += 17;  // Jump hue for random colors
            }
            break;

        case 2:  // Alternating colors
            {
                CRGB color1 = CHSV(hue, 255, 255);
                CRGB color2 = CHSV(hue + 128, 255, 255);
                for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                    CRGB color = (i % 2 == 0) ? color1 : color2;
                    leds_strip_1[i] = color;
                    leds_strip_2[i] = color;
                    leds_strip_3[i] = color;
                }
                hue += 5;
            }
            break;

        case 3:  // Sparkle effect
            {
                // Random sparkles
                for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                    if (random8() < 50) {  // 20% chance of sparkle
                        CRGB sparkle = CHSV(random8(), 255, 255);
                        leds_strip_1[i] = sparkle;
                        leds_strip_2[i] = sparkle;
                        leds_strip_3[i] = sparkle;
                    } else {
                        leds_strip_1[i] = CRGB::Black;
                        leds_strip_2[i] = CRGB::Black;
                        leds_strip_3[i] = CRGB::Black;
                    }
                }
            }
            break;
    }

    FastLED.show();
}
