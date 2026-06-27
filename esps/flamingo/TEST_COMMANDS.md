# Flamingo ESP32 - Test Commands

Quick reference for testing the Mega Special Mode and other features.

## Default IP Address
```bash
192.168.1.200
```

## Mega Special Mode Test

### Using curl (simplest method)
```bash
# Trigger mega special mode
curl -X POST http://192.168.1.200/mega-special

# With JSON response formatting (requires jq)
curl -X POST http://192.168.1.200/mega-special | jq
```

### Expected Response
```json
{
  "status": "success",
  "action": "mega_special",
  "duration_seconds": 23,
  "message": "MEGA SPECIAL MODE ACTIVATED!"
}
```

### Using Python Test Script
```bash
# Default IP (192.168.1.200)
python test_mega_special.py

# Custom IP
python test_mega_special.py 192.168.50.100
```

## Animation Timeline

When you trigger mega special, watch for this sequence:

```
0-2s:    🔴 MALFUNCTION - Random flickering/glitching
2-5s:    🌈 DRAIN - Color drains from center to edges
5-8s:    ⚫ WAIT - All LEDs off (3 seconds)
8-13s:   💗 PINK PULSATE - Vivid pink breathing effect
13-23s:  🎉 CRAZY PARTY MODE - 5 rapid patterns cycling!
         └─ Rainbow chase
         └─ Strobe effect
         └─ Alternating colors
         └─ Dense sparkles
         └─ Running wave
23s+:    🎊 CONFETTI - Colorful explosion until end
```

## Other Test Commands

### Check Status
```bash
# Get current status
curl http://192.168.1.200/status | jq

# Check specific values
curl -s http://192.168.1.200/status | jq '.current_plan'
curl -s http://192.168.1.200/status | jq '.ip_address'
curl -s http://192.168.1.200/status | jq '.uptime'
```

### Health Check
```bash
curl http://192.168.1.200/health
# Expected: OK
```

### Version Check
```bash
curl http://192.168.1.200/version | jq
```

### Trigger Other Modes
```bash
# Idle mode (flowing rainbow)
curl -X POST http://192.168.1.200/idle

# Skip mode (moving dot)
curl -X POST http://192.168.1.200/skip

# Show mode (moving dot)
curl -X POST http://192.168.1.200/show

# Special mode (moving dot)
curl -X POST http://192.168.1.200/special
```

## Monitoring the Animation

### Real-time Status Monitoring
```bash
# Watch status every 1 second
watch -n 1 'curl -s http://192.168.1.200/status | jq ".current_plan"'
```

### Plan Numbers
- 0 = IDLE
- 1 = SKIP
- 2 = SHOW
- 3 = SPECIAL
- 4 = MIXED_COLORS
- 5 = PARTY
- 6 = **MEGA_SPECIAL**

### Serial Monitor (if connected via USB)
```bash
pio device monitor

# or
screen /dev/cu.usbserial-* 115200
```

## UDP Simulation (Advanced)

Simulate 3 stations pressing all buttons to trigger mega special:

```python
#!/usr/bin/env python3
import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
flamingo_ip = "192.168.1.200"
udp_port = 5000

print("Simulating 3 stations pressing all buttons...")

# All buttons mask = 0x1F (binary: 00011111)
# Station 1
sock.sendto(bytes([1, 0x1F]), (flamingo_ip, udp_port))
print("Station 1: ALL BUTTONS")
time.sleep(0.3)

# Station 2
sock.sendto(bytes([2, 0x1F]), (flamingo_ip, udp_port))
print("Station 2: ALL BUTTONS")
time.sleep(0.3)

# Station 3 - This should trigger mega special!
sock.sendto(bytes([3, 0x1F]), (flamingo_ip, udp_port))
print("Station 3: ALL BUTTONS - MEGA SPECIAL TRIGGERED!")

print("\nWatch the flamingo LEDs!")
```

## Troubleshooting

### Connection Issues
```bash
# Ping the flamingo
ping -c 3 192.168.1.200

# Check if HTTP server is responding
curl -v http://192.168.1.200/health

# Scan network for ESP32 devices
arp-scan --localnet | grep -i espressif
```

### LED Not Updating
```bash
# Check current plan
curl -s http://192.168.1.200/status | jq '.current_plan'

# Force back to idle
curl -X POST http://192.168.1.200/idle

# Then try mega special again
curl -X POST http://192.168.1.200/mega-special
```

### Wrong IP Address
```bash
# Find ESP32 via mDNS
ping flamingo-esp32.local

# Or check your router's DHCP leases
# Or connect via USB and check serial output
```

## Quick Test Sequence

Test all features in sequence:

```bash
echo "Testing Flamingo ESP32..."

echo "1. Health check"
curl http://192.168.1.200/health

echo -e "\n2. Get status"
curl http://192.168.1.200/status | jq

echo -e "\n3. Trigger MEGA SPECIAL MODE!"
curl -X POST http://192.168.1.200/mega-special | jq

echo -e "\nWatch the LEDs for 23 seconds of awesome!"
echo "Timeline:"
echo "  0-2s:    Malfunction"
echo "  2-5s:    Drain"
echo "  5-8s:    Wait (dark)"
echo "  8-13s:   Pink pulsate"
echo "  13-23s:  CRAZY PARTY! ★★★"
echo "  23s+:    Confetti"
```

## Integration with Raspberry Pi

If you have the Raspberry Pi control hub running:

```bash
# Trigger from raspberry Pi server
curl -X POST http://localhost:8000/devices/flamingo/mega-special

# Or via the React app
# Navigate to: http://<raspberry-pi-ip>:8000/
# Click "Mega Special" button
```

## Notes

- **Duration**: Full sequence is 23 seconds
- **Trigger Window**: 3 stations must press all buttons within 2 seconds
- **After Animation**: Returns to IDLE mode automatically
- **UDP Port**: 5000 for station button data
- **HTTP Port**: 80 for web endpoints
- **Serial Baud**: 115200 for debugging

## Emergency Reset

If the flamingo gets stuck:

```bash
# Force back to idle
curl -X POST http://192.168.1.200/idle

# Or power cycle the ESP32
# Or press the physical reset button
```
