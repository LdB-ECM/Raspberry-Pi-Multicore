#include <stdint.h>
#include "xRTOS.h"
#include "rpi-smartstart.h"
#include "mmu.h"
#include "task.h"

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskRUNNING_CHAR		( 'X' )
#define tskBLOCKED_CHAR		( 'B' )
#define tskREADY_CHAR		( 'R' )
#define tskDELETED_CHAR		( 'D' )
#define tskSUSPENDED_CHAR	( 'S' )


/* The name allocated to the Idle task */
#ifndef configIDLE_TASK_NAME
	#define configIDLE_TASK_NAME "IDLE"
#endif


#define CoreEnterCritical DisableInterrupts
#define CoreExitCritical EnableInterrupts
#define ImmediateYield __asm volatile ("svc 0")

typedef struct TaskControlBlock* task_ptr;

/*--------------------------------------------------------------------------}
{						 TASK LIST STRUCTURE DEFINED						}
{---------------------------------------------------------------------------}
.  A task list is a simple double link list of tasks. Each task control
.  block conatins a prev and next pointer. Starting at the head task in the 
.  list structure and moving thru each task next pointer you will arrive at
.  the tail pointer in the list being the last task. The head, tail values
.  will be NULL for a no task situation. The moment you have a task in list
.  head->prev will be NULL and tail->next will be NULL as error checking.
.--------------------------------------------------------------------------*/
typedef struct tasklist
{
	struct TaskControlBlock* head;								/*< Head entry for task list */
	struct TaskControlBlock* tail;								/*< Tail entry for task list */
} TASK_LIST_t;


/*--------------------------------------------------------------------------}
{				 TASK CONTROL BLOCK STRUCTURE DEFINED						}
{---------------------------------------------------------------------------}
.  A task control block (TCB) is allocated for each task, and stores task  
.  state information, including a pointer to the task's context (the task's 
.  run time environment and data, including all register values)     
.--------------------------------------------------------------------------*/
typedef struct TaskControlBlock 
{
	volatile RegType_t	*pxTopOfStack;							/*< Points to the location of the last item placed on the tasks stack.
																	THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT AND MUST BE VOLATILE.
																	It changes each task switch and the optimizer needs to know that */
	struct {
		volatile RegType_t	uxCriticalNesting : 8;				/*< Holds the critical section nesting depth */
		RegType_t			_reserved : sizeof(RegType_t) * 8 - 11;
		volatile RegType_t	ucDelayAborted : 1;					/*< Set to 1 if the task delay is aborted */
		volatile RegType_t	ucStaticallyAllocated : 1;			/*< Set to 1 if the task is a statically allocated to ensure no attempt is made to free the memory. */
		volatile RegType_t	uxTaskUsesFPU : 1;					/*< If task uses FPU this flag will be set to 1 and FPU registers save on context switch */
	}	pxTaskFlags;											/*< Task flags ... these flags will be save FPU, nested count etc in future
																	THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT AND MUST BE VOLATILE.
																	It changes each task switch and the optimizer needs to know that */

	RegType_t* pxStack;											/*< Points to the start of the stack allocated when task created */

	/* These form the task state double link list system */
	struct TaskControlBlock* next;								/*< Next task in list */
	struct TaskControlBlock* prev;								/*< Prev task in list */
	RegType_t ReleaseTime;										/*< Core OSTickCounter at which task will be released from delay ... only valid if task in delayed list */
	RegType_t waitMessageID;									/*< When in the wait on message list this is the unique message ID that will release it */
	struct {
		RegType_t		uxPriority : 8;							/*< The priority of the task.  0 is the lowest priority. */
		RegType_t		taskState : 8;							/*< Task state running, delayed, blocked etc */
		RegType_t		_reserved : sizeof(RegType_t)*8 - 20;
		RegType_t		assignedCore : 3;						/*< Core this task is assigned to on a multicore, always 0 on single core */
		RegType_t		inUse : 1;								/*< This task is in use field */
	};
	
	char				pcTaskName[ configMAX_TASK_NAME_LEN ];	/*< Descriptive name given to task when created.  Facilitates debugging only. */ 
} TCB_t;


/*--------------------------------------------------------------------------}
{				 CORE CONTROL BLOCK STRUCTURE DEFINED						}
{---------------------------------------------------------------------------}
.  A CPU control block (coreCB) is allocated for each CPU core and stores 
.  information, specific to the tasks being run on that CPU core.
.--------------------------------------------------------------------------*/
static struct CoreControlBlock
{
	volatile TCB_t* pxCurrentTCB;							/*< Points to the current task that is running on this CPU core.
																THIS MUST BE THE FIRST MEMBER OF THE CORE CONTROL BLOCK STRUCT AND MUST BE VOLATILE.
																It changes each task switch and the optimizer needs to know that */
	TaskHandle_t xIdleTaskHandle;							/*< Holds the handle of the core idle task. The idle task is created automatically when the scheduler is started. */
	TASK_LIST_t	readyTasks;									/*< List of tasks that are ready to run */
	TASK_LIST_t delayedTasks;								/*< List of tasks that are in delayed state */
	TASK_LIST_t waitMsgTasks;								/*< List of tasks that are waiting on messages */
	RegType_t OSTickCounter;								/*< Incremented each tick timer - Used in delay and timeout functions */
	struct TaskControlBlock coreTCB[MAX_TASKS_PER_CORE];	/*< This cores list of tasks on the core */
	struct {
		volatile unsigned uxCurrentNumberOfTasks : 16;		/*< Current number of task running on this core */
		volatile unsigned uxPercentLoadCPU : 16;			/*< Last CPU load calculated = uxIdleTickCount/configTICK_RATE_HZ * 100 in percent for last 1 sec frame   */
		volatile unsigned uxIdleTickCount : 16;			    /*< Current ticks in 1 sec analysis frame that idle was current task */
		volatile unsigned uxCPULoadCount: 16;				/*< Current count in 1 sec analysis frame .. one sec =  configTICK_RATE_HZ ticks */
		volatile unsigned uxSchedulerSuspended : 16;		/*< Context switches are held pending while the scheduler is suspended.  */
		unsigned xSchedulerRunning : 1;						/*< Set to 1 if the scheduler is running on this core */
		unsigned xCoreBlockInitialized : 1;					/*< Set to 1 if the core block has been initialized */
	};
} coreCB[MAX_CPU_CORES] = { 0 };

/***************************************************************************}
{					   PRIVATE INTERNAL DATA STORAGE					    }
****************************************************************************/

static RegType_t TestStack[16384] __attribute__((aligned(16)));
static RegType_t* TestStackTop = &TestStack[16384];

RegType_t ulCriticalNesting = 0;
static uint64_t m_nClockTicksPerHZTick = 0;							// Divisor to generat tick frequency

/***************************************************************************}
{					    PRIVATE INTERNAL ROUTINES						    }
****************************************************************************/

/*--------------------------------------------------------------------------}
{			Adds the task into the task double linked list					}
{--------------------------------------------------------------------------*/
static void AddTaskToList (TASK_LIST_t* list, struct TaskControlBlock* task)
{
	if (list->tail != 0)											// List already has at least one task
	{
		/* Insert task into double linked ready list */
		list->tail->next = task;									// Add task to current list tail
		task->prev = list->tail;									// Set task prev to current lits tail
		list->tail = task;											// Now task becomes the list tail
		task->next = 0;												// Zero the task nextready pointer
	}
	else {															// No existing tasks in list
		/* Init ready list */
		list->tail = task;											// Task is the tail
		list->head = task;											// Task is also head
		task->next = 0;												// Task has no next ready
		task->prev = 0;												// Task has no prev ready
	}
}

/*--------------------------------------------------------------------------}
{				Removes the task from task double linked list				}
{--------------------------------------------------------------------------*/
static void RemoveTaskFromList (TASK_LIST_t* list, struct TaskControlBlock* task)
{
	if (task == list->head)											// Task is the first one in the list		
	{
		if (task == list->tail)										// If task is also the last one on list it is only task in list
		{
			list->tail = 0;											// Zero the tail task ptr
			list->head = 0;											// Zero the head task ptr
		}
		else {
			list->head = task->next;								// Move next task up to list head position
			list->head->prev = 0;									// This task is now top its previous is then null (it would have been the task we are removing)
		}
	}
	else {															// Task is not head then must be in list
		if (task == list->tail)										// Is task the tail ptr in list
		{
			list->tail = task->prev;								// List tail will now become tasks previous 
			list->tail->next = 0;									// Zero that tasks next pointer (it would have been the task we are removing)
		}
		else {
			task->next->prev = task->prev;							// Our next prev will point to our prev (unchains task from next links)
			task->prev->next = task->next;							// Our prev next will point to our next (unchains task from prev links)
		}
	}
}

/*--------------------------------------------------------------------------}
{				The default idle task .. that does nothing :-)				}
{--------------------------------------------------------------------------*/
static void prvIdleTask(void* pvParameters)
{
	/* Stop warnings. */
	(void)pvParameters;

	/** THIS IS THE RTOS IDLE TASK - WHICH IS CREATED AUTOMATICALLY WHEN THE
	SCHEDULER IS STARTED. **/
	for (;; )
	{

	}
}

/*--------------------------------------------------------------------------}
{			Starts the tasks running on the core just as it says			}
{--------------------------------------------------------------------------*/
static void StartTasksOnCore(void)
{
	MMU_enable();													// Enable MMU											
	EL0_Timer_Set(m_nClockTicksPerHZTick);							// Set the EL0 timer
	EL0_Timer_Irq_Setup();											// Setup the EL0 timer interrupt

	EnableInterrupts();												// Enable interrupts on core
	xStartFirstTask();												// Restore context starting the first task
}

/***************************************************************************}
{					    PUBLIC INTERFACE ROUTINES						    }
****************************************************************************/

/*-[xRTOS_Init]-------------------------------------------------------------}
.  Initializes xRTOS system and must be called before any other xRTOS call
.--------------------------------------------------------------------------*/
void xRTOS_Init (void)
{
	for (int i = 0; i < MAX_CPU_CORES; i++)
	{
		RPi_coreCB_PTR[i] = &coreCB[i];								// Set the core block pointers in the smartstart system needed by irq and swi vectors
		coreCB[i].xCoreBlockInitialized = 1;						// Set the core block initialzied flag to state this has been done
	}
}

/*-[ xTaskCreate ]----------------------------------------------------------}
.  Creates an xRTOS task on the given core.
.--------------------------------------------------------------------------*/
void xTaskCreate (uint8_t corenum,									// The core number to run task on
				  void (*pxTaskCode) (void* pxParam),				// The code for the task
				  const char * const pcName,						// The character string name for the task
				  const unsigned int usStackDepth,					// The stack depth in register size for the task stack
				  void * const pvParameters,						// Private parameter that may be used by the task
				  uint8_t uxPriority,								// Priority of the task
				  TaskHandle_t* const pxCreatedTask)				// A pointer to return the task handle (NULL if not required)
{
	int i;
	for (i = 0; (i < MAX_TASKS_PER_CORE) && (coreCB[corenum].coreTCB[i].inUse != 0); i++) {};
	if (i < MAX_TASKS_PER_CORE)
	{
		struct CoreControlBlock* cb;
		struct TaskControlBlock* task;
		CoreEnterCritical();										// Entering core critical area	
		cb = &coreCB[corenum];										// Set pointer to core block
		task = &cb->coreTCB[i];										// This is the task we are talking about
		task->pxStack = TestStackTop;								// Hold the top of task stack
		task->pxTopOfStack = taskInitialiseStack(TestStackTop, pxTaskCode, pvParameters);
		//task->pxTaskFlags = 0;										// Make sure the task flags are clear
		TestStackTop -= usStackDepth;
		task->uxPriority = uxPriority;								// Hold the task priority
		task->inUse = 1;											// Set the task is in use flag
		task->assignedCore = corenum;								// Hold the core number task assigned to 
		if (pcName) {
			int j;
			for (j = 0; (j < configMAX_TASK_NAME_LEN - 1) && (pcName[j] != 0); j++)
				task->pcTaskName[j] = pcName[j];					// Transfer the taskname
			task->pcTaskName[j] = 0;								// Make sure asciiz
		}
		cb->uxCurrentNumberOfTasks++;								// Increment task count on core
		if (cb->pxCurrentTCB == 0) cb->pxCurrentTCB = task;			// If current task on core make this the current
		task->taskState = tskREADY_CHAR;							// Set the read char state
		AddTaskToList(&cb->readyTasks, task);						// Add task to read task lits
		if (pxCreatedTask) (*pxCreatedTask) = task;
		CoreExitCritical();											// Exiting core critical area
	}
}

/*-[ xTaskDelay ]-----------------------------------------------------------}
.  Moves an xRTOS task from the ready task list into the delayed task list
.  until the time wait in timer ticks is expired. This effectively stalls
.  any task processing at that time for the fixed period of time.
.--------------------------------------------------------------------------*/
void xTaskDelay (const unsigned int time_wait)
{
	if (time_wait)													// Non zero wait time requested
	{
		struct TaskControlBlock* task;
		unsigned int corenum = getCoreID();							// Get the core ID
		struct CoreControlBlock* cb = &coreCB[corenum];				// Set pointer to core block
		CoreEnterCritical();										// Entering core critical area
		task = (struct TaskControlBlock*) cb->pxCurrentTCB;			// Set temp task pointer .. typecast is to stop volatile dropped warning						
		task->ReleaseTime = cb->OSTickCounter + time_wait;			// Calculate release tick value
		RemoveTaskFromList(&cb->readyTasks, task);					// Remove task from ready list
		task->taskState = tskBLOCKED_CHAR;							// Change task state to blocked
		AddTaskToList(&cb->delayedTasks, task);						// Add the task to delay list
		CoreExitCritical();											// Exiting core critical area
		ImmediateYield;												// Immediate yield ... store task context, reschedule new current task and switch to it
	}
}

/*-[ xTaskWaitOnMessage ]---------------------------------------------------}
.  Moves an xRTOS task from the ready task list into wait on message task 
.  list. This effectively stalls any task processing at that time until a
.  message arrives to release. The caller must provide a unique message ID
.  that will release this task.
.--------------------------------------------------------------------------*/
void xTaskWaitOnMessage (const RegType_t userMessageID)
{
	if (userMessageID)												// Non zero user Message ID must be used
	{
		struct TaskControlBlock* task;
		unsigned int corenum = getCoreID();							// Get the core ID
		struct CoreControlBlock* cb = &coreCB[corenum];				// Set pointer to core block
		CoreEnterCritical();										// Entering core critical area
		task = (struct TaskControlBlock*) cb->pxCurrentTCB;			// Set temp task pointer .. typecast is to stop volatile dropped warning							
		RemoveTaskFromList(&cb->readyTasks, task);					// Remove task from ready list
		task->waitMessageID = userMessageID;						// Set wait on message ID
		task->taskState = tskBLOCKED_CHAR;							// Change task state to blocked
		AddTaskToList(&cb->waitMsgTasks, task);						// Add the task to wait message task list
		CoreExitCritical();											// Exiting core critical area
		ImmediateYield;												// Immediate yield ... store task context, reschedule new current task and switch to it
	}
}

/*-[ xTaskReleaseMessage ]--------------------------------------------------}
.  Moves an xRTOS task from the wait on message task to the ready task list
.  This effectively resumes task processing at that time. It is valid to go 
.  cross core with this call. The current core will check first before 
.  trying other core lists.
.--------------------------------------------------------------------------*/
void xTaskReleaseMessage (const RegType_t userMessageID)
{
	if (userMessageID)												// Non zero user Message ID must be used
	{
		struct TaskControlBlock* task;
		unsigned int corenum = getCoreID();							// Get the core ID
		struct CoreControlBlock* cb = &coreCB[corenum];				// Set pointer to core block
		/* Check current core */
		task = cb->waitMsgTasks.head;								// Set task to wait for message head
		while (task != 0)
		{
			if (userMessageID == task->waitMessageID)				// Check if message matches
			{
				RemoveTaskFromList(&cb->waitMsgTasks, task);		// Remove the task from wait for messsage list
				task->taskState = tskREADY_CHAR;					// Set the read char state
				AddTaskToList(&cb->readyTasks, task);				// Add the task to the ready list
				return;												// Only one task release per message
			}
			task = task->next;										// Next message task
		}

		/* Check previous cores */
		for (int i = 0; i < corenum; i++)
		{
			cb = &coreCB[i];										// Set pointer to core block
			task = cb->waitMsgTasks.head;							// Set task to wait for message head
			while (task != 0)
			{
				if (userMessageID == task->waitMessageID)			// Check if message matches
				{
					RemoveTaskFromList(&cb->waitMsgTasks, task);	// Remove the task from wait for messsage list
					task->taskState = tskREADY_CHAR;				// Set the read char state
					AddTaskToList(&cb->readyTasks, task);			// Add the task to the ready list
					return;											// Only one task release per message
				}
				task = task->next;									// Next message task
			}
		}
		/* Check later cores */
		for (int i = corenum + 1; i < MAX_CPU_CORES; i++)
		{
			cb = &coreCB[i];										// Set pointer to core block
			task = cb->waitMsgTasks.head;							// Set task to wait for message head
			while (task != 0)
			{
				if (userMessageID == task->waitMessageID)			// Check if message matches
				{
					RemoveTaskFromList(&cb->waitMsgTasks, task);	// Remove the task from wait for messsage list
					task->taskState = tskREADY_CHAR;				// Set the read char state
					AddTaskToList(&cb->readyTasks, task);			// Add the task to the ready list
					return;											// Only one task release per message
				}
				task = task->next;									// Next message task
			}
		}
	}
}

/*-[ xTaskStartScheduler ]--------------------------------------------------}
.  starts the xRTOS task scheduler effectively starting the whole system
.--------------------------------------------------------------------------*/
void xTaskStartScheduler( void )
{
	/* Add the idle task at the lowest priority to each core */
	for (int i = 0; i < MAX_CPU_CORES; i++)
	{
		xTaskCreate(i, prvIdleTask,	configIDLE_TASK_NAME,
			configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY, 
			&coreCB[i].xIdleTaskHandle); 
	}

	/* Calculate divisor to create Timer Tick frequency on EL0 timer */
	m_nClockTicksPerHZTick = (EL0_Timer_Frequency() / configTICK_RATE_HZ);

	/* MMU table setup done by core 0 */
	MMU_setup_pagetable();

	/* Start each core in reverse order because core0 is running this code  */
	CoreExecute(3, StartTasksOnCore);								// Start tasks on core3
	CoreExecute(2, StartTasksOnCore);								// Start tasks on core2
	CoreExecute(1, StartTasksOnCore);								// Start tasks on core1
	StartTasksOnCore();												// Start tasks on core0
}

/*-[ xTaskGetNumberOfTasks ]------------------------------------------------}
.  Returns the number of xRTOS tasks assigned to the core this is called
.--------------------------------------------------------------------------*/
unsigned int xTaskGetNumberOfTasks (void )
{
	return coreCB[getCoreID()].uxCurrentNumberOfTasks;				// Return number of tasks on current core
}

/*-[ xLoadPercentCPU ]------------------------------------------------------}
.  Returns the load on the core this is called from in percent (0 - 100)
.--------------------------------------------------------------------------*/
unsigned int xLoadPercentCPU (void)
{
	return (((configTICK_RATE_HZ - coreCB[getCoreID()].uxPercentLoadCPU) * 100) / configTICK_RATE_HZ);
}


/*
 * Called from the real time kernel tick via the EL0 timer irq this increments 
 * the tick count and checks if any tasks that are blocked for a finite period 
 * required removing from a delayed list and placing on the ready list.
 */
void xTaskIncrementTick (void)
{
	struct CoreControlBlock* ccb = &coreCB[getCoreID()];

	if (ccb->xCoreBlockInitialized == 1)							// Check the core block is initialized  
	{
		if (ccb->uxSchedulerSuspended == 0)							// Core scheduler not suspended
		{

			/* LdB - Addition to calc CPU Load */
			if (ccb->pxCurrentTCB == ccb->xIdleTaskHandle)			// Is the core current task the core idle task
			{
				ccb->uxIdleTickCount++;								// Inc idle tick count in the current 1 second analysis frame
			}
			if (ccb->uxCPULoadCount >= configTICK_RATE_HZ)			// If configTICK_RATE_HZ ticks done, time to see how many were idle
			{
				ccb->uxCPULoadCount = 0;							// Zero the config count for next analysis process period to start again
				ccb->uxPercentLoadCPU = ccb->uxIdleTickCount;		// Transfer the idletickcount to uxPercentLoadCPU we will only do calc when asked
				ccb->uxIdleTickCount = 0;							// Zero the idle tick count
			}
			else ccb->uxCPULoadCount++;								// Increment the process tick count

			/* Increment timer tick and check delaytasks for timeout */
			ccb->OSTickCounter++;									// Increment OS tick counter
			struct TaskControlBlock* task = ccb->delayedTasks.head;	// Set task to delay head
			while (task != 0)
			{
				if (ccb->OSTickCounter >= task->ReleaseTime)		// Check if release time is up
				{
					RemoveTaskFromList(&ccb->delayedTasks, task);	// Remove the task from delay list
					task->taskState = tskREADY_CHAR;				// Set the read char state
					AddTaskToList(&ccb->readyTasks, task);			// Add the task to the ready list
				}
				task = task->next;									// Next delayed task
			}
		}
	}
}


/*
 * Simple round robin scheduler on tasks that are in the readyTasks list
 */
void xSchedule (void)
{
	struct CoreControlBlock* ccb = &coreCB[getCoreID()];			// Pointer to core control block
	if (ccb->xCoreBlockInitialized == 1)							// Check the core block is initialized  
	{
		if (ccb->uxSchedulerSuspended == 0)							// Core scheduler not suspended
		{
			struct TaskControlBlock* next = ccb->pxCurrentTCB->next;
			if (next)												// Check current task has a next ready task
				ccb->pxCurrentTCB = next;							// Simply load next ready
				else ccb->pxCurrentTCB = ccb->readyTasks.head;		// No next ready so load readyTasks head
		}
	}
}

/*
 *	This is the TICK interrupt service routine, note. no SAVE/RESTORE_CONTEXT here
 *	as thats done in the bottom-half of the ISR in assembler.
 */
void xTickISR(void)
{
	xTaskIncrementTick();											// Run the timer tick
	xSchedule();													// Run scheduler selecting next task 
	EL0_Timer_Set(m_nClockTicksPerHZTick);							// Set EL0 timer again for timer tick period
}





