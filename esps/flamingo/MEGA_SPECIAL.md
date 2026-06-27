# Mega Special Mode - Flamingo ESP32

## Overview

The **Mega Special Mode** is an epic 23-second animation sequence triggered when **all 3 stations press all 5 buttons simultaneously** (within a 2-second window). This creates a spectacular visual effect across all flamingo LED strips.

## Animation Sequence

### Total Duration: 23 seconds

1. **Stage 0: MALFUNCTION** (0-2 seconds)
   - Random flickering and glitching colors
   - 40% of LEDs randomly light up with random colors
   - Updates every 30ms for chaotic effect
   - Simulates a system malfunction

2. **Stage 1: DRAIN** (2-5 seconds)
   - Color drains from center (LED 100) outward to edges
   - Rainbow colors that change over time
   - Smooth animation over 3 seconds
   - Visual effect of "color leaking out"

3. **Stage 2: WAIT** (5-8 seconds)
   - All LEDs turn off completely
   - 3-second pause in darkness
   - Builds anticipation for next stage

4. **Stage 3: PINK PULSATE** (8-13 seconds)
   - Vivid pink color (HSV: hue 233, saturation 200)
   - Smooth pulsating effect using beatsin8
   - 5 seconds of rhythmic breathing
   - Bright and eye-catching

5. **Stage 4: CRAZY PARTY MODE** (13-23 seconds) **← NEW!**
   - **10 seconds of the craziest party mode ever!**
   - 5 different patterns cycling every 200ms:
     1. **Rapid Rainbow Chase** - Fast rotating rainbow hues
     2. **Strobe Effect** - Intense color strobes
     3. **Alternating Colors** - Fast switching complementary colors
     4. **Dense Sparkle** - 30% density random sparkles
     5. **Running Wave** - Sinusoidal wave with shifting hues
   - Updates at 60 FPS for maximum intensity
   - Absolutely bonkers visual effect!

6. **Stage 5: CONFETTI** (23+ seconds until mode ends)
   - Colorful confetti explosion
   - Random colored pixels appear across strips
   - Fade trail effect for visual persistence
   - 8 new confetti pieces per frame
   - Continues until mode timeout

## Triggering Conditions

### Automatic Trigger (UDP-based)
The mega special mode is automatically triggered when:
- **3 different stations** press all 5 buttons (button mask = 0x1F)
- All 3 stations must do this within a **2-second window** (`MEGA_SPECIAL_TIMEOUT_MS`)
- Uses UDP packets from stations (port 5000)

### Priority vs Party Mode
- If only **1 station** presses all buttons → **Party Mode** (10 seconds)
- If **3 stations** press all buttons within 2s window → **Mega Special Mode** (13 seconds)

### Manual Trigger (HTTP endpoint)
For testing purposes, you can manually trigger the mode:

```bash
# Using curl
curl -X POST http://192.168.1.200/mega-special

# Using Python test script
python test_mega_special.py 192.168.1.200
```

## Technical Implementation

### Files Modified

1. **include/led_plans.h**
   - Added `playMegaSpecialPattern()` function declaration

2. **src/led_plans.cpp**
   - Implemented complete 5-stage animation sequence
   - Uses FastLED functions: `random8()`, `CHSV()`, `beatsin8()`, `fadeToBlackBy()`
   - State machine with stage transitions based on elapsed time

3. **src/main.cpp**
   - Added `PLAN_MEGA_SPECIAL = 6` to `LightingPlan` enum
   - Added tracking variables:
     - `megaSpecialStartTime` - When mode started
     - `stationAllButtonsPressed[]` - Which stations have all buttons pressed
     - `stationAllButtonsTime[]` - When each station pressed all buttons
   - Modified `handleUDP()` to detect 3-station all-buttons condition
   - Added timeout handling in `loop()`
   - Added HTTP endpoint `/mega-special` for manual testing
   - Updated switch statement in `loop()` to call pattern function

### Key Configuration

```cpp
#define MEGA_SPECIAL_DURATION_MS 23000   // Total animation length (updated!)
#define MEGA_SPECIAL_TIMEOUT_MS 2000     // Window for 3 stations to sync
```

### Detection Logic

```cpp
// In handleUDP():
if (buttonMask == 0x1F) {  // All 5 buttons pressed
    // Mark this station
    stationAllButtonsPressed[stationId - 1] = true;
    stationAllButtonsTime[stationId - 1] = now;

    // Count how many stations within timeout window
    int activeStationsWithAllButtons = 0;
    for (int i = 0; i < 3; i++) {
        if (stationAllButtonsPressed[i] &&
            (now - stationAllButtonsTime[i] < MEGA_SPECIAL_TIMEOUT_MS)) {
            activeStationsWithAllButtons++;
        }
    }

    // If 3 stations → MEGA SPECIAL!
    if (activeStationsWithAllButtons >= 3) {
        currentPlan = PLAN_MEGA_SPECIAL;
        megaSpecialStartTime = now;
    }
}
```

## Testing

### Method 1: Manual HTTP Trigger
```bash
# Test with default IP (192.168.1.200)
python test_mega_special.py

# Test with custom IP
python test_mega_special.py 192.168.50.100

# Or use curl
curl -X POST http://192.168.1.200/mega-special
```

### Method 2: Simulate 3-Station Trigger
If you have access to stations or can simulate UDP packets:

```python
import socket
import time

# Send all-buttons (0x1F) from 3 different station IDs
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
flamingo_ip = "192.168.1.200"
udp_port = 5000

# Station 1 all buttons
sock.sendto(bytes([1, 0x1F]), (flamingo_ip, udp_port))
time.sleep(0.5)

# Station 2 all buttons
sock.sendto(bytes([2, 0x1F]), (flamingo_ip, udp_port))
time.sleep(0.5)

# Station 3 all buttons (should trigger mega special!)
sock.sendto(bytes([3, 0x1F]), (flamingo_ip, udp_port))
```

### Method 3: Physical Station Testing
1. Have 3 people at 3 different stations
2. Coordinate to press all 5 buttons simultaneously
3. Must happen within 2-second window
4. Watch the flamingo LEDs for the mega special sequence!

## Serial Debug Output

When mega special is triggered, you'll see:

```
*** ALL 3 STATIONS ALL BUTTONS PRESSED - MEGA SPECIAL MODE! ***
Mega Special: Starting Stage 0 - MALFUNCTION
Mega Special: Stage 1 - DRAIN
Mega Special: Stage 2 - WAIT
Mega Special: Stage 3 - PINK PULSATE
Mega Special: Stage 4 - CRAZY PARTY MODE!
Mega Special: Stage 5 - CONFETTI
Mega Special mode ended - returning to IDLE
```

## Status Monitoring

Check current mode via HTTP:

```bash
curl http://192.168.1.200/status | jq .current_plan
```

Plan values:
- 0 = IDLE
- 1 = SKIP
- 2 = SHOW
- 3 = SPECIAL
- 4 = MIXED_COLORS
- 5 = PARTY
- 6 = **MEGA_SPECIAL**

## Build and Upload

```bash
# Build firmware
make build
# or
pio run

# Upload via USB
make upload
# or
pio run --target upload

# Upload via OTA (if flamingo is already running)
export ESP_OTA_PASSWORD=flamingods2024
pio run -e esp32dev-ota --target upload --upload-port 192.168.1.200
```

## Performance Notes

- **Update rates vary by stage:**
  - Malfunction: 30ms (33 FPS)
  - Drain: 30ms (33 FPS)
  - Pink Pulsate: 20ms (50 FPS)
  - Crazy Party: 16ms (60 FPS) ← Fastest!
  - Confetti: 20ms (50 FPS)

- **Memory usage:**
  - Static variables in pattern function
  - No dynamic allocation
  - Safe for ESP32 with 320KB RAM

- **LED count:** Works with 200 LEDs per strip (3 strips total = 600 LEDs)

## Customization

To adjust animation parameters, edit `src/led_plans.cpp`:

```cpp
// Stage 0 - Malfunction
if (random8() < 100)  // Change 100 to adjust density (0-255)

// Stage 1 - Drain speed
if (now - lastUpdate >= 30)  // Lower = faster, higher = slower

// Stage 3 - Pink color
CRGB pink = CHSV(233, 200, brightness);  // Adjust hue/saturation

// Stage 3 - Pulse speed
uint8_t brightness = beatsin8(60, 50, 255);  // 60 BPM, adjust for faster/slower

// Stage 4 - Confetti density
int numConfetti = 8;  // More = denser confetti
fadeToBlackBy(leds, NUM_LEDS_PER_STRIP, 20);  // Higher = faster fade
```

## Future Enhancements

Possible additions:
- Sound effects via speaker (if hardware added)
- Trigger countdown indicator on stations
- Configurable duration via HTTP params
- Multiple animation variants (random selection)
- Integration with raspberry Pi server for logging

## Troubleshooting

**Issue:** Mega special not triggering with 3 stations

**Solutions:**
- Check serial output: ensure stations are sending UDP packets
- Verify station IDs are 1, 2, 3 (not 0-indexed)
- Confirm all stations press within 2-second window
- Check button mask = 0x1F (all 5 buttons)

**Issue:** Animation looks wrong

**Solutions:**
- Verify LED count in `led_plans.h` matches physical strips
- Check `NUM_LEDS_PER_STRIP = 200`
- Ensure RBG color order is correct for your LED strips
- Test with HTTP endpoint first before station triggers

**Issue:** Mode doesn't end after 13 seconds

**Solutions:**
- Check serial output for timeout message
- Verify `MEGA_SPECIAL_DURATION_MS = 13000`
- Restart ESP32 if stuck

## Credits

- Designed for Flamingods Midburn art installation
- Pattern inspired by "malfunction → drain → rebirth" concept
- FastLED library for LED control
- ESP32 Arduino framework
