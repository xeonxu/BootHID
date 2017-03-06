/*******************************************************************************
 * File Name	: includes.h
 * Project	: Generic AVR based
 * Date		: 2014/01/30
 * Version      : 1.2
 * Target MCU   : AT90USB8/162, ATMEGA16/32U4/U2, AT90USB64/1286
 * Tool Chain   : Atmel AVR Studio 4.19 730, avr-gcc, avr-libc
 * Author       : "Detlef Mueller" <detlef@gmail.com>
 * Release Notes:
 *
 * $Id$
 ******************************************************************************/

#ifndef	__includes_h__
#define	__includes_h__

//------------------------------------------------------------------------------

#define __FIND_BL_ADDR

//------------------------------------------------------------------------------

#if defined(__AVR_AT90USB162__) || defined(__AVR_AT90USB82__)
 #define __AVR_AT90USBX2__
#elif defined(__AVR_ATmega16U4__) || defined(__AVR_ATmega32U4__)
 #define __AVR_ATmegaXU4__
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
 #define __AVR_AT90USBX6__
#elif defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega32U2__)
 #define __AVR_ATmegaXU2__
#else
 #error "Unsupported device"
#endif

// #define __PROG_TYPES_COMPAT__

//------------------------------------------------------------------------------

#ifdef	__ASSEMBLER__

//------------------------------------------------------------------------------

#define	__SFR_OFFSET	0
//#define	_VECTOR(N)	__vector_ ## N	/* io.h does not define this for asm */

#include <avr/io.h>

//------------------------------------------------------------------------------

#else  // ! __ASSEMBLER__

//------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include <util/delay_basic.h>

//------------------------------------------------------------------------------

#define	VP( p )			(void *)(p)
#define ARRSZ( a )		(sizeof(a) / sizeof(*(a)))

#ifndef	FALSE
 #define FALSE			0
#endif

#ifndef	TRUE
 #define TRUE			(! FALSE)
#endif

#ifndef NULL
 #define NULL			0
#endif

//-------------------------------------------------------------------------------
// Things not defined in iom32u4.h

#if defined(__AVR_ATmegaXU4__)

#define	PRTIM4		4	/* Power reduction register bit 4 for Timer 4 */

#define	PB7		PORTB7
#define	PB6		PORTB6
#define	PB5		PORTB5
#define	PB4		PORTB4
#define	PB3		PORTB3
#define	PB2		PORTB2
#define	PB1		PORTB1
#define	PB0		PORTB0

#define	PC7		PORTC7
#define	PC6		PORTC6

#define	PD7		PORTD7
#define	PD6		PORTD6
#define	PD5		PORTD5
#define	PD4		PORTD4
#define	PD3		PORTD3
#define	PD2		PORTD2
#define	PD1		PORTD1
#define	PD0		PORTD0

#define	PE6		PORTE6
#define	PE2		PORTE2

#define	PF7		PORTF7
#define	PF6		PORTF6
#define	PF5		PORTF5
#define	PF4		PORTF4
#define	PF1		PORTF1
#define	PF0		PORTF0

#endif

//------------------------------------------------------------------------------
// Missing in power.h for the ATmegaXU2 in avr-libc 1.6.7 (WinAVR 20100110)

#if defined(__AVR_ATmegaXU2__)

 #if ! defined(clock_prescale_get)

typedef enum
    {
	clock_div_1   = 0, clock_div_2   = 1,
	clock_div_4   = 2, clock_div_8   = 3,
	clock_div_16  = 4, clock_div_32  = 5,
	clock_div_64  = 6, clock_div_128 = 7,
	clock_div_256 = 8
    }
    clock_div_t ;

#define clock_prescale_set( x )			\
	do {					\
		uint8_t tmp = _BV(CLKPCE) ;	\
		__asm__ __volatile__ (		\
		"in __tmp_reg__,__SREG__" "\n\t"\
		"cli" "\n\t"			\
		"sts %1, %0" "\n\t"		\
		"sts %1, %2" "\n\t"		\
		"out __SREG__, __tmp_reg__"	\
		: /* no outputs */		\
		: "d" (tmp),			\
		  "M" (_SFR_MEM_ADDR(CLKPR)),	\
		  "d" (x)			\
		: "r0" ) ;			\
	} while (0)

#define clock_prescale_get()			\
	(clock_div_t)(CLKPR & (uint8_t)		\
	((1<<CLKPS0) | (1<<CLKPS1) |		\
	 (1<<CLKPS2) | (1<<CLKPS3)) )
 #endif
#endif

//------------------------------------------------------------------------------

#define set_bit( sfr, bit )	(_SFR_BYTE(sfr) |=  _BV(bit))
#define clr_bit( sfr, bit )	(_SFR_BYTE(sfr) &= ~_BV(bit))
#define tog_bit( sfr, bit )	(_SFR_BYTE(sfr) ^=  _BV(bit))

#define set_bits( sfr, msk )	(_SFR_BYTE(sfr) |=  (msk))
#define clr_bits( sfr, msk )	(_SFR_BYTE(sfr) &= ~(msk))
#define tog_bits( sfr, msk )	(_SFR_BYTE(sfr) ^=  (msk))

#define	bits_are_set( n, msk )	 (((n) & (msk)) == (msk))
#define	bits_are_clear( n, msk ) (!((n) & (msk)))

//------------------------------------------------------------------------------

#define	__FA__( _func, ... )	__attribute__((__VA_ARGS__)) _func

#define	FA_NAKED( _f )		__FA__( _f, __naked__ )
#define	FA_NORETURN( _f )	__FA__( _f, __noreturn__ )
#define	FA_NOINLINE( _f )	__FA__( _f, __noinline__ )
#define	FA_NOINRET( _f )	__FA__( _f, __noinline__,__noreturn__ )
#define	FA_INIT3( _f )		__FA__( _f, __used__,__naked__,__section__(".init3") )

#define	VA_PROGMEM( _v )	_v __attribute__((__progmem__))
#define	VA_NOINIT( _v )		_v __attribute__((__section__(".noinit")))

#ifdef __PROG_TYPES_COMPAT__
 #define TA_PROGMEM( _t )	PROGMEM _t
 #warning "Deprecated since the usage of PROGMEM on a type is not supported in GCC"
#else
 #define TA_PROGMEM( _t )	_t
#endif

//------------------------------------------------------------------------------

#define	__WRAP__( __c )		do __c while (0)

//------------------------------------------------------------------------------

#define	CRITICAL_VAR()		uint8_t __sSREG
#define	ENTER_CRITICAL()	__WRAP__( { __sSREG = SREG ; cli() ; } )
#define	EXIT_CRITICAL()		(SREG = __sSREG)
#define	EXIT_CRITICAL_RET( n )	__WRAP__( { SREG = __sSREG ; return ( n ) ; } )

//------------------------------------------------------------------------------

#define	JMP( _a )	__asm__ __volatile__ ( "jmp %[_ad]\n\t" :: [_ad] "i" (_a) )

#define	RESET()		JMP( 0 )

#define	RET()		__asm__ __volatile__ ( "ret" "\n\t" :: )
#define	SLEEP()		__asm__ __volatile__ ( "sleep" "\n\t" :: )
#define	NOP()		__asm__ __volatile__ ( "nop" "\n\t" :: )
#define	NOP2()		__asm__ __volatile__ ( "rjmp .+0" "\n\t" :: )

//------------------------------------------------------------------------------

static inline uint8_t
	__FA__( ror8, __always_inline__ ) ( uint8_t _v )
{
    __asm__ __volatile__
    (
	"bst    %0,0"	"\n\t"
	"lsr    %0"	"\n\t"
	"bld    %0,7"	"\n\t"
	: "+r" (_v)
	: "0" (_v)
	: "cc"
    ) ;

    return ( _v ) ;
}

static inline uint16_t
	__FA__( ror16, __always_inline__ ) ( uint16_t _v )
{
    __asm__ __volatile__
    (
	"bst    %A0,0"	"\n\t"
	"ror    %B0"	"\n\t"
	"bld    %B0,7"	"\n\t"
	"ror    %A0"	"\n\t"
	: "+r" (_v)
	: "0" (_v)
	: "cc"
    ) ;

    return ( _v ) ;
}

static inline uint8_t
	__FA__( rol8, __always_inline__ ) ( uint8_t _v )
{
    __asm__ __volatile__
    (
	"bst    %0,7"	"\n\t"
	"lsl    %0"	"\n\t"
	"bld    %0,0"	"\n\t"
	: "+r" (_v)
	: "0" (_v)
	: "cc"
    ) ;

    return ( _v ) ;
}

static inline uint16_t
	__FA__( rol16, __always_inline__ ) ( uint16_t _v )
{
    __asm__ __volatile__
    (
	"bst    %B0,7"	"\n\t"
	"rol    %A0"	"\n\t"
	"bld    %A0,0"	"\n\t"
	"rol    %B0"	"\n\t"
	: "+r" (_v)
	: "0" (_v)
	: "cc"
    ) ;

    return ( _v ) ;
}

static inline uint8_t
	__FA__( _bv8, __always_inline__ ) ( uint8_t _v )
{
    __asm__ __volatile__
    (
	"clr  __tmp_reg__"		"\n\t"
	"inc  __tmp_reg__"		"\n\t"
	"rjmp 2f"			"\n\t"
	"1:"				"\n\t"
	"add  __tmp_reg__,__tmp_reg__"	"\n\t"
	"2:"				"\n\t"
	"dec  %0"			"\n\t"
	"brpl 1b"			"\n\t"
	"mov  %0,__tmp_reg__"		"\n\t"
	: "+r" (_v)
	: "0" (_v)
	: "cc"
    ) ;

    return ( _v ) ;
}

//------------------------------------------------------------------------------

#if defined(__FIND_BL_ADDR)

#include <avr/boot.h>

#define	_mBOOTSZ1	((uint8_t)~FUSE_BOOTSZ1)
#define	_mBOOTSZ0	((uint8_t)~FUSE_BOOTSZ0)
#define	_mBOOTSZ	(_mBOOTSZ1 | _mBOOTSZ0)

#define	_BL_ADDR( sz )	((unsigned long)FLASHEND -		\
			 (((unsigned long)FLASHEND > 0x8000u) ?	\
			  ((sz) << 1) : (sz)) + 1)

//typedef
//    void (*__jmp)( void ) __attribute__((__noreturn__)) ;
//
//#define __JMP( adr )  ((__jmp)(adr >> 1))()

static inline void
	__FA__( jmp_bootloader, __always_inline__,__noreturn__ ) ( void )
{
    uint8_t
	bls_sz = boot_lock_fuse_bits_get( GET_HIGH_FUSE_BITS ) & _mBOOTSZ ;

    if ( bls_sz == _mBOOTSZ )
	JMP( _BL_ADDR( 512 ) ) ;
    else
    if ( bls_sz == _mBOOTSZ1 )
	JMP( _BL_ADDR( 1024 ) ) ;
    else
    if ( bls_sz == _mBOOTSZ0 )
	JMP( _BL_ADDR( 2048 ) ) ;

    JMP( _BL_ADDR( 4096 ) ) ;

    for ( ;; ) ;			// Shut up GCC
}

#else

#if   defined(__AVR_ATmega16U2__)
 #define __BL_ADDR			0x3000		/* KeyCard */
#elif defined(__AVR_AT90USB162__)
 #define __BL_ADDR			0x3E00		/* Teensy 1.0 */
#elif defined(__AVR_ATmega32U4__)
 #define __BL_ADDR			0x7E00		/* Teensy 2.0 */
#elif defined(__AVR_AT90USB646__)
 #define __BL_ADDR			0xFC00		/* Teensy++ 1.0 */
#elif defined(__AVR_AT90USB1286__)
 #define __BL_ADDR			0x1FC00		/* Teensy++ 2.0 */
#endif 


static inline void
	__FA__( jmp_bootloader, __always_inline__,__noreturn__ ) ( void )
{
    JMP( __BL_ADDR ) ;

    for ( ;; ) ;			// Shut up GCC
}

#endif

//-------------------------------------------------------------------------------
// Timer prescaler settings

#define	TPS_1( n )	(_B0(CS ## n ## 2) | _B0(CS ## n ## 1) | _B1(CS ## n ## 0))
#define	TPS_8( n )	(_B0(CS ## n ## 2) | _B1(CS ## n ## 1) | _B0(CS ## n ## 0))
#define	TPS_64( n )	(_B0(CS ## n ## 2) | _B1(CS ## n ## 1) | _B1(CS ## n ## 0))
#define	TPS_256( n )	(_B1(CS ## n ## 2) | _B0(CS ## n ## 1) | _B0(CS ## n ## 0))
#define	TPS_1024( n )	(_B1(CS ## n ## 2) | _B0(CS ## n ## 1) | _B1(CS ## n ## 0))

//-------------------------------------------------------------------------------
// Macros to deal w/ timers

#define	ResetTM( n, del )	__WRAP__( {				\
					TCNT ## n = (del) ;		\
					TIFR ## n = _BV( TOV ## n ) ;	\
				} )

#define	TMexp( n )		bit_is_set( TIFR ## n, TOV ## n )

#define	TMocrfa( n )		bit_is_set( TIFR ## n, OCF ## n ## A )

#define	ResetOCRFA( n )		TIFR ## n = _BV( OCF ## n ## A )

#define	SetTMPS( n, ps )	TCCR ## n ## B = TPS_ ## ps ( n )

//-------------------------------------------------------------------------------
// Timer Macros, msec or usec to timer ticks. ps = prescaler value

#define MS2TM( ms, ps ) (int16_t)(((ms) * (F_CPU /    1000.)) / (ps) + .5)
#define US2TM( us, ps ) (int16_t)(((us) * (F_CPU / 1000000.)) / (ps) + .5)

//------------------------------------------------------------------------------

#endif // __ASSEMBLER__

//------------------------------------------------------------------------------

#define	_B1( b )		_BV(b)
#define	_B0( b )		0

//------------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------
