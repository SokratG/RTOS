// MemoryManager.c
// this module is a implementation kernel memory manager
// operate memory in systems. Include all stantard
// function for contol memory.
// 29.07.2019
#include <stdint.h>
#include <stdbool.h>
#include "MemoryManager.h"


#define START_POINTER_HEAP (Heap_Block)
#define END_POINTER_HEAP (START_POINTER_HEAP + HEAP_NUM_WORDS)



// the Heap_Block it's array in system for control memory with standart function
static int32_t Heap_Block[HEAP_NUM_WORDS];

//========================================================================
// Definition of the helper function for control heap																			 
static int32_t MarkBlockUsed(int32_t* memBlock);
static int32_t MarkBlockUnused(int32_t* memBlock);
static bool BlockUsed(int32_t* Block);
static bool BlockUnused(int32_t* Block);
static bool BlockRoom(int32_t* Block);
static int32_t* BlockHeader(int32_t* BlockEnd);
static int32_t* BlockRest(int32_t* BlockStart);
static int32_t* NewBlockHeader(int32_t* BlockStart);
static int32_t* PrevBlockHeader(int32_t* BlockStart);
static bool InHeapRange(int32_t* addr);
static int32_t SplitMarkBlock(int32_t* upBlockStart, int32_t numRoom);
static void MergeBlock(int32_t* upBlockStart);
static void SetToZero(int32_t* memBlock, int32_t numBytes);
//=========================================================================

//---------------------------------------
// ========= Mem_Init ==========
int32_t Mem_Init(void)
{
	int32_t* blockStart = START_POINTER_HEAP;
	int32_t* blockEnd = END_POINTER_HEAP - 1;
	
	*blockStart = -(int32_t)(HEAP_NUM_WORDS - 2);
	*blockEnd = -(int32_t)(HEAP_NUM_WORDS-2);
	return STAT_OK;
}


//---------------------------------------
// ========= Mem_Alloc ==========
void* Mem_Alloc(uint32_t NumBytes)
{
	int32_t NumWords = (NumBytes + sizeof(int32_t) - 1)/sizeof(int32_t);
	int32_t* blockStart = START_POINTER_HEAP;
	if (NumWords <= NULL)
		return NULL;
	
	while(InHeapRange(blockStart)){
		//find first suitable  block
		if (BlockUnused(blockStart) && NumWords <= BlockRoom(blockStart)){
			if (SplitMarkBlock(blockStart, NumWords))
				return NULL;
			return (blockStart + 1);
		}
		blockStart = NewBlockHeader(blockStart);
	}
	return NULL;
}




//---------------------------------------
// ========= Mem_Realloc ==========
void* Mem_Realloc(void* m_block, uint32_t NumBytes)
{
	int32_t* oldBlock;
	int32_t* NewBlock;
	int32_t* oldBlockStart;
	int32_t oldRoom, newRoom, CopyBlock;
	
	oldBlock = (int32_t*) m_block;
	
	oldBlockStart = oldBlock - 1;
	if (!InHeapRange(oldBlockStart) || BlockUnused(oldBlockStart))
		return NULL;
	
	NewBlock = Mem_Alloc(NumBytes);
	
	if (NewBlock == NULL)
		return NULL;
	
	oldRoom = BlockRoom(oldBlockStart);
	newRoom = BlockRoom(NewBlock - 1);
	
	CopyBlock = (oldRoom < newRoom) ? oldRoom : newRoom;
	
	for (int32_t i = 0; i < CopyBlock; ++i){
		NewBlock[i] = oldBlock[i];
	}
	if (Mem_Free(oldBlock))
		return NULL;
	return NewBlock;
}

//---------------------------------------
// ========= Mem_Realloc ==========
int32_t  Mem_Free(void* mem_block)
{
	int32_t* blockStart;
	int32_t* blockEnd;
	int32_t* nextBlock;
	
	blockStart = ((int32_t*)mem_block)-1;
	
	if (InHeapRange(blockStart))
		return FAIL_OUT_RANGE;
	
	if (BlockUnused(blockStart))
		return FAIL_BLOCK_HEAP;
	
	blockEnd = BlockRest(blockStart);
	if (!InHeapRange(blockEnd) || BlockUnused(blockEnd))
		return FAIL_BLOCK_HEAP;
	
	if(MarkBlockUnused(blockStart))
		return FAIL_BLOCK_HEAP;
	
	if (blockStart > START_POINTER_HEAP){
		int32_t* prevBlock = PrevBlockHeader(blockStart);
		if (BlockUnused(prevBlock)){
			MergeBlock(prevBlock);
			blockStart = prevBlock;
		}
	}
	
	nextBlock = NewBlockHeader(blockStart);
	if (InHeapRange(nextBlock) && BlockUnused(nextBlock))
		MergeBlock(blockStart);
	
	return STAT_OK;	
}


//---------------------------------------
// ========= State_Heap ==========
heap_state State_Heap(void)
{
	int32_t* blockStart = START_POINTER_HEAP;
	heap_state state;	
	
	state.AllocWord = NULL;
	state.blockUsed = NULL;
	state.blocksUnused = NULL;
	state.AvailableWord = NULL;
	
	while(InHeapRange(blockStart)){
		if (BlockUsed(blockStart)){
			state.AllocWord += BlockRoom(blockStart);
			state.blockUsed++;
		}
		else{
			state.AvailableWord += BlockRoom(blockStart);
			state.blocksUnused++;
		}
		blockStart = NewBlockHeader(blockStart);
	}
	state.TopWord = HEAP_NUM_WORDS - state.AllocWord - state.AvailableWord;
	return state;
}





// Name: BlockUsed
// input: pointer to a block
// output: marked number of given block
static bool BlockUsed(int32_t* Block)
{
	return *Block > NULL;
}

// Name: BlockUnused
// input: pointer to a block
// output: marked number of given block
static bool BlockUnused(int32_t* Block)
{
	return *Block < NULL;
}

// Name: BlockRoom
// input: pointer to a block
// output: number of words in block
static bool BlockRoom(int32_t* Block)
{
	if (*Block > NULL)
		return *Block;
	return -*Block;
}

// Name: InHeapRange
// input: pointer 
// output: is there a pointer inside the heap
static bool InHeapRange(int32_t* addr)
{
	return (addr >= START_POINTER_HEAP) && (addr < END_POINTER_HEAP);
}



// Name: BlockHeader
// input: pointer to a block
// output: pointer to the head of this block
static int32_t* BlockHeader(int32_t* BlockEnd)
{
	return BlockEnd - (BlockRoom(BlockEnd) - 1);
}

// Name: BlockHeader
// input: pointer to a head block
// output: pointer to the trailer of this block
static int32_t* BlockRest(int32_t* BlockStart)
{
	return BlockStart + (BlockRoom(BlockStart) + 1);
}



// Name: NewBlockHeader
// input: pointer to a head block
// output: pointer to the next block after header
static int32_t* NewBlockHeader(int32_t* BlockStart)
{
	return BlockRest(BlockStart)+1;
}


// Name: PrevBlockHeader
// input: pointer to a head block
// output: pointer to the previous block a header
static int32_t* PrevBlockHeader(int32_t* BlockStart)
{
	return BlockHeader(BlockStart-1);
}


// Name: MarkBlockUsed
// input: pointer to a memory block
// output: status of given pointer:
// 1) STAT_OK - is memory block can mark when used 
// 2) FAIL_BLOCK_HEAP - if memory can't mark when used
static int32_t MarkBlockUsed(int32_t* memBlock)
{
	int32_t* blockEnd = BlockRest(memBlock);
	if (*memBlock != *blockEnd || BlockUsed(blockEnd))
		return FAIL_BLOCK_HEAP;
	*memBlock = -*memBlock;
	*blockEnd = -*blockEnd;
	return STAT_OK;
}
	

// Name: MarkBlockUnused
// input: pointer to a memory block
// output: status of given pointer:
// 1) STAT_OK - is memory block can mark mark when unused
// 2) FAIL_BLOCK_HEAP - if memory can't mark when unused
static int32_t MarkBlockUnused(int32_t* memBlock)
{
	int32_t* blockEnd = BlockRest(memBlock);
	if (*memBlock != *blockEnd || BlockUnused(blockEnd))
		return FAIL_BLOCK_HEAP;
	*memBlock = -*memBlock;
	*blockEnd = -*blockEnd;
	return STAT_OK;
}

// Name: SplitMarkBlock
// Split given block that  new upper block hold numRoom(number of words)
// Marks the upper block as used, lower block is unused.
// input: 1) upBlockStart - pointer to a head memory block
// 2) numRoom - amount of words will be in new upper block
// output: status of given pointer:
// 1) STAT_OK - is memory block can splitted
// 2) FAIL_BLOCK_HEAP - if memory can't splitted
static int32_t SplitMarkBlock(int32_t* upBlockStart, int32_t numRoom)
{
	int32_t Room = BlockRoom(upBlockStart) - numRoom - 2;
	if (Room > NULL){
		int32_t* upperBlockEnd = upBlockStart + numRoom + 1;
		int32_t* lowerBlockStart = upperBlockEnd + 1;
		int32_t* lowerBlockEnd = BlockRest(upBlockStart);
		// mark used
		*upBlockStart = numRoom;
		*upperBlockEnd = numRoom;
		//	mark unused
		*lowerBlockStart = -Room;
		*lowerBlockEnd = -Room;
	}
	// can't split block
	else{
		//mark used
		if (MarkBlockUsed(upBlockStart))
			return FAIL_BLOCK_HEAP;
	}
	return STAT_OK;
}



// Name: MergeBlock
// Will be merged block with below of given block
// input: pointer to a header memory block
// output: none
static void MergeBlock(int32_t* upBlockStart)
{
	int32_t* upperBlockEnd = BlockRest(upBlockStart);
	int32_t* lowerBlockStart = upperBlockEnd + 1;
	int32_t* lowerBlockEnd = BlockRest(lowerBlockStart);
	
	int32_t Room = lowerBlockEnd - upBlockStart - 1;
	*upBlockStart = -Room;
	*lowerBlockEnd = -Room;
}

static void SetToZero(int32_t* memBlock, int32_t numBytes)
{
	if (memBlock == NULL || numBytes <= NULL)
		return;
	int32_t endClean = *(memBlock - 1);
	for (int32_t i = 0; i < endClean; ++i){
		memBlock[i] = NULL;
	}
}

