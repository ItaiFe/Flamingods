#ifndef LED_PLANS_H
#define LED_PLANS_H

#include <FastLED.h>

// LED configuration
#define NUM_LEDS_PER_STRIP 100  // Number of LEDs in each strip
#define BRIGHTNESS 100          // LED brightness (0-255)

// Define pins for LED strips
#define LED_RED_PIN    4
#define LED_GREEN_PIN  2
#define LED_BLUE_PIN   5
#define LED_YELLOW_PIN 18

// Pattern states
enum PatternState {
  PATTERN_IDLE,
  PATTERN_MOVING
};

// LED arrays for each strip
extern CRGB leds_red[NUM_LEDS_PER_STRIP];
extern CRGB leds_green[NUM_LEDS_PER_STRIP];
extern CRGB leds_blue[NUM_LEDS_PER_STRIP];
extern CRGB leds_yellow[NUM_LEDS_PER_STRIP];

// LED pattern functions
void clearAllLeds();
void playIdleAnimation();
void playMovingPattern();

#endif // LED_PLANS_H
