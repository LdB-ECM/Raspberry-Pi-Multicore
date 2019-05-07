#ifndef xRTOS_CONFIG_H
#define xRTOS_CONFIG_H

#define MAX_CPU_CORES							( 4	)				// The Raspberry Pi3 has 4 cores	
#define MAX_TASKS_PER_CORE						( 8 )				// For the moment task storage is static so we need some size
#define configTICK_RATE_HZ						( 1000 )			// Timer tick frequency	
#define tskIDLE_PRIORITY						( 0	)				// Idle priority is 0 .. rarely would this ever change	
#define configMAX_TASK_NAME_LEN					( 16 )				// Maxium length of a task name
#define configMINIMAL_STACK_SIZE				( 128 )				// Minimum stack size used by idle task


#endif 

