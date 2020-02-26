#include "fraaRTOS.h"

/* GLOBAL VARIABLES USED BY THE OS */

//Easy data structure - array - to keep track of the active threads
OS_Thread_Type 	                OS_ActiveThreads[NTHREAD_LIMIT]		; //How to init this? 
OS_ThreadIdx_Type   	OS_ThreadIdx 					= 0	;
OS_ThreadIdx_Type   	OS_ThreadIdx_Next				= 0	;
OS_ThreadIdx_Type   	OS_ThreadCnt 					= 0	;
int		  volatile		OS_FirstEntry					= 1 ;
unsigned int volatile   OS_gTime						= 0 ; 	  //Keep track of ms



//Declare the NTHREADS stacks (static allocation of memory) -- How to automate this? maybe using static when 

void OS_ThreadInit(OS_ThreadHandler  		threadHandler,
	               OS_StackPtr_Type  		threadStack, 
	               int               		threadStack_size
	              )
{
	//Init stack pointer to the last element because it grows backwards
	OS_StackPtr_Type sp = &(threadStack[0]) + threadStack_size;
	OS_StackPtr_Type sp_limit = &(threadStack[0]) - 1;


	//Init a stack frame
	*(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (unsigned int)threadHandler; /* PC */
    *(--sp) = 0x0000000EU; /* LR  */
    *(--sp) = 0x0000000CU; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    /* additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    //Set stack in the thread struct
    OS_ActiveThreads[OS_ThreadIdx]._sp = sp;
    OS_ThreadIdx++;
    OS_ThreadCnt++;

}

void OS_Start()
{
	//The threadIdx is sitting at the next thread which doesn't exist, let's reset it
	OS_ThreadIdx = 0;
	//Suggested by arm in case some external lib is changing the priority groups
	NVIC_SetPriorityGrouping(0U);
	//Pending must be the least to run, so lowest prio
	__NVIC_SetPriority(PendSV_IRQn,15);
	//Set this to switch from main contex just the first time
	OS_FirstEntry = 1;
}

void OS_Sched()
{
	//Do the scheduling algorithm here 
	//Update current thread, which is what was next before. Then choose what goes next.
	OS_ThreadIdx=OS_ThreadIdx_Next;
	OS_ThreadIdx_Next = (OS_ThreadIdx + 1) % OS_ThreadCnt;


    //__NVIC_SetPendingIRQ(PendSV_IRQn); must do it manually if irq number is negative
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler(void)
{
	OS_gTime++;
	OS_Sched();
}

void PendSV_Handler(void)
{
	//Critical code, disable other ISR
	__disable_irq();


	//Tricky part is the first we enter the interrupt, coming from the main
	//Here the registers are not to be saved in any stack 
	if (OS_FirstEntry == 0)
	{
		/*
			Save registers which are not saved by the interrupt HW
			Save the stack pointer in the current thread pointer
			OS_ActiveThreads[OS_ThreadIdx]->_sp = sp
			Shift is needed because we have to add the wordsize(four)
		*/
		asm(
			"push {r4-r11}                          \n\t"
			//pointer arithmetic, we move by "sizeof ThreadType at a time"
			//with this mul we can increase the size of thread type without breaking this asm 
			"mul r0,%1,%2        \n\t"
			//add the offset to the OS_Active thread vector base ptr
			"add r1,%0,r0               \n\t"
			//_sp is the first member of the struct, so r1 + 0 is where we want it 
			"str  sp,[r1,#0]             \n\t"
			: 
			: "r" (OS_ActiveThreads), "r" (OS_ThreadIdx), "r" (sizeof(OS_Thread_Type)) :);
	}

	OS_FirstEntry = 0;


	//Adjust the stack pointer to the new thread, then pop r4-r11
	    asm(
			//ptr arithmetic, see above
			"mul r0,%1,%2        \n\t"
			"add r1,%0,r0        \n\t"
			//Load sp from next thread 
			"ldr  sp,[r1,#0]             \n\t"
			//Pop thread registers into the new stack
			"pop {r4-r11}                           \n\t"
			: 
			: "r" (OS_ActiveThreads), "r" (OS_ThreadIdx_Next), "r" (sizeof(OS_Thread_Type)) :);

	//Enable interrupts again
	__enable_irq();

}

