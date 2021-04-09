#include <SPI.h>
#include <SD.h>

#define NUM_LED           (18)          // Number of LEDs
#define NUM_BYTES         (NUM_LED*3)   // Number of total bytes (3 per each LED)
#define DIGITAL_PIN       (5)           // Digital port number
#define PORT              (PORTD)       // Digital pin's port
#define PORT_PIN          (PORTD5)      // Digital pin's bit position
#define HALL_EFFECT_PIN   (2)           // Digital Pin to handle Hall-Effect interrupts
#define SD_CS_PIN         (4)           // SD card reader pin
#define NUM_BITS          (8)           // Bits per byte
#define RGB_FILENAME      ("theX.txt")  // Name of preprocessed image text file containing RGB vals
#define NUM_SLICES        (32)          // Number of time slices
#define MAX_BRIGHTNESS    (32)          // Self-Explanatory, runs from 0-255

uint8_t* currentRGB = NULL; 
uint16_t currentDelay = 0;
uint32_t lastTime = 0;
uint32_t currentTime;
uint8_t* RGBs;
uint8_t* RGBreset;
uint16_t rotations = 0;
bool     hallCheck = false;

File f;

/*  TODO: Reset currency pointer to beginning once end is hit
 *   - Detect when end is hit
 *   - Figure out LSEEK for type File
 *   - File is type from SD Library
 *   
 *  SD Card Reader Pins
 *  
 *  MOSI - pin 11
 *  MISO - pin 12
 *  CLK - pin 13
 *  CS - pin 4 
 */
void setup() {
    //Serial Init for testing purposes, will be removed in final version
    Serial.begin(9600);
    while (!Serial) {}
    
    pinMode(DIGITAL_PIN,OUTPUT);
    pinMode(HALL_EFFECT_PIN, INPUT);

    //Enable Hall Effect Sensor Interrupts 
    attachInterrupt(digitalPinToInterrupt(HALL_EFFECT_PIN), hallEffectISR, CHANGE);
    digitalWrite(DIGITAL_PIN,0);
  
    //SD reader init
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Couldn't initialize SD card reader, exiting");
        return;
    }

    f = SD.open(RGB_FILENAME);
    if (!f) {
        Serial.println("error opening RGB file from SD card, exiting");
        return;
    }

    //Make space in memory for RGB values
    RGBs = (uint8_t *)malloc(NUM_BYTES);
    memset(RGBs, 0, NUM_BYTES);
    RGBreset = RGBs;

    //Send an initial (0, 0, 0) signal for LEDs
    sendSignal();
}

//Used for resetting memory each time slice
void resetRGBMem() {
    RGBs = RGBreset;
    memset(RGBs, 0, NUM_BYTES);
}

/*
*  Hall Effect ISR
*
*  Whenever this triggers another rotation has happened
*  Calculate delay based on current RPM
*/
void hallEffectISR() {
    if (hallCheck) {
        //Set Delay in ms
        currentDelay = ((micros() - lastTime)/NUM_SLICES) * 1000;
        Serial.println(micros()); 
    }
    else
        hallCheck = !hallCheck;
}

void loop() {
    uint8_t i;
    // Load RGB vals into program memory from preprocessor
    SD_read(f);
    currentTime = micros();
    sendSignal();
    resetRGBMem();
  //  delay(currentDelay);
    delay(200); //Test Delay
    sendSignal();
}

/*
   Populate RGB memory with values read in from SD card 
*/
void setColorRGB(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if(index < NUM_LED) {
        //Point to current set of 3 bytes
        uint8_t *p = &RGBs[index * 3]; 
        
        *p++ = green;  
        *p++ = red;
        *p   = blue;
    }
    else
        Serial.println("We've gone out of bounds for some reason\n");
}

//Write to file, but why would we ever need to do that?
void SD_write(File SDFile, char* msg) {
    SDFile.print(msg);
    SDFile.println();
}

void SD_read(File SDFile) {
    char c;
    char LED_value[3]; // Up to 3 digit character LED value from SD card
    uint8_t j = 0;     // LED index
    uint8_t lv = 0;    // LED_Value index
    uint8_t val[3];    // Integer LED value (0-255)
    uint8_t v = 0;     // Integer LED value index
    while(j < NUM_LED) {
        c = SDFile.read();
        // Preprocessor outputs a space character as the value separator
        // 32 = ASCII value for ' '
        if(c != 32) {
            LED_value[lv++] = c;
            continue; 
        }
        
        lv = 0;
        val[v++] = atoi(LED_value);
        memset(LED_value, 0, 3);
        
        //Wait until we have R, G, and B values loaded in
        if (v >= 3) {
         /*
           Serial.print(val[0]);
           Serial.print(", ");
           Serial.print(val[1]);
           Serial.print(", ");
           Serial.println(val[2]);
         */
           // Load RGB into memory for LED at index j
           setColorRGB(j, (val[0] * MAX_BRIGHTNESS)/255, (val[1] * MAX_BRIGHTNESS)/255, (val[2] * MAX_BRIGHTNESS)/255);
           memset(val, 0, v); 
           v = 0;
           j++; 
        }
   }
   //Serial.println("-------------------");
}

/*
*  Read RGB values from program mem, use AVR assembly
*  magic to send signal formatted so WS2812B LEDs can
*  read it.  
*/
void sendSignal(void) {

    if(!RGBs)
        return;

    //micros() - Returns the number of microSeconds passed since 
    //start of code execution.
    //Need to wait 50 microSeconds for the data to latch
    while((micros() - currentTime) < 50L);

    volatile uint8_t  
        *p    = RGBs,                 // Copy the start address of our data array
        val  = *p++,                  // Get the current byte value & point to next byte
        high = PORT |  _BV(PORT_PIN), // Bitmask for sending HIGH to pin 
        low  = PORT & ~_BV(PORT_PIN), // Bitmask for sending LOW to pin
        tmp  = low,                   // Swap variable to adjust duty cycle 
        nbits= NUM_BITS;              // Bit counter for inner loop

    volatile uint16_t nbytes = NUM_BYTES; // Byte counter for outer loop
       /*   The Arduino Nano/Uno/Mega run at 16MHz, which means each clock 
            cycle takes roughly 0.0625 uS. In order to send a 1 value, pulse is held
            HIGH for 0.8 uS then LOW for 0.45 uS for a total period of 1.25 uS which
            equates to 20 CLK cycles. 

            For a 0 value the signal is held HIGH for 0.4 uS and LOW for 0.85 uS
        
            Having the constraint of hitting exactly 20 clock cycles per transferred bit
            is made considerably easier by the following two AVR asm commands:

            nop  - Idle for one clock cycle
            rjmp - Idle for two clock cycles (Used by passing .+0 flag)
        */
        //cli() - Disable Interrupts, needed for precise timing
        //Once this happens should I 
        //TODO: Test how badly delaying interrupts messes with Hall-Effect 
        //This may break things
        cli();
        
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
          //The Timing for this loop falls within a timing window which the
          //WS2812B LEDs do not recognize, so there is no false data transmitted
         "nextbyte:\n\t"                                   
          "ldi  %5, 8\n\t"      // 1    reset bitcount              
          "ld   %4, %a8+\n\t"   // 3    val = *p++                  
          "cbi   %0, %1\n\t"    // 5    signal LOW, will be held at the start of every loop if we're done loading in bytes                  
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
        //sei() - Re-Enable interrupts
        sei();
        currentTime = micros();
}
