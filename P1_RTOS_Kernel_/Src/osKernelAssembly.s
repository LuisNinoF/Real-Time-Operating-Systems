/*
 * osKernelAssembly.s - Assembly support for round-robin RTOS kernel
 *
 * Includes:
 *   - PendSV_Handler: performs context switch
 *   - osSchedulerLaunch: starts the first thread
 */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

    .section .text
    .align 4

    .extern currentPt           // defined in osKernel.c
    .global PendSV_Handler
    .global osSchedulerLaunch
    .global osScheduler


/*
 * ---> PendSV_Handler
 * Called every quanta.
 * Saves current thread context (r4–r11), updates currentPt to next thread, and restores that thread’s context.
 * The CPU automatically saves r0–r3, r12, LR, PC, xPSR. No need to save them explicitly.
*/
    .type PendSV_Handler, %function
PendSV_Handler:
    CPSID   I                 	 // Disable interrupts
    PUSH    {R4-R11}          	 // Save remaining registers onto current stack

    LDR     R0, =currentPt    	 // R0 = &currentPt
    LDR     R1, [R0]          	 // R1 = currentPt (points to current tcb)
    STR     SP, [R1]          	 // Save current SP into currentPt->stackPt

    PUSH	{R0, LR}
    BL		osScheduler			 // calls a C function (osScheduler) that decides which task to run next. So now the scheduler is not “hardwired” — it’s dynamic.
    POP		{R0, LR}

    LDR		R1, [R0]			 // Reload currentPt (updated by scheduler)
    LDR     SP, [R1]           	 // SP = currentPt->stackPt (load next thread’s stack)

    POP     {R4-R11}             // Restore R4–R11 from new thread stack
    CPSIE   I                 	 // Re-enable interrupts
    BX      LR                	 // Return from exception
    .size PendSV_Handler, .-PendSV_Handler


/*
 * ---> osSchedulerLaunch
 * Starts the first thread.
 * Loads its SP from currentPt->stackPt and restores registers.
 * Then executes BX LR to jump to the thread entry point.
*/
    .type osSchedulerLaunch, %function
osSchedulerLaunch:
    LDR     R0, =currentPt     // R0 = &currentPt
    LDR     R2, [R0]           // R2 = currentPt (points to tcb)
    LDR     SP, [R2]           // SP = currentPt->stackPt (load thread stack)

    POP     {R4-R11}           // Restore R4–R11
    POP     {R12}              // Restore R12
    POP     {R0-R3}            // Restore R0–R3
    ADD     SP, SP, #4         // Skip
    POP     {LR}               // Pop PC into LR (will BX LR to jump to task)
    ADD     SP, SP, #4         // Skip
    CPSIE   I                  // Enable interrupts
    BX      LR                 // Jump to thread entry (task0/task1/task2)
    .size osSchedulerLaunch, .-osSchedulerLaunch

    .end
