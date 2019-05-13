#ifndef _QA7_H_
#define _QA7_H_

#ifdef __cplusplus								// If we are including to a C++
extern "C" {									// Put extern C directive wrapper around
#endif

#include <stdbool.h>		// C standard unit needed for bool and true/false
#include <stdint.h>			// C standard unit needed for uint8_t, uint32_t, etc

/*==========================================================================}
{				 MULTICORE LOCAL TIMER API ROUTINES							}
{==========================================================================*/

/*-[ ClearLocalTimerIrq ]---------------------------------------------------}
. Simply clear the local timer interrupt by hitting the clear registers.
. Any irq or fiq local timer interrupt should call this before exiting.
. On BCM2835 (Pi1 ARM6) it does not have core timer so call fails.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool ClearLocalTimerIrq (void);

/*-[ LocalTimerSetup ]------------------------------------------------------}
. Sets the clock rate to the period in usec for the local clock timer.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. All cores share this clock so setting it from any core changes all cores.
. On BCM2835 (Pi1 ARM6) it does not have core timer so call fails.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerSetup (uint32_t period_in_us);						// Period between timer interrupts in usec

/*-[ LocalTimerIrqSetup ]---------------------------------------------------}
. The local timer irq interrupt rate is set to the period in usec between
. triggers. On BCM2835 (ARM6) it does not have core timer so call fails.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerIrqSetup (uint32_t period_in_us,						// Period between timer interrupts in usec
						 uint8_t coreNum,							// Core number
						 bool secureMode);							// Secure mode or not

/*-[ LocalTimerFiqSetup ]---------------------------------------------------}
. The local timer fiq interrupt rate is set to the period in usec between
. triggers. On BCM2835 (ARM6) it does not have core timer so call fails.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerFiqSetup (uint32_t period_in_us,						// Period between timer interrupts in usec
						 uint8_t coreNum,							// Core number
						 bool secureMode);							// Secure mode or not

/*==========================================================================}
{					 CORE MAILBOX API ROUTINES								}
{==========================================================================*/

/*-[ SendCoreMessage ]------------------------------------------------------}
. Send a message to the request core on one of it's four mailboxes (0..3). 
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool SendCoreMessage (uint32_t msg,									// Message to send core
					  uint8_t coreNum,								// Core number
					  uint8_t mailbox);								// Mailbox number of core

/*-[ ReadCoreMessage ]------------------------------------------------------}
. Read a message from the request core on one of it's four mailboxes (0..3).
. You can poll but typically this is  in response to IRQ or FIQ from mailbox
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool ReadCoreMessage (uint32_t* msg,								// Pointer to read result
					  uint8_t coreNum,								// Core number
					  uint8_t mailbox);								// Mailbox number of core

/*==========================================================================}
{					 CORE MAILBOX FIQ API ROUTINES							}
{==========================================================================*/

/*-[ CoreMailboxFiqSetup ]--------------------------------------------------}
. Sets the core mailbox FIQ to call the given routine at the address.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool CoreMailboxFiqSetup (void (*ARMaddress) (void),				// Address of FIQ handler
						  uint8_t coreNum,							// Core number
						  uint8_t mailbox);							// Mailbox



#ifdef __cplusplus								// If we are including to a C++ file
}												// Close the extern C directive wrapper
#endif

#endif