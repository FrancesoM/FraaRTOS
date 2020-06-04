#include "fraaRTOS.h"

/* GLOBAL VARIABLES USED BY THE OS */

//Easy data structure - array - to keep track of the active threads
OS_ThreadPtr_Type       OS_ActiveThreads[NTHREAD_LIMIT]		; //How to init this? 
OS_ThreadIdx_Type   	OS_ThreadIdx_Current 			= 0	;
OS_ThreadIdx_Type   	OS_ThreadIdx_Next				= 0	;
OS_ThreadIdx_Type   	OS_ThreadCnt 					= 0	;
int		  	 NO_OPT		OS_FirstEntry					= 1 ;


void OS_IdleThread()
{
  while(1)
  {
  	//DO NOTHIGN AS IDLE
  }
}
OS_Thread_Type IDLE_WAIT = OS_Thread_Type(OS_IdleThread,10);
OS_Thread_Type IDLE_ACTIVE = OS_Thread_Type(OS_IdleThread,10);

//Declare the NTHREADS stacks (static allocation of memory) -- How to automate this? maybe using static when 
//The returned value is for the user to refer to this thread in advance
OS_Thread_Type::OS_Thread_Type(  
					OS_ThreadHandler  		threadHandler ,
					int               		threadStack_size)
{

	this->_thread_stack_size = threadStack_size;
	this->_threadHandler = threadHandler;

	//This is needed so tha HEAD will point to itself if list is empty
	this->next = this;
	this->prev = this;
};

//OS constructor - init the stack counter to last element because stack grows backwards
OS::OS(): _stack_counter(TOTAL_STACK) 
{
	//Reset active thread count - useful when class is reimplemented 
	//TODO: find a better way to fix this, like registering IDLE in the OS Start maybe?
	
	//Reset global variables
	OS_ThreadIdx_Current = 0;
    OS_ThreadCnt = 0;

    //Initialize internal variables
    this->OS_gTime = 0;
	OS_RegisterThread(&IDLE_WAIT); 
	OS_RegisterThread(&IDLE_ACTIVE); 
};

//Set the stack thread as pointer to internal vector - this is also useful 
//Implementation detail, it would be great to pass pThread as reference but we can't as we need the address
//of a global objeect
int OS::OS_RegisterThread(OS_Thread_Type* pThread)
{
	//Init stack pointer to the last element because it grows backwards
	OS_StackPtr_Type sp = &(this->_total_stack[this->_stack_counter - 1]);

	int lower_limit = this->_stack_counter - pThread->_thread_stack_size;

	if (lower_limit < 0)
	{
		//Return the amount of bytes that are exceeding the stack size
		return lower_limit;
	}

	//The limit is the current point minus the thread stack size
	OS_StackPtr_Type sp_limit = &(this->_total_stack[lower_limit]);

	//Update then the OS stack counter to be correct at the next thread registration into the OS
	this->_stack_counter = lower_limit;

	//TODO: stack must account for floating point unit - calling convention
	//Init a stack frame
	*(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (unsigned int)pThread->_threadHandler; /* PC */
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

    //Set stack in the thread struct - remember this is fundamental for the ASM to work 
    pThread->_sp = sp;

    //Set variables 
	OS_ActiveThreads[OS_ThreadIdx_Current] = pThread;
    pThread->_threadID = OS_ThreadIdx_Current;
    OS_ThreadIdx_Current++;
    OS_ThreadCnt++;

    //Set up the linked list - all threads are active when registered - critical time
    vInsertInList<OS_Thread_Type>(pThread,&IDLE_ACTIVE,OS_STATE_RUN);

    return 0;

};


void OS::OS_Advance()
{
	#warning "Fix this because it will overflow "
	this->OS_gTime++; 

	//OS_StartSlive starts at 0
	if( this->OS_gTime - this->OS_T_StartSlice >= this->OS_SliceDuration )
	{
		this->OS_T_StartSlice = this->OS_gTime;
		this->OS_Sched();
	}
}

void OS::OS_Start()
{
	//Add the IDLE thread
	//IDLE.OS_ThreadInit(OS_IdleThread,OS_IdleStack,100);
	//The threadIdx is sitting at the next thread which doesn't exist, let's reset it
	OS_ThreadIdx_Current = 0;
	//Suggested by arm in case some external lib is changing the priority groups
	NVIC_SetPriorityGrouping(0U);
	//Pending must be the least to run, so lowest prio
	__NVIC_SetPriority(PendSV_IRQn,15);
	//Set this to switch from main contex just the first time
	OS_FirstEntry = 1;
	// Init all the waiting periods, don't set them to RUN here because 
	// the scheduling decisions will be done based on the linked list they belong
	// to, rather than their actual status
 	for( int i = 0; i < OS_ThreadCnt; i++)
	{
		auto pcurrent = OS_ActiveThreads[i];
		//pcurrent->_state = OS_STATE_RUN;
		pcurrent->_time_to_wake = 0;
		pcurrent->_time_at_wait = 0;
	}   
}

void OS::OS_Sched()
{
	//Do the scheduling algorithm here 
	
	__disable_irq();
	//Scheduling step has to take into consideration the state of the threads, if on wait, don't bother scheduling

	//The first step is then to update all the states
	OS_Update_Threads();

	OS_SchedAlgo();

	__enable_irq();

    //__NVIC_SetPendingIRQ(PendSV_IRQn); must do it manually if irq number is negative
    
    
}

inline void OS::OS_SchedAlgo()
{
	//The second step is to update the current thread. 
	//Update current thread, which is what was next before. Then choose what goes next.

	//Idea is to scan the active list
	auto pActiveHead = &IDLE_ACTIVE;
	auto pcurrent = IDLE_ACTIVE.next;

	auto highestPrio = pcurrent->_priority;
	auto highestPrioID = pcurrent->_threadID;

	//Already compare next
	pcurrent = pcurrent->next;

	//Iterate until find a thread that is in the run state, or until you circle to where the search began
	while( pcurrent !=  pActiveHead )
	{
		// Choose based on scheduling policy - find highest prio
		if (pcurrent->_priority > highestPrio)
		{
			highestPrio = pcurrent->_priority;
			highestPrioID = pcurrent->_threadID;
		}
		//Go to the next one
		pcurrent = pcurrent->next;
	}

	//Issue a context switch if next is different than current
	if( highestPrioID != OS_GetCurrentPointer() )
	{
		//A candidate next thread has been found, set pendSV and break while
		OS_SetNextPointer(highestPrioID);
		SCB->ICSR 			|= SCB_ICSR_PENDSVSET_Msk;
	}
}

OS_ThreadIdx_Type OS::OS_GetThreadCnt()
{
	return OS_ThreadCnt;
}

OS_ThreadPtr_Type OS::OS_GetThreadBasePtr(OS_ThreadIdx_Type idx)
{
	return OS_ActiveThreads[idx];
}

OS_ThreadIdx_Type OS::OS_GetCurrentPointer()
{
	return OS_ThreadIdx_Current;
}

void OS::OS_SetNextPointer(OS_ThreadIdx_Type next)
{
	OS_ThreadIdx_Next = next;
}

void OS::OS_Update_Threads()
{
	__disable_irq();
	//Scan the wait list 
	auto pWaitHead = &IDLE_WAIT;
	auto pcurrent = IDLE_WAIT.next;
	while( pcurrent != pWaitHead )
	{
		// This check is needed because a thread might be in the wait list and not waiting, but sleeping
		// the difference is that sleep finishes when someone calls wakeup, so it's not bound to time
		if( pcurrent->_state == OS_STATE_WAIT)
		{
			int elapsed_time = this->OS_gTime - pcurrent->_time_at_wait;
			if (elapsed_time >= pcurrent->_time_to_wake)
			{
				//Update list pcurrent belongs to, hence move to active list
				vUpdateInList<OS_Thread_Type>(pcurrent,&IDLE_ACTIVE,OS_STATE_RUN);
			}			
		}

		pcurrent = pcurrent->next;

	}
	__enable_irq();
}

void OS::OS_Wait(unsigned int ms)
{
	//Critical section, we don't want this to be interrupted by OS_Sched, for instance. This must always operate 
	//on the current thread which called OS_Wait, otherwise everything is broken.
	__disable_irq();
	//Get current thread - TODO: THIS MECHANISM MUST BE IMPROVED  to not use OS_ThreadIdx_Current
	auto pcurrent = OS_ActiveThreads[OS_ThreadIdx_Current];
	pcurrent->_time_to_wake = ms;
	pcurrent->_time_at_wait = OS_gTime;
	//Remove from active list - put in wait list
	vUpdateInList<OS_Thread_Type>(pcurrent,&IDLE_WAIT,OS_STATE_WAIT);	
	__enable_irq();
	//We now need to call the sched, that will do the context switch to IDLE if eerythin is OS_STATE_WAIT
	//This call is important, otherwise we might keep going on with the thread in the WAIT state, something tht 
	//must never happen. With this call to sched instead we ensure that a thread in the WAIT state is never run, 
	//because a context switch is always done. 
	OS_Sched();
}

void OS::OS_Sleep()
{
	//Critical section, we don't want this to be interrupted by OS_Sched, for instance. This must always operate 
	//on the current thread which called OS_Wait, otherwise everything is broken.
	__disable_irq();
	//Get current thread TODO: THIS MECHANISM MUST BE IMPROVED  to not use OS_ThreadIdx_Current
	auto pcurrent = OS_ActiveThreads[OS_ThreadIdx_Current];
	//Remove from active list - put in wait list
	vUpdateInList<OS_Thread_Type>(pcurrent,&IDLE_WAIT,OS_STATE_SLEEP);
	__enable_irq();
	//We now need to call the sched, that will do the context switch to IDLE if eerythin is OS_STATE_WAIT
	OS_Sched();
}

//How does the user get the thread ID? 
void OS::OS_Awake(int threadID)
{
	__disable_irq();
	//Get current thread
	auto pcurrent = OS_ActiveThreads[threadID];
	//Remove from WAIT list - put in ACTIVE list
	vUpdateInList<OS_Thread_Type>(pcurrent,&IDLE_ACTIVE,OS_STATE_RUN);
	__enable_irq();
	//OS_Sched();
}

int OS::OS_SetTimeResoltion(unsigned int u32us)
{


	//1/Fclk * TICKS = INTERRUPT_PERIOD
	//TICKS = INTERRUPT_PERIOD * Fclk
	//WARN: cortex frequency must be multiple of MHz otherwise division is a problem

	uint32_t u32load = (uint32_t)((FREQ_CORTEX / 1000000U * u32us ) - 1UL);

	//This value can't be greater than 24bits, so 
	if ( u32load >= 0xFFFFFFU )
	{
		//Try again with the clock frequency divided by 8 
		u32load = (uint32_t)((FREQ_CORTEX / 8000000U * u32us ) - 1UL);
		//If it is still greater then return error
		if ( u32load >= 0xFFFFFFU )
		{
			//TODO: have some ret codes
			return -1;
		}
		else
		{
			SysTick->LOAD  = u32load;  /* set reload register */
			SysTick->VAL   = 0UL;                                       /* Load the SysTick Counter Value */
			SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
							 SysTick_CTRL_ENABLE_Msk;                   /* Enable the Systick Timer */
			LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK_DIV8);
		}

	}
	else
	{
		SysTick->LOAD  = u32load;  /* set reload register */
		SysTick->VAL   = 0UL;                                       /* Load the SysTick Counter Value */
		SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
						 SysTick_CTRL_ENABLE_Msk;                   /* Enable the Systick Timer */
		LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	}

	LL_SYSTICK_EnableIT();

	//Set a default  duration for the time slice of 10 "base units"
	this->OS_SliceDuration = 10;

	return 0;
}

void OS::OS_SetTimeSlice(uint32_t u32n_baseTicks)
{
	this->OS_SliceDuration = u32n_baseTicks;
}

