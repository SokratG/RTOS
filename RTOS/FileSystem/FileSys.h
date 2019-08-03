// FileSys.h
// This header is a definitor function file system 
// for high-level abstraction on of the based on FAT.
// Author: Gasparyan Sokrat
// 02.08.2019

#define MAX_BUFF 512
#define FILE_SYS_SIZE 256


// Name: File_New
// Inputs: none
// Outputs: number of a new file
//          or 255 on failure or disk full
uint8_t File_New(void);



// Name: File_Size
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
uint8_t File_Size(uint8_t num);



// Name: File_Append
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// 				  255 on failure or disk full
uint8_t File_Append(uint8_t num, uint8_t buf[MAX_BUFF]);


// Name: File_Read
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
//          255 on failure because no data
uint8_t File_Read(uint8_t num, uint8_t location, uint8_t buf[MAX_BUFF]);
                     


// Name: File_Flush
// Update working buffers onto the disk.
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// 				  255 on disk write failure
uint8_t File_Flush(void);



// Name: File_Format
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// 				  255 on disk write failure
uint8_t File_Format(void);






