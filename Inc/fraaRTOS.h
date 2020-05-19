#ifndef FRAARTOS_H
#define FRAARTOS_H

#include "stm32f407xx.h"
#include "core_cm4.h"
//TODO: maybe use just core_m4.h functions and not those middlewares
#include "stm32f4xx_ll_cortex.h"

//Max number of threads 
#define NTHREAD_LIMIT 5
#define TOTAL_STACK 400
#define NO_OPT volatile

typedef enum { OS_STATE_RUN, OS_STATE_WAIT, OS_STATE_SLEEP} OS_State_Type;
typedef enum {OS_HIGH_P, OS_LOW_O} OS_Priority_Type;

//first we need a handler to a thread. Every thread is a function that returns void and takes no arguments
//This is going to be the first instruction of the 
typedef void (*OS_ThreadHandler)();
typedef unsigned int* 	OS_StackPtr_Type; /*Must be word aligned*/ 
//Then we need a type for a struct that keeps track of the stack pointer for each thread, to be expanded with other thread info
// typedef struct {
// 	OS_StackPtr_Type  NO_OPT	_sp;   //This must stay the first element in this struct for sched optimization
// 	OS_State_Type	 NO_OPT	_state;
// 	OS_Priority_Type NO_OPT _priority;
// 	int  NO_OPT			_time_to_wake; //Need to be negative because of comparison if time has elapsed
// 	int  NO_OPT		_time_at_wait;
// 	/** more info later **/
// } OS_Thread_Type;

// Attention! This class needs to be POD (plain old data) because the first element must be _sp for the asm code to work
// This can't have virtual functions because we don't want the virtual table 
struct OS_Thread_Type
{

public:
	OS_StackPtr_Type  	NO_OPT		_sp;   //This must stay the first element in this struct for sched optimization
	OS_State_Type	 	NO_OPT		_state;
	OS_Priority_Type 	NO_OPT 		_priority;
	int  				NO_OPT		_time_to_wake; //Need to be negative because of comparison if time has elapsed
	int  				NO_OPT		_time_at_wait;
	int 				NO_OPT		_threadID;
	unsigned int 		NO_OPT 		_thread_stack_size;
	OS_ThreadHandler  	NO_OPT		_threadHandler;
	/** more info later **/

	//Set some variables needed when we want to register the thread
	OS_Thread_Type(  OS_ThreadHandler  		threadHandler ,
					 int               		threadStack_size);

};

//Types to keep track active threads
typedef OS_Thread_Type* OS_ThreadPtr_Type;
typedef int 			volatile OS_ThreadIdx_Type;

class OS{
private:

	//This is an array which keeps each thread stack
	//They will never cross and they won't know they are in the same array 
	//The guard is ensured by checks on ptr arithmetic 
	unsigned int _total_stack[TOTAL_STACK];
	// This is an index that keeps track on where we are for the next stack initialization
	unsigned int _stack_counter;

public:

	//Constructor
	OS();

	//Must be called after all thread init functions
	void OS_Start();

	//Set the stack thread as pointer to internal vector - this is done to avoid the user to reserve a vector everytime
	//They need to instantiate a thread
	int OS_RegisterThread(OS_Thread_Type* Thread);

	//Set the time resolution (micro seconds) at which the OS counts internally - this is a function to configure the systick
	//And will be the smallest unit of measurement for the OS functions 
	int OS_SetTimeResoltion(unsigned int u32us);

	//This sets for how long a thread is allowed to run before being pre-empted
	//Notice that the scheduler may decide to not preempt the current thread and return without doing the context switch
	//If for example the time resolution is 100us, then 10 base units means schedule each 10*100us = 1ms
	void OS_SetTimeSlice(uint32_t u32n_baseTicks);


	//Critical code, if need to switch threads this function changes the registers 
    void OS_Sched();
	void OS_Wait(unsigned int ms);
	void OS_Sleep();
	void OS_Awake(int threadID);

	static 		OS_ThreadIdx_Type OS_GetCurrentPointer();
	static void OS_SetNextPointer(OS_ThreadIdx_Type next);
	static OS_ThreadPtr_Type OS_GetThreadBasePtr(OS_ThreadIdx_Type idx);
	static OS_ThreadIdx_Type OS_GetThreadCnt();

    void OS_Update_Threads();
    virtual void OS_SchedAlgo();

};	


#ifdef __cplusplus
extern "C" {
#endif
//Interrupts 
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif //FRAARTOS_H