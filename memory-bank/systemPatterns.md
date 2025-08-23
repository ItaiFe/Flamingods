# System Patterns

## Architecture Overview

### System Architecture
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Main Control │    │  Raspberry Pi   │    │   ESP Devices   │
│   Application  │◄──►│   Tuya Server   │◄──►│  (LEDs, Buttons)│
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Interface │    │  Tuya LSPA10    │    │   Local Network │
│   & Dashboard   │    │  WiFi Sockets   │    │   Communication │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Component Relationships
1. **Main Control Application** ↔ **Raspberry Pi Server**
   - REST API communication
   - WebSocket real-time updates
   - Configuration management

2. **Raspberry Pi Server** ↔ **Tuya LSPA10 Sockets**
   - Local network discovery
   - Device control commands
   - Status monitoring

3. **Raspberry Pi Server** ↔ **ESP Devices**
   - WiFi communication
   - LED control commands
   - Button input processing

## Design Patterns

### 1. Observer Pattern
- **WebSocket Manager** observes device state changes
- **Main Control App** subscribes to real-time updates
- **Status Monitor** tracks device health

### 2. Factory Pattern
- **Device Factory** creates appropriate device handlers
- **Protocol Factory** handles different communication protocols
- **Endpoint Factory** generates API endpoints

### 3. Strategy Pattern
- **Device Control Strategies** for different device types
- **Communication Strategies** for various protocols
- **Error Handling Strategies** for different failure modes

### 4. Singleton Pattern
- **Device Manager** maintains single instance of device registry
- **Configuration Manager** provides centralized configuration
- **Logging Manager** centralizes logging operations

## Communication Patterns

### REST API Pattern
```
Request: POST /devices/{id}/power
Body: {"power": true}
Response: {"success": true, "device_id": "lspa10_001", "power": true}
```

### WebSocket Pattern
```
Connection: ws://raspberry-pi:8001/ws
Events: 
  - device_status_update
  - device_discovered
  - device_error
  - system_health
```

### Device Discovery Pattern
```
1. Network scan for Tuya devices
2. Device authentication and registration
3. Status verification and health check
4. Device registry update
5. Notification to subscribers
```

## Error Handling Patterns

### 1. Circuit Breaker Pattern
- **Device Communication** fails after 3 consecutive attempts
- **Automatic Recovery** after timeout period
- **Fallback Behavior** for critical operations

### 2. Retry Pattern
- **Network Operations** with exponential backoff
- **Device Commands** with configurable retry count
- **Status Checks** with progressive delays

### 3. Graceful Degradation
- **Partial System Failure** continues operation
- **Device Offline** maintains other device control
- **Network Issues** provides offline status

## Configuration Patterns

### 1. Environment-Based Configuration
- **Development** vs **Production** settings
- **Network** configuration for different environments
- **Device** settings for various installations

### 2. Hierarchical Configuration
- **System** level settings
- **Device** level configurations
- **User** level preferences

### 3. Dynamic Configuration
- **Runtime** configuration updates
- **Hot Reload** of device settings
- **Configuration** validation and verification

## Security Patterns

### 1. Authentication Pattern
- **Device Authentication** for Tuya devices
- **API Authentication** for control endpoints
- **Network Authentication** for secure communication

### 2. Authorization Pattern
- **Role-Based Access** control
- **Device-Level Permissions** for operations
- **Operation-Level Restrictions** for safety

### 3. Validation Pattern
- **Input Validation** for all API endpoints
- **Command Validation** for device operations
- **Configuration Validation** for system settings
