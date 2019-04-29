#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#ifdef __cplusplus								// If we are including to a C++
extern "C" {									// Put extern C directive wrapper around
#endif
#include <stdint.h>								// Needed for uint8_t, uint32_t, etc
#include "rpi-SmartStart.h"						// Needed for RegType_t

typedef	struct Semaphore_t* SemaphoreHandle_t;

/*-[ xSemaphoreCreateBinary ]-----------------------------------------------}
.  Create Binary Semaphore
.--------------------------------------------------------------------------*/
SemaphoreHandle_t xSemaphoreCreateBinary (void);

/*-[ xSemaphoreTake ]-------------------------------------------------------}
.  Take a Binary Semaphore
.--------------------------------------------------------------------------*/
void xSemaphoreTake (SemaphoreHandle_t sem);

/*-[ xSemaphoreGive ]-------------------------------------------------------}
.  Give a Binary Semaphore
.--------------------------------------------------------------------------*/
void xSemaphoreGive (SemaphoreHandle_t sem);

#ifdef __cplusplus								// If we are including to a C++ file
}												// Close the extern C directive wrapper
#endif

#endif
