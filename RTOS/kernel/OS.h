// OS.h
// This header is a kernel of task manager
// definition for real-time operating system
// which include - switch thread between themselves.
// The following attributes affect thread switching:
// priority thread, blocking, sleep, semaphore, mutex, suspend thread,
// FIFO data stuctured for save data, periodic task switcher.
// Author: Gasparyan Sokrat
// 14.07.2019


#ifndef __OS_H
#define __OS_H  123

typedef enum CODE{
	ERROR=0,
	SUCCESSFUL=1,
	FAIL=-1
} CODE;

// Name: __os_init__ 
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt.
// Bus clock as fast as possible
// Inputs: two pointer to timer initialization 
// Outputs: none
void __os_init__(void(*)(void(*)(void), uint32_t, uint32_t), void(*)(void(*)(void), uint32_t, uint32_t));

// Name: __os_addthreads__ 
// Add main threads to the scheduler task
// Inputs: function pointers to eight void/void main threads
//         priorites for each main thread (0 highest)
// Outputs: SUCCESSFUL if successful, FAIL if this thread can not be added(full)
// This function will only be called once, after __os_init__ and before __os_launch__
CODE __os_addthreads__(void(*)(void), uint32_t);

// Name: __os_add_periodic_threads__ 
// Add periodic event threads
// Typically this function receives the highest priority
// Inputs: pointers to a void/void event thread function
//         periods in msec
// Outputs: SUCCESSFUL if successful, ERROR if this thread cannot be added
CODE __os_add_periodic_threads__(void(*)(void), uint32_t);


// Name: __os_launch__ 
// Start the scheduler
// Inputs: number of clock cycles for each time slice
// Outputs: none 
// Errors: TimeSlice must be less than 16,777,216
CODE __os_launch__(uint32_t TimeSlice, int32_t, int32_t, int32_t);

// Name: __os_sleep__ 
// sleep thread at the given time
// input:  number of msec to sleep
// output: none
// __os_sleep__(0) implements cooperative multitasking
void __os_sleep__(uint32_t);

// Name: __os_suspend__ 
// Called by main thread suspend task
// Inputs: none
// Outputs: none
void __os_suspend__(void);


// Name: __os_waittask__ 
// Decrement semaphore and block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void __os_waittask__(int32_t*);

// Name: __os_init_semaphore__ 
// Initialize semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void __os_init_semaphore__(int32_t*, int32_t);

// Name: __os_signal__ 
// Increment semaphore
// Inputs:  pointer to a counting semaphore
// Outputs: none
void __os_signal__(int32_t*);


// Name: __os_fifo_init__ 
// Initialize FIFO data structures for save data
// from thread, initialize two pointers on FIFO
// and semaphore.  
// Inputs:  none
// Outputs: SUCCESSFUL if successful, ERROR if this thread cannot init FIFO
void __os_fifo_init__(void);

// Name: __os_fifo_put__ 
// Put an entry in the FIFO.
// Inputs:  data to be stored
// Outputs: SUCCESSFUL if successful, FAIL if FIFO is full
CODE __os_fifo_put__(int32_t);

// Name: __os_fifo_get__ 
// Get an entry from the FIFO. 
// Inputs:  none
// Outputs: data retrieved
int32_t __os_fifo_get__(void);


#endif








