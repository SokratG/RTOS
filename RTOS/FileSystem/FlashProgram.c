// FlashProgram.c
// Runs on LM4F120/TM4C123
// This module provide implementation functions that initialize the flash memory,
// write 32-bit data to flash, write an array of 32-bit data to flash,
// and erase a 1 KB block.
// 31.07.2019
/* take this module file from book:
	   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2016
*/


#include <stdint.h>
#include "FlashProgram.h"


#define FLASH_FMA_R             (*((volatile uint32_t *)0x400FD000))
#define FLASH_FMA_OFFSET_MAX    0x0003FFFF  // Address Offset max
#define FLASH_FMD_R             (*((volatile uint32_t *)0x400FD004))
#define FLASH_FMC_R             (*((volatile uint32_t *)0x400FD008))
#define FLASH_FMC_WRKEY         0xA4420000  // FLASH write key (KEY bit of FLASH_BOOTCFG_R set)
#define FLASH_FMC_WRKEY2        0x71D50000  // FLASH write key (KEY bit of FLASH_BOOTCFG_R cleared)
#define FLASH_FMC_MERASE        0x00000004  // Mass Erase Flash Memory
#define FLASH_FMC_ERASE         0x00000002  // Erase a Page of Flash Memory
#define FLASH_FMC_WRITE         0x00000001  // Write a Word into Flash Memory
#define FLASH_FMC2_R            (*((volatile uint32_t *)0x400FD020))
#define FLASH_FMC2_WRBUF        0x00000001  // Buffered Flash Memory Write
#define FLASH_FWBN_R            (*((volatile uint32_t *)0x400FD100))
#define FLASH_BOOTCFG_R         (*((volatile uint32_t *)0x400FE1D0))
#define FLASH_BOOTCFG_KEY       0x00000010  // KEY Select

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// Check if address offset is valid for write operation
// Writing addresses must be 4-byte aligned and within range
static int WriteAddrValid(uint32_t addr){
  // check if address offset works for writing
  // must be 4-byte aligned
  return (((addr % 4) == 0) && (addr <= FLASH_FMA_OFFSET_MAX));
}
// Check if address offset is valid for mass writing operation
// Mass writing addresses must be 32-word (128-byte) aligned and within range
static int MassWriteAddrValid(uint32_t addr){
  // check if address offset works for mass writing
  // must be 32-word (128-byte) aligned
  return (((addr % 128) == 0) && (addr <= FLASH_FMA_OFFSET_MAX));
}
// Check if address offset is valid for erase operation
// Erasing addresses must be 1 KB aligned and within range
static int EraseAddrValid(uint32_t addr){
  // check if address offset works for erasing
  // must be 1 KB aligned
  return (((addr % 1024) == 0) && (addr <= FLASH_FMA_OFFSET_MAX));
}

//------------Flash_Init------------
void Flash_Init(uint8_t systemClockFreqMHz){
  // do nothing; flash and EEPROM memory configured in PLL_Init()
  // if the processor is executing code out of flash memory,
  // presumably everything is configured correctly
}

//------------Flash_Write------------
int Flash_Write(uint32_t addr, uint32_t data){
  uint32_t flashkey;
  if(WriteAddrValid(addr)){
    DisableInterrupts();                            // may be optional step
                                                    // wait for hardware idle
    while(FLASH_FMC_R&(FLASH_FMC_WRITE|FLASH_FMC_ERASE|FLASH_FMC_MERASE)){
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };
    FLASH_FMD_R = data;
    FLASH_FMA_R = addr;
    if(FLASH_BOOTCFG_R&FLASH_BOOTCFG_KEY){          // by default, the key is 0xA442
      flashkey = FLASH_FMC_WRKEY;
    } else{                                         // otherwise, the key is 0x71D5
      flashkey = FLASH_FMC_WRKEY2;
    }
    FLASH_FMC_R = (flashkey|FLASH_FMC_WRITE);       // start writing
    while(FLASH_FMC_R&FLASH_FMC_WRITE){
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };           // wait for completion (~3 to 4 usec)
    EnableInterrupts();
    return NOERROR;
  }
  return ERROR;
}

//------------Flash_WriteArray------------
int Flash_WriteArray(uint32_t *source, uint32_t addr, uint16_t count){
  uint16_t successfulWrites = 0;
  while((successfulWrites < count) && (Flash_Write(addr + 4*successfulWrites, source[successfulWrites]) == NOERROR)){
    successfulWrites = successfulWrites + 1;
  }
  return successfulWrites;
}

//------------Flash_FastWrite------------
int Flash_FastWrite(uint32_t *source, uint32_t addr, uint16_t count){
  uint32_t flashkey;
  uint32_t volatile *FLASH_FWBn_R = (uint32_t volatile*)0x400FD100;
  int writes = 0;
  if(MassWriteAddrValid(addr)){
    DisableInterrupts();                            // may be optional step
    while(FLASH_FMC2_R&FLASH_FMC2_WRBUF){           // wait for hardware idle
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };
    while((writes < 32) && (writes < count)){
      FLASH_FWBn_R[writes] = source[writes];
      writes = writes + 1;
    }
    FLASH_FMA_R = addr;
    if(FLASH_BOOTCFG_R&FLASH_BOOTCFG_KEY){          // by default, the key is 0xA442
      flashkey = FLASH_FMC_WRKEY;
    } else{                                         // otherwise, the key is 0x71D5
      flashkey = FLASH_FMC_WRKEY2;
    }
    FLASH_FMC2_R = (flashkey|FLASH_FMC2_WRBUF);     // start writing
    while(FLASH_FMC2_R&FLASH_FMC2_WRBUF){
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };           // wait for completion (~3 to 4 usec)
    EnableInterrupts();
  }
  return writes;
}

//------------Flash_Erase------------
int Flash_Erase(uint32_t addr){
  uint32_t flashkey;
  if(EraseAddrValid(addr)){
    DisableInterrupts();                            // may be optional step
                                                    // wait for hardware idle
    while(FLASH_FMC_R&(FLASH_FMC_WRITE|FLASH_FMC_ERASE|FLASH_FMC_MERASE)){
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };
    FLASH_FMA_R = addr;
    if(FLASH_BOOTCFG_R&FLASH_BOOTCFG_KEY){          // by default, the key is 0xA442
      flashkey = FLASH_FMC_WRKEY;
    } else{                                         // otherwise, the key is 0x71D5
      flashkey = FLASH_FMC_WRKEY2;
    }
    FLASH_FMC_R = (flashkey|FLASH_FMC_ERASE);       // start erasing 1 KB block
    while(FLASH_FMC_R&FLASH_FMC_ERASE){
                 // to do later: return ERROR if this takes too long
                 // remember to re-enable interrupts
    };           // wait for completion (~3 to 4 usec)
    EnableInterrupts();
    return NOERROR;
  }
  return ERROR;
}
