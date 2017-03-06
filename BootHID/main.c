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

//------------------------------------------------------------------------------
// Watchdog is not turned off by a reset, see avr-libc's wdt.h documentation

// uint8_t VA_NOINIT( mcusr_cpy ) ;

void FA_INIT3( Init3 ) ( void )
{
//  mcusr_cpy = MCUSR ;			// If we need to examine reset reason
    MCUSR = 0 ;
    wdt_disable() ;
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

    usb_init() ;			// initialize USB

    // Set sleep mode to "idle", enable sleep

    SMCR = _B0(SM2) | _B0(SM1) | _B0(SM0) | _B1(SE) ;

    for ( ;; ) 				// Forever..
	sleep_cpu() ;
}

//------------------------------------------------------------------------------
// End of main.c
