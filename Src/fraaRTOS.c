#include "fraaRTOS.h"

/* GLOBAL VARIABLES USED BY THE OS */

//Easy data structure - array - to keep track of the active threads
OS_Thread_Type 	                OS_ActiveThreads[NTHREAD_LIMIT]		; //How to init this? 
OS_ThreadIdx_Type   	OS_ThreadIdx 					= 0	;
OS_ThreadIdx_Type   	OS_ThreadIdx_Next				= 0	;
OS_ThreadIdx_Type   	OS_ThreadCnt 					= 0	;
int		          volatile		OS_FirstEntry					= 1;
const int   OS_SizeOfThread_Type = sizeof(OS_Thread_Type);


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
	//OS_ThreadIdx=OS_ThreadIdx_Next;
	OS_ThreadIdx_Next = (OS_ThreadIdx + 1) % OS_ThreadCnt;


    //__NVIC_SetPendingIRQ(PendSV_IRQn); must do it manually if irq number is negative
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


void __attribute__((naked)) PendSV_Handler(void)
{


	__disable_irq();
	//Tricky part is the first we enter the interrupt, coming from the main
	//Here the registers are not to be saved in any stack 
		/*
			Save registers which are not saved by the interrupt HW
			Save the stack pointer in the current thread pointer
			OS_ActiveThreads[OS_ThreadIdx]->_sp = sp
			Shift is needed because we have to add the wordsize(four)
		*/
		asm volatile(
			//disable interrupts
			//"cpsid i 		\n\t"
			//load first entry
			".global OS_ActiveThreads\n\t"
			".global OS_ThreadIdx\n\t"
			".global OS_ThreadIdx_Next\n\t"
			".global OS_FirstEntry\n\t"
			".global OS_SizeOfThread_Type\n\t"

			"ldr r1,=OS_FirstEntry 		\n\t"
			//Load base address acrive threads
			"ldr r2,=OS_ActiveThreads		\n\t" //OS_ActiveThreads
			//If first entry is true, restore context 
			"ldr r1,[r1]\n\t" //r1 had the address of the variable
			"cbz r1, save_current_context		\n\t"
			"restore_next_context:		\n\t"
			"ldr r3,=OS_ThreadIdx_Next		\n\t" //r3 has the address of idx next
			"ldr r3,[r3]\n\t"//r3 has the next index
			"ldr r0,=OS_SizeOfThread_Type \n\t"  //r0 has the address of sizeof
			"ldr r0,[r0]\n\t"
			"mul r0,r3,r0	\n\t"
			"ldr sp,[r2,r0]		\n\t" //$sp = OS_ActiveThreads[next].sp -->  = $sp = *(OS_ActiveThreads+(OS_ThreadIdx * sizeof(OS_Thread_Type)))
			"pop {r4-r11}		\n\t" //restore reg for next thread
			//set first entry to 0
			"mov r0,#0		\n\t"
			"ldr r1,=OS_FirstEntry	\n\t" //r1 has the address of first entry
			"str r0,[r1]		\n\t"   //Set first entry to zero
			//current = next
			"ldr r1,=OS_ThreadIdx\n\t"  //r1 has address of current thread
			"str r3,[r1]		\n\t"
			"cpsie i		\n\t"
			"bx lr		\n\t"
			"save_current_context:		\n\t"
			"push {r4-r11}                          \n\t"
			//pointer arithmetic, we move by "sizeof ThreadType at a time\n\t"
			//with this mul we can increase the size of thread type without breaking this asm 
			//Now we can use whatever register we want
			"ldr r3,=OS_ThreadIdx\n\t" //@ of OS_ThreadIdx
			"ldr r3,[r3]\n\t"
			"ldr r0,=OS_SizeOfThread_Type\n\t"  //r0 has the address of sizeof
			"ldr r0,[r0]\n\t"
			"mul r0,r3,r0\n\t" //OS_ThreadIdx * sizeof(OS_Thread_Type)
			//This works because sp is the first member, so +0 from the base pointer
			"str sp,[r2,r0]\n\t" //OS_ActiveThreads[OS_ThreadIdx].sp = $sp --> *(OS_ActiveThreads+(OS_ThreadIdx * sizeof(OS_Thread_Type))) = $sp
			"b restore_next_context\n\t"
			: 
			: 
			: );

}



/*
 0:   b672            cpsid   i
 2:   4b10            ldr     r3, [pc, #64]   ; (44 <PendSV_Handler+0x44>)
 4:   681a            ldr     r2, [r3, #0]
 6:   b17a            cbz     r2, 28 <PendSV_Handler+0x28>
 8:   4a0f            ldr     r2, [pc, #60]   ; (48 <PendSV_Handler+0x48>)
 a:   4910            ldr     r1, [pc, #64]   ; (4c <PendSV_Handler+0x4c>)
 c:   2000            movs    r0, #0
 e:   6018            str     r0, [r3, #0]
10:   680b            ldr     r3, [r1, #0]
12:   2104            movs    r1, #4
14:   fb03 f001       mul.w   r0, r3, r1
18:   eb02 0100       add.w   r1, r2, r0
1c:   f8d1 d000       ldr.w   sp, [r1]
20:   e8bd 0ff0       ldmia.w sp!, {r4, r5, r6, r7, r8, r9, sl, fp}
24:   b662            cpsie   i
26:   4770            bx      lr
28:   4909            ldr     r1, [pc, #36]   ; (50 <PendSV_Handler+0x50>)
2a:   4a07            ldr     r2, [pc, #28]   ; (48 <PendSV_Handler+0x48>)
2c:   6809            ldr     r1, [r1, #0]
2e:   2004            movs    r0, #4
30:   e92d 0ff0       stmdb   sp!, {r4, r5, r6, r7, r8, r9, sl, fp}
34:   fb01 f000       mul.w   r0, r1, r0
38:   eb02 0100       add.w   r1, r2, r0
3c:   f8c1 d000       str.w   sp, [r1]
40:   e7e3            b.n     a <PendSV_Handler+0xa>
42:   bf00            nop
*/