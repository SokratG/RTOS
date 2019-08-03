// FileSys.c
// This module is a implementation function file system 
// for high-level abstraction on of the based on FAT.
// Author: Gasparyan Sokrat
// 02.08.2019

#include <stdint.h>
#include "DiskFlash.h"
#include "FileSys.h"


#define NULL 0
#define RAM_ACTIVE 1
#define NULLPTR 255
#define ERROR_FILE 255


typedef uint8_t byte;

uint32_t DirectoryLoaded = NULL;	// switch disk between ROM(complete) and RAM version
byte Directory[FILE_SYS_SIZE];	//Directory file in file system
byte FAT[FILE_SYS_SIZE];				// Table file allocation
byte Buff[MAX_BUFF];					// Buffer for temporary used operation with file




//========================================================================
// Definition of the helper function for control file system		
static int32_t max(int32_t val1, int32_t val2);
void MountDirectory(void);
byte lastsector(byte start);
byte findfreesector(void);
byte appendfat(byte numfile, byte idx);
//========================================================================

byte File_New()
{
	if(DirectoryLoaded == NULL)
		MountDirectory();
	byte num = 0;
	for(; Directory[num] != NULLPTR; ++num){
		if (num == NULLPTR)
			return ERROR_FILE;
	}
	return num;
}
	
	
	
byte File_Size(byte num)
{
	byte size = 0;
	uint16_t nextSector = Directory[num];
	
	if (nextSector == NULLPTR)
		return NULL;		//empty
	
	while(FAT[nextSector] != NULLPTR){
		++size;
		nextSector = FAT[nextSector];
	}
	size += 1;
	
	return size;
}

	
byte File_Append(byte num, byte buf[MAX_BUFF])
{
	if (DirectoryLoaded == NULL)
		MountDirectory();
	
	byte free = findfreesector();
	if (free == NULLPTR)
		return ERROR_FILE;
	
	if (Disk_WriteSector(buf, free) == RW_ERROR)
		return ERROR_FILE;
	
	appendfat(num, free);
	
	return NULL; //successful
}

	
	
byte File_Read(byte num, byte location, byte buf[MAX_BUFF])
{
	byte start = Directory[num];
	if (start == NULLPTR)
		return ERROR_FILE;
	
	uint16_t idx = 0;
	
	while(idx < location){
		start = FAT[start];
		if (start == NULLPTR)
			return ERROR_FILE;
		++idx;
	}
	
	return Disk_ReadSector(buf, start);
}	
	
	
	
byte File_Flush()
{
	if (DirectoryLoaded == RAM_ACTIVE){
		for(int16_t i = 0; i < NULLPTR + 1; ++i){
			Buff[i] = Directory[i];
			Buff[i+NULLPTR+1] = FAT[i];
		}
		if (Disk_WriteSector(Buff, NULLPTR) == INVALID_PARAM)
			return ERROR_FILE;	
	}
	
	return NULL; //successful

}
	
	
	
byte File_Format()
{
	if (Disk_Format() == RW_ERROR)
		return ERROR_FILE;
	for(int16_t i = 0; i < NULLPTR + 1; ++i){
			Directory[i] = NULLPTR;
			FAT[i] = NULLPTR;
	}
	DirectoryLoaded = NULL;
	
	return NULL; //successful
}	
	
	
// Name: MountDirectory
// mount directory means if directory and 
// file allocation table(FAT) not loaded RAM,
// load them in RAM from disk
// Inputs:  none
// Outputs: none
void MountDirectory()
{
	if (DirectoryLoaded == RAM_ACTIVE)
		return;
	
	if (DirectoryLoaded == NULL){
		if (Disk_ReadSector(Buff, NULLPTR) == INVALID_PARAM)
			return;
		for (int16_t i = 0; i < NULLPTR+1; ++i){
			Directory[i] = Buff[i];
			FAT[i] = Buff[i+NULLPTR+1];
		}
		DirectoryLoaded = RAM_ACTIVE;
	}
	else
		return;
	
}
	

// Name: lastsector
// return index the last sector in file
// associated with a given start point to sector
// Inputs:  uint8_t start sector
// Outputs: uint8_t last sector
byte lastsector(byte start)
{
	if (start == NULLPTR)
		return NULLPTR;
	byte s= FAT[start];
	byte ls = start;
	while(s != NULLPTR){
		ls = s;
		s = FAT[ls];
	}
	return ls;
}


// Name: findfreesector
// check all table in FAT and 
// return index the first free sector
// Inputs:  none
// Outputs: uint8_t free sector
byte findfreesector()
{
	int16_t free = -1;
	uint16_t start = 0;
	byte ls = lastsector(Directory[start]);
	
	while( ls != NULLPTR){
		free = max(free, ls);
		++start;
		ls = lastsector(Directory[start]);
	}
	free += 1;  //next sector is free
	return free;
}	
	
// Name: appendfat
// append a sector with index "idx" to end given file "numfile"
// Inputs:  uint8_t Number of file
//  				uint8_t idx - index sector
// Outputs: uint8_t status of the append (0 - successful)
byte appendfat(byte numfile, byte idx)
{
	byte n = Directory[numfile];
	if (n == NULLPTR){
		Directory[numfile] = idx;
		return NULL; // successful
	}
	byte start = FAT[n];
	
	while (start != NULLPTR){
		n = start;
		start = FAT[n];
	}
	FAT[n] = idx;
	
	return NULL;	// successful
}	
	


// Name: Max
// find maximum beetwen two value
// Inputs:  two value
// Outputs: maximum data
// Errors:  none
static int32_t max(int32_t val1, int32_t val2)
{
	return val1>val2 ? val1 : val2;
}	

