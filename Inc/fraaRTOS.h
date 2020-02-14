#ifndef FRAARTOS_H
#define FRAARTOS_H

#include "stm32f407xx.h"
#include "core_cm4.h"

//Max number of threads 
#define NTHREAD_LIMIT 5

extern unsigned int thread1_stack[100];

//first we need a handler to a thread. Every thread is a function that returns void and takes no arguments
//This is going to be the first instruction of the 
typedef void (*OS_ThreadHandler)();
typedef unsigned int* 	OS_StackPtr_Type; /*Must be word aligned*/ 
//Then we need a type for a struct that keeps track of the stack pointer for each thread, to be expanded with other thread info
typedef struct {
	OS_StackPtr_Type 	_sp;
	/** more info later **/
} OS_Thread_Type;

//Types to keep track active threads
typedef OS_Thread_Type* OS_ThreadPtr_Type;
typedef int 			volatile OS_ThreadIdx_Type;

//We want to initialize the stack of our thread, so we pass the thread struct, the handler (to init PC) and the stack
//The stack must be allocated by the user in this early implementation
void OS_ThreadInit(OS_ThreadHandler  		threadHandler,
	               OS_StackPtr_Type  		threadStack, 
	               int               		threadStack_size  
	              );

//Must be called after all thread init functions
void OS_Start();

//Critical code, if need to switch threads this function changes the registers 
void OS_Sched();

void PendSV_Handler(void);


#endif //FRAARTOS_H