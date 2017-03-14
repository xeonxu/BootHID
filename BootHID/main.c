/******************************************************************************
 * File Name	: main.c
 * Project	: BootHID
 * Date		: 2014/03/24
 * Version      : 1.0
 * Target MCU   : AVR
 * Tool Chain   : Atmel AVR Studio 4.19 730, avr-gcc, avr-libc
 * Author       : "Detlef Mueller" <detlef@gmail.com>
 * Release Notes:
 *
 * $Id$
 ******************************************************************************/

#include <includes.h>

#include "usb_hid.h"

#define MAGIC_BOOT_KEY             0xDC
//------------------------------------------------------------------------------
// Watchdog is not turned off by a reset, see avr-libc's wdt.h documentation

uint8_t VA_NOINIT( mcusr_cpy ) ;

/* Bootloader timeout timer */
// MAH 8/15/12- add this switch so timeouts work properly when the chip is running at 8MHz instead of 16.
#if F_CPU == 8000000 
#define EXT_RESET_TIMEOUT_PERIOD	375
#else
#define EXT_RESET_TIMEOUT_PERIOD  750
#endif

// MAH 8/15/12- make this volatile, since we modify it in one place and read it in another, we want to make
//  sure we're always working on the copy in memory and not an erroneous value stored in a cache somewhere.
volatile uint16_t Timeout = 0;
// MAH 8/15/12- added this for delay during startup. Did not use existing Timeout value b/c it only increments
//  when there's a sketch at the top of the memory.
volatile uint16_t resetTimeout = 0;

static uint8_t RunBootloader = 1;
/** Magic lock for forced application start. If the HWBE fuse is programmed and BOOTRST is unprogrammed, the bootloader
 *  will start if the /HWB line of the AVR is held low and the system is reset. However, if the /HWB line is still held
 *  low when the application attempts to start via a watchdog reset, the bootloader will re-start. If set to the value
 *  \ref MAGIC_BOOT_KEY the special init function \ref Application_Jump_Check() will force the application to start.
 */
uint16_t MagicBootKey = 0;

void FA_INIT3( Init3 ) ( void )
{
    mcusr_cpy = MCUSR ;			// If we need to examine reset reason
    MCUSR = 0 ;
    wdt_disable() ;
}

//uint16_t ctr = 0;
ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
	/* Reset counter */
	TCNT1H = 0;
	TCNT1L = 0;

	resetTimeout++;
}

//------------------------------------------------------------------------------
// Boot loader main

int FA_NORETURN( main ) ( void )
{
    clock_prescale_set( clock_div_1 ) ;

    // If the boot loader always starts (BOOTRST fuse programmed):

//    Configure for start_app_condition test here.
//
//    if ( start_app_condition )
//    {
//	Undo aboves configuration here if necessary
//
//	MCUCR = _B1(IVCE) ;		// enable change of interrupt vectors
//	MCUCR = _B0(IVSEL) ;		// move interrupts to application flash section
//	RESET() ;			// 
//    }

    OCR1AH = 0;
    OCR1AL = 250;
    TIMSK1 = (1 << OCIE1A);					// enable timer 1 output compare A match interrupt
    TCCR1B = ((1 << CS11) | (1 << CS10));	// 1/64 prescaler on timer 1 input

    /* If the reset source was the bootloader and the key is correct, clear it and jump to the application */
    if ((mcusr_cpy & (1 << EXTRF)) && (MagicBootKey != MAGIC_BOOT_KEY))
    {
	    MagicBootKey = MAGIC_BOOT_KEY;
	    
	    sei();
	    while (RunBootloader) 
	    {
		if (resetTimeout > EXT_RESET_TIMEOUT_PERIOD)
		    RunBootloader = 0;
	    }
	    cli();	    /* Clear the boot key and jump to the user application */
	    MagicBootKey = 0;
	    RunBootloader = 1;

	    // cppcheck-suppress constStatement
	    if( *(uint8_t *)0x0000 != 0xFF )
	    {
		((void (*)(void))0x0000)();
	    }
    }

    MagicBootKey = 0;
    usb_init() ;			// initialize USB

    // Set sleep mode to "idle", enable sleep

    SMCR = _B0(SM2) | _B0(SM1) | _B0(SM0) | _B1(SE) ;

    for ( ;; ) 				// Forever..
	sleep_cpu() ;
}

//------------------------------------------------------------------------------
// End of main.c
