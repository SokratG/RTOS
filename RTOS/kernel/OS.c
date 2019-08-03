// OS.c
// 
// This moduele is a implementation kernel of task manager
// for real-time operating system
// which include - switch thread between themselves.
// The following attributes affect thread switching:
// priority thread, blocking, sleep, semaphore, mutex, suspend thread,
// FIFO data stuctured for save data, periodic task switcher.
// DisableInterrupts(), EnableInterrupts(), StartCritical(), EndCritical() - it's
// platform dependent function!
// Author: Gasparyan Sokrat
// 14.07.2019

#include <stdint.h>
#include <stdbool.h>
#include "os.h"
#include "CortexM.h"
#define NULL 0
#define MAXPRIO 255
 
// function in osasm.s
void __start_os__(void);
//#define DEBUG 1
#define NUMTHREADS  8        // maximum number of threads
#define NUMPERIODIC 3        // maximum number of periodic threads
#define STACKSIZE   100      // number of 32-bit words in stack per thread
#define MAXFREQ     16777216 // maximum number of frequency
#define TIMECOUNT 	1000     // 1 msec
#define NUMBEREXEC  -10
typedef struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running
  struct tcb *next;  // linked-list pointer
	int32_t* blocked;  // nonzero if blocked on this semaphore
	int32_t sleep;     // nonzero if this thread is sleeping
	uint8_t priority;  // 0 is highest, 254 lowest
} TypeTcb;
/*
typedef struct ptcb
{
	void(*Timer_Init)(void(*task)(void), uint32_t period, uint8_t priority); //pointer to timer init
	void(*Timer_Handler)(void);  //pointer to timer handler(called task on given period)
	void(*p_task)(void); //pointer to periodic task
	int32_t Time;
} TypePTcb;
*/

typedef struct {
	 void (*UserPeriodicTask)(void); //Pointer to user function
	 uint32_t period;
		int32_t* PeriodSemaphore;
} PeriodTask;


TypeTcb tcbs[NUMTHREADS];
PeriodTask PerTcbs[NUMPERIODIC];
TypeTcb *Pt_Task;   //current thread

static uint32_t CountScheduleTask;
static int32_t MaxPeriodTask;
int32_t Stacks_Thread[NUMTHREADS][STACKSIZE];
static void  RunPeriodicEvent(void);
static void  RunPeriodicCount(void);
void (*Timer_Init_Count)(void(*task)(void), uint32_t period, uint32_t priority);
void (*Timer_Init_Event)(void(*task)(void), uint32_t period, uint32_t priority);
static int32_t max(int32_t , int32_t );
#define FSIZE 15    // can be any size
uint32_t PutIdx;      // index of where to put next
uint32_t GetIdx;      // index of where to get next
int32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data



//---------------------------------------
//---------------------------------------
// Name: SchedulerTask
// check all task in tasks table and according to
// condition execute task
// Inputs:  none
// Outputs: none
// Errors:  none
// runs every ms
void SchedulerTask()  // every time slice
{      
// look at all threads in TCB list choose
// highest priority thread not blocked and not sleeping 
// If there are multiple highest priority (not blocked, not sleeping) run these round robin
	uint32_t max = MAXPRIO;
	TypeTcb* pt;
	TypeTcb* bestPt;
	pt = Pt_Task; // search for highest thread not blocked or sleeping
	do{
		pt = pt->next;
		if ((pt->priority < max) && (pt->blocked==NULL) && (pt->sleep==NULL)){
			max = pt->priority;
			bestPt=pt;
		}
	}while(Pt_Task != pt); // look at all possible threads
	Pt_Task = bestPt;
}

//---------------------------------------
// ========= RunPeriodicCount ==========
static void RunPeriodicCount()
{
	// decrement sleep counters
	int32_t status = StartCritical();
  for (int16_t i = 0; i < NUMTHREADS; ++i){
		if (tcbs[i].sleep){
			tcbs[i].sleep--;
		}
	}
	EndCritical(status);
}

//---------------------------------------
// ========= __os_init__ ==========
void __os_init__(void(*Timer_Init_Task)(void(*)(void), uint32_t, uint32_t), void(*Timer_Init_Task2)(void(*)(void), uint32_t, uint32_t))
	{
  DisableInterrupts(); 
	// set up periodic timer to run RunPeriodicEvent to implement sleeping
  
	CountScheduleTask = NULL;
	Timer_Init_Count = Timer_Init_Task;		
	Timer_Init_Event = Timer_Init_Task2;
	for (int8_t i = 0; i < NUMPERIODIC; ++i){
		PerTcbs[i].period = NULL;
		PerTcbs[i].UserPeriodicTask = NULL;
		PerTcbs[i].PeriodSemaphore = NULL;
	}
	MaxPeriodTask = NULL;
}

//---------------------------------------
// Name: SetInitialStack
// initial stackpointer in task table
// Inputs:  index table
// Outputs: none
// Errors:  none
void SetInitialStack(int i)
	{
	#ifdef DEBUG //for debug - must fill field
		tcbs[i].sp = &Stacks_Thread[i][STACKSIZE-16]; //thread stack pointer
		Stacks_Thread[i][STACKSIZE-1] = 0x01000000; //thumb bit
		Stacks_Thread[i][STACKSIZE-2] = 0x14141414;   // R14
		Stacks_Thread[i][STACKSIZE-3] = 0x12121212;   // R12
		Stacks_Thread[i][STACKSIZE-4] = 0x03030303;   // R3
		Stacks_Thread[i][STACKSIZE-5] = 0x02020202;   // R2
		Stacks_Thread[i][STACKSIZE-7] = 0x01010101;   // R1
		Stacks_Thread[i][STACKSIZE-8] = 0x00000000;   // R0
		Stacks_Thread[i][STACKSIZE-9] = 0x11111111;   // R11
		Stacks_Thread[i][STACKSIZE-10] = 0x10101010;  // R10
		Stacks_Thread[i][STACKSIZE-11] = 0x09090909;  // R9
		Stacks_Thread[i][STACKSIZE-12] = 0x08080808;  // R8
		Stacks_Thread[i][STACKSIZE-13] = 0x07070707;  // R7
		Stacks_Thread[i][STACKSIZE-14] = 0x06060606;  // R6
		Stacks_Thread[i][STACKSIZE-15] = 0x05050505;  // R5
		Stacks_Thread[i][STACKSIZE-16] = 0x04040404;  // R4
	#else
		tcbs[i].sp = &Stacks_Thread[i][STACKSIZE-16]; //thread stack pointer
		Stacks_Thread[i][STACKSIZE-1] = 0x01000000; //thumb bit
	#endif
}

//---------------------------------------
// ========= __os_addthreads__ ==========
CODE __os_addthreads__(void(*thread)(void), uint32_t priority)
{
	int32_t status = StartCritical();
	if ((CountScheduleTask+1) > NUMTHREADS){
		return FAIL;
	}
	else{
		tcbs[CountScheduleTask].blocked = NULL;
		tcbs[CountScheduleTask].sleep = NULL;
		tcbs[CountScheduleTask].priority = priority;
		SetInitialStack(CountScheduleTask);
		Stacks_Thread[CountScheduleTask][STACKSIZE-2] = (int32_t)(thread);
		if((CountScheduleTask+1) == NUMTHREADS){
			tcbs[CountScheduleTask].next = &tcbs[NULL];
		}
		else{
			tcbs[CountScheduleTask].next = &tcbs[CountScheduleTask+1];
		}
	}
	if (CountScheduleTask == NULL) //start schedule pointer
		Pt_Task = &tcbs[NULL];
	
	CountScheduleTask++;
	EndCritical(status);
	
	return SUCCESSFUL;
}

//---------------------------------------
// ========= __os_launch__ ==========
CODE __os_launch__(uint32_t TimeSlice, int32_t TimerPeriod, int32_t PriorityTimerCount, int32_t PriorityTimerEvent)
{
	if (TimeSlice >= MAXFREQ)
		return ERROR;
	STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = TimeSlice - 1; 		// reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
	Timer_Init_Count(RunPeriodicCount, TIMECOUNT, PriorityTimerCount);  //call every 1msec
	Timer_Init_Event(RunPeriodicEvent, TimerPeriod, PriorityTimerEvent);
	
  __start_os__();              // start on the first task
	
	return SUCCESSFUL;
}

//---------------------------------------
// ========= __os_suspend__ ==========
void __os_suspend__()
{
	STCURRENT = 0;    
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

//---------------------------------------
// ========= __os_sleep__ ==========
void __os_sleep__(uint32_t sleepTime)
	{
// suspend, stops running
	Pt_Task->sleep = sleepTime;
	__os_suspend__();
}


//---------------------------------------
// ========= __os_init_semaphore__ ==========
void __os_init_semaphore__(int32_t *pt_Semaphore, int32_t value)
	{
	DisableInterrupts();
	*pt_Semaphore = value;       //pointer to semaphore init value
	EnableInterrupts();
}

//---------------------------------------
// ========= __os_waittask__ ==========
void __os_waittask__(int32_t* pt_Semaphore)
{
	DisableInterrupts();
	(*pt_Semaphore) = (*pt_Semaphore)-1;
	if ((*pt_Semaphore) < NULL){
		Pt_Task->blocked = pt_Semaphore; //reason it blocked
		EnableInterrupts();
		__os_suspend__();
	}
	EnableInterrupts();  
}

//---------------------------------------
// ========= __os_signal__ ==========
void __os_signal__(int32_t *pt_Semaphore){
	TypeTcb* pt;
	DisableInterrupts();
	(*pt_Semaphore) = (*pt_Semaphore)+1;
	if ((*pt_Semaphore) <= NULL){
		pt = Pt_Task->next; //search for one blocked on this
		while(pt->blocked != pt_Semaphore){
			pt = pt->next;
		}
		pt->blocked = NULL;
	}
	EnableInterrupts();
}

//---------------------------------------
// ========= __os_fifo_init__ ==========
//Section with FIFO handler:
void __os_fifo_init__(){
  	PutIdx = GetIdx = NULL;
		__os_init_semaphore__(&CurrentSize, NULL);
		LostData = NULL;
}

//---------------------------------------
// ========= __os_fifo_put__ ==========
CODE __os_fifo_put__(int32_t data){
	if (CurrentSize == FSIZE){
		LostData++;
		return FAIL; //full
	}
	else{
		Fifo[PutIdx] = data;
		PutIdx = (PutIdx+1)%FSIZE;
		__os_signal__(&CurrentSize);
	}
  return SUCCESSFUL; // success
}

//---------------------------------------
// ========= __os_fifo_get__ ==========
int32_t __os_fifo_get__(){
	int32_t data;
	__os_waittask__(&CurrentSize);
	data = Fifo[GetIdx];
	GetIdx = (GetIdx+1)%FSIZE;
  return data;
}

//---------------------------------------
// Name: RunPeriodicEvent
// check all task in table and 
// execute thread if them time is coming
// Inputs:  none
// Outputs: none
// Errors:  none
static int32_t TimeEvent = NULL;
static void RunPeriodicEvent()
{
	TimeEvent = (TimeEvent+1)%MaxPeriodTask;

	for (int8_t i = 0; i < NUMPERIODIC; ++i){
			if (PerTcbs[i].period%TimeEvent == NULL && PerTcbs[i].UserPeriodicTask != NULL)
					PerTcbs[i].UserPeriodicTask();
	}	
}

//---------------------------------------
// ========= __os_add_periodic_threads__ ==========
static int8_t ptNextEvent = NULL;
CODE __os_add_periodic_threads__(void(*task)(void), uint32_t period)
{
	if (ptNextEvent+1 >= NUMPERIODIC) //table is full
		return FAIL;
	else{
		MaxPeriodTask = max(MaxPeriodTask, period);
		PerTcbs[ptNextEvent].UserPeriodicTask = task;
		PerTcbs[ptNextEvent].period = period;
	}
	++ptNextEvent;
	return SUCCESSFUL;
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

