// Test.h
// Implementation test function definition.
// Test on board TMC4C123
// 03.08.2019

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "tm4c123gh6pm.h"
#include "Test.h"
#include "OS.h"
#include "MemoryManager.h"
//#include "FileSystem/DiskFlash.h"
#include "FileSys.h"

#define NULL 0
#define PRIO 0
#define PRIO1 1
#define PRIO2 2
#define PRIO3 3
#define PRIO4 4
#define PRIO5 5
#define PRIO6 6
#define PRIO7 7


//// ----------- Section Board Init ------------
#define PE0  (*((volatile uint32_t *)0x40024004))  /* PE0 */
#define PE0_BIT 0x01
#define PE1  (*((volatile uint32_t *)0x40024008))  /* PE1 */
#define PE1_BIT 0x02
#define PE2  (*((volatile uint32_t *)0x40024010))  /* PE2 */
#define PE2_BIT 0x04
#define PE3  (*((volatile uint32_t *)0x40024020))  /* PE3 */
#define PE3_BIT 0x08
#define PF1  (*((volatile uint32_t *)0x40025008))  /* PF1 */
#define PF1_BIT 0x02
#define PC5  (*((volatile uint32_t *)0x40006080))  /* PC5 */
#define PC5_BIT 0x20
#define PC7  (*((volatile uint32_t *)0x40006200))  /* PC7 */
#define PC7_BIT 0x80

void Board_Init(){
	
  SYSCTL_RCGCGPIO_R |= 0x00000004; // 1) activate clock for Port C
  while((SYSCTL_PRGPIO_R&0x04) == 0){};// allow time for clock to stabilize                                   
  GPIO_PORTC_AMSEL_R &= ~0xA0;     // 2) disable analog on PC7,5                               
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0x0F0FFFFF)+0x00000000;
  GPIO_PORTC_DIR_R |= 0xA0;        // 3) make PC7,5 output
  GPIO_PORTC_AFSEL_R &= ~0xA0;     // 4) disable alt funct on PC7,5
  GPIO_PORTC_PUR_R &= ~0xA0;       // disable pull-up on PC7,5
  GPIO_PORTC_PDR_R &= ~0xA0;       // disable pull-down on PC7,5
  GPIO_PORTC_DEN_R |= 0xA0;        // 5) enable digital I/O on PC7,5
  GPIO_PORTC_DATA_R &= ~0xA0;      // 6) initialize PC7,5 low
		
		
  SYSCTL_RCGCGPIO_R |= 0x00000010; // 1) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){};// allow time for clock to stabilize                               
  GPIO_PORTE_AMSEL_R &= ~0x0F;     // 2) disable analog on PE3-0                                  
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTE_DIR_R |= 0x0F;        // 3) make PE3-0 output
  GPIO_PORTE_AFSEL_R &= ~0x0F;     // 4) disable alt funct on PE3-0
  GPIO_PORTE_PUR_R &= ~0x0F;       // disable pull-up on PE3-0
  GPIO_PORTE_PDR_R &= ~0x0F;       // disable pull-down on PE3-0
  GPIO_PORTE_DEN_R |= 0x0F;        // 5) enable digital I/O on PE3-0
  GPIO_PORTE_DATA_R &= ~0x0F;      // 6) initialize PE3-0 low
		
		
  SYSCTL_RCGCGPIO_R |= 0x00000020; // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20) == 0){};// allow time for clock to stabilize                                
  GPIO_PORTF_AMSEL_R &= ~0x02;     // 2) disable analog on PF1                                
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFF0F)+0x00000000;
  GPIO_PORTF_DIR_R |= 0x02;        // 3) make PF1 output
  GPIO_PORTF_AFSEL_R &= ~0x02;     // 4) disable alt funct on PF1
  GPIO_PORTF_PUR_R &= ~0x02;       // disable pull-up on PF1
  GPIO_PORTF_PDR_R &= ~0x02;       // disable pull-down on PF1
  GPIO_PORTF_DEN_R |= 0x02;        // 5) enable digital I/O on PF1
  GPIO_PORTF_DATA_R &= ~0x02;      // 6) initialize PF1 low
}

void PE0_Toggle()
{PE0 ^= PE0_BIT;}

void PE1_Toggle()
{PE1 ^= PE1_BIT;}

void PE2_Toggle()
{PE2 ^= PE2_BIT;}

void PE3_Toggle()
{PE3 ^= PE3_BIT;}

void PF1_Toggle()
{PF1 ^= PF1_BIT;}

void PC5_Toggle()
{PC5 ^= PC5_BIT;}

void PC7_Toggle()
{PC7 ^= PC7_BIT;}

void (*PeriodicTask1)(void);   // user function
void Timer0_Init(void(*task)(void), uint32_t period, uint32_t priority){
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  PeriodicTask1 = task;          // user function
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|(priority << 5); // 8) priority 
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER0A timeout
  (*PeriodicTask1)();                // execute user task
}

void (*PeriodicTask2)(void);   // user function
void Timer1_Init(void(*task)(void), uint32_t period, uint32_t priority){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  PeriodicTask2 = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|(priority << 5); // 8) priority 
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
  (*PeriodicTask2)();                // execute user task
}
//// ----------- End Section Board Init ------------
//============================================================



// ----------- Section test manager scheduler ------------
int32_t semaphore34, semaphore67;
int32_t Counter7, Counter8;
int32_t Task1data=0;
void Some_task1(){ // producer as event thread every 10 ms
  int i; int num = (Task1data%3)+1;
  PE1_Toggle();
  for(i=0; i<num; i++){
    __os_fifo_put__(Task1data);  // Some_task2 can proceed
    Task1data++;
  }
}
int32_t Task2expected, Task2actual, Task2LostData;
void Some_task2(){ // consumer as main thread
  Task2expected = 0;
  Task2LostData = 0;
  while(1){
    Task2actual = __os_fifo_get__();  // signaled by Some_task1
    if(Task2actual!= Task2expected){
      Task2LostData++;
      Task2expected = Task2actual;
    }else{
      Task2expected++;
    }
    PE2_Toggle();
  }
}
int32_t Counter1=0;
void Some_task3(){ // event thread every 100 ms
  Counter1++;
  PE3_Toggle();
  __os_signal__(&semaphore34);
}
int32_t Counter2;
void Some_task4(){ // connected to Some_task4
  Counter2 = 0;
  while(1){
    __os_waittask__(&semaphore34);
    Counter2++;
    PF1_Toggle();
  }
}
int32_t Counter3;
void Some_task5(){ // sleeping 30 ms
  Counter3 = 0;
  while(1){
    Counter3++;
    PE0_Toggle();
    __os_sleep__(30);
  }
}
int32_t Counter4;
void Some_task6(){ // sleeping 40 ms
  Counter4 = 0;
  while(1){
    Counter4++;
		__os_waittask__(&semaphore67);
    PC5_Toggle();
		
    __os_sleep__(40);
  }
}
int32_t Counter5;
void Some_task7(){ // sleeping 50 ms
  Counter5 = 0;
  while(1){
    Counter5++;
    PC7_Toggle();
		__os_signal__(&semaphore67);
    __os_sleep__(50);
  }
}
int32_t Counter6;
void Some_task8(){ // dummy
  Counter6 = 0;
  while(1){
    Counter6++;
  }
}

void General_Test_Task(uint32_t freq)
{
	__os_init__(Timer0_Init, Timer1_Init);
	Board_Init();
	__os_fifo_init__();
	__os_init_semaphore__(&semaphore34, NULL);
	__os_init_semaphore__(&semaphore67, NULL);
	
	//__os_addthreads__(Some_task1, PRIO);
	__os_addthreads__(Some_task2, PRIO1);
	//__os_addthreads__(Some_task3, PRIO2);
	__os_addthreads__(Some_task4, PRIO3);
	__os_addthreads__(Some_task5, PRIO4);
	__os_addthreads__(Some_task6, PRIO5);
	__os_addthreads__(Some_task7, PRIO6);
	__os_addthreads__(Some_task8, PRIO7);
	
	__os_add_periodic_threads__(Some_task1, 10);
	__os_add_periodic_threads__(Some_task3, 100);
	
	__os_launch__((freq/1000), 32000, PRIO2, PRIO5);
		
}
//// ----------- End Section test manager scheduler ------------
//============================================================




//// ----------- Section Test Memory Manager ------------
void Test_Mem_Alloc()
{
	int32_t status;
	heap_state mstate = State_Heap();
	status = Mem_Init();
	assert(status == STAT_OK);
	
	int32_t* pInt = (int32_t*)Mem_Alloc(sizeof(int32_t));
	*pInt = 1234;
	status = (*pInt) - 1234;
	assert(status == NULL);
	
	
	pInt = Mem_Realloc(pInt, 3*sizeof(int32_t));
	
	pInt[1] = 4321;
	status = pInt[1] - 4321;
	assert(status == NULL);
	
	
	status = Mem_Free(pInt);
	assert(status == STAT_OK);	
	

}
//// ----------- End Section Test Memory Manager ------------
//============================================================






//// ----------- Section Test File System ------------
// but the access to internal data is used here for debugging
extern uint8_t Buff[512];
extern uint8_t Directory[256], FAT[256];
// Test function: Copy a NULL-terminated 'inString' into the
// 'Buff' global variable with a maximum of 512 characters.
// Uninitialized characters are set to 0xFF.
// Inputs:  inString  pointer to NULL-terminated character string
// Outputs: none
void TestFillBuffer(char *inString){
  uint32_t i = 0;
  while((i < 512) && (inString[i] != 0)){
    Buff[i] = inString[i];
    i = i + 1;
  }
  while(i < 512){
    Buff[i] = 0xFF;             // fill the remainder of the buffer with 0xFF
    i = i + 1;
  }
}

void Test_File_RW()
{
	int8_t status;
	uint8_t m, n, p;              // file numbers
	volatile int32_t i;
	
	status = Disk_Init(NULL);
	assert(status == NULL);
	
	status = File_Format();								// erase disk
	assert(status == NULL);
	
	n = File_New();            // n = 0, 3, 6, 9, ...
  TestFillBuffer("buf0");
  File_Append(n, Buff);      // 0x00020000
  TestFillBuffer("buf1");
  File_Append(n, Buff);      // 0x00020200
  TestFillBuffer("buf2");
  File_Append(n, Buff);      // 0x00020400
  TestFillBuffer("buf3");
  File_Append(n, Buff);      // 0x00020600
  TestFillBuffer("buf4");
  File_Append(n, Buff);      // 0x00020800
  TestFillBuffer("buf5");
  File_Append(n, Buff);      // 0x00020A00
  TestFillBuffer("buf6");
  File_Append(n, Buff);      // 0x00020C00
  TestFillBuffer("buf7");
  File_Append(n, Buff);      // 0x00020E00
  m = File_New();            // m = 1, 4, 7, 10, ...
  TestFillBuffer("dat0");
  File_Append(m, Buff);      // 0x00021000
  TestFillBuffer("dat1");
  File_Append(m, Buff);      // 0x00021200
  TestFillBuffer("dat2");
  File_Append(m, Buff);      // 0x00021400
  TestFillBuffer("dat3");
  File_Append(m, Buff);      // 0x00021600
  p = File_New();            // p = 2, 5, 8, 11, ...
  TestFillBuffer("arr0");
  File_Append(p, Buff);      // 0x00021800
  TestFillBuffer("arr1");
  File_Append(p, Buff);      // 0x00021A00
  TestFillBuffer("buf8");
  File_Append(n, Buff);      // 0x00021C00
  TestFillBuffer("buf9");
  File_Append(n, Buff);      // 0x00021E00
  TestFillBuffer("arr2");
  File_Append(p, Buff);      // 0x00022000
  TestFillBuffer("dat4");
  File_Append(m, Buff);      // 0x00022200
	
  i = File_Size(n);          // i = 10
	assert(i == 10);
  i = File_Size(m);          // i = 5
	assert(i == 5);
  i = File_Size(p);          // i = 3
	assert(i == 3);
  i = File_Size(p+1);        // i = 0
	assert(i == NULL);
		
	
  status = File_Flush();              // 0x0003FE00	
	assert(status == NULL);
}
//// ----------- End  Section Test File System ------------
//============================================================






















