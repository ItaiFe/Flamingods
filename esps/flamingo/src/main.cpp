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
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
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

// UDP Server for station button streaming
WiFiUDP udp;
#define UDP_PORT 5000
#define MAX_STATIONS 4

// Lighting plan enum
enum LightingPlan {
    PLAN_IDLE = 0,
    PLAN_SKIP = 1,
    PLAN_SHOW = 2,
    PLAN_SPECIAL = 3,
    PLAN_MIXED_COLORS = 4,   // Shows blended colors from stations
    PLAN_PARTY = 5,           // Crazy party mode (10 seconds)
    PLAN_MEGA_SPECIAL = 6,    // All 3+ stations all-buttons-pressed (23 second sequence)
    PLAN_STATION_1_SPECIAL = 7,  // Station 1: Fire Wave (8 seconds)
    PLAN_STATION_2_SPECIAL = 8,  // Station 2: Ocean Storm (8 seconds)
    PLAN_STATION_3_SPECIAL = 9,  // Station 3: Northern Lights (8 seconds)
    PLAN_STATION_4_SPECIAL = 10  // Station 4: Electric Pulse (8 seconds)
};

// Station button state tracking (UDP-based)
struct StationState {
    uint8_t buttonMask;      // 5 bits: [white][yellow][blue][green][red]
    unsigned long lastUpdate;  // Last UDP packet received
    bool active;              // Station is actively sending data
};

StationState stations[MAX_STATIONS];
#define STATION_TIMEOUT_MS 5000  // Station considered inactive after 5000ms (5 seconds)

// Packet-based color mixing (each packet has extended lifetime for smooth blending)
#define PACKET_LIFETIME_MS 300   // Each UDP packet affects LEDs for 300ms (increased for smoothness)
#define MAX_ACTIVE_PACKETS 100   // Maximum number of recent packets to track (increased buffer)
struct ColorPacket {
    uint8_t stationId;
    uint8_t buttonMask;
    unsigned long timestamp;
    bool active;
};

ColorPacket recentPackets[MAX_ACTIVE_PACKETS];
int packetIndex = 0;  // Circular buffer index

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

// Mega Special mode tracking (all 3 stations all-buttons-pressed)
#define MEGA_SPECIAL_DURATION_MS 23000  // 23 seconds total sequence (added 10s party)
#define MEGA_SPECIAL_TIMEOUT_MS 2000     // 2 second window for all 3 stations
unsigned long megaSpecialStartTime = 0;
bool stationAllButtonsPressed[MAX_STATIONS] = {false, false, false, false};
unsigned long stationAllButtonsTime[MAX_STATIONS] = {0, 0, 0, 0};

// Station-specific special modes (single station all-buttons-pressed)
#define STATION_SPECIAL_DURATION_MS 10000  // 10 seconds per station special mode
unsigned long stationSpecialStartTime = 0;

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
void handleMegaSpecial();
void handleStation1Special();
void handleStation2Special();
void handleStation3Special();
void handleStation4Special();
void handleStatus();
void handleHealth();
void handleVersion();
void handleOTA();
void handleOTAStatus();
void handleNotFound();
void playPartyMode();

// UDP and station management functions
void handleUDP();
void checkStationTimeouts();
CRGB getBlendedColorFromPackets();
CRGB getBlendedColorFromStations();
void playMixedColorsPattern();

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Flamingo ESP32 Starting ===");
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);

    // Initialize station states
    for (int i = 0; i < MAX_STATIONS; i++) {
        stations[i].buttonMask = 0;
        stations[i].lastUpdate = 0;
        stations[i].active = false;
    }

    // Initialize packet buffer
    for (int i = 0; i < MAX_ACTIVE_PACKETS; i++) {
        recentPackets[i].stationId = 0;
        recentPackets[i].buttonMask = 0;
        recentPackets[i].timestamp = 0;
        recentPackets[i].active = false;
    }

    // Initialize LED strips (3 strips on GPIO 2, 4, 12 - all display same pattern)
    // LED strips use RBG color order
    FastLED.addLeds<WS2812B, LED_STRIP_1_PIN, RBG>(leds_strip_1, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_2_PIN, RBG>(leds_strip_2, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_3_PIN, RBG>(leds_strip_3, NUM_LEDS_PER_STRIP);

    FastLED.setBrightness(BRIGHTNESS);
    clearAllLeds();
    
    // Setup Ethernet (primary connection)
    setupEthernet();
    
    // Setup WiFi (fallback connection)
    if (!ethernetConnected) {
        setupWiFi();
    }

    // Setup mDNS for flamingo-esp32.local hostname
    if (ethernetConnected || wifiConnected) {
        if (MDNS.begin("flamingo-esp32")) {
            Serial.println("mDNS responder started: flamingo-esp32.local");
        } else {
            Serial.println("Error setting up mDNS responder!");
        }
    }

    // Setup OTA
    setupOTA();
    
    // Setup HTTP server
    setupServer();

    // Setup UDP server for station button streaming
    udp.begin(UDP_PORT);
    Serial.printf("UDP server started on port %d\n", UDP_PORT);

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

    // Check if mega special mode timeout
    if (currentPlan == PLAN_MEGA_SPECIAL) {
        if (millis() - megaSpecialStartTime >= MEGA_SPECIAL_DURATION_MS) {
            Serial.println("Mega Special mode ended - returning to IDLE");
            currentPlan = PLAN_IDLE;
            // Reset all-buttons tracking
            for (int i = 0; i < MAX_STATIONS; i++) {
                stationAllButtonsPressed[i] = false;
                stationAllButtonsTime[i] = 0;
            }
        }
    }

    // Check if station special mode timeout
    if (currentPlan >= PLAN_STATION_1_SPECIAL && currentPlan <= PLAN_STATION_4_SPECIAL) {
        if (millis() - stationSpecialStartTime >= STATION_SPECIAL_DURATION_MS) {
            Serial.println("Station Special mode ended - returning to IDLE");
            currentPlan = PLAN_IDLE;
        }
    }

    // Handle UDP button state packets from stations
    handleUDP();

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
        case PLAN_MEGA_SPECIAL:
            playMegaSpecialPattern();
            break;
        case PLAN_STATION_1_SPECIAL:
            playStation1Special();
            break;
        case PLAN_STATION_2_SPECIAL:
            playStation2Special();
            break;
        case PLAN_STATION_3_SPECIAL:
            playStation3Special();
            break;
        case PLAN_STATION_4_SPECIAL:
            playStation4Special();
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
            ETH.setHostname("flamingo-esp32");
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
    StaticJsonDocument<512> doc;

    // Get blended color from stations
    CRGB blended = getBlendedColorFromStations();
    JsonObject blendedObj = doc.createNestedObject("blended");
    blendedObj["r"] = blended.r;
    blendedObj["g"] = blended.g;
    blendedObj["b"] = blended.b;

    // Add active stations
    JsonArray active = doc.createNestedArray("stations");
    for (int i = 0; i < MAX_STATIONS; i++) {
        if (stations[i].active && stations[i].buttonMask != 0) {
            JsonObject station = active.createNestedObject();
            station["station_id"] = i + 1;
            station["button_mask"] = stations[i].buttonMask;
            station["last_update"] = stations[i].lastUpdate;
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
    server.on("/mega-special", HTTP_POST, handleMegaSpecial);

    // Station-specific special modes
    server.on("/station1-special", HTTP_POST, handleStation1Special);
    server.on("/station2-special", HTTP_POST, handleStation2Special);
    server.on("/station3-special", HTTP_POST, handleStation3Special);
    server.on("/station4-special", HTTP_POST, handleStation4Special);

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
    // HTTP endpoint deprecated - use UDP for button streaming
    Serial.println("POST /station-color - DEPRECATED: Use UDP port 5000 instead");
    server.send(200, "application/json", "{\"status\":\"deprecated\",\"message\":\"Use UDP port 5000 for button streaming\"}");
}

void handleStationMixedColor() {
    // HTTP endpoint deprecated - use UDP for button streaming
    Serial.println("POST /station-mixed-color - DEPRECATED: Use UDP port 5000 instead");
    server.send(200, "application/json", "{\"status\":\"deprecated\",\"message\":\"Use UDP port 5000 for button streaming\"}");
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

void handleMegaSpecial() {
    Serial.println("POST /mega-special - Activating MEGA SPECIAL MODE (manual trigger)");

    // Activate mega special mode
    currentPlan = PLAN_MEGA_SPECIAL;
    megaSpecialStartTime = millis();

    // Send success response
    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["action"] = "mega_special";
    response["duration_seconds"] = MEGA_SPECIAL_DURATION_MS / 1000;
    response["message"] = "MEGA SPECIAL MODE ACTIVATED!";

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStation1Special() {
    Serial.println("POST /station1-special - Activating STATION 1 FIRE WAVE MODE");

    currentPlan = PLAN_STATION_1_SPECIAL;
    stationSpecialStartTime = millis();

    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["action"] = "station1_special";
    response["mode"] = "Fire Wave";
    response["duration_seconds"] = STATION_SPECIAL_DURATION_MS / 1000;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStation2Special() {
    Serial.println("POST /station2-special - Activating STATION 2 OCEAN STORM MODE");

    currentPlan = PLAN_STATION_2_SPECIAL;
    stationSpecialStartTime = millis();

    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["action"] = "station2_special";
    response["mode"] = "Ocean Storm";
    response["duration_seconds"] = STATION_SPECIAL_DURATION_MS / 1000;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStation3Special() {
    Serial.println("POST /station3-special - Activating STATION 3 NORTHERN LIGHTS MODE");

    currentPlan = PLAN_STATION_3_SPECIAL;
    stationSpecialStartTime = millis();

    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["action"] = "station3_special";
    response["mode"] = "Northern Lights";
    response["duration_seconds"] = STATION_SPECIAL_DURATION_MS / 1000;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void handleStation4Special() {
    Serial.println("POST /station4-special - Activating STATION 4 ELECTRIC PULSE MODE");

    currentPlan = PLAN_STATION_4_SPECIAL;
    stationSpecialStartTime = millis();

    StaticJsonDocument<256> response;
    response["status"] = "success";
    response["action"] = "station4_special";
    response["mode"] = "Electric Pulse";
    response["duration_seconds"] = STATION_SPECIAL_DURATION_MS / 1000;

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
    doc["device"] = "flamingo-esp32";
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
    doc["device"] = "flamingo-esp32";
    
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
 * Handle incoming UDP packets from stations
 * Packet format: [station_id(1 byte)][button_mask(1 byte)]
 * Button mask bits: [white][yellow][blue][green][red]
 * Each packet is stored with 100ms lifetime for blending
 */
void handleUDP() {
    int packetSize = udp.parsePacket();
    if (packetSize >= 2) {
        uint8_t buffer[2];
        udp.read(buffer, 2);

        uint8_t stationId = buffer[0];
        uint8_t buttonMask = buffer[1];

        // Validate station ID (1-4)
        if (stationId >= 1 && stationId <= MAX_STATIONS) {
            unsigned long now = millis();

            // Store packet in circular buffer for 100ms lifetime blending
            recentPackets[packetIndex].stationId = stationId;
            recentPackets[packetIndex].buttonMask = buttonMask;
            recentPackets[packetIndex].timestamp = now;
            recentPackets[packetIndex].active = true;
            packetIndex = (packetIndex + 1) % MAX_ACTIVE_PACKETS;

            // Also update station state for timeout tracking
            int idx = stationId - 1;  // Convert to 0-based index
            stations[idx].buttonMask = buttonMask;
            stations[idx].lastUpdate = now;
            stations[idx].active = true;

            // Debug output (reduced to avoid spam)
            static unsigned long lastDebug = 0;
            if (now - lastDebug > 100) {
                Serial.printf("Station %d: buttons=0x%02X\n", stationId, buttonMask);
                lastDebug = now;
            }

            // Check if all 5 buttons pressed
            if (buttonMask == 0x1F) {
                // Mark this station as having all buttons pressed
                int idx = stationId - 1;
                stationAllButtonsPressed[idx] = true;
                stationAllButtonsTime[idx] = now;

                // Count how many stations have all buttons pressed within timeout window
                int activeStationsWithAllButtons = 0;
                for (int i = 0; i < 3; i++) {  // Check first 3 stations only
                    if (stationAllButtonsPressed[i] && (now - stationAllButtonsTime[i] < MEGA_SPECIAL_TIMEOUT_MS)) {
                        activeStationsWithAllButtons++;
                    }
                }

                // If 3+ stations have all buttons pressed within timeout, trigger MEGA SPECIAL!
                if (activeStationsWithAllButtons >= 3 && currentPlan != PLAN_MEGA_SPECIAL) {
                    Serial.printf("*** ALL 3+ STATIONS ALL BUTTONS PRESSED - MEGA SPECIAL MODE! ***\n");
                    currentPlan = PLAN_MEGA_SPECIAL;
                    megaSpecialStartTime = now;
                }
                // Otherwise single station all-buttons = station-specific special mode
                else if (currentPlan != PLAN_PARTY && currentPlan != PLAN_MEGA_SPECIAL &&
                        currentPlan < PLAN_STATION_1_SPECIAL) {  // Not already in a station special
                    // Trigger station-specific special mode
                    switch (stationId) {
                        case 1:
                            Serial.printf("Station 1: ALL BUTTONS PRESSED - FIRE WAVE MODE!\n");
                            currentPlan = PLAN_STATION_1_SPECIAL;
                            break;
                        case 2:
                            Serial.printf("Station 2: ALL BUTTONS PRESSED - OCEAN STORM MODE!\n");
                            currentPlan = PLAN_STATION_2_SPECIAL;
                            break;
                        case 3:
                            Serial.printf("Station 3: ALL BUTTONS PRESSED - NORTHERN LIGHTS MODE!\n");
                            currentPlan = PLAN_STATION_3_SPECIAL;
                            break;
                        case 4:
                            Serial.printf("Station 4: ALL BUTTONS PRESSED - ELECTRIC PULSE MODE!\n");
                            currentPlan = PLAN_STATION_4_SPECIAL;
                            break;
                    }
                    stationSpecialStartTime = now;
                }
            }
            // If not all buttons, clear the all-buttons flag for this station
            else {
                int idx = stationId - 1;
                stationAllButtonsPressed[idx] = false;
            }

            // Automatically switch to MIXED_COLORS when any buttons are pressed
            if (buttonMask != 0 && buttonMask != 0x1F && currentPlan != PLAN_PARTY && currentPlan != PLAN_MEGA_SPECIAL) {
                if (currentPlan != PLAN_MIXED_COLORS) {
                    Serial.println("Switching to MIXED_COLORS mode");
                    currentPlan = PLAN_MIXED_COLORS;
                }
            }
        }
    }

    // Check for inactive stations and return to IDLE if all inactive
    checkStationTimeouts();
}

/**
 * Check for inactive stations and switch to IDLE if all are inactive
 */
void checkStationTimeouts() {
    unsigned long now = millis();
    bool anyActive = false;
    bool anyButtonsPressed = false;

    for (int i = 0; i < MAX_STATIONS; i++) {
        if (stations[i].active) {
            // Check if station timed out
            if (now - stations[i].lastUpdate > STATION_TIMEOUT_MS) {
                Serial.printf("Station %d timed out\n", i + 1);
                stations[i].active = false;
                stations[i].buttonMask = 0;
            } else {
                anyActive = true;
                if (stations[i].buttonMask != 0) {
                    anyButtonsPressed = true;
                }
            }
        }
    }

    // Return to IDLE if no buttons pressed
    if (!anyButtonsPressed && currentPlan == PLAN_MIXED_COLORS) {
        Serial.println("No buttons pressed - returning to IDLE");
        currentPlan = PLAN_IDLE;
    }
}

/**
 * Blend colors from all packets within their 100ms lifetime window
 * Button mask bits: [white][yellow][blue][green][red]
 * Returns blended color from all active packets
 */
CRGB getBlendedColorFromPackets() {
    unsigned long now = millis();
    int colorCount = 0;
    long totalR = 0, totalG = 0, totalB = 0;
    bool hasWhite = false;

    // Standard button colors (white at 75% brightness)
    // LED strips are RBG order, CRGB handles this automatically
    const CRGB buttonColors[5] = {
        CRGB(255, 0, 0),     // Bit 0 (0x01) - Red
        CRGB(0, 255, 0),     // Bit 1 (0x02) - Green
        CRGB(0, 0, 255),     // Bit 2 (0x04) - Blue
        CRGB(180, 180, 0),   // Bit 3 (0x08) - Yellow at ~70% brightness
        CRGB(191, 191, 191)  // Bit 4 (0x10) - White at 75% brightness
    };

    // Scan all packets and blend those still within 100ms lifetime
    for (int i = 0; i < MAX_ACTIVE_PACKETS; i++) {
        if (recentPackets[i].active) {
            unsigned long age = now - recentPackets[i].timestamp;

            // Check if packet is still within lifetime
            if (age <= PACKET_LIFETIME_MS) {
                uint8_t buttonMask = recentPackets[i].buttonMask;

                // Check for white button (bit 4)
                if (buttonMask & (1 << 4)) {
                    hasWhite = true;
                }

                // Check other color buttons (bits 0-3)
                for (int bit = 0; bit < 4; bit++) {
                    if (buttonMask & (1 << bit)) {
                        totalR += buttonColors[bit].r;
                        totalG += buttonColors[bit].g;
                        totalB += buttonColors[bit].b;
                        colorCount++;
                    }
                }
            } else {
                // Packet expired, mark as inactive
                recentPackets[i].active = false;
            }
        }
    }

    // If no colors active at all, return black
    if (colorCount == 0 && !hasWhite) {
        return CRGB::Black;
    }

    // If only white is pressed, return white at 75% brightness
    if (colorCount == 0 && hasWhite) {
        return CRGB(191, 191, 191);
    }

    // Average the base colors (excluding white)
    CRGB blended;
    blended.r = totalR / colorCount;
    blended.g = totalG / colorCount;
    blended.b = totalB / colorCount;

    // If white is also pressed, lighten the color
    // Mix 70% base color + 30% white for a lighter/pastel effect
    if (hasWhite) {
        blended.r = (blended.r * 7 + 255 * 3) / 10;
        blended.g = (blended.g * 7 + 255 * 3) / 10;
        blended.b = (blended.b * 7 + 255 * 3) / 10;
    }

    return blended;
}

/**
 * Blend colors from all active stations based on button presses (DEPRECATED)
 * Button mask bits: [white][yellow][blue][green][red]
 * Kept for backward compatibility with HTTP endpoints
 */
CRGB getBlendedColorFromStations() {
    int colorCount = 0;
    long totalR = 0, totalG = 0, totalB = 0;
    bool hasWhite = false;

    // Standard button colors (white at 75% brightness)
    // LED strips are RBG order, CRGB handles this automatically
    const CRGB buttonColors[5] = {
        CRGB(255, 0, 0),     // Bit 0 (0x01) - Red
        CRGB(0, 255, 0),     // Bit 1 (0x02) - Green
        CRGB(0, 0, 255),     // Bit 2 (0x04) - Blue
        CRGB(180, 180, 0),   // Bit 3 (0x08) - Yellow at ~70% brightness
        CRGB(191, 191, 191)  // Bit 4 (0x10) - White at 75% brightness
    };

    // First pass: check if white is pressed and collect other colors
    for (int i = 0; i < MAX_STATIONS; i++) {
        if (stations[i].active && stations[i].buttonMask != 0) {
            // Check for white button (bit 4)
            if (stations[i].buttonMask & (1 << 4)) {
                hasWhite = true;
            }

            // Check other color buttons (bits 0-3)
            for (int bit = 0; bit < 4; bit++) {
                if (stations[i].buttonMask & (1 << bit)) {
                    totalR += buttonColors[bit].r;
                    totalG += buttonColors[bit].g;
                    totalB += buttonColors[bit].b;
                    colorCount++;
                }
            }
        }
    }

    // If no colors active at all, return black
    if (colorCount == 0 && !hasWhite) {
        return CRGB::Black;
    }

    // If only white is pressed, return white at 75% brightness
    if (colorCount == 0 && hasWhite) {
        return CRGB(191, 191, 191);
    }

    // Average the base colors (excluding white)
    CRGB blended;
    blended.r = totalR / colorCount;
    blended.g = totalG / colorCount;
    blended.b = totalB / colorCount;

    // If white is also pressed, lighten the color
    // Mix 70% base color + 30% white for a lighter/pastel effect
    if (hasWhite) {
        blended.r = (blended.r * 7 + 255 * 3) / 10;
        blended.g = (blended.g * 7 + 255 * 3) / 10;
        blended.b = (blended.b * 7 + 255 * 3) / 10;
    }

    return blended;
}

/**
 * Display the blended color on all LED strips
 * Uses packet-based blending: each packet has 300ms lifetime
 * Multiple packets arriving within 300ms window blend together
 * Color interpolation creates smooth transitions between states
 */
void playMixedColorsPattern() {
    static unsigned long lastDebug = 0;
    static unsigned long lastUpdate = 0;
    static CRGB currentDisplayColor = CRGB::Black;
    unsigned long now = millis();

    // Update LEDs every 50ms for smooth but not too rapid updates
    if (now - lastUpdate < 50) {
        return;
    }
    lastUpdate = now;

    // Get target blended color from all packets within their 300ms lifetime
    CRGB targetColor = getBlendedColorFromPackets();

    // Smooth color interpolation: blend 80% current + 20% target
    // This creates gradual transitions instead of instant jumps
    currentDisplayColor.r = (currentDisplayColor.r * 4 + targetColor.r) / 5;
    currentDisplayColor.g = (currentDisplayColor.g * 4 + targetColor.g) / 5;
    currentDisplayColor.b = (currentDisplayColor.b * 4 + targetColor.b) / 5;

    // Debug output every 500ms
    if (now - lastDebug > 500) {
        lastDebug = now;
        Serial.printf("Smooth blend: RGB(%d,%d,%d) -> RGB(%d,%d,%d)\n",
                     targetColor.r, targetColor.g, targetColor.b,
                     currentDisplayColor.r, currentDisplayColor.g, currentDisplayColor.b);
    }

    // Fill all 3 strips with the smoothly interpolated color
    fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, currentDisplayColor);
    fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, currentDisplayColor);
    fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, currentDisplayColor);

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
