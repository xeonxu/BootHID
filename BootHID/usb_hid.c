/*******************************************************************************
 * File Name	: usb_hid.c
 * Project	: BootHID
 * Date		: 2014/03/24
 * Version      : 1.0
 * Target MCU   : AVR
 * Tool Chain   : Atmel AVR Studio 4.19 730, avr-gcc, avr-libc 1.8.0
 * Author       : "Detlef Mueller" <detlef@gmail.com>
 * Release Notes:
 *	This boot loader is compatible to Objective Developments V-USB based
 *	BootloadHID (see http://www.obdev.at/products/vusb/bootloadhid.html)
 *	but uses the native Atmel USB module included in certain AVR8 chips.
 *	Currently the code supports the following chips (note that not all are
 *	tested !):
 *
 *	AT90usb[82 | 162 | 646 | 1286]		@ 8 or 16MHz
 *	ATMega[8U2 | 16U2 | 32U2 | 16U4 | 32U4]	@ 8 or 16MHz
 *
 *	The loader compiles to less than 1kB, making it an attractive alternative
 *	to Atmels default DFU loader (4kB), esp. with the 8 or 16kB chips.
 *	The host application to upload new application firmware is available as
 *	source code (GPL, see link above) and binaries exist for a variety of
 *	operating systems.
 *
 *	As-is, the boot loader gets invoked by either directly calling it from
 *	the application or via a manual reset while /HWB is pulled low (HWBE
 *	fuse programmed.) If you want to always start the boot loader after a
 *	reset or power-up (BOOTRST fuse programmed), main() needs to be expanded
 *	to check for an activation condition and move the interrupt vectors to
 *	the application section before starting the application if the condition
 *	is not met. Keep it simple so your extensions won't grow the boot loader
 *	size above 1kB (or use a 2kB boot section if they do.) Check BootHID.map
 *	for the .text segment size.
 *
 *	To configure the boot loader for a specific chip, proceed as follows:
 *	- select your chip (Device) and oscillator speed (Frequency) in the
 *	  dialog under Project --> Configuration Options --> General.
 *	  The frequencies supported are 8000000 or 16000000. In AVRS 4.19 the
 *	  ATMega16U2 is not included in the Device selection, you have to
 *	  manually fill in "atmega16u2" (without the quotes.)
 *	- enter the boot loader base address under Project --> Configuration
 *	  Options --> Memory Settings. Double click on the existing entry to
 *	  edit it and change the Address (Hex) field to the desired value.
 *	  Note that this address is a word address ! For a 1kB boot section,
 *	  the following addresses are valid:
 *	  For a chip with: 8kB flash - 0x0e00, 16kB flash - 0x1e00,
 *	  32kB flash - 0x3e00, 64kB flash - 0x7e00, 128kB flash - 0xfe00
 *	  Change the "e" in the addresses above to "c" for a 2kB boot section.
 *	- last (but not least), to hook in the customized vector table we need
 *	  to use a custum linker skript. Open Project --> Configuration Options
 *	  --> Custom Options and select [Linker Options]. Due to a bug in AVRS
 *	  4.19 there will be two entries ("-T" and "..\avr5b.x"), in reality
 *	  this is only one option. If you need to change it, remove the two
 *	  entries, type in the new option and press Add followed by OK.
 *	  Depending on your chip, use one of the folling options (w/o quotes):
 *	   "-T ..\avr5.x"  - ATMega[16U4|32U4], AT90usb646
 *	   "-T ..\avr51.x" - AT90usb1286
 *	   "-T ..\avr35.x" - ATMega[8U2|16U2|32U2], AT90usb[82|162]
 *	  The Custom Options dialog also configures the locations of avr-gcc
 *	  and make. Make sure the entries match your tool chain configuration.
 *
 *	To finally compile the boot loader, execute Build --> Build. If all
 *	goes well you will find BootHID.hex in the .\default directory.
 *
 *	Enjoy ! DM, 2014/04/26
 *
 * $Id$
 ******************************************************************************/

#include <includes.h>
#include <avr/boot.h>

#define __usb_hid__
#include "usb_hid.h"

//------------------------------------------------------------------------------

// Configurable Options

#define VENDOR_ID		0x16C0		// Objective Development
#define PRODUCT_ID		0x05DF		// Boot loader

#define VENDOR_NAME		'o', 'b', 'd', 'e', 'v', '.', 'a', 't'
#define VENDOR_NAME_SZ		8

#define DEVICE_NAME		'H', 'I', 'D', 'B', 'o', 'o', 't'
#define DEVICE_NAME_SZ		7

#define	USE_LED			1

#define LED_CONFIG()		set_bit(  DDRD, PD5 )
#define LED_ON()		clr_bit( PORTD, PD5 )
#define LED_OFF()		set_bit( PORTD, PD5 )
#define LED_TOG()		tog_bit( PORTD, PD5 )

//------------------------------------------------------------------------------
// Endpoint Buffer Configuration

#define ENDPOINT0_SIZE		64

//------------------------------------------------------------------------------

typedef 
  #if FLASHEND > 0xFFFF
    uint32_t
  #else
    uint16_t
  #endif
	addr_t ;

//------------------------------------------------------------------------------
// Descriptor Data

static const uint8_t
    device_descriptor[] PROGMEM =
    {
	18,				// bLength
	1,				// bDescriptorType
	LVAL(0x0110),			// bcdUSB
	0,				// bDeviceClass
	0,				// bDeviceSubClass
	0,				// bDeviceProtocol
	ENDPOINT0_SIZE,			// bMaxPacketSize0
	LVAL(VENDOR_ID),		// idVendor
	LVAL(PRODUCT_ID),		// idProduct
	LVAL(0x0100),			// bcdDevice
	1,				// iManufacturer
	2,				// iProduct
	0,				// iSerialNumber
	1				// bNumConfigurations
    } ;

static const uint8_t
    hid_report_descriptor[] PROGMEM =
    {
	0x06, 0x00, 0xFF,		// USAGE_PAGE (Generic Desktop)
	0x09, 0x01,			// USAGE (Vendor Usage 1)
	0xA1, 0x01,			// COLLECTION (Application)
	0x15, 0x00,			//   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,		//   LOGICAL_MAXIMUM (255)
	0x75, 0x08,			//   REPORT_SIZE (8)

	0x85, 0x01,			//   REPORT_ID (1)
	0x95, 0x06,			//   REPORT_COUNT (6)
	0x09, 0x00,			//   USAGE (Undefined)
	0xB2, 0x02, 0x01,		//   FEATURE (Data,Var,Abs,Buf)

	0x85, 0x02,			//   REPORT_ID (2)
	0x95, 0x83,			//   REPORT_COUNT (131)
	0x09, 0x00,			//   USAGE (Undefined)
	0xB2, 0x02, 0x01,		//   FEATURE (Data,Var,Abs,Buf)
	0xC0				// END_COLLECTION
    } ;

#define CONFIG_DESC_SIZE	(9 + 9 + 9 + 7)
#define HID_DESC_OFFSET		(9 + 9)

#if __CD_NOT_IN_VT

// config_descriptor is stored in vector table !!

static const uint8_t
    config_descriptor[CONFIG_DESC_SIZE] PROGMEM =
    {
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 				// bLength;
	2,				// bDescriptorType;
	LVAL(CONFIG_DESC_SIZE),		// wTotalLength
	1,				// bNumInterfaces
	1,				// bConfigurationValue
	0,				// iConfiguration
	0xC0,				// bmAttributes (selfpowered)
	50,				// bMaxPower
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,				// bLength
	4,				// bDescriptorType
	0,				// bInterfaceNumber
	0,				// bAlternateSetting
	1,				// bNumEndpoints
	0x03,				// bInterfaceClass (0x03 = HID)
	0x00,				// bInterfaceSubClass
	0x00,				// bInterfaceProtocol
	0,				// iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,				// bLength
	0x21,				// bDescriptorType
	LVAL(0x0111),			// bcdHID
	0,				// bCountryCode
	1,				// bNumDescriptors
	0x22,				// bDescriptorType
	LVAL(sizeof(hid_report_descriptor)),// wDescriptorLength
	7,				// Size of this descriptor in bytes (7)
	5,				// Endpoint (0x05)
	0x81,				// Endpoint address
	3,				// Endpoint attributes
	LVAL( 64 ),			// Max. packet size this EP can transfer
	200				// Polling interval for this EP in ms
    } ;
#else

extern const uint8_t
    config_descriptor[CONFIG_DESC_SIZE] PROGMEM ;

#endif

static const uint16_t
    strVendor[] PROGMEM = { (VENDOR_NAME_SZ * 2 + 2) | (3 << 8), VENDOR_NAME },
    strDevice[] PROGMEM = { (DEVICE_NAME_SZ * 2 + 2) | (3 << 8), DEVICE_NAME } ;

static const struct
    {
	uint8_t  repid ;
	uint16_t pagesz ;
	uint32_t flashsz ;
    }
    report1 PROGMEM =
    {
	1,
	SPM_PAGESIZE,
	(uint32_t)FLASHEND + 1
    } ;

//------------------------------------------------------------------------------
// Variables

static volatile uint8_t
    VA_NOINIT( hid_report[132] ) ;

// zero when we are not configured, non-zero when enumerated

static volatile uint8_t
    VA_NOINIT( usb_configuration ) ;

static union
    {
	addr_t   a ;
	uint8_t  b[sizeof(addr_t)] ;
    }
    VA_NOINIT( addr ) ;

//------------------------------------------------------------------------------
// Initialize USB

void usb_init ( void )
{
   UHW_CONFIG() ;

    // configure the LED

  #if USE_LED
    LED_CONFIG() ;
    LED_OFF() ;
  #endif

    MCUCR = _B1(IVCE) ;			// enable change of interrupt vectors
    MCUCR = _B1(IVSEL) ;		// move interrupts to boot flash section

    // fire up USB

    USBCON = _B1(USBE) | _B1(FRZCLK) ;	// enable USB

    // config PLL, wait for PLL lock

    for ( PLL_CONFIG() ; bit_is_clear( PLLCSR, PLOCK ) ; )
	;

    USB_CONFIG() ;			// start USB clock

    UDCON = 0 ;				// enable attach resistor
    UDIEN = _B1(EORSTE) ;

    sei() ;				// enable INTs
}

//------------------------------------------------------------------------------
// USB Device Interrupt

ISR( USB_GEN_vect, ISR_NAKED )
{
    uint8_t
	i ;

    i = UDINT ;
    UDINT = 0 ;

    if ( i & _BV( EORSTI ) )
    {
	UENUM   = 0 ;
	UECONX  = 1 ;
	UECFG0X = EP_TYPE_CONTROL ;
	UECFG1X = EP_SIZE( ENDPOINT0_SIZE ) | EP_SINGLE_BUFFER ;
	UEIENX  = _BV( RXSTPE ) ;

	usb_configuration = 0 ;
    }

    reti() ;
}

//------------------------------------------------------------------------------
// Misc functions to wait for ready and send/receive packets

static inline void usb_wait_in_ready ( void )
{
    for ( ; bit_is_clear( UEINTX, TXINI ) ; )
	;
}

static inline void usb_send_in ( void )
{
    UEINTX = ~_BV( TXINI ) ;
}

static inline void usb_wait_receive_out ( void )
{
    for ( ; bit_is_clear( UEINTX, RXOUTI ) ; )
	;
}

static inline void usb_ack_out ( void )
{
    UEINTX = ~_BV( RXOUTI ) ;
}

//------------------------------------------------------------------------------
// Send data block from flash via EP0

static void FA_NOINLINE( usb_send_EP0 ) ( const uint8_t *p, uint8_t len )
{
    uint8_t
	i, n ;

    do
    {
	// wait for host ready for IN packet

	do
	{
	    i = UEINTX ;
	}
	while ( bits_are_clear( i, _BV( TXINI ) | _BV( RXOUTI ) ) ) ;

	if ( i & _BV( RXOUTI ) )
	    return ;				// abort

	// send IN packet

	n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE ;

	for ( i = n ; i-- ; )
	    UEDATX = pgm_read_byte( p++ ) ;

	usb_send_in() ;

	len -= n ;
    }
    while ( len || n == ENDPOINT0_SIZE ) ;
}

//------------------------------------------------------------------------------
// Pass control to main app

static inline void FA_NORETURN( exit_bl ) ( void )
{
    UDIEN  = 0 ;
    UDCON  = _B1(DETACH) ;		// Detach from USB
    USBCON = _B0(USBE) | _B1(FRZCLK) ;	// Stop USB module

    boot_rww_enable() ;			// Enable rd-while-wr memory section

    MCUCR = _B1(IVCE) ;			// enable change of interrupt vectors
    MCUCR = _B0(IVSEL) ;		// move interrupts to application flash section

    _delay_loop_2( (uint16_t)MS2TM( 15, 4 ) ) ;

    RESET() ;

    for ( ;; )			// Shut up gcc
	;
}

//------------------------------------------------------------------------------
// USB Endpoint Interrupt - endpoint 0 is handled here.

ISR( USB_COM_vect, ISR_NAKED )
{
    uint8_t
	bmRequestType, bRequest,
	i, n ;
    uint16_t
	wValue, wIndex, wLength ;
    uint8_t
	*p ;

    UENUM = 0 ;
    i = UEINTX ;

    if ( i & _BV( RXSTPI ) )
    {
	bmRequestType = UEDATX ;
	bRequest      = UEDATX ;

	wValue   =  UEDATX ;
	wValue  |= (UEDATX << 8) ;

	wIndex   =  UEDATX ;
	wIndex  |= (UEDATX << 8) ;

	wLength  =  UEDATX ;
	wLength |= (UEDATX << 8) ;

	UEINTX = ~(_BV( RXSTPI ) | _BV( RXOUTI ) | _BV( TXINI )) ;

	if ( bRequest == GET_DESCRIPTOR )
	{
	    if ( (wValue >> 8) == 0x01 )
	    {
		p = VP( device_descriptor ) ;
		n = sizeof( device_descriptor ) ;
	    }
	    else
	    if ( (wValue >> 8) == 0x02 )
	    {
		p = VP( config_descriptor ) ;
		n = sizeof( config_descriptor ) ;
	    }
	    else
	    if ( (wValue >> 8) == 0x22 )
	    {
		p = VP( hid_report_descriptor ) ;
		n = sizeof( hid_report_descriptor ) ;
	    }
//	    else
//	    if ( (wValue >> 8) == 0x21 )
//	    {
//		p = VP( config_descriptor + HID_DESC_OFFSET ) ;
//		n = 9 ;
//	    }
	    else
	    if ( wValue == USBLV( 0x03, 1 ) )
	    {
		p = VP( strVendor ) ;
		n = sizeof( strVendor ) ;
	    }
	    else
	    if ( wValue == USBLV( 0x03, 2 ) )
	    {
		p = VP( strDevice ) ;
		n = sizeof( strDevice ) ;
	    }
	    else
		goto _Stall ;

	    if ( wLength & 0xFF00 )
		i = 255 ;
	    else
		i = wLength ;

	    if ( i > n )
		i = n ;

	    usb_send_EP0( p, i ) ;

	    reti() ;
	}

	if ( bRequest == SET_ADDRESS )
	{
	    usb_send_in() ;			// Ack via ZLP

	    usb_wait_in_ready() ;		// Wait until sent

	    UDADDR = wValue | _BV( ADDEN ) ;	// Set & enable USB address

	    reti() ;
	}

	if ( bRequest == SET_CONFIGURATION && bmRequestType == 0 )
	{
	    usb_send_in() ;			// Ack

	    usb_configuration = wValue ;

	    UENUM  = 1 ;
	    UECONX = _B1(EPEN) ;
	    UECFG0X = EP_TYPE_INTERRUPT_IN ;
	    UECFG1X = (_B0(EPSIZE2) | _B1(EPSIZE1) | _B1(EPSIZE0)) | EP_SINGLE_BUFFER ;
	    UERST = 0x7E ;
	    UERST = 0 ;

	    reti() ;
	}

	if ( bRequest == GET_CONFIGURATION && bmRequestType == 0x80 )
	{
	    usb_wait_in_ready() ;

	    UEDATX = usb_configuration ;

	    usb_send_in() ;

	    reti() ;
	}

	if ( bRequest == HID_SET_IDLE && bmRequestType == 0x21 )
	{
	    usb_send_in() ;                     // Ack

	    // MSB(wValue): duration in 4ms clicks, 0 for indefinite
	    // LSB(wValue): report ID
	    // wIndex: interface

	    reti() ;
	}

	if ( bRequest == HID_SET_REPORT && bmRequestType == 0x21 )
	{
	  #if USE_LED
	    LED_ON() ;
	  #endif

	    if ( (wValue & 0xFF) == 1 )		// Got #1 report, signal to reboot
		exit_bl() ;

	    // wValue contains report type (h) and id (l), see HID 1,11, 7.2.2
	    // type shoud be "feature" (3), id should be 1 or 2
	    // wLength should be sizeof( report )

	    n = wLength ;
	    p = VP( hid_report ) ;

	    do
	    {
		usb_wait_receive_out() ;

		if ( n > ENDPOINT0_SIZE )
		{
		    i  = ENDPOINT0_SIZE ;
		    n -= ENDPOINT0_SIZE ;
		}
		else
		{
		    i = n ;
		    n = 0 ;
		}

		for ( ; i-- ; )
		    *p++ = UEDATX ;

		usb_ack_out() ;			// Ack OUT packet
	    }
	    while ( n ) ;			// what if last == ZLP ? (Shouldn't happen)

	    usb_send_in() ;			// Ack transfer via ZLP

	    addr.b[0] = hid_report[1] ;
	    addr.b[1] = hid_report[2] ;
	  #if FLASHEND > 0xFFFF
	    addr.b[2] = hid_report[3] ;
	    addr.b[3] = 0 ;
	  #endif

	    p = VP( hid_report + 4 ) ;		// Skip ID & 3-byte address

	    for ( i = (sizeof( hid_report ) - 4) >> 1 ; i-- ; )
	    {
		if ( ! (addr.a & (SPM_PAGESIZE - 1)) )
		{				// if page start: erase
		    boot_page_erase( addr.a ) ;	// erase page
		    boot_spm_busy_wait() ;	// wait until page is erased
		}

		boot_page_fill( addr.a, *(uint16_t *)p ) ;

		p += sizeof( uint16_t ) ;

		// write page when we cross page boundary

		addr.a += sizeof( uint16_t ) ;

		if ( ! (addr.a & (SPM_PAGESIZE - 1)) )
		{
		    boot_page_write( addr.a - sizeof( uint16_t ) ) ;
		    boot_spm_busy_wait() ;
		}
            }

	  #if USE_LED
	    LED_OFF() ;
	  #endif

	    reti() ;
	}

	if ( bRequest == HID_GET_REPORT && bmRequestType == 0xA1 && (wValue & 0xFF) == 1 )
	{
	    usb_send_EP0( VP( &report1 ), sizeof( report1 ) ) ;

	    reti() ;
	}
    }

_Stall:
    UECONX = _BV( STALLRQ ) | _BV( EPEN ) ;	// stall
    reti() ;
}

//------------------------------------------------------------------------------
// End of usb_hid.c
