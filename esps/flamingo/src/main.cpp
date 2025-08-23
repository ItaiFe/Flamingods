/**
 * LED Pattern Controller for ESP32
 * 
 * Controls 4 WS2812B LED strips with simple patterns.
 * Pins: 2 (red), 4 (green), 5 (blue), 18 (yellow)
 */

#include <Arduino.h>
#include <FastLED.h>
#include "led_plans.h"

// Pattern state
PatternState currentPattern = PATTERN_MOVING;

// Setup function
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== LED Pattern Controller Starting ===");
  
  // Initialize FastLED for each strip
  FastLED.addLeds<WS2811, LED_RED_PIN, GRB>(leds_green, NUM_LEDS_PER_STRIP);    // Pin 4 is actually green
  FastLED.addLeds<WS2811, LED_GREEN_PIN, GRB>(leds_red, NUM_LEDS_PER_STRIP);    // Pin 2 is actually red
  FastLED.addLeds<WS2811, LED_BLUE_PIN, GRB>(leds_blue, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2811, LED_YELLOW_PIN, GRB>(leds_yellow, NUM_LEDS_PER_STRIP);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Clear all LEDs
  clearAllLeds();
  
  // Start with moving pattern
  currentPattern = PATTERN_MOVING;
}

// Main loop
void loop() {
  switch (currentPattern) {
    case PATTERN_IDLE:
      // Play idle animation
      playIdleAnimation();
      break;
      
    case PATTERN_MOVING:
      // Play the moving pattern
      playMovingPattern();
      break;
  }
}
