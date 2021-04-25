#include <SPI.h>
#include <SD.h>

#define NUM_LED           (50)          // Number of LEDs
#define NUM_BYTES         (NUM_LED*3)   // Number of total bytes (3 per each LED)
#define DIGITAL_PIN       (5)           // Digital port number
#define PORT              (PORTD)       // Digital pin's port
#define PORT_PIN          (PORTD5)      // Digital pin's bit position
#define HALL_EFFECT_PIN   (2)           // Digital Pin to handle Hall-Effect interrupts
#define SD_CS_PIN         (4)           // SD card reader init pin
#define NUM_BITS          (8)           // Bits per byte
#define RGB_FILENAME      ("image.fan") // Name of preprocessed image text file containing RGB vals
#define NUM_SLICES        (32)          // Number of time slices
#define MAX_BRIGHTNESS    (32)          // Self-Explanatory, runs from 0-255

uint32_t currentDelay = 10; // Timeslice offset delay set by hall-effect sensor
uint32_t lastTime = 0;      // Used by hall-effect sensor to calculate slice time 
uint32_t currentTime;       // Tracks current runtime 
uint32_t lastSlice = 0;     // Tracks time since previous slice began
uint8_t* RGBs;              // Current RGB values memory space
uint8_t* RGBreset;          // Base address of RGBs, used for memory resets

File f; // SD card reader file

/*    
 *  SD Card Reader Pins
 *  
 *  MOSI - pin 11
 *  MISO - pin 12
 *  CLK - pin 13
 *  CS - pin 4 
 */
void setup() {
    // Init Pins
    pinMode(DIGITAL_PIN,OUTPUT);
    pinMode(HALL_EFFECT_PIN, INPUT);
   
    //enable 10k pullup resistor between signal and 5v on hall effect sensor 
    digitalWrite(HALL_EFFECT_PIN,HIGH); 

    //Enable Hall Effect Sensor Interrupts 
    attachInterrupt(digitalPinToInterrupt(HALL_EFFECT_PIN), hallEffectISR, RISING);
    digitalWrite(DIGITAL_PIN,0);
    digitalWrite(HALL_EFFECT_PIN,HIGH);
    
    //SD reader init
    if (!SD.begin(SD_CS_PIN)) {
        return;
    }
    
    // Open RGB file
    f = SD.open(RGB_FILENAME);
    if (!f) {
        return;
    }

    // Make space in memory for RGB values
    RGBs = (uint8_t *)malloc(NUM_BYTES);
    memset(RGBs, 0, NUM_BYTES);
    RGBreset = RGBs;

    // Send an initial (0, 0, 0) signal for LEDs
    sendSignal();

    // Initialize Hall-Effect reference time
    lastTime = micros();
    currentTime = micros();
}

/*
*  Clears current time slice's RGB values from memory
*/
void resetRGBMem() {
    RGBs = RGBreset;
    memset(RGBs, 0, NUM_BYTES);
}

/*
*  Hall Effect ISR 
*
*  Triggers every time a full rotation has completed 
*  Calculate delay based on elapsed time since last pass 
*/
void hallEffectISR() {
    currentDelay = ((micros() - lastTime)/(NUM_SLICES));
    lastTime = micros();
}


/*
 * Main loop of the function, every time it is entered it checks the time, then sees if the time should
 * be in another time slice of code.
 * an example would be that lets say at time = 100ms, time slice 4 is supposed to start
 * and that time slice 5 does not start until 140ms (based on current rotational speed as seen by the hall sensor)
 * every time the loop enters, it checks the time (100ms, 108ms, 114ms, etc)
 * once that time is greater than 140ms, it has entered time slice 5 and buffers the next set of values to be used
 * in addition to setting saying that we won't enter the next time slice until 180ms
 */
void loop() {
    currentTime = micros(); 
    
    // Allows new timeslice operations based on elapsed time of current timeslice with the delay modifier
    if(currentTime > lastSlice+(currentDelay)) {
        lastSlice = currentTime; 
        // Ensure currency pointer has not reached EOF, reset it if so
        if (!f.available()){
            f.seek(0);
        }
        // Take the next NUM_LEDs values from the sd card file and store them in uint8_t* RGBs
        SD_read(f);
        // Take the RGB values in uint8_t* RGBs and send them to our LEDs
        sendSignal();
        // Flush RGB buffer
        resetRGBMem();
  }
}

/*
*  Populate RGB memory with values read in from SD card 
*/
void setColorRGB(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if(index < NUM_LED) {
        // Point to current set of 3 bytes
        uint8_t *p = &RGBs[index * 3]; 
        // Load RGB values into memory
        *p++ = green;  
        *p++ = red;
        *p   = blue;
    }
}

/*
 * Pulls values from the SD card using the arduino SD card library
 * These values are then stored in a global global variable so that
 * the sendSignal function can access them and turn on the LEDs
 */
void SD_read(File SDFile) {
    // Integer LED value (0-255) corresponding to brightness of R G or B
    // The array will hold RGB values for every LED we have in the order of
    // R G B, so val[0] is R for LED 0, val[1] is B for 0, val[6] is R for LED 2
    uint8_t val[(NUM_LED*3)];

    // Takes NUM_LED*3 values from the SD card and puts them in array val
    SDFile.read(val, (NUM_LED*3)); 

    // Send all buffered values to program memory
    for(uint8_t j = 0; j < NUM_LED; j++){ 
        setColorRGB(j, val[(j*3)+0], val[(j*3)+1] , val[(j*3)+2]); 
    }
    // Clear buffer memory
    memset(val, 0, (NUM_LED*3)); 
}

/*
*  Read RGB values from program mem, use AVR assembly
*  magic to send signal formatted so WS2812B LEDs can
*  read it.  
*/
void sendSignal(void) {
    if(!RGBs)
        return;

    // micros() - Returns the number of microSeconds passed since 
    // Ensure 50 microSeconds has passed for data to latch
    while((micros() - currentTime) < 50L);

    volatile uint8_t  
        *p    = RGBs,                 // Copy the start address of our data array
        val  = *p++,                  // Get the current byte value & point to next byte
        high = PORT |  _BV(PORT_PIN), // Bitmask for sending HIGH to pin 
        low  = PORT & ~_BV(PORT_PIN), // Bitmask for sending LOW to pin
        tmp  = low,                   // Swap variable to adjust duty cycle 
        nbits= NUM_BITS;              // Bit counter for inner loop

    volatile uint16_t nbytes = NUM_BYTES; // Byte counter for outer loop
        /*
        *    The Arduino Nano/Uno/Mega run at 16MHz, which means each clock 
        *    cycle takes roughly 0.0625 uS. In order to send a 1 value, pulse is held
        *    HIGH for 0.8 uS then LOW for 0.45 uS for a total period of 1.25 uS which
        *    equates to 20 CLK cycles. 
        *    For a 0 value the signal is held HIGH for 0.4 uS and LOW for 0.85 uS
        *  
        *    Having the constraint of hitting exactly 20 clock cycles per transferred bit
        *    is made considerably easier by the following two AVR asm commands:
        *    nop  - Idle for one clock cycle
        *    rjmp - Idle for two clock cycles (Used by passing .+0 flag)
        */
        asm volatile(
                                // Cycles
         "nextbit:\n\t"
          "sbi  %0, %1\n\t"     // 2    signal HIGH
          "sbrc %4, 7\n\t"      // 3    if MSB set
          "mov  %6, %3\n\t"     // 4    tmp'll set signal high
          "dec  %5\n\t"         // 5    decrease bitcount
          "nop\n\t"             // 6    idle 1 clock cycle
          "st   %a2, %6\n\t"    // 8    set PORT to tmp
          "mov  %6, %7\n\t"     // 9    reset tmp to low (default)
          "breq nextbyte\n\t"   // 10   if !bitcount -> nextbyte
          "rol  %4\n\t"         // 11   shift MSB leftwards
          "rjmp .+0\n\t"        // 13   idle 2 clock cycles
          "cbi   %0, %1\n\t"    // 15   signal LOW
          "rjmp .+0\n\t"        // 17
          "nop\n\t"             // 18
          "rjmp nextbit\n\t"    // 20   bitcount !=0 -> nextbit
          //Signal Idles LOW
         "nextbyte:\n\t"
          "ldi  %5, 8\n\t"      // 1    reset bitcount
          "ld   %4, %a8+\n\t"   // 3    val = *p++
          "cbi   %0, %1\n\t"    // 5    signal LOW
          "rjmp .+0\n\t"        // 7                         
          "nop\n\t"             // 8                             
          "dec %9\n\t"          // 9    decrease bytecount          
          "brne nextbit\n\t"    // 10   if bytecount !=0 -> nextbit 
          ::
          // Input operands
          "I" (_SFR_IO_ADDR(PORT)), // %0
          "I" (PORT_PIN),           // %1
          "e" (&PORT),              // %a2
          "r" (high),               // %3
          "r" (val),                // %4
          "r" (nbits),              // %5
          "r" (tmp),                // %6
          "r" (low),                // %7
          "e" (p),                  // %a8
          "w" (nbytes)              // %9
        );
        currentTime = micros();
}
