// CortexM.c
// Cortex M registers and basic functions used in these labs
// take this module file from book:
//   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
//      ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2016
// September 18, 2016


#include <stdint.h>
#include "CortexM.h"

// delay function
// which delays 3.3*ulCount cycles
// ulCount=23746 => 1ms = 23746*3.3cycle/loop/80,000
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void delay(unsigned long ulCount){
  __asm (  "pdloop:  subs    r0, #1\n"
      "    bne    pdloop\n");
}

#else
  //Keil uVision Code
  __asm void
  delay(unsigned long ulCount)
  {
    subs    r0, #1
    bne     delay
    bx      lr
  }

#endif
  
// ------------Clock_Delay1ms------------
// Simple delay function which delays about n milliseconds.
// Inputs: n, number of msec to wait
// Outputs: none
void Clock_Delay1ms(uint32_t n){
  while(n){
    delay(23746);  // 1 msec, tuned at 80 MHz, originally part of LCD module
    n--;
  }
}
