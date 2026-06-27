# Station-Specific Special Modes - Flamingo ESP32

## Overview

Each of the 4 stations now has its own **unique special mode** triggered when that station presses all 5 buttons simultaneously! Each mode runs for 8 seconds and has a distinct visual theme.

## The 4 Station Modes

### Station 1: FIRE WAVE 🔥
**Theme:** Hot/Energy
**Colors:** Red, orange, yellow
**Effect:** Multiple wave layers creating flickering fire patterns
**Duration:** 8 seconds
**Update Rate:** 50 FPS

**Visual Description:**
- Three overlapping sinusoidal waves simulate fire movement
- Colors transition smoothly through fire spectrum (hue 0-45)
- Dynamic brightness based on wave intensity
- Looks like flames dancing across the LED strips

### Station 2: OCEAN STORM ⚡🌊
**Theme:** Cool/Water with lightning
**Colors:** Cyan, blue, purple
**Effect:** Turbulent ocean waves with random lightning strikes
**Duration:** 8 seconds
**Update Rate:** 50 FPS

**Visual Description:**
- Three turbulent wave patterns create stormy ocean effect
- Random lightning flashes (bright white) every 1-2 seconds
- Deep blues and purples for water depth
- Dynamic brightness simulates wave crests and troughs

### Station 3: NORTHERN LIGHTS 🌌
**Theme:** Aurora Borealis
**Colors:** Green, teal, purple
**Effect:** Smooth flowing aurora with shimmering particles
**Duration:** 8 seconds
**Update Rate:** 33 FPS (smooth and ethereal)

**Visual Description:**
- Slow, flowing waves create aurora curtain effect
- Colors shift through green-teal-purple spectrum (hue 80-160)
- Random shimmer particles add sparkle
- Smooth and mesmerizing like real northern lights

### Station 4: ELECTRIC PULSE ⚡💜
**Theme:** Electric/Energy
**Colors:** Magenta and cyan alternating
**Effect:** Racing pulses with trailing tails
**Duration:** 8 seconds
**Update Rate:** 60 FPS (fastest!)

**Visual Description:**
- 3 pulses race along the strip simultaneously
- Each pulse has a 15-LED trailing tail
- Colors alternate between magenta (hue 200) and cyan (hue 128)
- Fading trail effect creates motion blur
- High-energy, fast-paced visual

## Triggering Modes

### Automatic Trigger (UDP)
When a station sends UDP packet with all 5 buttons pressed (mask = 0x1F), the corresponding station mode activates automatically.

**Priority System:**
- If **3+ stations** press all buttons within 2 seconds → **MEGA SPECIAL MODE** (23 seconds)
- If **1 station** presses all buttons → **That station's special mode** (8 seconds)

### Manual Testing via HTTP

Each station mode can be triggered via curl for testing:

#### Station 1: Fire Wave
```bash
curl -X POST http://192.168.1.200/station1-special

# With JSON output
curl -X POST http://192.168.1.200/station1-special | jq
```

**Expected Response:**
```json
{
  "status": "success",
  "action": "station1_special",
  "mode": "Fire Wave",
  "duration_seconds": 8
}
```

#### Station 2: Ocean Storm
```bash
curl -X POST http://192.168.1.200/station2-special

# With JSON output
curl -X POST http://192.168.1.200/station2-special | jq
```

**Expected Response:**
```json
{
  "status": "success",
  "action": "station2_special",
  "mode": "Ocean Storm",
  "duration_seconds": 8
}
```

#### Station 3: Northern Lights
```bash
curl -X POST http://192.168.1.200/station3-special

# With JSON output
curl -X POST http://192.168.1.200/station3-special | jq
```

**Expected Response:**
```json
{
  "status": "success",
  "action": "station3_special",
  "mode": "Northern Lights",
  "duration_seconds": 8
}
```

#### Station 4: Electric Pulse
```bash
curl -X POST http://192.168.1.200/station4-special

# With JSON output
curl -X POST http://192.168.1.200/station4-special | jq
```

**Expected Response:**
```json
{
  "status": "success",
  "action": "station4_special",
  "mode": "Electric Pulse",
  "duration_seconds": 8
}
```

## Testing All Modes in Sequence

```bash
#!/bin/bash
echo "=== Testing All 4 Station Modes ==="
echo ""

echo "1. Testing Station 1: Fire Wave 🔥"
curl -X POST http://192.168.1.200/station1-special | jq
sleep 9

echo ""
echo "2. Testing Station 2: Ocean Storm ⚡🌊"
curl -X POST http://192.168.1.200/station2-special | jq
sleep 9

echo ""
echo "3. Testing Station 3: Northern Lights 🌌"
curl -X POST http://192.168.1.200/station3-special | jq
sleep 9

echo ""
echo "4. Testing Station 4: Electric Pulse ⚡💜"
curl -X POST http://192.168.1.200/station4-special | jq
sleep 9

echo ""
echo "=== All Station Modes Tested! ==="
```

Save as `test_all_station_modes.sh` and run with:
```bash
chmod +x test_all_station_modes.sh
./test_all_station_modes.sh
```

## UDP Simulation

Simulate station button presses via UDP:

```python
#!/usr/bin/env python3
import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
flamingo_ip = "192.168.1.200"
udp_port = 5000

# Test each station's special mode
stations = [
    (1, "Fire Wave"),
    (2, "Ocean Storm"),
    (3, "Northern Lights"),
    (4, "Electric Pulse")
]

for station_id, mode_name in stations:
    print(f"\nStation {station_id}: Pressing all buttons - {mode_name}")

    # Send all-buttons packet (0x1F)
    sock.sendto(bytes([station_id, 0x1F]), (flamingo_ip, udp_port))

    print(f"Watch for {mode_name} animation (8 seconds)...")
    time.sleep(9)  # Wait for animation to complete + 1 second

    # Release buttons
    sock.sendto(bytes([station_id, 0x00]), (flamingo_ip, udp_port))
    time.sleep(1)

print("\nAll station modes tested!")
```

## Serial Debug Output

When each station mode is triggered, you'll see:

```
Station 1: ALL BUTTONS PRESSED - FIRE WAVE MODE!
Station 1 Special: FIRE WAVE
Station Special mode ended - returning to IDLE
```

```
Station 2: ALL BUTTONS PRESSED - OCEAN STORM MODE!
Station 2 Special: OCEAN STORM
Station Special mode ended - returning to IDLE
```

```
Station 3: ALL BUTTONS PRESSED - NORTHERN LIGHTS MODE!
Station 3 Special: NORTHERN LIGHTS
Station Special mode ended - returning to IDLE
```

```
Station 4: ALL BUTTONS PRESSED - ELECTRIC PULSE MODE!
Station 4 Special: ELECTRIC PULSE
Station Special mode ended - returning to IDLE
```

## Status Monitoring

Check which mode is currently active:

```bash
curl -s http://192.168.1.200/status | jq '.current_plan'
```

**Plan Numbers:**
- 0 = IDLE
- 1 = SKIP
- 2 = SHOW
- 3 = SPECIAL
- 4 = MIXED_COLORS
- 5 = PARTY
- 6 = MEGA_SPECIAL
- **7 = STATION_1_SPECIAL** (Fire Wave)
- **8 = STATION_2_SPECIAL** (Ocean Storm)
- **9 = STATION_3_SPECIAL** (Northern Lights)
- **10 = STATION_4_SPECIAL** (Electric Pulse)

## Technical Details

### Animation Parameters

**Station 1 (Fire Wave):**
```cpp
// Update rate: 20ms (50 FPS)
// Hue range: 0-45 (red → orange → yellow)
// Wave count: 3 overlapping waves
// Wave speeds: /8, /12, /16 (different speeds for complexity)
```

**Station 2 (Ocean Storm):**
```cpp
// Update rate: 20ms (50 FPS)
// Hue range: 128-200 (cyan → blue → purple)
// Wave count: 3 turbulent waves
// Lightning: ~8% chance per frame after 1s cooldown
// Brightness range: 80-220 (dark depths to wave crests)
```

**Station 3 (Northern Lights):**
```cpp
// Update rate: 30ms (33 FPS - slower for smooth aurora)
// Hue range: 80-160 (green → teal → purple)
// Wave count: 2 slow flowing waves
// Shimmer: 4% chance per pixel
// Saturation: 180-255 (varies for depth)
```

**Station 4 (Electric Pulse):**
```cpp
// Update rate: 16ms (60 FPS - fastest!)
// Colors: Hue 200 (magenta) and 128 (cyan) alternating
// Pulse count: 3 simultaneous pulses
// Trail length: 15 LEDs
// Fade rate: 30 per frame
// Pulse spacing: 70 LEDs apart
```

### Memory Usage

- Each pattern uses static variables (no dynamic allocation)
- Minimal memory footprint
- Safe for ESP32 with 320KB RAM
- Flash usage: 70.3% (920KB / 1310KB)

### Configuration

Duration can be adjusted in `main.cpp`:
```cpp
#define STATION_SPECIAL_DURATION_MS 8000  // Change to adjust duration
```

## Customization

To modify animation parameters, edit `src/led_plans.cpp`:

**Fire Wave intensity:**
```cpp
uint8_t hue = map(combinedWave, 0, 255, 0, 45);  // Change 45 to adjust color range
```

**Ocean Storm lightning frequency:**
```cpp
if (now - lastLightning > 1000 && random8() < 20) {  // Change 20 for more/less lightning
```

**Northern Lights shimmer:**
```cpp
if (random8() < 10) {  // Change 10 to adjust shimmer density
```

**Electric Pulse speed:**
```cpp
uint8_t pulsePos = (elapsed / 20) % NUM_LEDS_PER_STRIP;  // Change 20 to adjust speed
```

## Comparison Table

| Station | Mode Name | Theme | Colors | Key Feature | FPS |
|---------|-----------|-------|--------|-------------|-----|
| 1 | Fire Wave | Hot/Energy | Red/Orange/Yellow | 3 overlapping waves | 50 |
| 2 | Ocean Storm | Cool/Water | Cyan/Blue/Purple | Lightning strikes | 50 |
| 3 | Northern Lights | Aurora | Green/Teal/Purple | Shimmer particles | 33 |
| 4 | Electric Pulse | Energy | Magenta/Cyan | Racing pulses | 60 |

## Integration with Mega Special

The **Mega Special Mode** is still available when 3+ stations press all buttons simultaneously:

```bash
# Trigger mega special (23 seconds)
curl -X POST http://192.168.1.200/mega-special
```

**Priority:**
- 3+ stations all-buttons → MEGA SPECIAL (23 seconds)
- 1 station all-buttons → That station's mode (8 seconds)
- Mixed buttons → MIXED_COLORS mode (continuous)

## Troubleshooting

**Issue:** Mode doesn't trigger from station

**Solutions:**
- Check UDP packets arriving: monitor serial output
- Verify station ID is 1-4
- Confirm button mask is 0x1F (all 5 buttons)
- Test with curl first to verify animation works

**Issue:** Animation looks wrong

**Solutions:**
- Check LED count matches NUM_LEDS_PER_STRIP (200)
- Verify RBG color order in led_plans.h
- Test other modes to isolate issue
- Check brightness setting (100/255)

**Issue:** Mode doesn't end after 8 seconds

**Solutions:**
- Check serial output for timeout message
- Verify STATION_SPECIAL_DURATION_MS = 8000
- Force back to idle: `curl -X POST http://192.168.1.200/idle`

## Credits

- **Station 1 (Fire Wave)**: Inspired by campfires and flames
- **Station 2 (Ocean Storm)**: Inspired by stormy seas and lightning
- **Station 3 (Northern Lights)**: Inspired by Aurora Borealis
- **Station 4 (Electric Pulse)**: Inspired by neon lights and electricity
- Designed for Flamingods Midburn art installation
- FastLED library for LED animations
