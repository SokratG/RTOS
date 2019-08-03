#include <stdint.h>
#include "Test.h"

#define NULL 0


static uint32_t freq = 16000000;

int main(void)
{
	//=========== Test function ==============
	
	// Test task scheduler
	#ifdef TEST_TASK_MANAGER	
	General_Test_Task(freq/2);
	#endif
	
	
	// Test memory manager
	#ifdef TEST_MEMORY_MANAGER
	Test_Mem_Alloc();
	#endif
	
	
	// Test file system
	#ifdef TEST_FILE_SYSTEM
	Test_File_RW();
	#endif
	
	//========================================
	while(1)
	{;}
	return 0;
}












