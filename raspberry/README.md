# Sonoff WiFi Socket & Stage LED Server

A FastAPI-based server for controlling Sonoff WiFi sockets and Stage ESP32 LED controllers in the Midburn project. This server provides local control over Sonoff devices and stage lighting without requiring cloud connectivity.

## 🎯 Features

- **Local Device Discovery**: Automatically find Sonoff devices on your network
- **HTTP API Control**: RESTful API for device control and monitoring
- **Real-time Updates**: WebSocket support for live device status
- **Power Monitoring**: Support for devices with power monitoring capabilities
- **Bulk Operations**: Control multiple devices simultaneously
- **Offline Operation**: Works completely offline without internet connection
- **Stage LED Control**: Control Stage ESP32 LED controllers with 4 lighting plans
- **Lighting Plans**: IDLE, SKIP, SHOW, and SPECIAL lighting modes
- **Real-time Stage Updates**: WebSocket integration for stage lighting changes

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Main Control  │    │  Sonoff Server  │    │  Sonoff Devices │
│   Application   │◄──►│   (Raspberry Pi)│◄──►│   (S26, S31,   │
│                 │    │                 │    │    S40, S60)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌─────────────────┐
                       │   Local Network │
                       │   (WiFi/LAN)    │
                       └─────────────────┘
```

## 🚀 Quick Start

### Prerequisites

- Python 3.8+
- `uv` package manager
- Sonoff WiFi devices on your network
- Raspberry Pi or similar device

### Installation

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd raspberry/
   ```

2. **Install dependencies**:
   ```bash
   # Install production dependencies
   make install
   
   # Or install development dependencies
   make install-dev
   ```

3. **Configure environment**:
   ```bash
   cp env.example .env
   # Edit .env with your network settings
   ```

4. **Run the server**:
   ```bash
   make run
   ```

### Alternative Installation with uv

```bash
# Install directly with uv
uv pip install -e .

# Run server
uv run python main.py
```

## 📡 Device Discovery

The server automatically discovers Sonoff devices on your local network:

```bash
# Run device discovery
make discover

# Or manually
uv run python -c "
import asyncio
from sonoff_manager import device_manager
async def main():
    await device_manager.start()
    devices = await device_manager.discover_devices()
    print(f'Found {len(devices)} devices')
    for device in devices:
        print(f'- {device.name}: {device.ip_address}')
    await device_manager.stop()
asyncio.run(main())
"
```

## 🔌 API Endpoints

### Device Management

- `GET /devices` - List all discovered devices
- `GET /devices/{device_id}` - Get device information
- `POST /discover` - Trigger device discovery

### Device Control

- `POST /devices/{device_id}/control` - Control device with full parameters
- `POST /devices/{device_id}/power/{power_state}` - Set power state (on/off)
- `POST /devices/{device_id}/toggle` - Toggle device power

### Bulk Operations

- `POST /devices/bulk/control` - Control multiple devices simultaneously

### Stage LED Control

- `POST /stage/idle` - Switch to IDLE lighting plan (ambient background)
- `POST /stage/skip` - Switch to SKIP lighting plan (quick transitions)
- `POST /stage/show` - Switch to SHOW lighting plan (performance mode)
- `POST /stage/special` - Switch to SPECIAL lighting plan (special effects)
- `GET /stage/status` - Get current stage device status
- `GET /stage/health` - Check stage device health

### System Information

- `GET /health` - Server health status
- `GET /system/status` - System status and statistics
- `GET /system/clients` - WebSocket client information

### WebSocket

- `WS /ws` - Real-time device updates and control

## 📱 Usage Examples

### Control a Device

```bash
# Turn device on
curl -X POST "http://localhost:8000/devices/sonoff_192_168_1_100/power/on"

# Turn device off
curl -X POST "http://localhost:8000/devices/sonoff_192_168_1_100/power/off"

# Toggle device
curl -X POST "http://localhost:8000/devices/sonoff_192_168_1_100/toggle"
```

### Bulk Control

```bash
# Turn on multiple devices
curl -X POST "http://localhost:8000/devices/bulk/control" \
  -H "Content-Type: application/json" \
  -d '{
    "devices": ["device1", "device2", "device3"],
    "power": "on",
    "delay": 0.5
  }'
```

### WebSocket Connection

```javascript
const ws = new WebSocket('ws://localhost:8000/ws');

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Device update:', data);
};

// Subscribe to specific events
ws.send(JSON.stringify({
    type: 'subscribe',
    subscriptions: ['device_status_update', 'device_control']
}));
```

### Stage LED Control

```bash
# Switch to IDLE mode (ambient background)
curl -X POST "http://localhost:8000/stage/idle"

# Switch to SHOW mode (performance lighting)
curl -X POST "http://localhost:8000/stage/show"

# Switch to SKIP mode (quick transitions)
curl -X POST "http://localhost:8000/stage/skip"

# Switch to SPECIAL mode (special effects)
curl -X POST "http://localhost:8000/stage/special"

# Check stage status
curl "http://localhost:8000/stage/status"

# Check stage health
curl "http://localhost:8000/stage/health"
```

## ⚙️ Configuration

### Environment Variables

Copy `env.example` to `.env` and modify:

```bash
# Network configuration
NETWORK_LOCAL_NETWORK=192.168.1.0/24

# Server settings
SERVER_PORT=8000
SERVER_DEBUG=false

# Device discovery
SONOFF_DISCOVERY_TIMEOUT=30
SONOFF_MAX_DEVICES=100
SONOFF_REQUEST_TIMEOUT=10
SONOFF_RETRY_ATTEMPTS=3
SONOFF_RETRY_DELAY=1.0

# Stage ESP32 LED Controller
STAGE_BASE_URL=http://192.168.1.100
STAGE_TIMEOUT=5.0
STAGE_RETRY_ATTEMPTS=2
STAGE_RETRY_DELAY=1.0
STAGE_DEVICE_NAME=stage-esp32
STAGE_MAX_BRIGHTNESS=255
STAGE_HEALTH_CHECK_INTERVAL=30
STAGE_HEALTH_CHECK_TIMEOUT=3.0
```

### Network Configuration

The server scans your local network for Sonoff devices. Configure the network range in your `.env` file:

```bash
# Example: 192.168.1.0/24 for 192.168.1.x network
NETWORK_LOCAL_NETWORK=192.168.1.0/24

# Example: 10.0.0.0/24 for 10.0.0.x network
NETWORK_LOCAL_NETWORK=10.0.0.0/24
```

## 🔧 Development

### Project Structure

```
raspberry/
├── main.py              # FastAPI server entry point
├── config.py            # Configuration management
├── models.py            # Pydantic data models
├── sonoff_manager.py    # Device management and control
├── websocket_manager.py # WebSocket connection management
├── pyproject.toml       # Project configuration and dependencies
├── Makefile            # Development commands
├── env.example         # Environment template
└── README.md           # This file
```

### Development Commands

```bash
# Install development dependencies
make install-dev

# Run tests
make test

# Run linting
make lint

# Format code
make format

# Clean up
make clean

# Build package
make build
```

### Running Tests

```bash
# Run all tests
uv run pytest

# Run with coverage
uv run pytest --cov

# Run specific test file
uv run pytest test_sonoff_manager.py
```

## 🚨 Troubleshooting

### Common Issues

#### Device Discovery Fails

1. **Check network configuration**:
   ```bash
   # Verify network range in .env
   NETWORK_LOCAL_NETWORK=192.168.1.0/24
   ```

2. **Check device connectivity**:
   ```bash
   # Ping device IP
   ping 192.168.1.100
   
   # Check if port 80 is open
   curl -I http://192.168.1.100
   ```

3. **Verify device is Sonoff**:
   - Device must be on the same network
   - Device must have HTTP interface on port 80
   - Device must respond to Sonoff-specific endpoints

#### Server Won't Start

1. **Check dependencies**:
   ```bash
   make install
   ```

2. **Check configuration**:
   ```bash
   # Verify .env file exists
   ls -la .env
   
   # Check for syntax errors
   cat .env
   ```

3. **Check port availability**:
   ```bash
   # Check if port 8000 is in use
   netstat -tulpn | grep :8000
   ```

### Debug Mode

Enable debug mode for detailed logging:

```bash
# Set in .env
SERVER_DEBUG=true
SERVER_LOG_LEVEL=DEBUG

# Or set environment variable
export SERVER_DEBUG=true
export SERVER_LOG_LEVEL=DEBUG
```

## 🔒 Security Considerations

- **Local Network Only**: Server only listens on local network
- **No Authentication**: By default, no authentication required
- **CORS Configuration**: Configure allowed origins in `.env`
- **API Key**: Optional API key authentication can be enabled

## 📊 Monitoring

### Health Check

```bash
curl http://localhost:8000/health
```

### System Status

```bash
curl http://localhost:8000/system/status
```

### WebSocket Statistics

```bash
curl http://localhost:8000/system/clients
```

## 🚀 Deployment

### Production Deployment

1. **Install dependencies**:
   ```bash
   make install
   ```