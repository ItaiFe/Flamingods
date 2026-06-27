# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Flamingods is an interactive art installation for the Israeli Burning Man (Midburn) event featuring IoT-controlled components including LED systems, interactive buttons, automated stage appliances, and an audio system. The system operates locally without cloud dependencies and provides real-time interaction and automation.

## System Architecture

The project consists of three major components:

1. **Raspberry Pi Control Hub** - FastAPI-based server controlling Sonoff WiFi sockets, Stage ESP32 LED controllers, and audio playback system
2. **ESP32/ESP8266 Devices** - WiFi-enabled LED controllers and button interfaces (crown, stage, flamingo, button, station, touch)
3. **Web Interfaces** - React dashboards for stage LED control and audio system management

### Communication Flow
```
React Apps ←HTTP/WS→ Raspberry Pi Server ←HTTP→ ESP Devices
                              ↓
                         Sonoff Sockets
                              ↓
                         Audio System
```

## Repository Structure

- **`raspberry/`** - Main control server (Python/FastAPI) for Sonoff sockets, stage LED control, and audio system
- **`esps/`** - ESP32/ESP8266 firmware projects (Arduino/PlatformIO)
  - `crown/` - LED crown controller (3 lighting plans: IDLE, BUTTON, WiFi Fallback)
  - `stage/` - Stage LED controller (4 plans: IDLE, SKIP, SHOW, SPECIAL)
  - `flamingo/` - 4-strip LED controller (moving pattern, idle pattern)
  - `button/`, `station/`, `touch/` - Button interface controllers
- **`application/`** - Stage LED control React dashboard
- **`react-audio-app/`** - Audio system control React interface
- **`memory-bank/`** - Project documentation and context (READ THESE FILES FIRST for comprehensive project understanding)
- **`.cursor/rules/`** - Cursor-specific workflow rules (PLAN/ACT mode patterns)

## Build and Run Commands

### Raspberry Pi Server

```bash
# Navigate to raspberry directory
cd raspberry

# Install dependencies (uses uv package manager)
make install          # Production dependencies
make install-dev      # Development dependencies

# Run the server
make run              # Start FastAPI server on port 8000
uv run python main.py # Alternative direct method

# Device discovery
make discover         # Discover Sonoff devices on network

# Development commands
make test             # Run tests with pytest
make lint             # Run code linting
make format           # Format code with black
make clean            # Clean up build artifacts
```

### ESP32/ESP8266 Devices

All ESP devices use PlatformIO. Commands are similar across devices:

```bash
# Navigate to specific ESP directory (e.g., crown, stage, flamingo)
cd esps/crown

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Build, upload, and monitor in one command
pio run --target upload && pio device monitor
```

### React Applications

**Stage LED Dashboard** (`application/`):
```bash
cd application
npm install
npm start    # Development server
npm build    # Production build
npm test     # Run tests
```

**Audio Control App** (`react-audio-app/`):
```bash
cd react-audio-app
npm install
npm start    # Development server
npm build    # Production build
npm test     # Run tests
```

## Development Workflow Patterns

### PLAN/ACT Mode (from .cursor/rules/core.mdc)

This repository uses a two-mode workflow:

1. **PLAN Mode** - Gather information, define plan, but make no changes
2. **ACT Mode** - Execute the approved plan

- Start in PLAN mode by default
- Type `ACT` to move to ACT mode (or explicit user approval)
- Type `PLAN` to return to PLAN mode
- Always output the mode at the beginning: `# Mode: PLAN` or `# Mode: ACT`

### Memory Bank System (from .cursor/rules/memory-bank.mdc)

**IMPORTANT**: Read ALL memory-bank files at the start of EVERY task. Files:
- `projectbrief.md` - Core requirements and goals
- `productContext.md` - Why project exists, problems it solves
- `systemPatterns.md` - Architecture, design patterns, component relationships
- `techContext.md` - Technologies, development setup, constraints
- `activeContext.md` - Current work focus, recent changes, next steps
- `progress.md` - What works, what's left, known issues

These files provide essential context that isn't obvious from code alone.

## Key Technical Details

### Technology Stack

**Backend (Raspberry Pi)**:
- Python 3.11+
- FastAPI for REST API
- Uvicorn ASGI server
- Package manager: `uv` (not pip)
- WebSocket for real-time updates
- pygame for audio playback
- mutagen for audio metadata

**ESP Devices**:
- ESP32/ESP8266 microcontrollers
- Arduino framework
- PlatformIO build system
- FastLED library for LED control
- ArduinoJson for API communication

**Frontend**:
- React 18
- TypeScript
- Axios for HTTP requests
- React Router for navigation
- Tailwind CSS (audio app)

### Network Configuration

- **Raspberry Pi**: Port 8000 (HTTP/WebSocket)
- **ESP Devices**: Port 80 (HTTP)
- **WiFi Networks**:
  - Stage: "Flamingods" / "Aa123456!"
  - Crown: "DiMax Residency 2.4Ghz" / "33355555DM"
- **Sonoff**: Local network discovery, port 6668 (Tuya protocol)

### Audio System

Located in `raspberry/`, integrated with main server:

- **Music folder**: `raspberry/music/`
- **Playlists**: `raspberry/music/playlists/`
- **Supported formats**: MP3, WAV, FLAC, OGG, M4A, AAC
- **API endpoints**: `/audio/*` (play, pause, stop, volume, tracks, upload, search)

Key features:
- Play music from local folder
- Playlist management (shuffle, repeat, auto-advance)
- Volume control (0-100%)
- Metadata extraction
- File upload (single/batch)
- Search and filtering
- WebSocket events for real-time status

### ESP Device HTTP APIs

Each ESP device exposes HTTP endpoints for control:

**Crown** (`esps/crown/`):
- `POST /idle` - Soft yellow pulsating halo
- `POST /button` - Crazy colors party mode (auto-returns to IDLE after 10s)
- `GET /status` - Current status
- `GET /health` - Health check

**Stage** (`esps/stage/`):
- `POST /idle` - Ambient lighting
- `POST /skip` - Quick transitions
- `POST /show` - Performance lighting
- `POST /special` - Special effects
- `GET /status` - Current status
- `GET /health` - Health check

### Raspberry Pi API Endpoints

**Sonoff/Stage Control**:
- `GET /devices` - List all devices
- `POST /devices/{id}/power/{state}` - Control device power
- `POST /devices/{id}/toggle` - Toggle device
- `POST /stage/{plan}` - Stage lighting (idle, skip, show, special)
- `GET /health` - Server health

**Audio System**:
- `POST /audio/play`, `/audio/pause`, `/audio/stop`
- `POST /audio/next`, `/audio/previous`
- `POST /audio/volume/{level}`, `/audio/mute`
- `GET /audio/tracks`, `/audio/playlists`
- `POST /audio/upload` - Upload single file
- `POST /audio/upload/batch` - Upload multiple files
- `GET /audio/tracks/search?query=...` - Search tracks
- `GET /audio/tracks/random` - Random track selection
- `POST /audio/scan` - Scan music library
- `WS /ws` - WebSocket for real-time events

## Common Development Tasks

### Adding a New ESP Device

1. Create directory in `esps/`
2. Initialize PlatformIO project: `pio init --board esp32dev`
3. Configure `platformio.ini` with FastLED and ArduinoJson dependencies
4. Implement HTTP endpoints in `src/main.cpp`
5. Create LED plans in separate files (e.g., `led_plans.cpp`)
6. Add README.md with device-specific documentation

### Modifying LED Patterns

LED pattern code is in each ESP's `src/` or `include/` directory:
- `main.cpp` - Main loop and HTTP handlers
- `led_plans.h` - Pattern declarations
- `led_plans.cpp` - Pattern implementations

Use FastLED library functions for LED control. Test by uploading to device and calling HTTP endpoints.

### Adding Audio Features

1. Update `audio_models.py` for new data structures
2. Implement logic in `audio_manager.py`
3. Add API endpoints in `audio_endpoints.py`
4. Update React app in `react-audio-app/src/`
5. Test with curl or React interface

### Testing ESP Devices

Each ESP directory may have Python test scripts (e.g., `test_endpoints.py`):
```bash
python test_endpoints.py
```

Update `ESP_IP` variable in test scripts to match your device's IP.

## Important Notes

- **No internet required**: System operates entirely on local network
- **OTA Updates**: ESP devices support Over-The-Air firmware updates (see `esps/OTA_README.md`, `esps/ota_manager.py`)
- **uv not pip**: Raspberry Pi project uses `uv` package manager, not pip
- **WiFi Fallback**: Crown device has special WiFi fallback mode with halo + running pixels
- **Auto-return**: Crown's BUTTON mode auto-returns to IDLE after 10 seconds
- **Pin Swapping**: Flamingo device has red/green pins swapped (red→pin 4, green→pin 2)
- **Audio Integration**: Audio events can trigger stage lighting and device control via callbacks
- **Memory Bank**: Always read memory-bank files first to understand project context and current state

## Configuration Files

- **Raspberry Pi**: `.env` file (see `env.example`)
- **ESP Devices**: WiFi credentials in `src/main.cpp`
- **React Apps**: `package.json` proxy settings or `.env` for API URL

## Troubleshooting

### Raspberry Pi Server Issues
- Check `.env` configuration
- Verify network range for Sonoff discovery
- Check port 8000 availability
- Review logs with `SERVER_DEBUG=true`

### ESP Device Issues
- Verify WiFi credentials
- Check power supply (5V, adequate current)
- Monitor serial output at 115200 baud
- Verify pin connections

### Audio System Issues
- Check `music/` folder exists and has files
- Verify pygame initialization
- Check supported audio formats
- Review `/audio/health` endpoint

### React App Issues
- Verify Raspberry Pi server is running
- Check API URL in `.env` or `package.json` proxy
- Check CORS configuration on server
- Review browser console for errors
