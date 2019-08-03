// MemoryManager.h
// this header is kernel memory manager
// operate memory in systems. Include some stantard
// function for contol memory
// 29.07.2019
#ifndef MEM_H
#define MEM_H

#define NULL 0
#define STAT_OK 0
#define FAIL_OUT_RANGE 1
#define FAIL_BLOCK_HEAP 2
// Definition if heap size. Which can resizable 
#define HEAP_SIZE 4096
#define HEAP_NUM_WORDS (HEAP_SIZE / sizeof(uint32_t))
	
/*
typedef enum CODE{
	SUCCESSFUL=0,
	ERROR_CORRUPTED=1,
	ERROR_OUT_OF_RANGE=2,
	FAIL_ALLOC = -1
} STATUS;
*/

// struct heap_state store current state the heap
typedef struct heap_state{
	int32_t AllocWord; 	//how machine words allocated in heap
	int32_t  TopWord;  //save top machine word in heap
	int32_t AvailableWord; // current free word space in heap
	int32_t blockUsed;   //block memory wich used in program
	int32_t blocksUnused; //block memory wich free in program
}	heap_state;

// Name: Mem_Init
// Initialization heap for use in program.
// when calling again will clean state where memory is free 
// Input: None
// output: status is OK wich equal 0
// TODO: add init value of size memory block for heap
int32_t Mem_Init(void);



// Name: Mem_Alloc
// Find free space in heap and allocate memory then
// input: NumBytes -  number of bytes for allocate
// output: pointer void* to allocate space
// or return STATUS FAIL_ALLOC if not enough heap free space
void* Mem_Alloc(uint32_t NumBytes);



// Name: Mem_Realloc
// reallocate memory block to a given new size
// old memory block will free alloc
// input: m_block - pointer to old memory block  
// and NumBytes - number of bytes for new size block
// output: pointer void* to allocate space
// or return STATUS FAIL_ALLOC if not enough heap free space
void* Mem_Realloc(void* m_block, uint32_t NumBytes);



// Name: Mem_Free
// Free allocated given memory block and return them in heap
// input: mem_block - pointer to mem block for unallocated
// output: number status of state free block
// STAT_OK - 0 if unallocated memory block is successful
// FAIL_OUT_RANGE - 1 if given pointer out range the heap
// FAIL_BLOCK_HEAP - 2 if given pointer it's pointer to unallocated block in heap
int32_t  Mem_Free(void* mem_block);



// Name: State_Heap
// return struct with state the heap
// input: None
// output: struct the heap_state
heap_state State_Heap(void);



#endif
