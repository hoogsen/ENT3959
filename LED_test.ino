#define NUM_RGB       (7)         // Number of WS281X we have connected
#define NUM_BYTES     (NUM_RGB*3) // Number of LEDs (3 per each WS281X)
#define DIGITAL_PIN   (5)         // Digital port number
#define PORT          (PORTD)     // Digital pin's port
#define PORT_PIN      (PORTD5)    // Digital pin's bit position
#define R             (128)        // Intensity of Red LED
#define G             (128)        // Intensity of Green LED
#define B             (128)        // Intensity of Blue LED

#define NUM_BITS      (8)         // Constant value: bits per byte

uint8_t* rgb_arr = NULL;
uint32_t t_f;

void setup() 
{
  pinMode(DIGITAL_PIN,OUTPUT);
  digitalWrite(DIGITAL_PIN,0);
  if((rgb_arr = (uint8_t *)malloc(NUM_BYTES)))             
  {                 
    memset(rgb_arr, 0, NUM_BYTES);                         
  }        
  render();
}

void loop() 
{
  int i;
  for(i=0;i<NUM_RGB;i++)
    setColorRGB(i,R,G,B);
  render();
  delay(1000);
}

void setColorRGB(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) 
{
  if(idx < NUM_RGB) 
  {
    uint8_t *p = &rgb_arr[idx*3]; 
    *p++ = g;  
    *p++ = r;
    *p = b;
  }
}

void render(void) 
{
  if(!rgb_arr) return;

  while((micros() - t_f) < 50L);  // wait for 50us (data latch)

  cli(); // Disable interrupts so that timing is as precise as possible
  volatile uint8_t  
   *p    = rgb_arr,
    val  = *p++,
    high = PORT |  _BV(PORT_PIN),
    low  = PORT & ~_BV(PORT_PIN),
    tmp  = low,
    nbits= NUM_BITS;
  volatile uint16_t
    nbytes = NUM_BYTES;
  asm volatile(
      // Instruction        CLK   
   "nextbit:\n\t"         // -    
    "sbi  %0, %1\n\t"     // 2    
    "sbrc %4, 7\n\t"      // 1-2  
     "mov  %6, %3\n\t"    // 0-1  
    "dec  %5\n\t"         // 1    
    "nop\n\t"             // 1    
    "st   %a2, %6\n\t"    // 2    
    "mov  %6, %7\n\t"     // 1    
    "breq nextbyte\n\t"   // 1-2  
    "rol  %4\n\t"         // 1    
    "rjmp .+0\n\t"        // 2    
    "cbi   %0, %1\n\t"    // 2    
    "rjmp .+0\n\t"        // 2    
    "nop\n\t"             // 1    
    "rjmp nextbit\n\t"    // 2    
   "nextbyte:\n\t"        // -    
    "ldi  %5, 8\n\t"      // 1    
    "ld   %4, %a8+\n\t"   // 2    
    "cbi   %0, %1\n\t"    // 2    
    "rjmp .+0\n\t"        // 2    
    "nop\n\t"             // 1    
    "dec %9\n\t"          // 1    
    "brne nextbit\n\t"    // 2    
    ::
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
  sei();                          // Enable interrupts
  t_f = micros();                 // t_f will be used to measure the 50us 
}
