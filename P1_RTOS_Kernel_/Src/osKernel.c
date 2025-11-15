/* Main idea:
 * For threads, two-step yield: Thread → SysTick → PendSV.
 *
 */

#include "osKernel.h"

#define NUM_OF_THREADS		3					// each thread is going to be a tcb (see struct tcb)
#define STACKSIZE			400					// 100 X 32 bit values = 100 x 4 bytes = 400 bytes
#define BUS_FREQ			16000000

#define CTRL_ENABLE			(1U<<0)
#define CTRL_TICKINT		(1U<<1)
#define CTRL_CLKSRC 		(1U<<2)
#define CTRL_COUNTFLAG 		(1U<<16)

#define INTCTRL				(*(volatile uint32_t *)0xE000ED04)

#define	PERIOD				100

uint32_t MILLIS_PRESCALER;
extern void osSchedulerLaunch(void);


struct tcb{										// create a thread control block (tcb)
	int32_t *stackPt;
	struct tcb *nextPt;
};

typedef struct tcb tcbType;						// short alias for struct tcb type

tcbType	tcbs[NUM_OF_THREADS];					// define the thread control block array

tcbType	*currentPt;								// define current thread control block

int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];	// define stack for threads


/*
 * x3 functions:
 * 0) initialize the kernel stack (auxiliary function)
 * 1) initialize the kernel
 * 2) add threads (x3)
 * 3) launch the kernel
 *
 */


/*
 * 0) initialize the kernel stack: os stack init function
 * whenever an interrupt occurs the contents of the key cpu registers must be saved to the stack for context switching
 *
 * These registers represent the stack frame:
 * xPSR		Program status register
 * R15(PC)	Program counter
 * R14(LR)	Link register
 * R12
 * R3
 * R2
 * R1
 * R0
 */

void osKernelStackInit (int i)
{
	tcbs[i].stackPt = &TCB_STACK[i][STACKSIZE-16];	// Stack Pointer

	TCB_STACK[i][STACKSIZE-1] = (1U<<24);		// PSR: Program Status Register. Set PSR to 1 to operate in thumb mode


	// TCB_STACK[i][STACKSIZE-2] =  0xAAAAAAAA;	// r15(PC): Program Counter -> Will be initialized in a different function

	TCB_STACK[i][STACKSIZE-3] =  0xAAAAAAAA;	// r14(LR)
	TCB_STACK[i][STACKSIZE-4] =  0xAAAAAAAA;	// r12
	TCB_STACK[i][STACKSIZE-5] =  0xAAAAAAAA;	// r3
	TCB_STACK[i][STACKSIZE-6] =  0xAAAAAAAA;	// r2
	TCB_STACK[i][STACKSIZE-7] =  0xAAAAAAAA;	// r1
	TCB_STACK[i][STACKSIZE-8] =  0xAAAAAAAA;	// r0

	TCB_STACK[i][STACKSIZE-9] =  0xAAAAAAAA;	// r11
	TCB_STACK[i][STACKSIZE-10] = 0xAAAAAAAA;	// r10
	TCB_STACK[i][STACKSIZE-11] = 0xAAAAAAAA;	// r9
	TCB_STACK[i][STACKSIZE-12] = 0xAAAAAAAA;	// r8
	TCB_STACK[i][STACKSIZE-13] = 0xAAAAAAAA;	// r7
	TCB_STACK[i][STACKSIZE-14] = 0xAAAAAAAA;	// r6
	TCB_STACK[i][STACKSIZE-15] = 0xAAAAAAAA;	// r5
	TCB_STACK[i][STACKSIZE-16] = 0xAAAAAAAA;	// r4
}

/*
 * 1) initialize the kernel
 * Reduce the from from seconds to milliseconds - short and simple
 *
 */

void osKernelInit(void)
{
	MILLIS_PRESCALER = (BUS_FREQ/1000);
}

/*
 * 2) add threads: os kernel add threads function
 * Return a flag
 * Pass address of the thread functions (x3 functions in our case)
 */

uint8_t osKernelAddThreads(void (*task0)(void), void (*task1)(void), void (*task2)(void))
{
	// Disable global interrupts
	__disable_irq();

	// define the order or execution
	tcbs[0].nextPt = &tcbs[1];	//after task 0 finishes its execution time (quanta), task 1 goes to running state
	tcbs[1].nextPt = &tcbs[2];
	tcbs[2].nextPt = &tcbs[0];

	// initialize the stack for each thread and its PC (Program counter), which wasn't initialized in osKernelStackInit because we didn't have the PC there
	osKernelStackInit(0);
	TCB_STACK[0][STACKSIZE-2] = (int32_t)task0;

	osKernelStackInit(1);
	TCB_STACK[1][STACKSIZE-2] = (int32_t)task1;

	osKernelStackInit(2);
	TCB_STACK[2][STACKSIZE-2] = (int32_t)task2;

	// Start from task 0
	currentPt= &tcbs[0];

	// Enable global interrupt again
	__enable_irq();

	return 1;
}

/*
 * 3) launch the kernel
 *
 */

void osKernelLaunch(uint32_t quanta)
{
	// Reset systick
	SysTick->CTRL = 0;

	// Clear systick by writing current value register
	SysTick->VAL = 0;

	// Load quanta
	SysTick->LOAD = (quanta*MILLIS_PRESCALER)-1;

	// Set systick priority to low priority (so that interrupts can have higher priorities and execute)
	NVIC_SetPriority(SysTick_IRQn, 7);
	NVIC_SetPriority(PendSV_IRQn, 15);

	// Enable systick, select internal clock
	SysTick->CTRL = CTRL_CLKSRC | CTRL_ENABLE;

	// Enable systick interrupt
	SysTick->CTRL |= CTRL_TICKINT;

	// Launch scheduler (see osKernelAssembly.s declaration)
	osSchedulerLaunch();

}

// PendSV is an interrupt mode used by most RTOS to force a context switch if no other interrupt is active
// Main advantage: it releases SysTick, to avoid missed ticks

void SysTick_Handler(void)
{
	INTCTRL = 0x10000000;	// Trigger PendSV // PENDSVSET pend SV set
}

// Inside the PendSV_Handler, the osScheduler is called
// function to implement periodic scheduler WITH tcbs (thread control blocks)
void osScheduler(void)
{
	currentPt = currentPt->nextPt;		// Scheduler logic: go to next task in linked list
}

void osThreadYield(void)
{
	SysTick->VAL = 0;
	INTCTRL = 0x04000000; 				// Trigger Systick, i.e. PENDSTSET pend ST set
}
