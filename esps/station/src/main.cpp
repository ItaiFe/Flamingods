/**
 * Station ESP32 - Button Controller for Flamingo Server
 *
 * Features:
 * - 5 physical buttons (Red, Green, Blue, Yellow, White)
 * - HTTP communication with Flamingo server
 * - Color mixing when multiple buttons are pressed
 * - 4 stations, each connected to flamingo server
 * - Dual network support: Ethernet (primary) and WiFi (fallback)
 *
 * Includes OTA (Over-The-Air) update capability
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "station_config.h"

// WiFi Configuration
const char* ssid = "DiMax Residency 2.4Ghz";
const char* password = "33355555DM";

// Flamingo Server Configuration (UDP-based button streaming)
const char* flamingoHost = "flamingo-esp32.local";  // Flamingo ESP hostname
IPAddress flamingoIP;  // Resolved IP address
const int flamingoUdpPort = 5000;  // UDP port for button streaming



// Firmware version
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Button Configuration
#define BUTTON_RED_PIN     15    // Swapped: was 2
#define BUTTON_GREEN_PIN   2     // Swapped: was 4
#define BUTTON_BLUE_PIN    4     // Swapped: was 15
#define BUTTON_YELLOW_PIN  12    // Unchanged
#define BUTTON_WHITE_PIN   14    // Unchanged

// Button debounce time
#define DEBOUNCE_TIME     50

// UDP stream interval - send button state every 20ms for responsive color updates
#define UDP_SEND_INTERVAL 20

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

// UDP client for button streaming
WiFiUDP udp;

// Web server for status endpoint
WebServer server(80);

// Status variables
bool wifiConnected = false;
bool ethernetConnected = false;
unsigned long lastStatusUpdate = 0;
unsigned long lastButtonCheck = 0;
unsigned long lastUdpSend = 0;  // Track when to send UDP packet
unsigned long startTime = 0;
unsigned long buttonPressCount = 0;
uint8_t lastButtonMask = 0;  // Track last button state to avoid redundant sends

// OTA variables
bool otaInProgress = false;
unsigned long otaStartTime = 0;
int otaProgress = 0;

// Function prototypes
void setupNetwork();
void setupButtons();
void setupOTA();
void setupWebServer();
void handleStatus();
void handleRoot();
void checkButtons();
void sendButtonStateUDP(uint8_t buttonMask);
void WiFiEvent(WiFiEvent_t event);

// Ethernet event handler
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            ETH.setHostname(STATION_NAME);
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
    Serial.begin(115200);
    delay(1000);  // Give time for serial to initialize

    Serial.println("\n=== Station ESP32 Starting ===");
    Serial.printf("Station ID: %d (%s)\n", STATION_ID, STATION_NAME);
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);

    // Record start time
    startTime = millis();

    // Setup buttons
    setupButtons();

    // Setup Network (Ethernet + WiFi fallback)
    setupNetwork();

    // Setup OTA and Web Server (if connected)
    if (ethernetConnected || wifiConnected) {
        setupOTA();
        setupWebServer();

        // Setup mDNS for hostname resolution (station-1.local, station-2.local, etc.)
        if (MDNS.begin(STATION_NAME)) {
            Serial.printf("mDNS responder started: %s.local\n", STATION_NAME);
        } else {
            Serial.println("Error setting up mDNS responder!");
        }

        // Resolve flamingo hostname to IP
        Serial.printf("Resolving flamingo hostname: %s\n", flamingoHost);
        if (WiFi.hostByName(flamingoHost, flamingoIP)) {
            Serial.printf("Flamingo IP resolved: %s\n", flamingoIP.toString().c_str());
        } else {
            Serial.println("Failed to resolve flamingo hostname - will retry later");
        }
    } else {
        Serial.println("No network connection - OTA and Web Server disabled");
    }

    Serial.println("Station ESP32 initialization complete!");
}

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();

    // Handle web server requests
    server.handleClient();

    // Check buttons every 10ms
    if (millis() - lastButtonCheck > 10) {
        checkButtons();
        lastButtonCheck = millis();
    }

    // Status updates
    if (millis() - lastStatusUpdate > 5000) {
        lastStatusUpdate = millis();
        if (ethernetConnected) {
            Serial.printf("Status: Ethernet: %s, Link: %s, Speed: %d Mbps, OTA: %s\n",
                ETH.localIP().toString().c_str(),
                ETH.linkUp() ? "UP" : "DOWN",
                ETH.linkSpeed(),
                otaInProgress ? "In Progress" : "Idle");
        } else if (wifiConnected) {
            Serial.printf("Status: WiFi: %s, RSSI: %d, OTA: %s\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI(),
                otaInProgress ? "In Progress" : "Idle");
        } else {
            Serial.println("Status: No network connection");
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

void setupNetwork() {
    // Register Ethernet event handler
    WiFi.onEvent(WiFiEvent);

    // Try Ethernet first (built-in ESP32-ETH01)
    Serial.println("Initializing built-in Ethernet...");

    // Manual power control for ESP32-ETH01V1.4
    Serial.println("Setting up power pin GPIO 5...");
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);  // Power on the PHY
    delay(500);  // Give time for power to stabilize

    // Initialize Ethernet with ESP32-ETH01V1.4 configuration
    // PHY address 1, GPIO 16 power, GPIO0_IN clock
    Serial.println("Configuration: PHY_ADDR=1, POWER=16, MDC=23, MDIO=18, TYPE=LAN8720, CLK=GPIO0_IN");

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
        return;  // Success, no need for WiFi
    } else {
        Serial.println();
        Serial.println("Built-in Ethernet connection failed, trying WiFi...");
    }

    // Fallback to WiFi if Ethernet failed
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

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

void setupWebServer() {
    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);

    // Press all buttons endpoint
    server.on("/press-all", HTTP_POST, []() {
        Serial.println("HTTP: Simulating all button press - PARTY MODE!");
        buttonPressCount++;
        sendButtonStateUDP(0x1F);  // All 5 buttons: 0x1F = 0b11111
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Party mode activated!\"}");
    });

    // Individual button press endpoints
    server.on("/press/red", HTTP_POST, []() {
        Serial.println("HTTP: Pressing red button");
        buttonPressCount++;
        sendButtonStateUDP(0x04);  // Red = bit 2 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"red\"}");
    });

    server.on("/press/green", HTTP_POST, []() {
        Serial.println("HTTP: Pressing green button");
        buttonPressCount++;
        sendButtonStateUDP(0x01);  // Green = bit 0 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"green\"}");
    });

    server.on("/press/blue", HTTP_POST, []() {
        Serial.println("HTTP: Pressing blue button");
        buttonPressCount++;
        sendButtonStateUDP(0x02);  // Blue = bit 1 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"blue\"}");
    });

    server.on("/press/yellow", HTTP_POST, []() {
        Serial.println("HTTP: Pressing yellow button");
        buttonPressCount++;
        sendButtonStateUDP(0x08);  // Yellow = bit 3
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"yellow\"}");
    });

    server.on("/press/white", HTTP_POST, []() {
        Serial.println("HTTP: Pressing white button");
        buttonPressCount++;
        sendButtonStateUDP(0x10);  // White = bit 4
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"white\"}");
    });

    // Color combination endpoints
    server.on("/press/purple", HTTP_POST, []() {
        Serial.println("HTTP: Pressing purple (red+blue)");
        buttonPressCount++;
        sendButtonStateUDP(0x06);  // Red + Blue = 0x04 | 0x02 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"purple\"}");
    });

    server.on("/press/cyan", HTTP_POST, []() {
        Serial.println("HTTP: Pressing cyan (blue+green)");
        buttonPressCount++;
        sendButtonStateUDP(0x03);  // Blue + Green = 0x02 | 0x01 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"cyan\"}");
    });

    server.on("/press/orange", HTTP_POST, []() {
        Serial.println("HTTP: Pressing orange (red+yellow)");
        buttonPressCount++;
        sendButtonStateUDP(0x0C);  // Red + Yellow = 0x04 | 0x08 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"orange\"}");
    });

    server.on("/press/brown", HTTP_POST, []() {
        Serial.println("HTTP: Pressing brown (red+green)");
        buttonPressCount++;
        sendButtonStateUDP(0x05);  // Red + Green = 0x04 | 0x01 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"brown\"}");
    });

    server.on("/press/lime", HTTP_POST, []() {
        Serial.println("HTTP: Pressing lime (green+yellow)");
        buttonPressCount++;
        sendButtonStateUDP(0x09);  // Green + Yellow = 0x01 | 0x08 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"lime\"}");
    });

    server.on("/press/teal", HTTP_POST, []() {
        Serial.println("HTTP: Pressing teal (blue+yellow)");
        buttonPressCount++;
        sendButtonStateUDP(0x0A);  // Blue + Yellow = 0x02 | 0x08 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"teal\"}");
    });

    server.on("/press/pink", HTTP_POST, []() {
        Serial.println("HTTP: Pressing pink (red+white)");
        buttonPressCount++;
        sendButtonStateUDP(0x14);  // Red + White = 0x04 | 0x10 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"pink\"}");
    });

    server.on("/press/lightblue", HTTP_POST, []() {
        Serial.println("HTTP: Pressing light blue (blue+white)");
        buttonPressCount++;
        sendButtonStateUDP(0x12);  // Blue + White = 0x02 | 0x10 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"lightblue\"}");
    });

    server.on("/press/lightyellow", HTTP_POST, []() {
        Serial.println("HTTP: Pressing light yellow (yellow+white)");
        buttonPressCount++;
        sendButtonStateUDP(0x18);  // Yellow + White = 0x08 | 0x10
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"lightyellow\"}");
    });

    server.on("/press/lightgreen", HTTP_POST, []() {
        Serial.println("HTTP: Pressing light green (green+white)");
        buttonPressCount++;
        sendButtonStateUDP(0x11);  // Green + White = 0x01 | 0x10 (corrected)
        server.send(200, "application/json", "{\"status\":\"success\",\"button\":\"lightgreen\"}");
    });

    server.begin();
    Serial.println("Web server started on port 80");
    if (ethernetConnected) {
        Serial.printf("Access at: http://%s/\n", ETH.localIP().toString().c_str());
    } else if (wifiConnected) {
        Serial.printf("Access at: http://%s/\n", WiFi.localIP().toString().c_str());
    }
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Station " + String(STATION_ID) + "</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; }";
    html += "h1 { color: #333; }";
    html += ".info { background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += ".buttons { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin: 20px 0; }";
    html += "button { padding: 20px; font-size: 18px; border: none; border-radius: 8px; cursor: pointer; color: white; font-weight: bold; }";
    html += ".btn-red { background-color: #e74c3c; }";
    html += ".btn-green { background-color: #27ae60; }";
    html += ".btn-blue { background-color: #3498db; }";
    html += ".btn-yellow { background-color: #f39c12; color: black; }";
    html += ".btn-white { background-color: #ecf0f1; color: black; border: 2px solid #999; }";
    html += ".btn-all { background-color: #9b59b6; grid-column: span 2; }";
    html += ".btn-purple { background: linear-gradient(135deg, #e74c3c 50%, #3498db 50%); }";
    html += ".btn-cyan { background: linear-gradient(135deg, #3498db 50%, #27ae60 50%); }";
    html += ".btn-orange { background: linear-gradient(135deg, #e74c3c 50%, #f39c12 50%); color: black; }";
    html += ".btn-brown { background: linear-gradient(135deg, #e74c3c 50%, #27ae60 50%); }";
    html += ".btn-lime { background: linear-gradient(135deg, #27ae60 50%, #f39c12 50%); color: black; }";
    html += ".btn-teal { background: linear-gradient(135deg, #3498db 50%, #f39c12 50%); }";
    html += ".btn-pink { background: linear-gradient(135deg, #e74c3c 50%, #ecf0f1 50%); }";
    html += ".btn-lightblue { background: linear-gradient(135deg, #3498db 50%, #ecf0f1 50%); }";
    html += ".btn-lightyellow { background: linear-gradient(135deg, #f39c12 50%, #ecf0f1 50%); color: black; }";
    html += ".btn-lightgreen { background: linear-gradient(135deg, #27ae60 50%, #ecf0f1 50%); }";
    html += "button:hover { opacity: 0.8; }";
    html += "button:active { transform: scale(0.95); }";
    html += ".status { margin: 20px 0; padding: 10px; background: #d4edda; border-radius: 5px; display: none; }";
    html += "a { color: #3498db; text-decoration: none; }";
    html += "a:hover { text-decoration: underline; }";
    html += "</style></head><body>";

    html += "<h1>Station ESP32 - " + String(STATION_NAME) + "</h1>";

    html += "<div class='info'>";
    html += "<strong>Station ID:</strong> " + String(STATION_ID) + "<br>";
    html += "<strong>Firmware:</strong> " + String(FIRMWARE_VERSION) + "<br>";
    if (ethernetConnected) {
        html += "<strong>Network:</strong> Ethernet<br>";
        html += "<strong>IP:</strong> " + ETH.localIP().toString() + "<br>";
        html += "<strong>Speed:</strong> " + String(ETH.linkSpeed()) + " Mbps";
    } else if (wifiConnected) {
        html += "<strong>Network:</strong> WiFi<br>";
        html += "<strong>IP:</strong> " + WiFi.localIP().toString() + "<br>";
        html += "<strong>RSSI:</strong> " + String(WiFi.RSSI()) + " dBm";
    }
    html += "</div>";

    html += "<h2>Basic Colors (Hold to keep pressed)</h2>";
    html += "<div class='buttons'>";
    html += "<button class='btn-red' id='btn-red'>RED</button>";
    html += "<button class='btn-green' id='btn-green'>GREEN</button>";
    html += "<button class='btn-blue' id='btn-blue'>BLUE</button>";
    html += "<button class='btn-yellow' id='btn-yellow'>YELLOW</button>";
    html += "<button class='btn-white' id='btn-white'>WHITE</button>";
    html += "<button class='btn-all' id='btn-all'>ALL</button>";
    html += "</div>";

    html += "<h2>Color Combinations</h2>";
    html += "<div class='buttons'>";
    html += "<button class='btn-purple' id='btn-purple'>PURPLE<br>(Red+Blue)</button>";
    html += "<button class='btn-cyan' id='btn-cyan'>CYAN<br>(Blue+Green)</button>";
    html += "<button class='btn-orange' id='btn-orange'>ORANGE<br>(Red+Yellow)</button>";
    html += "<button class='btn-brown' id='btn-brown'>BROWN<br>(Red+Green)</button>";
    html += "<button class='btn-lime' id='btn-lime'>LIME<br>(Green+Yellow)</button>";
    html += "<button class='btn-teal' id='btn-teal'>TEAL<br>(Blue+Yellow)</button>";
    html += "<button class='btn-pink' id='btn-pink'>PINK<br>(Red+White)</button>";
    html += "<button class='btn-lightblue' id='btn-lightblue'>LIGHT BLUE<br>(Blue+White)</button>";
    html += "<button class='btn-lightyellow' id='btn-lightyellow'>LIGHT YELLOW<br>(Yellow+White)</button>";
    html += "<button class='btn-lightgreen' id='btn-lightgreen'>LIGHT GREEN<br>(Green+White)</button>";
    html += "</div>";

    html += "<div class='status' id='status'></div>";

    html += "<p><a href='/status' target='_blank'>View JSON Status</a></p>";

    html += "<script>";
    html += "let activeButton = null;";
    html += "let sendInterval = null;";
    html += "let sendCount = 0;";
    html += "const MAX_SENDS = 500;";
    html += "function sendPress(color) {";
    html += "  fetch('/press/' + color, { method: 'POST' })";
    html += "    .catch(e => showStatus('Error: ' + e, true));";
    html += "}";
    html += "function press(color) {";
    html += "  if (sendInterval) clearInterval(sendInterval);";
    html += "  activeButton = color;";
    html += "  sendCount = 0;";
    html += "  showStatus('Sending ' + color + ' 500 times...');";
    html += "  sendInterval = setInterval(() => {";
    html += "    if (sendCount >= MAX_SENDS) {";
    html += "      clearInterval(sendInterval);";
    html += "      sendInterval = null;";
    html += "      activeButton = null;";
    html += "      showStatus('Completed 500 sends');";
    html += "      return;";
    html += "    }";
    html += "    sendPress(color);";
    html += "    sendCount++;";
    html += "    if (sendCount % 50 === 0) {";
    html += "      showStatus('Sending ' + color + ': ' + sendCount + '/500');";
    html += "    }";
    html += "  }, 5);";
    html += "}";
    html += "function pressAll() {";
    html += "  if (sendInterval) clearInterval(sendInterval);";
    html += "  activeButton = 'all';";
    html += "  sendCount = 0;";
    html += "  showStatus('Sending all buttons 500 times...');";
    html += "  sendInterval = setInterval(() => {";
    html += "    if (sendCount >= MAX_SENDS) {";
    html += "      clearInterval(sendInterval);";
    html += "      sendInterval = null;";
    html += "      activeButton = null;";
    html += "      showStatus('Completed 500 sends');";
    html += "      return;";
    html += "    }";
    html += "    fetch('/press-all', { method: 'POST' })";
    html += "      .catch(e => showStatus('Error: ' + e, true));";
    html += "    sendCount++;";
    html += "    if (sendCount % 50 === 0) {";
    html += "      showStatus('Sending all: ' + sendCount + '/500');";
    html += "    }";
    html += "  }, 5);";
    html += "}";
    html += "function showStatus(msg, isError) {";
    html += "  const s = document.getElementById('status');";
    html += "  s.textContent = msg;";
    html += "  s.style.display = 'block';";
    html += "  s.style.background = isError ? '#f8d7da' : '#d4edda';";
    html += "}";
    html += "['red','green','blue','yellow','white'].forEach(c => {";
    html += "  const btn = document.getElementById('btn-' + c);";
    html += "  btn.addEventListener('click', (e) => { e.preventDefault(); press(c); });";
    html += "});";
    html += "['purple','cyan','orange','brown','lime','teal','pink','lightblue','lightyellow','lightgreen'].forEach(c => {";
    html += "  const btn = document.getElementById('btn-' + c);";
    html += "  btn.addEventListener('click', (e) => { e.preventDefault(); press(c); });";
    html += "});";
    html += "const btnAll = document.getElementById('btn-all');";
    html += "btnAll.addEventListener('click', (e) => { e.preventDefault(); pressAll(); });";
    html += "</script>";

    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleStatus() {
    StaticJsonDocument<512> doc;

    // Station info
    doc["station_id"] = STATION_ID;
    doc["station_name"] = STATION_NAME;
    doc["firmware_version"] = FIRMWARE_VERSION;

    // Network info
    if (ethernetConnected) {
        doc["network_type"] = "ethernet";
        doc["ip"] = ETH.localIP().toString();
        doc["mac"] = ETH.macAddress();
        doc["link_speed"] = ETH.linkSpeed();
        doc["full_duplex"] = ETH.fullDuplex();
    } else if (wifiConnected) {
        doc["network_type"] = "wifi";
        doc["ip"] = WiFi.localIP().toString();
        doc["rssi"] = WiFi.RSSI();
        doc["ssid"] = WiFi.SSID();
    } else {
        doc["network_type"] = "disconnected";
        doc["ip"] = "0.0.0.0";
    }

    // Status info
    doc["uptime"] = (millis() - startTime) / 1000;  // uptime in seconds
    doc["buttons_pressed"] = buttonPressCount;
    doc["ota_in_progress"] = otaInProgress;

    // Flamingo server info (UDP)
    doc["flamingo_host"] = flamingoHost;
    doc["flamingo_ip"] = flamingoIP.toString();
    doc["flamingo_udp_port"] = flamingoUdpPort;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
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
                    buttonPressCount++;
                    Serial.printf("Button %s pressed\n", buttonColors[i]);
                } else {  // Button released
                    buttons[i].active = false;
                    Serial.printf("Button %s released\n", buttonColors[i]);
                }
            }
        }
        
        buttons[i].lastState = reading;
    }
    
    // Calculate button mask from active buttons
    // Bits: [white(4)][yellow(3)][blue(2)][green(1)][red(0)]
    uint8_t buttonMask = 0;
    for (int i = 0; i < 5; i++) {
        if (buttons[i].active) {
            buttonMask |= (1 << i);
        }
    }

    // Stream button state via UDP only when buttons are pressed (not when idle)
    // This prevents sending buttonMask=0 packets that cause flickering
    if (buttonMask != 0) {
        // Send every 20ms when buttons are active, or immediately on state change
        if (millis() - lastUdpSend >= UDP_SEND_INTERVAL || buttonMask != lastButtonMask) {
            lastUdpSend = millis();
            sendButtonStateUDP(buttonMask);
            lastButtonMask = buttonMask;
        }
    } else if (lastButtonMask != 0) {
        // Button was just released - send one final 0 packet then stop
        sendButtonStateUDP(0);
        lastButtonMask = 0;
    }
}

/**
 * Send button state to flamingo via UDP
 * Packet format: [station_id(1 byte)][button_mask(1 byte)]
 * Button mask bits: [white(4)][yellow(3)][blue(2)][green(1)][red(0)]
 */
void sendButtonStateUDP(uint8_t buttonMask) {
    // Check network connection
    if (!ethernetConnected && !wifiConnected) {
        return;  // No network, skip silently
    }

    // Resolve flamingo IP if not yet resolved
    if (flamingoIP == IPAddress(0, 0, 0, 0)) {
        if (!WiFi.hostByName(flamingoHost, flamingoIP)) {
            return;  // Failed to resolve, skip silently
        }
    }

    // Create 2-byte packet: [station_id][button_mask]
    uint8_t packet[2];
    packet[0] = STATION_ID;
    packet[1] = buttonMask;

    // Send UDP packet to flamingo
    udp.beginPacket(flamingoIP, flamingoUdpPort);
    udp.write(packet, 2);
    udp.endPacket();

    // Debug output (only when buttons change state, not every packet)
    static uint8_t lastDebugMask = 0xFF;
    if (buttonMask != lastDebugMask) {
        Serial.printf("UDP -> Flamingo: station=%d, buttons=0x%02X\n", STATION_ID, buttonMask);
        lastDebugMask = buttonMask;
    }
}
