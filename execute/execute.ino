#define NUM_LED       (15)         // Number of LEDs
#define NUM_BYTES     (NUM_LED*3)  // Number of total bytes (3 per each LED)
#define DIGITAL_PIN   (2)          // Digital port number
#define PORT          (PORTD)      // Digital pin's port
#define PORT_PIN      (PORTD2)     // Digital pin's bit position

#define NUM_BITS      (8)          // Bits per byte

uint8_t* currentRGB = NULL; 
uint32_t currentTime;
uint8_t* RGBs = NULL;

void setup() {
    pinMode(DIGITAL_PIN,OUTPUT);
    digitalWrite(DIGITAL_PIN,0);
    RGBs = (uint8_t *)malloc(NUM_BYTES);
    memset(RGBs, 0, NUM_BYTES);
    //Send out a (0, 0, 0) signal first
    sendSignal();
}

void loop() { 
    uint8_t i;
    // Load RGB vals into program memory from preprocessor
    for(i=0;i<NUM_LED;i++)
        //TODO: Grab color here from preprocessor struct, all white for now
        setColorRGB(i, 30, 10, 0);
    
    sendSignal();
    //TODO: figure out delay, I believe this depends on fan speed?
    delay(999);
}

void setColorRGB(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
    
    if(index < NUM_LED) {
        //Point to current set of 3 bytes
        uint8_t *p = &RGBs[index * 3]; 

        *p++ = green;  
        *p++ = red;
        *p   = blue;
    }
}

void sendSignal(void) {

    if(!RGBs)
        return;

    //micros() - Returns the number of microSeconds passed since 
    //start of code execution.
    //Need to wait 50 microSeconds for the data to latch
    while((micros() - currentTime) < 50L);

    //cli() - Disable Interrupts, needed for precise timing
    cli(); 

    volatile uint8_t  
        *p    = RGBs,   // Copy the start address of our data array
        val  = *p++,      // Get the current byte value & point to next byte
        high = PORT |  _BV(PORT_PIN), // Bitmask for sending HIGH to pin
        low  = PORT & ~_BV(PORT_PIN), // Bitmask for sending LOW to pin
        tmp  = low,       // Swap variable to adjust duty cycle 
        nbits= NUM_BITS;  // Bit counter for inner loop

    volatile uint16_t nbytes = NUM_BYTES; // Byte counter for outer loop

       /*   The Arduino Nano/Uno/Mega run at 16MHz, which means each clock 
            cycle takes roughly 0.0625 uS. In order to send a 1 value, pulse is held
            HIGH for 0.8 uS then LOW for 0.45 uS for a total period of 1.25 uS which
            equates to 20 CLK cycles. 

            For a 0 value the signal is held HIGH for 0.4 uS and LOW for 0.85 uS
        
            Having the constraint of hitting exactly 20 clock cycles per transferred bit
            is made considerably easier by the following two AVR asm commands:

            nop  - Idle for one clock cycle
            rjmp - Idle for two clock cycles (Used by passing +0 flag)
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
          //The Timing for this loop falls within a timing window which the
          //WS2812B LEDs do not recognize, so there is no false data transmitted
         "nextbyte:\n\t"                                   
          "ldi  %5, 8\n\t"      // 1    reset bitcount              
          "ld   %4, %a8+\n\t"   // 3    val = *p++                  
          "cbi   %0, %1\n\t"    // 5    signal LOW                  
          "rjmp .+0\n\t"        // 7                         
          "nop\n\t"             // 8                             
          "dec %9\n\t"          // 9    decrease bytecount          
          "brne nextbit\n\t"    // 10   if bytecount !=0 -> nextbit 
          ::
          // Input operands         Operand Id (w/ constraint)
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
        //Question - Should I? We're not sending interrupts, I guess it re-enables watchdog/other timer sys-interrupts?
        sei();                         
        currentTime = micros();         
}
