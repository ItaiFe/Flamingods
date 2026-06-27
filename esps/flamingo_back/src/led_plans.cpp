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
