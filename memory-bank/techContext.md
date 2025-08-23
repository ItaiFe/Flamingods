# Technical Context

## Technology Stack

### Hardware Components
1. **Raspberry Pi** - Main control hub
   - Model: Raspberry Pi 4 (recommended)
   - OS: Raspberry Pi OS (Debian-based)
   - Network: WiFi + Ethernet capabilities

2. **Tuya WiFi Sockets**
   - Model: LSPA10
   - Protocol: Tuya IoT local network
   - Control: Individual on/off functionality
   - Network: WiFi 2.4GHz

3. **ESP Devices**
   - Microcontrollers: ESP32 or ESP8266
   - Purpose: LED control and button interfaces
   - Communication: WiFi connectivity

### Software Stack
1. **Backend Server**
   - Language: Python 3.8+
   - Framework: FastAPI
   - ASGI Server: Uvicorn
   - Package Manager: uv (modern Python packaging)

2. **Communication Protocols**
   - REST API: HTTP/JSON for control commands
   - WebSocket: Real-time status updates
   - MQTT: IoT device communication (future)
   - Tuya Protocol: Local device control (port 6668)

3. **Development Tools**
   - Version Control: Git
   - Package Management: uv with pyproject.toml
   - Environment: uv virtual environments
   - Testing: pytest with coverage
   - Code Quality: black, isort, ruff, mypy

## Development Setup

### uv Package Manager
The project uses [uv](https://github.com/astral-sh/uv) for fast Python package management:

```bash
# Install uv
curl -LsSf https://astral.sh/uv/install.sh | sh

# Create virtual environment
uv venv

# Install project
uv pip install -e .

# Install with development dependencies
uv pip install -e ".[dev]"
```

### Raspberry Pi Requirements
- Python 3.8+ installed
- uv package manager
- Network access to Tuya devices
- Sufficient storage for logs and configurations
- Service management (systemd)

### Network Configuration
- Local network with Tuya devices
- Port forwarding for external access (if needed)
- Firewall rules for device communication
- Static IP assignment for reliability

### Dependencies (pyproject.toml)
```toml
dependencies = [
    "fastapi>=0.68.0",
    "uvicorn[standard]>=0.15.0",
    "websockets>=10.0",
    "pydantic>=1.8.0",
    "python-multipart>=0.0.5",
    "asyncio-mqtt>=0.11.0",
    "aiofiles>=0.8.0",
    "python-dotenv>=0.19.0",
    "pyyaml>=6.0",
    "structlog>=21.5.0",
]

[project.optional-dependencies]
dev = [
    "pytest>=6.2.5",
    "pytest-asyncio>=0.16.0",
    "httpx>=0.24.0",
    "black>=22.0.0",
    "isort>=5.0.0",
    "flake8>=4.0.0",
    "mypy>=0.950",
]
```

## Technical Constraints
1. **Network Limitations**
   - Tuya devices require 2.4GHz WiFi
   - Local network operation only
   - Device discovery limitations
   - Port 6668 for Tuya protocol

2. **Hardware Constraints**
   - Raspberry Pi processing power
   - Memory limitations for large device counts
   - Network bandwidth considerations

3. **Protocol Limitations**
   - Tuya local protocol complexity
   - Device authentication requirements
   - Real-time communication challenges

## Security Considerations
1. **Network Security**
   - Local network isolation
   - Device authentication
   - API endpoint protection

2. **Device Security**
   - Tuya device local keys
   - Control command validation
   - Access control mechanisms

## Development Workflow

### Using Makefile Commands
```bash
# Quick start
make start

# Development setup
make dev

# Run tests
make test

# Code formatting
make format

# Linting
make lint

# Clean up
make clean
```

### Local Development
```bash
# Install development dependencies
uv pip install -e ".[dev]"

# Run server
uv run python main.py

# Run discovery
uv run python local_discovery.py

# Run tests
uv run pytest

# Format code
uv run black .
uv run isort .
```

### Production Deployment
```bash
# Install production dependencies
uv pip install -e .

# Build package
uv build

# Deploy
make deploy
```
