#include "led_plans.h"

// Define the LED arrays for each strip (all strips display the same pattern)
CRGB leds_strip_1[NUM_LEDS_PER_STRIP];
CRGB leds_strip_2[NUM_LEDS_PER_STRIP];
CRGB leds_strip_3[NUM_LEDS_PER_STRIP];

// Idle animation variables
uint8_t hue = 0;
uint8_t wave = 0;
unsigned long lastIdleUpdate = 0;
#define IDLE_UPDATE_INTERVAL 20  // How often to update the idle animation (ms)
#define WAVE_SPEED 8            // Speed of the wave motion
#define MAX_BRIGHTNESS 200      // Maximum brightness for the wave

// Moving pattern variables
uint8_t movingPatternPosition = 0;
unsigned long lastMovingPatternUpdate = 0;
#define MOVING_PATTERN_UPDATE_INTERVAL 100  // How often to update the moving pattern (ms)

// Clear all LEDs
void clearAllLeds() {
  fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
}

// Idle animation - creates a flowing rainbow pattern (same on all 3 strips)
void playIdleAnimation() {
  unsigned long currentTime = millis();

  // Only update the animation every IDLE_UPDATE_INTERVAL milliseconds
  if (currentTime - lastIdleUpdate >= IDLE_UPDATE_INTERVAL) {
    lastIdleUpdate = currentTime;

    // Create dynamic wave patterns - same on all strips
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Calculate wave position for each LED
      uint8_t wave_pos = wave + i * WAVE_SPEED;

      // Create sinusoidal wave for brightness
      uint8_t brightness = (sin8(wave_pos) * MAX_BRIGHTNESS) >> 8;

      // Create color with dynamic brightness (same on all 3 strips)
      CRGB color = CHSV(hue, 255, brightness);
      leds_strip_1[i] = color;
      leds_strip_2[i] = color;
      leds_strip_3[i] = color;
    }

    FastLED.show();

    // Update animation parameters
    wave += 2;  // Controls wave motion speed
    hue++;      // Controls color change speed
  }
}

// Moving pattern - simple loop that plays the same pattern on all 3 strips
void playMovingPattern() {
  unsigned long currentTime = millis();

  // Only update every 100ms for simple animation
  if (currentTime - lastMovingPatternUpdate >= MOVING_PATTERN_UPDATE_INTERVAL) {
    lastMovingPatternUpdate = currentTime;

    // Clear all LEDs first
    clearAllLeds();

    // Simple moving dot - same pattern on all 3 strips (pins 2, 4, 5)
    leds_strip_1[movingPatternPosition] = CRGB::Blue;
    leds_strip_2[movingPatternPosition] = CRGB::Blue;
    leds_strip_3[movingPatternPosition] = CRGB::Blue;

    FastLED.show();

    // Move to next position
    movingPatternPosition++;
    if (movingPatternPosition >= NUM_LEDS_PER_STRIP) {
      movingPatternPosition = 0;  // Start over
    }
  }
}

// Mega Special Pattern - 6 stage animation
// Stage 0 (0-2s): Malfunction - random flickering/glitching
// Stage 1 (2-5s): Drain from center (LED 100) to edges
// Stage 2 (5-8s): Off/wait for 3 seconds
// Stage 3 (8-13s): Pulsating vivid pink for 5 seconds
// Stage 4 (13-23s): CRAZY PARTY MODE for 10 seconds
// Stage 5 (23s+): Confetti explosion
void playMegaSpecialPattern() {
  static unsigned long stageStartTime = 0;
  static int currentStage = 0;
  static unsigned long lastUpdate = 0;
  static uint8_t animationStep = 0;

  unsigned long now = millis();

  // Initialize on first call
  if (stageStartTime == 0) {
    stageStartTime = now;
    currentStage = 0;
    animationStep = 0;
    Serial.println("Mega Special: Starting Stage 0 - MALFUNCTION");
  }

  unsigned long elapsed = now - stageStartTime;

  // Stage transitions
  if (currentStage == 0 && elapsed >= 2000) {
    currentStage = 1;
    animationStep = 0;
    Serial.println("Mega Special: Stage 1 - DRAIN");
  } else if (currentStage == 1 && elapsed >= 5000) {
    currentStage = 2;
    clearAllLeds();
    Serial.println("Mega Special: Stage 2 - WAIT");
  } else if (currentStage == 2 && elapsed >= 8000) {
    currentStage = 3;
    animationStep = 0;
    Serial.println("Mega Special: Stage 3 - PINK PULSATE");
  } else if (currentStage == 3 && elapsed >= 13000) {
    currentStage = 4;
    animationStep = 0;
    Serial.println("Mega Special: Stage 4 - CRAZY PARTY MODE!");
  } else if (currentStage == 4 && elapsed >= 23000) {
    currentStage = 5;
    animationStep = 0;
    Serial.println("Mega Special: Stage 5 - CONFETTI");
  }

  // Stage animations
  switch (currentStage) {
    case 0: // MALFUNCTION (0-2s)
      if (now - lastUpdate >= 30) {  // Update every 30ms for glitchy effect
        lastUpdate = now;
        for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
          // Random glitchy colors - sometimes on, sometimes off
          if (random8() < 100) {  // 40% chance of being lit
            CRGB color = CHSV(random8(), 255, random8(100, 255));
            leds_strip_1[i] = color;
            leds_strip_2[i] = color;
            leds_strip_3[i] = color;
          } else {
            leds_strip_1[i] = CRGB::Black;
            leds_strip_2[i] = CRGB::Black;
            leds_strip_3[i] = CRGB::Black;
          }
        }
        FastLED.show();
      }
      break;

    case 1: // DRAIN from center to edges (2-5s, 3 seconds duration)
      if (now - lastUpdate >= 30) {  // Update every 30ms
        lastUpdate = now;

        // Calculate drain position (0 to 100)
        // Over 3 seconds (3000ms / 30ms = 100 steps)
        int drainRadius = animationStep;

        // Clear everything first
        clearAllLeds();

        // Light up from center (LED 100) outward by drainRadius
        int center = NUM_LEDS_PER_STRIP / 2;  // LED 100
        int remainingRadius = 100 - drainRadius;

        if (remainingRadius > 0) {
          // Rainbow colors that change over time
          uint8_t baseHue = animationStep * 2;

          for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
            int distanceFromCenter = abs(i - center);
            if (distanceFromCenter <= remainingRadius) {
              // Color based on distance from center
              uint8_t hue = baseHue + (distanceFromCenter * 2);
              CRGB color = CHSV(hue, 255, 200);
              leds_strip_1[i] = color;
              leds_strip_2[i] = color;
              leds_strip_3[i] = color;
            }
          }
        }

        FastLED.show();

        animationStep++;
        if (animationStep > 100) {
          animationStep = 100;  // Clamp at 100
        }
      }
      break;

    case 2: // WAIT - already cleared, just keep it off
      // Do nothing, LEDs stay off
      break;

    case 3: // PULSATING VIVID PINK (8-13s, 5 seconds)
      if (now - lastUpdate >= 20) {  // Update every 20ms for smooth pulsing
        lastUpdate = now;

        // Vivid pink color (HSV: hue ~233 (330 degrees in 0-255 scale), sat 200, varying brightness)
        uint8_t brightness = beatsin8(60, 50, 255);  // Pulse at 60 BPM
        CRGB pink = CHSV(233, 200, brightness);

        fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, pink);
        fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, pink);
        fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, pink);

        FastLED.show();
      }
      break;

    case 4: // CRAZY PARTY MODE (13-23s, 10 seconds)
      if (now - lastUpdate >= 16) {  // Update at 60 FPS for crazy fast effect
        lastUpdate = now;

        // Rapid cycling through different patterns
        uint8_t patternIndex = (elapsed / 200) % 5;  // Change pattern every 200ms

        switch (patternIndex) {
          case 0:  // Rapid rainbow chase
            {
              uint8_t baseHue = (elapsed / 10) % 256;  // Fast hue rotation
              for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                CRGB color = CHSV(baseHue + (i * 10), 255, 255);
                leds_strip_1[i] = color;
                leds_strip_2[i] = color;
                leds_strip_3[i] = color;
              }
            }
            break;

          case 1:  // Strobe effect with random colors
            {
              uint8_t strobeHue = (elapsed / 50) % 256;
              CRGB strobeColor = CHSV(strobeHue, 255, 255);
              fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, strobeColor);
              fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, strobeColor);
              fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, strobeColor);
            }
            break;

          case 2:  // Alternating colors with fast switching
            {
              uint8_t hue1 = (elapsed / 30) % 256;
              uint8_t hue2 = (hue1 + 128) % 256;
              CRGB color1 = CHSV(hue1, 255, 255);
              CRGB color2 = CHSV(hue2, 255, 255);
              for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                CRGB color = (i % 2 == 0) ? color1 : color2;
                leds_strip_1[i] = color;
                leds_strip_2[i] = color;
                leds_strip_3[i] = color;
              }
            }
            break;

          case 3:  // Sparkle effect with high density
            {
              for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                if (random8() < 80) {  // 30% chance of sparkle (very dense)
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

          case 4:  // Running wave with multiple hues
            {
              uint8_t baseHue = (elapsed / 20) % 256;
              for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
                uint8_t wave = sin8((i * 8) + (elapsed / 4));
                uint8_t hue = baseHue + (i * 3);
                CRGB color = CHSV(hue, 255, wave);
                leds_strip_1[i] = color;
                leds_strip_2[i] = color;
                leds_strip_3[i] = color;
              }
            }
            break;
        }

        FastLED.show();
      }
      break;

    case 5: // CONFETTI EXPLOSION
      if (now - lastUpdate >= 20) {  // Update every 20ms
        lastUpdate = now;

        // Fade all LEDs slightly for trail effect
        fadeToBlackBy(leds_strip_1, NUM_LEDS_PER_STRIP, 20);
        fadeToBlackBy(leds_strip_2, NUM_LEDS_PER_STRIP, 20);
        fadeToBlackBy(leds_strip_3, NUM_LEDS_PER_STRIP, 20);

        // Add random colored confetti
        int numConfetti = 8;  // Number of new confetti per frame
        for (int i = 0; i < numConfetti; i++) {
          int pos = random16(NUM_LEDS_PER_STRIP);
          CRGB color = CHSV(random8(), 255, 255);
          leds_strip_1[pos] = color;
          leds_strip_2[pos] = color;
          leds_strip_3[pos] = color;
        }

        FastLED.show();
      }
      break;
  }

  // Reset static variables when pattern ends (will be handled by main loop timeout)
  // This gets called on mode exit
  static bool resetNeeded = false;
  if (elapsed >= 23000 && !resetNeeded) {
    resetNeeded = true;
  }

  // Check if we need to reset (called from outside)
  static unsigned long lastResetCheck = 0;
  if (now - lastResetCheck > 1000) {
    lastResetCheck = now;
    // If we've been running for over 24 seconds, reset
    if (elapsed >= 24000) {
      stageStartTime = 0;
      currentStage = 0;
      animationStep = 0;
      resetNeeded = false;
    }
  }
}

// ============================================================================
// Station-Specific Special Modes (8 seconds each)
// ============================================================================

/**
 * Station 1 Special: FIRE WAVE
 * Hot colors (red, orange, yellow) in wave patterns like fire
 */
void playStation1Special() {
  static unsigned long lastUpdate = 0;
  static unsigned long startTime = 0;
  unsigned long now = millis();

  if (startTime == 0) {
    startTime = now;
    Serial.println("Station 1 Special: FIRE WAVE");
  }

  if (now - lastUpdate >= 20) {  // 50 FPS
    lastUpdate = now;
    unsigned long elapsed = now - startTime;

    // Fire wave effect with hot colors
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Multiple wave layers for fire effect
      uint8_t wave1 = sin8((i * 12) + (elapsed / 8));
      uint8_t wave2 = sin8((i * 8) + (elapsed / 12));
      uint8_t wave3 = sin8((i * 16) + (elapsed / 6));

      // Combine waves
      uint8_t combinedWave = (wave1 + wave2 + wave3) / 3;

      // Fire colors: hue 0-45 (red to orange to yellow)
      uint8_t hue = map(combinedWave, 0, 255, 0, 45);
      uint8_t sat = 255;
      uint8_t val = combinedWave;

      CRGB color = CHSV(hue, sat, val);
      leds_strip_1[i] = color;
      leds_strip_2[i] = color;
      leds_strip_3[i] = color;
    }

    FastLED.show();

    // Reset after 10 seconds
    if (elapsed >= 10000) {
      startTime = 0;
    }
  }
}

/**
 * Station 2 Special: OCEAN STORM
 * Cool colors (blue, cyan, purple) with turbulent waves and lightning
 */
void playStation2Special() {
  static unsigned long lastUpdate = 0;
  static unsigned long startTime = 0;
  static unsigned long lastLightning = 0;
  unsigned long now = millis();

  if (startTime == 0) {
    startTime = now;
    lastLightning = now;
    Serial.println("Station 2 Special: OCEAN STORM");
  }

  if (now - lastUpdate >= 20) {  // 50 FPS
    lastUpdate = now;
    unsigned long elapsed = now - startTime;

    // Check if lightning should strike (random every 1-2 seconds)
    bool lightning = false;
    if (now - lastLightning > 1000 && random8() < 20) {  // ~8% chance per frame
      lightning = true;
      lastLightning = now;
    }

    if (lightning) {
      // Lightning flash - bright white
      fill_solid(leds_strip_1, NUM_LEDS_PER_STRIP, CRGB::White);
      fill_solid(leds_strip_2, NUM_LEDS_PER_STRIP, CRGB::White);
      fill_solid(leds_strip_3, NUM_LEDS_PER_STRIP, CRGB::White);
    } else {
      // Ocean storm waves - cool colors
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        // Turbulent wave patterns
        uint8_t wave1 = sin8((i * 10) + (elapsed / 10));
        uint8_t wave2 = sin8((i * 15) - (elapsed / 7));
        uint8_t wave3 = sin8((i * 5) + (elapsed / 15));

        uint8_t combinedWave = (wave1 + wave2 + wave3) / 3;

        // Ocean colors: hue 128-200 (cyan to blue to purple)
        uint8_t hue = map(combinedWave, 0, 255, 128, 200);
        uint8_t sat = 255;
        uint8_t val = map(combinedWave, 0, 255, 80, 220);

        CRGB color = CHSV(hue, sat, val);
        leds_strip_1[i] = color;
        leds_strip_2[i] = color;
        leds_strip_3[i] = color;
      }
    }

    FastLED.show();

    // Reset after 10 seconds
    if (elapsed >= 10000) {
      startTime = 0;
    }
  }
}

/**
 * Station 3 Special: NORTHERN LIGHTS (Aurora Borealis)
 * Green, teal, purple flowing aurora with shimmer
 */
void playStation3Special() {
  static unsigned long lastUpdate = 0;
  static unsigned long startTime = 0;
  unsigned long now = millis();

  if (startTime == 0) {
    startTime = now;
    Serial.println("Station 3 Special: NORTHERN LIGHTS");
  }

  if (now - lastUpdate >= 30) {  // 33 FPS for smooth aurora
    lastUpdate = now;
    unsigned long elapsed = now - startTime;

    // Aurora effect - smooth flowing colors
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Slow flowing waves for aurora
      uint8_t wave1 = sin8((i * 6) + (elapsed / 20));
      uint8_t wave2 = sin8((i * 4) - (elapsed / 25));

      uint8_t combinedWave = (wave1 + wave2) / 2;

      // Aurora colors: hue 80-160 (green to teal to purple)
      uint8_t hue = map(combinedWave, 0, 255, 80, 160);
      uint8_t sat = map(combinedWave, 0, 255, 180, 255);
      uint8_t val = map(combinedWave, 0, 255, 100, 255);

      CRGB color = CHSV(hue, sat, val);

      // Add shimmer particles
      if (random8() < 10) {  // ~4% chance of shimmer
        color += CRGB(40, 40, 40);  // Brighten pixel
      }

      leds_strip_1[i] = color;
      leds_strip_2[i] = color;
      leds_strip_3[i] = color;
    }

    FastLED.show();

    // Reset after 10 seconds
    if (elapsed >= 10000) {
      startTime = 0;
    }
  }
}

/**
 * Station 4 Special: ELECTRIC PULSE
 * Magenta/electric blue pulses racing along the strip
 */
void playStation4Special() {
  static unsigned long lastUpdate = 0;
  static unsigned long startTime = 0;
  unsigned long now = millis();

  if (startTime == 0) {
    startTime = now;
    Serial.println("Station 4 Special: ELECTRIC PULSE");
  }

  if (now - lastUpdate >= 16) {  // 60 FPS for fast pulses
    lastUpdate = now;
    unsigned long elapsed = now - startTime;

    // Electric pulse effect
    uint8_t pulsePos = (elapsed / 20) % NUM_LEDS_PER_STRIP;

    // Fade all LEDs
    fadeToBlackBy(leds_strip_1, NUM_LEDS_PER_STRIP, 30);
    fadeToBlackBy(leds_strip_2, NUM_LEDS_PER_STRIP, 30);
    fadeToBlackBy(leds_strip_3, NUM_LEDS_PER_STRIP, 30);

    // Draw racing pulses
    for (int j = 0; j < 3; j++) {  // 3 pulses at once
      int pos = (pulsePos + (j * 70)) % NUM_LEDS_PER_STRIP;

      // Pulse trail
      for (int k = 0; k < 15; k++) {
        int trailPos = (pos - k + NUM_LEDS_PER_STRIP) % NUM_LEDS_PER_STRIP;
        uint8_t brightness = 255 - (k * 17);

        // Electric colors: magenta (hue ~200) and cyan (hue ~128) alternating
        uint8_t hue = (j % 2 == 0) ? 200 : 128;
        CRGB color = CHSV(hue, 255, brightness);

        leds_strip_1[trailPos] += color;
        leds_strip_2[trailPos] += color;
        leds_strip_3[trailPos] += color;
      }
    }

    FastLED.show();

    // Reset after 10 seconds
    if (elapsed >= 10000) {
      startTime = 0;
    }
  }
}
