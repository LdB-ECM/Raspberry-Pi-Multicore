#include <stdint.h>
#include <stdatomic.h>
#include "rpi-SmartStart.h"
#include "semaphore.h"

#define MAX_SEMAPHORE  50

struct __attribute__((__packed__, aligned(4))) Semaphore_t 
{
	uint32_t count;
	struct {
		uint32_t inUse : 1;
		uint32_t _reserved : 31;
	};
};

static struct Semaphore_t SemBlock [MAX_SEMAPHORE] = { 0 };


/*-[ xSemaphoreCreateBinary ]-----------------------------------------------}
.  Create Binary Semaphore
.--------------------------------------------------------------------------*/
SemaphoreHandle_t xSemaphoreCreateBinary(void)
{
	for (unsigned int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (SemBlock[i].inUse == 0)
		{
			SemBlock[i].inUse = 1;
			SemBlock[i].count = 0;
			return &SemBlock[i];
		}
	}
	return 0;
}

/*-[ xSemaphoreTake ]-------------------------------------------------------}
.  Take a Binary Semaphore
.--------------------------------------------------------------------------*/
void xSemaphoreTake (SemaphoreHandle_t sem)
{
	if (sem && sem->inUse)
	{
		semaphore_take(&sem->count);
	}
}

/*-[ xSemaphoreGive ]-------------------------------------------------------}
.  Give a Binary Semaphore
.--------------------------------------------------------------------------*/
void xSemaphoreGive (SemaphoreHandle_t sem)
{
	if (sem && sem->inUse)
	{
		semaphore_give(&sem->count);
	}
}