#include "fraaRTOS.h"

extern int NO_OPT 				OS_T_StartSlice 			;
extern int NO_OPT 				OS_SliceDuration			;
extern unsigned int NO_OPT     OS_gTime						;

void SysTick_Handler(void)
{
	OS_gTime++; //WARN: this will overflow

	//OS_StartSlive starts at 0
	if( OS_gTime - OS_T_StartSlice >= OS_SliceDuration )
	{
		OS_T_StartSlice = OS_gTime;
		OS_Sched();
	}
	
}

void __attribute__((naked)) PendSV_Handler(void)
{


	__disable_irq();
	//Tricky part is the first we enter the interrupt, coming from the main
	//Here the registers are not to be saved in any stack 
		/*
			Save registers which are not saved by the interrupt HW
			Save the stack pointer in the current thread pointer
			OS_ActiveThreads[OS_ThreadIdx_Current]->_sp = sp
			Shift is needed because we have to add the wordsize(four)
		*/
		asm volatile(
			//disable interrupts
			//"cpsid i 		\n\t"
			//load first entry
			".global OS_ActiveThreads\n\t"
			".global OS_ThreadIdx_Current\n\t"
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
			"ldr sp,[r2,r0]		\n\t" //$sp = OS_ActiveThreads[next].sp -->  = $sp = *(OS_ActiveThreads+(OS_ThreadIdx_Current * sizeof(OS_Thread_Type)))
			"pop {r4-r11}		\n\t" //restore reg for next thread
			//set first entry to 0
			"mov r0,#0		\n\t"
			"ldr r1,=OS_FirstEntry	\n\t" //r1 has the address of first entry
			"str r0,[r1]		\n\t"   //Set first entry to zero
			//current = next
			"ldr r1,=OS_ThreadIdx_Current\n\t"  //r1 has address of current thread
			"str r3,[r1]		\n\t"
			"cpsie i		\n\t"
			"bx lr		\n\t"
			"save_current_context:		\n\t"
			"push {r4-r11}                          \n\t"
			//pointer arithmetic, we move by "sizeof ThreadType at a time\n\t"
			//with this mul we can increase the size of thread type without breaking this asm 
			//Now we can use whatever register we want
			"ldr r3,=OS_ThreadIdx_Current\n\t" //@ of OS_ThreadIdx_Current
			"ldr r3,[r3]\n\t"
			"ldr r0,=OS_SizeOfThread_Type\n\t"  //r0 has the address of sizeof
			"ldr r0,[r0]\n\t"
			"mul r0,r3,r0\n\t" //OS_ThreadIdx_Current * sizeof(OS_Thread_Type)
			//This works because sp is the first member, so +0 from the base pointer
			"str sp,[r2,r0]\n\t" //OS_ActiveThreads[OS_ThreadIdx_Current].sp = $sp --> *(OS_ActiveThreads+(OS_ThreadIdx_Current * sizeof(OS_Thread_Type))) = $sp
			"b restore_next_context\n\t"
			: 
			: 
			: );

}