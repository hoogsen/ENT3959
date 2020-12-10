#include <FastLED.h>
//Digital data pin output on the arduino
#define LED_PIN 2
//NUM_LEDS cannot be greater than 8 bits due to data types
#define NUM_LEDS 18
//NUM_SLICES cannot be greater than 16 bits due to data types
#define NUM_SLICES 128
//amount of distinct values the encoder has
#define ENCODER_MAX 3600

//Basically a cheat function until we get a real encoder
uint16_t readEncoder(){
    static uint16_t slice = 0;
    slice++;
    if(slice >= ENCODER_MAX){ //manual rollover to change colors faster
        slice = 0;
    }
    //TODO implement delay 
    return slice;
}

void main(){ 
    CRGB leds[NUM_LEDS];
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

    //pixel definitions, copy pasted from other file output
    const CRGB pixels[NUM_LEDS][NUM_SLICES] = {





    }


    uint16_t slice = 0;
    while(1){
        slice = readEncoder()/(ENCODER_MAX/NUM_SLICES);
        for(uint8_t ring = 0; ring < NUM_LEDS; ring++){
            leds[ring] = pixels[ring][slice];
        }
        FastLED.show();
    }
}
