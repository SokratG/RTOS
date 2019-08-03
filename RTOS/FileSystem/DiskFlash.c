// DiskFlash.c
// this module is represent a implementation mid-level of
// the solid state disk device driver. Contain a definiton of
// abstraction over low level, hardware-specific
// flash memory interface.
// Author: Gasparyan Sokrat
// 01.08.2019




#include <stdint.h>
#include <stdio.h>
#include "DiskFlash.h"
#include "FlashProgram.h"

#define NULL 0
// can change this address for specific board
const int32_t ADDR_MIN = 0x00020000;  // Flash  minimum address
const int32_t ADDR_MAX = 0x0003FFFF;  // Flash  maximum address
const int32_t SECTOR_SIZE = 512; 			// size size of a bytes


CODE Disk_Init(uint32_t drive)
{
	// For some configurations the physical drive must initialization
  // however for the internal flash
	if (drive == 0)
		return SUCCESSFUL;
	
	return RW_ERROR;
}



CODE Disk_ReadSector(uint8_t *buff, uint8_t sector)
{
	int16_t idx = 0;
	uint32_t address = ADDR_MIN + SECTOR_SIZE*sector; // calc start ROM address of given sector
	
	if (address > ADDR_MAX)
		return INVALID_PARAM;
	
	uint8_t* ptAdr = (uint8_t*)address;	//take address sector
	while(idx < SECTOR_SIZE){
		*buff = *ptAdr; //read date
		++ptAdr; ++buff; ++idx;
	}
	return SUCCESSFUL;
}



CODE  Disk_WriteSector(const uint8_t *buff, uint8_t sector)
{
	uint32_t address = ADDR_MIN + SECTOR_SIZE*sector; // calc start ROM address of given sector
	
	if (address > ADDR_MAX)
		return INVALID_PARAM;

	uint8_t* ptAdr = (uint8_t*)address; //take address sector
	//write date
	for(int16_t idx = 0; idx < SECTOR_SIZE; ++idx){
		if (ptAdr > (uint8_t*)(ADDR_MAX))
			return INVALID_PARAM;
		Flash_Write((uint32_t)ptAdr, *(uint32_t*)buff);
		++ptAdr;
		++buff;
	}
	return SUCCESSFUL;
}
	
	
	
CODE Disk_Format(void)
{
	uint32_t address = ADDR_MIN;
	while(address < ADDR_MAX){
		if (Flash_Erase(address) == ERROR)
			return INVALID_PARAM;
		address += 2*SECTOR_SIZE; // mult sector by 2, cause erase date block 1 KB
	}
	return SUCCESSFUL;
}
	