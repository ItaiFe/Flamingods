#ifndef LED_PLANS_H
#define LED_PLANS_H

#include <FastLED.h>

// LED configuration
#define NUM_LEDS_PER_STRIP 200  // Number of LEDs in each strip (doubled)
#define NUM_STRIPS 3            // Number of LED strips
#define BRIGHTNESS 100          // LED brightness (0-255)

// Define pins for LED strips (3 strips total)
#define LED_STRIP_1_PIN  2
#define LED_STRIP_2_PIN  4
#define LED_STRIP_3_PIN  12

// Pattern states
enum PatternState {
  PATTERN_IDLE,
  PATTERN_MOVING
};

// LED arrays for each strip (all strips display the same pattern/color)
extern CRGB leds_strip_1[NUM_LEDS_PER_STRIP];
extern CRGB leds_strip_2[NUM_LEDS_PER_STRIP];
extern CRGB leds_strip_3[NUM_LEDS_PER_STRIP];

// LED pattern functions
void clearAllLeds();
void playIdleAnimation();
void playMovingPattern();

#endif // LED_PLANS_H
