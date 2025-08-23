# OTA (Over-The-Air) Update System

All ESP32 devices in the Flamingods project now support Over-The-Air firmware updates, allowing you to update firmware wirelessly without physical USB connections.

## 🚀 Features

- **Wireless Updates**: Update firmware over WiFi
- **Progress Monitoring**: Real-time upload progress tracking
- **Error Handling**: Automatic rollback on failed updates
- **Version Management**: Track firmware versions across devices
- **Batch Updates**: Update multiple ESPs simultaneously
- **Security**: Password-protected OTA updates

## 📱 Supported Devices

| Device | Hostname | OTA Password |
|--------|----------|--------------|
| Crown ESP | `crown-esp32` | `flamingods2024` |
| Stage ESP | `stage-esp32` | `flamingods2024` |
| Flamingo ESP | `flamingo-esp32` | `flamingods2024` |
| Button ESP | `button-esp32` | `flamingods2024` |
| Station ESP | `station-esp32` | `flamingods2024` |

## 🔧 OTA Endpoints

Each ESP provides these OTA-related HTTP endpoints:

### `GET /version`
Get current firmware version and device information.

**Response:**
```json
{
  "status": "success",
  "firmware_version": "1.0.0",
  "device": "crown-esp32"
}
```

### `POST /ota`
Prepare device for OTA update.

**Response:**
```json
{
  "status": "success",
  "message": "OTA update ready. Use Arduino IDE or esptool to upload firmware."
}
```

### `GET /ota-status`
Get current OTA update status and progress.

**Response:**
```json
{
  "status": "success",
  "ota_in_progress": false,
  "ota_progress": 0,
  "uptime": 12345
}
```

## 🛠️ Building for OTA

### PlatformIO Configuration

Each ESP has two build environments:

1. **`esp32dev`** - Standard build for initial USB upload
2. **`esp32dev-ota`** - OTA-enabled build for wireless updates

### Building OTA Firmware

```bash
# Navigate to ESP directory
cd esps/crown

# Build OTA firmware
pio run -e esp32dev-ota

# Build standard firmware
pio run -e esp32dev
```

### OTA Upload Target

```bash
# Set ESP IP address
export ESP_IP="192.168.1.100"

# Upload via OTA
pio run -e esp32dev-ota --target upload
```

## 📡 OTA Manager Script

The `ota_manager.py` script provides a comprehensive interface for managing OTA updates.

### Installation

```bash
cd esps
pip install requests
```

### Usage

#### Interactive Mode
```bash
python ota_manager.py
```

#### Command Line Mode
```bash
# Discover ESP devices
python ota_manager.py discover

# List discovered devices
python ota_manager.py list

# Check firmware versions
python ota_manager.py versions

# Upload to specific ESP
python ota_manager.py upload 192.168.1.100 firmware.bin

# Upload to all ESPs
python ota_manager.py upload-all firmware.bin
```

## 🔍 Device Discovery

The OTA manager automatically discovers ESP devices using:

1. **Hostname Resolution**: Tries common ESP hostnames
2. **Network Scanning**: Scans local network for ESP devices
3. **Health Checks**: Verifies devices respond to `/health` endpoint

## 📊 OTA Process

### 1. Preparation
- ESP switches to idle mode during OTA
- LED patterns continue running
- HTTP server remains active

### 2. Upload
- Firmware uploads via ArduinoOTA
- Progress reported via serial monitor
- Real-time progress tracking

### 3. Completion
- Automatic reboot after successful upload
- New firmware becomes active
- Device returns to normal operation

### 4. Error Handling
- Automatic rollback on failure
- Returns to previous lighting plan
- Error details reported via serial

## 🚨 Troubleshooting

### Common Issues

#### OTA Update Fails
- Check WiFi connection stability
- Verify ESP has sufficient power
- Ensure no other OTA updates in progress

#### Device Not Discovered
- Verify ESP is powered on
- Check WiFi connection
- Ensure ESP responds to `/health` endpoint

#### Upload Timeout
- Check network stability
- Verify ESP IP address
- Ensure firewall allows port 3232

### Debug Information

Monitor serial output for detailed OTA information:

```
OTA Update Started
OTA Progress: 25%
OTA Progress: 50%
OTA Progress: 75%
OTA Progress: 100%
OTA Update Completed
Rebooting in 3 seconds...
```

## 🔐 Security

- **Password Protection**: All OTA updates require password `flamingods2024`
- **Network Isolation**: OTA only works on local network
- **Authentication**: Hostname verification prevents unauthorized updates

## 📈 Version Management

### Firmware Versioning
- Version defined in `platformio.ini` as build flag
- Format: `MAJOR.MINOR.PATCH` (e.g., "1.0.0")
- Automatically included in status responses

### Version Checking
```bash
# Check all device versions
python ota_manager.py versions

# Individual device check
curl http://192.168.1.100/version
```

## 🎯 Best Practices

1. **Test Updates**: Always test firmware on one device before batch updates
2. **Power Stability**: Ensure stable power during OTA updates
3. **Network Quality**: Use reliable WiFi connection for large updates
4. **Backup**: Keep previous firmware versions for rollback
5. **Monitoring**: Watch serial output during updates

## 🔄 Update Workflow

### Development Workflow
1. Develop and test firmware locally
2. Build OTA firmware: `pio run -e esp32dev-ota`
3. Use OTA manager to deploy to test devices
4. Verify functionality
5. Deploy to production devices

### Production Deployment
1. Build production firmware
2. Use OTA manager to discover devices
3. Check current versions
4. Deploy updates (individual or batch)
5. Verify successful deployment

## 📚 Additional Resources

- [ArduinoOTA Documentation](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA)
- [PlatformIO OTA Guide](https://docs.platformio.org/en/latest/platforms/espressif32.html#over-the-air-ota-update)
- [ESP32 OTA Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/ota.html)

## 🆘 Support

For OTA-related issues:

1. Check serial monitor output
2. Verify network connectivity
3. Ensure proper power supply
4. Check firewall settings
5. Review error logs in OTA manager

---

**Note**: OTA updates require stable power and network connections. Always ensure devices have reliable power during updates to prevent corruption.
