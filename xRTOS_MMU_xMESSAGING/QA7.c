#include <stdbool.h>		// C standard unit needed for bool and true/false
#include <stdint.h>			// C standard unit needed for uint8_t, uint32_t, etc
#include "rpi-SmartStart.h"	// Include smartstart header
#include "QA7.h"			// Include this units header

/*--------------------------------------------------------------------------}
{  0x4000_0024 LOCAL TIMER IRQ ROUTING REGISTER - QA7_rev3.4.pdf page 18    }
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		enum {
			LOCALTIMER_TO_CORE0_IRQ = 0,
			LOCALTIMER_TO_CORE1_IRQ = 1,
			LOCALTIMER_TO_CORE2_IRQ = 2,
			LOCALTIMER_TO_CORE3_IRQ = 3,
			LOCALTIMER_TO_CORE0_FIQ = 4,
			LOCALTIMER_TO_CORE1_FIQ = 5,
			LOCALTIMER_TO_CORE2_FIQ = 6,
			LOCALTIMER_TO_CORE3_FIQ = 7,
		} Routing : 3;												// @0-2		Local Timer routing 
		unsigned _unused : 29;										// @3-31
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} local_timer_irq_route_reg_t;

/*--------------------------------------------------------------------------}
{  0x4000_0034 LOCAL TIMER CONTROL STATUS REGISTER - QA7_rev3.4.pdf page 17 }
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned ReloadValue : 28;									// @0-27	Reload value 
		unsigned TimerEnable : 1;									// @28		Timer enable (1 = enabled) 
		unsigned IntEnable : 1;										// @29		Interrupt enable (1= enabled)
		unsigned _unused : 1;										// @30		Unused
		unsigned IntPending : 1;									// @31		Timer Interrupt flag (Read-Only) 
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} local_timer_ctrl_status_reg_t;

/*--------------------------------------------------------------------------}
{  0x4000_0038 LOCAL TIMER CLEAR & RELOAD REGISTER - QA7_rev3.4.pdf page 18	}
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned _unused : 30;										// @0-29	unused 
		unsigned Reload : 1;										// @30		Local timer-reloaded when written as 1 
		unsigned IntClear : 1;										// @31		Interrupt flag clear when written as 1  
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} local_timer_clr_reload_reg_t;

/*--------------------------------------------------------------------------}
{   0x4000_0040 GENERIC TIMER INTERRUPT CONTROL - QA7_rev3.4.pdf page 13	}
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned nCNTPSIRQ_IRQ : 1;									// @0		Secure physical timer event IRQ enabled, This bit is only valid if bit 4 is clear otherwise it is ignored. 
		unsigned nCNTPNSIRQ_IRQ : 1;								// @1		Non-secure physical timer event IRQ enabled, This bit is only valid if bit 5 is clear otherwise it is ignored
		unsigned nCNTHPIRQ_IRQ : 1;									// @2		Hypervisor physical timer event IRQ enabled, This bit is only valid if bit 6 is clear otherwise it is ignored
		unsigned nCNTVIRQ_IRQ : 1;									// @3		Virtual physical timer event IRQ enabled, This bit is only valid if bit 7 is clear otherwise it is ignored
		unsigned nCNTPSIRQ_FIQ : 1;									// @4		Secure physical timer event FIQ enabled, If set, this bit overrides the IRQ bit (0) 
		unsigned nCNTPNSIRQ_FIQ : 1;								// @5		Non-secure physical timer event FIQ enabled, If set, this bit overrides the IRQ bit (1)
		unsigned nCNTHPIRQ_FIQ : 1;									// @6		Hypervisor physical timer event FIQ enabled, If set, this bit overrides the IRQ bit (2)
		unsigned nCNTVIRQ_FIQ : 1;									// @7		Virtual physical timer event FIQ enabled, If set, this bit overrides the IRQ bit (3)
		unsigned _unused : 24;										// @8-31	unused
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} generic_timer_int_ctrl_reg_t;

/*--------------------------------------------------------------------------}
{  0x4000_0050 MAILBOX INTERRUPT CONTROL REGISTER - QA7_rev3.4.pdf page 14 	}
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned IRQ_Routing : 4;									// @0-3		Mailbox IRQ routing  1 = mailbox0, 2 = mailbox1, 4 = mailbox2, 8 = mailbox3
		unsigned FIQ_Routing : 4;									// @4-7		Mailbox FIQ routing  1 = mailbox0, 2 = mailbox1, 4 = mailbox2, 8 = mailbox3
		unsigned _unused : 24;										// @8-31	unused
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} mailbox_int_ctrl_reg_t;

/*--------------------------------------------------------------------------}
{	0x4000_0060 CORE INTERRUPT SOURCE REGISTER - QA7_rev3.4.pdf page 16		}
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned CNTPSIRQ : 1;										// @0		CNTPSIRQ interrupt 
		unsigned CNTPNSIRQ : 1;										// @1		CNTPNSIRQ interrupt 
		unsigned CNTHPIRQ : 1;										// @2		CNTHPIRQ interrupt 
		unsigned CNTVIRQ : 1;										// @3		CNTVIRQ interrupt 
		unsigned Mailbox0_Int : 1;									// @4		Mailbox 0 interrupt
		unsigned Mailbox1_Int : 1;									// @5		Mailbox 1 interrupt
		unsigned Mailbox2_Int : 1;									// @6		Mailbox 2 interrupt
		unsigned Mailbox3_Int : 1;									// @7		Mailbox 3 interrupt
		unsigned GPU_Int : 1;										// @8		GPU interrupt <Can be high in one core only> 
		unsigned PMU_Int : 1;										// @9		PMU interrupt 
		unsigned AXI_Int : 1;										// @10		AXI-outstanding interrupt <For core 0 only!> all others are 0 
		unsigned Timer_Int : 1;										// @11		Local timer interrupt
		unsigned GPIO_Int : 16;										// @12-27	Peripheral 1..15 interrupt (Currently not used)
		unsigned reserved : 4;										// @28-31	reserved
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} core_irq_source_reg_t;

/*--------------------------------------------------------------------------}
{	   0x4000_0070 CORE FIQ SOURCE REGISTER - QA7_rev3.4.pdf page 16		}
{--------------------------------------------------------------------------*/
typedef union
{
	struct
	{
		unsigned CNTPSFIQ : 1;										// @0		CNTPS fast interrupt 
		unsigned CNTPNSFIQ : 1;										// @1		CNTPNS fast interrupt 
		unsigned CNTHPFIQ : 1;										// @2		CNTHP fast interrupt 
		unsigned CNTVFIQ : 1;										// @3		CNTV fast interrupt 
		unsigned Mailbox0_Fiq : 1;									// @4		Mailbox 0 fast interrupt 
		unsigned Mailbox1_Fiq : 1;									// @5		Mailbox 1 fast interrupt
		unsigned Mailbox2_Fiq : 1;									// @6		Mailbox 2 fast interrupt
		unsigned Mailbox3_Fiq : 1;									// @7		Mailbox 3 fast interrupt
		unsigned GPU_Fiq : 1;										// @8		GPU fast interrupt <Can be high in one core only> 
		unsigned PMU_Fiq : 1;										// @9		PMU fast interrupt 
		unsigned _always_zero : 1;									// @10		Always zero  
		unsigned Timer_Fiq : 1;										// @11		Local timer fast interrupt
		unsigned GPIO_Fiq : 16;										// @12-27	Peripheral 1..15 fast interrupt (Currently not used)
		unsigned reserved : 4;										// @28-31	reserved
	};
	uint32_t Raw32;													// Union to access all 32 bits as a uint32_t
} core_fiq_source_reg_t;

typedef struct
{
	uint32_t boxNumber[4];											// Cores have 4 mailboxes
} CoreMailbox;													

/*--------------------------------------------------------------------------}
{					 ALL QA7 REGISTERS IN ONE BIG STRUCTURE					}
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) QA7Registers 
{
	uint32_t Control_Register;										// 0x00
	uint32_t _unused;												// 0x04
	uint32_t Core_Timer_Prescaler;									// 0x08
	uint32_t GPU_Iterrupts_Routing;									// 0x0C
	uint32_t Performance_Monitor_Interrupts_Routing_Set;			// 0x10
	uint32_t Performance_Monitor_Interrupts_Routing_Clear;			// 0x14
	uint32_t _unused1;												// 0x18
	uint32_t CoreTimerAccess_LS;									// 0x1C
	uint32_t CoreTimerAccess_MS;									// 0x20
	local_timer_irq_route_reg_t TimerRouting;						// 0x24
	uint32_t GPIORouting;											// 0x28
	uint32_t AXIOutstandingCounters;								// 0x2C
	uint32_t AXIOutstandingIrq;										// 0x30
	local_timer_ctrl_status_reg_t TimerControlStatus;				// 0x34
	local_timer_clr_reload_reg_t TimerClearReload;					// 0x38
	uint32_t _unused2;												// 0x3C
	generic_timer_int_ctrl_reg_t CoreTimerIntControl[4];			// 0x40, 0x44, 0x48, 0x4C  .. One per core
	mailbox_int_ctrl_reg_t  CoreMailboxIntControl[4];				// 0x50, 0x54, 0x58, 0x5C  .. One per core
	core_irq_source_reg_t CoreIRQSource[4];							// 0x60, 0x64, 0x68, 0x6C  .. One per core
	core_fiq_source_reg_t CoreFIQSource[4];							// 0x70, 0x74, 0x78, 0x7C  .. One per core
	CoreMailbox CoreMailbox_Write[4];								// Cores have 4 mailboxes each for 4 cores 0x80 to 0xBF
	CoreMailbox CoreMailbox_Read_Clear[4];							// Cores have 4 mailboxes each for 4 cores 0xC0 to 0xFF
};

/***************************************************************************}
{        PRIVATE INTERNAL RASPBERRY PI REGISTER STRUCTURE CHECKS            }
****************************************************************************/

/*--------------------------------------------------------------------------}
{					 CODE TYPE STRUCTURE COMPILE TIME CHECKS	            }
{--------------------------------------------------------------------------*/
/* If you have never seen compile time assertions it's worth google search */
/* on "Compile Time Assertions". It is part of the C11++ specification and */
/* all compilers that support the standard will have them (GCC, MSC inc)   */
/*-------------------------------------------------------------------------*/
#include <assert.h>								// Need for compile time static_assert

static_assert(sizeof(struct QA7Registers) == 0x100, "QA7Registers should be 0x100 bytes in size");

#define QA7 ((volatile __attribute__((aligned(4))) struct QA7Registers*)(uintptr_t)(0x40000000))

/*==========================================================================}
{				 MULTICORE LOCAL TIMER API ROUTINES							}
{==========================================================================*/

/*-[ ClearLocalTimerIrq ]---------------------------------------------------}
. Simply clear the local timer interrupt by hitting the clear registers.
. Any irq or fiq local timer interrupt should call this before exiting.
. On BCM2835 (Pi1 ARM6) it does not have core timer so call fails.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool ClearLocalTimerIrq (void)
{
	if (RPi_CpuId.PartNumber != 0xB76)								// ARM6 cpu is single core and this clock does not exist
	{
		QA7->TimerClearReload.IntClear = 1;							// Clear interrupt
		QA7->TimerClearReload.Reload = 1;							// Reload now
		return true;												// Timer irq successfully cleared on BCM2836, BCM2837
	}
	return false;													// QA7 does not exist on BCM2835
}

/*-[ LocalTimerSetup ]------------------------------------------------------}
. Sets the clock rate to the period in usec for the local clock timer.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. All cores share this clock so setting it from any core changes all cores.
. On BCM2835 (Pi1 ARM6) it does not have core timer so call fails.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerSetup (uint32_t period_in_us)						// Period between timer interrupts in usec
{
	if ((RPi_CpuId.PartNumber != 0xB76) & (period_in_us <= 0xAAAAAA) )	// ARM6 cpu is single core and this clock does not exist
	{
		uint32_t divisor = 384 * period_in_us;						// Multiply the period * 384
		divisor /= 10;												// That is divisor required as clock is (2 * 19.2Mhz)
		QA7->TimerControlStatus.ReloadValue = divisor;				// Timer period set
		QA7->TimerControlStatus.TimerEnable = 1;					// Timer enabled
		QA7->TimerClearReload.Reload = 1;							// Reload now
		return true;												// Timer successfully setup on BCM2836, BCM2837
	}
	return false;													// QA7 does not exist on BCM2835
}

/*-[ LocalTimerIrqSetup ]---------------------------------------------------}
. The local timer irq interrupt rate is set to the period in usec between
. triggers. On BCM2835 (ARM6) it does not have core timer so call fails.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerIrqSetup (uint32_t period_in_us,						// Period between timer interrupts in usec
						 uint8_t coreNum,							// Core number
						 bool secureMode)							// Secure mode or not
{
	if ((coreNum <= RPi_CoresReady) && LocalTimerSetup(period_in_us))// Peripheral time set successful
	{
		QA7->TimerRouting.Routing = LOCALTIMER_TO_CORE0_IRQ + coreNum;// Route local timer IRQ to given Core
		QA7->TimerControlStatus.IntEnable = 1;						// Timer IRQ enabled
		QA7->TimerClearReload.IntClear = 1;							// Clear interrupt
		QA7->TimerClearReload.Reload = 1;							// Reload now
		if (secureMode)
		{
			QA7->CoreTimerIntControl[coreNum].nCNTPSIRQ_IRQ = 1;	// We are in secure EL1 so enable IRQ to core
			QA7->CoreTimerIntControl[coreNum].nCNTPSIRQ_FIQ = 0;	// Make sure secure EL1 FIQ is zero
		}
		else {
			QA7->CoreTimerIntControl[coreNum].nCNTPNSIRQ_IRQ = 1;	// We are in Non Secure EL1 so enable IRQ to core
			QA7->CoreTimerIntControl[coreNum].nCNTPNSIRQ_FIQ = 0;	// Make sure no secure FIQ is zero
		}
		return true;												// Return success									
	}
	return false;													// Return failure	
}

/*-[ LocalTimerFiqSetup ]---------------------------------------------------}
. The local timer fiq interrupt rate is set to the period in usec between
. triggers. On BCM2835 (ARM6) it does not have core timer so call fails.
. Largest period is 11,184,810 usec (11 sec) because of multiply & divid
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool LocalTimerFiqSetup (uint32_t period_in_us,						// Period between timer interrupts in usec
						 uint8_t coreNum,							// Core number
						 bool secureMode)							// Secure mode or not
{
	if ((coreNum <= RPi_CoresReady) && LocalTimerSetup(period_in_us))// Peripheral time set successful
	{
		QA7->TimerRouting.Routing = LOCALTIMER_TO_CORE0_FIQ + coreNum;// Route local timer FIQ to given Core
		QA7->TimerControlStatus.IntEnable = 1;						// Timer IRQ enabled
		QA7->TimerClearReload.IntClear = 1;							// Clear interrupt
		QA7->TimerClearReload.Reload = 1;							// Reload now
		if (secureMode)
		{
			QA7->CoreTimerIntControl[coreNum].nCNTPSIRQ_FIQ = 1;	// We are in secure EL1 so enable FIQ to core
			QA7->CoreTimerIntControl[coreNum].nCNTPSIRQ_IRQ = 0;	// Make sure secure IRQ is zero
		}
		else {
			QA7->CoreTimerIntControl[coreNum].nCNTPNSIRQ_FIQ = 1;	// We are in NS EL1 so enable FIQ to core
			QA7->CoreTimerIntControl[coreNum].nCNTPNSIRQ_IRQ = 0;	// Make sure none secure IRQ is zero
		}
		return true;												// Return success									
	}
	return false;													// Return failure	
}

/*==========================================================================}
{					 CORE MAILBOX API ROUTINES								}
{==========================================================================*/

/*-[ SendCoreMessage ]------------------------------------------------------}
. Send a message to the request core on one of it's four mailboxes (0..3).
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool SendCoreMessage (uint32_t msg,									// Message to send core
					  uint8_t coreNum,								// Core number
					  uint8_t mailbox)								// Mailbox number of core
{
	if ((coreNum <= RPi_CoresReady) && (mailbox < 4))				// Core number and mailbox valid
	{
		QA7->CoreMailbox_Write[coreNum].boxNumber[mailbox] = msg;	// Write the message
		return true;												// Return success									
	}
	return false;													// Return failure	
}

/*-[ ReadCoreMessage ]------------------------------------------------------}
. Read a message from the request core on one of it's four mailboxes (0..3).
. You can poll but typically this is  in response to IRQ or FIQ from mailbox
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool ReadCoreMessage (uint32_t* msg,								// Pointer to read result
					  uint8_t coreNum,								// Core number
					  uint8_t mailbox)								// Mailbox number of core
{
	if (msg && (coreNum <= RPi_CoresReady) && (mailbox < 4))		// Check Msg pointer, Core number and mailbox valid
	{
		uint32_t t;
		t = QA7->CoreMailbox_Read_Clear[coreNum].boxNumber[mailbox];// Read the message
		QA7->CoreMailbox_Read_Clear[coreNum].boxNumber[mailbox] = t;// Clear the message
		msg[0] = t;													// Return the read value
		if (t != 0)	return true;									// Return success									
	}
	return false;													// Return failure	
}

/*==========================================================================}
{					 CORE MAILBOX FIQ API ROUTINES							}
{==========================================================================*/

/*-[ CoreMailboxFiqSetup ]--------------------------------------------------}
. Sets the core mailbox FIQ to call the given routine at the address.
. RETURN: TRUE if successful, FALSE for any failure
.--------------------------------------------------------------------------*/
bool CoreMailboxFiqSetup (void (*ARMaddress) (void),				// Address of FIQ handler
						  uint8_t coreNum,							// Core number
						  uint8_t mailbox)							// Mailbox
{
	if ((coreNum <= RPi_CoresReady) && (mailbox < 4))				// Check Core number and mailbox valid
	{
		setFiqFuncAddress(ARMaddress);								// Set the FIQ address
		QA7->CoreMailbox_Read_Clear[coreNum].boxNumber[mailbox] = 0xFFFFFFFF; // Make sure mailbox clear
		QA7->CoreMailboxIntControl[coreNum].FIQ_Routing = (1 << mailbox);// Route mailbox FIQ to given Core
		QA7->CoreMailboxIntControl[coreNum].IRQ_Routing = 0;		// Make sure Route mailbox IRQ to given Core is zero
		return true;												// Return success
	}
	return false;													// Return failure	
}

