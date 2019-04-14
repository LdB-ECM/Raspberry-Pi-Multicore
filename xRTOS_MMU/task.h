#ifndef INC_TASK_H
#define INC_TASK_H

#include <stdint.h>
#include "rpi-smartstart.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------}
{							TASK HANDLE DEFINED								}
{--------------------------------------------------------------------------*/
struct TaskControlBlock;
typedef struct TaskControlBlock* TaskHandle_t;

/***************************************************************************}
{					    PUBLIC INTERFACE ROUTINES						    }
****************************************************************************/

/*-[ xRTOS_Init ]-----------------------------------------------------------}
.  Initializes xRTOS system and must be called before any other xRTOS call
.--------------------------------------------------------------------------*/
void xRTOS_Init (void);


/*-[ xTaskCreate ]----------------------------------------------------------}
.  Creates an xRTOS task on the given core.
.--------------------------------------------------------------------------*/
void xTaskCreate (uint8_t corenum,									// The core number to run task on
				  void (*pxTaskCode) (void* pxParam),				// The code for the task
				  const char* const pcName,							// The character string name for the task
				  const unsigned int usStackDepth,					// The stack depth in register size for the task stack 
				  void* const pvParameters,							// Private parameter that may be used by the task
				  uint8_t uxPriority,								// Priority of the task
				  TaskHandle_t* const pxCreatedTask);				// A pointer to return the task handle (NULL if not required)


/*-[ xTaskDelay ]-----------------------------------------------------------}
.  Moves an xRTOS task from the ready task list into the delayed task list
.  until the time wait in timer ticks is expired. This effectively stalls 
.  any task processing at that time for the fixed period of time. 
.--------------------------------------------------------------------------*/
void xTaskDelay (const unsigned int time_wait);


/*-[ xTaskStartScheduler ]--------------------------------------------------}
.  starts the xRTOS task scheduler effectively starting the whole system
.--------------------------------------------------------------------------*/
void xTaskStartScheduler (void);

/*-[ xTaskGetNumberOfTasks ]------------------------------------------------}
.  Returns the number of xRTOS tasks assigned to the core this is called
.--------------------------------------------------------------------------*/
unsigned int xTaskGetNumberOfTasks (void);

/*-[ xLoadPercentCPU ]------------------------------------------------------}
.  Returns the load on the core this is called from in percent (0 - 100)
.--------------------------------------------------------------------------*/
unsigned int xLoadPercentCPU(void);

#ifdef __cplusplus
}
#endif
#endif /* INC_TASK_H */



