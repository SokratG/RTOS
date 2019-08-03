// DiskFlash.h
// this header is represent a mid-level of
// the solid state disk device driver. Contain a definiton of
// abstraction over low level, hardware-specific
// flash memory interface.
// Author: Gasparyan Sokrat
// 01.08.2019



typedef enum STATUS{
	SUCCESSFUL = 0,				// successful
	RW_ERROR = 1,					// Read/Write error
	WRITE_PROTECT = 2,		// Write protected
	NOT_READY = 3,				// Not ready
	INVALID_PARAM = 4			// Invalid parameter
}CODE;



// Name: Disk_Init 
// Initialize the interface between microcontroller and disk
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  SUCCESSFUL        0: Successful
//  RW_ERROR     			1: Drive not initialized
CODE Disk_Init(uint32_t drive);



// Name: Disk_ReadSector 
// Read 1 sector of 512 bytes from the disk, data goes to RAM
// Inputs: pointer to an empty RAM buffer
//         sector number of disk to read: 0,1,2,...255(max)
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
CODE Disk_ReadSector(uint8_t *buff, uint8_t sector);
         
      
// Name: Disk_WriteSector 
// Write 1 sector of 512 bytes of data to the disk, data comes from RAM
// Inputs: pointer to RAM buffer with information
//         sector number of disk to write: 0,1,2,...,255
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
CODE  Disk_WriteSector(const uint8_t *buff, uint8_t sector);
    
          

// Name: Disk_Format 
// Erase all files and all data by resetting the flash to all 1's
// Inputs: none
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
CODE Disk_Format(void);

