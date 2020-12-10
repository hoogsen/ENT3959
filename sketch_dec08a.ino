#include <FastLED.h>
#define LED_PIN 2
#define NUM_LEDS 18
#define NUM_SLICES 32
#define ENCODER_MAX 3600

CRGB leds[NUM_LEDS];

uint16_t readEncoder(){
  static uint16_t slice = 0;
  slice++;
  if(slice >= ENCODER_MAX){
    slice = 0;
  }
  //optional(?) wait statement so the slice value isn't updating as fast as the little 16 MHz clock can go
  delay(100/NUM_SLICES); //100ms is about 1 fan full spin, we want to delay until the next color change
  return slice;
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  const CRGB pixels[NUM_LEDS][NUM_SLICES] = {
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}},
{{169,13,245}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}, {189, 100, 3}}
  };
  uint16_t slice = readEncoder()/(ENCODER_MAX/NUM_SLICES);
  for(uint8_t ring = 0; ring < NUM_LEDS; ring++){
    leds[ring] = pixels[ring][slice];
  }
  FastLED.show();
}
