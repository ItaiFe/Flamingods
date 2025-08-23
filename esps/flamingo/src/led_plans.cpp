#include "led_plans.h"

// Define the LED arrays for each strip
CRGB leds_red[NUM_LEDS_PER_STRIP];
CRGB leds_green[NUM_LEDS_PER_STRIP];
CRGB leds_blue[NUM_LEDS_PER_STRIP];
CRGB leds_yellow[NUM_LEDS_PER_STRIP];

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
  fill_solid(leds_red, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds_green, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds_blue, NUM_LEDS_PER_STRIP, CRGB::Black);
  fill_solid(leds_yellow, NUM_LEDS_PER_STRIP, CRGB::Black);
  FastLED.show();
}

// Idle animation - creates a flowing rainbow pattern across all LED strips
void playIdleAnimation() {
  unsigned long currentTime = millis();
  
  // Only update the animation every IDLE_UPDATE_INTERVAL milliseconds
  if (currentTime - lastIdleUpdate >= IDLE_UPDATE_INTERVAL) {
    lastIdleUpdate = currentTime;
    
    // Create dynamic wave patterns
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Calculate wave position for each LED
      uint8_t wave_pos = wave + i * WAVE_SPEED;
      
      // Create sinusoidal waves for brightness
      uint8_t bright_red = (sin8(wave_pos) * MAX_BRIGHTNESS) >> 8;
      uint8_t bright_green = (sin8(wave_pos + 64) * MAX_BRIGHTNESS) >> 8;
      uint8_t bright_blue = (sin8(wave_pos + 128) * MAX_BRIGHTNESS) >> 8;
      uint8_t bright_yellow = (sin8(wave_pos + 192) * MAX_BRIGHTNESS) >> 8;
      
      // Create color with dynamic brightness
      leds_red[i] = CHSV(hue, 255, bright_red);
      leds_green[i] = CHSV(hue + 64, 255, bright_green);
      leds_blue[i] = CHSV(hue + 128, 255, bright_blue);
      leds_yellow[i] = CHSV(hue + 192, 255, bright_yellow);
    }
    
    FastLED.show();
    
    // Update animation parameters
    wave += 2;  // Controls wave motion speed
    hue++;      // Controls color change speed
  }
}

// Moving pattern - simple loop that plays the same pattern on pins 2, 4, 5
void playMovingPattern() {
  unsigned long currentTime = millis();
  
  // Only update every 100ms for simple animation
  if (currentTime - lastMovingPatternUpdate >= MOVING_PATTERN_UPDATE_INTERVAL) {
    lastMovingPatternUpdate = currentTime;
    
    // Clear all LEDs first
    clearAllLeds();
    
    // Simple moving dot - same pattern on pins 2, 4, 5
    leds_red[movingPatternPosition] = CRGB::Blue;     // Pin 2
    leds_green[movingPatternPosition] = CRGB::Blue;   // Pin 4  
    leds_blue[movingPatternPosition] = CRGB::Blue;    // Pin 5
    
    FastLED.show();
    
    // Move to next position
    movingPatternPosition++;
    if (movingPatternPosition >= NUM_LEDS_PER_STRIP) {
      movingPatternPosition = 0;  // Start over
    }
  }
}
