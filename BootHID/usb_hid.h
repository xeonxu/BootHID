/*******************************************************************************
 * File Name	: usb_hid.h
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

#ifndef __usb_hid_h__
#define __usb_hid_h__

void
    usb_init( void ) ;			// initialize everything

//------------------------------------------------------------------------------
// Everything below this point is only intended for usb_hid.c

#ifdef __usb_hid__

#if defined(__AVR_AT90USBX2__) || defined(__AVR_ATmegaXU2__)

 #define UHW_CONFIG()

 #if F_CPU == 8000000
  #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | \
				  _B0(PLLP2)  | _B0(PLLP1)   | _B0(PLLP0))
 #else
  #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | \
				  _B0(PLLP2)  | _B0(PLLP1)   | _B1(PLLP0))
 #endif

 #define USB_CONFIG()	(USBCON = _B1(USBE)   | _B0(FRZLK))

#elif defined(__AVR_AT90USBX6__)

 #define UHW_CONFIG()	(UHWCON = _B1(UVREGE) | \
				  _B1(UIMOD)  | _B0(UIDE)    | _B0(UVCONE))
 #define USB_CONFIG()	(USBCON = _B1(USBE)   | _B0(FRZCLK)  | \
 				  _B0(HOST)   | _B1(OTGPADE) | _B0(IDTE) | _B0(VBUSTE))

 #if F_CPU == 8000000
  #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | \
				  _B0(PLLP2)  | _B1(PLLP1)   | _B1(PLLP0))
 #else
  #if defined(__AVR_AT90USB646__)
   #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | \
				  _B1(PLLP2)  | _B1(PLLP1)   | _B0(PLLP0))
  #elif defined(__AVR_AT90USB1286__)
   #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | \
				  _B1(PLLP2)  | _B0(PLLP1)   | _B1(PLLP0))
  #endif
 #endif

#elif defined(__AVR_ATmegaXU4__)

 #define UHW_CONFIG()	(UHWCON = _B1(UVREGE))

 #if F_CPU == 8000000
  #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | _B0(PINDIV))
 #else
  #define PLL_CONFIG()	(PLLCSR = _B1(PLLE)   | _B0(PLOCK)   | _B1(PINDIV))
 #endif

 #define USB_CONFIG()	(USBCON = _B1(USBE)   | _B0(FRZCLK)  | \
				  _B1(OTGPADE)| _B0(VBUSTE))
#else
 #error "Current device not supported"
#endif

#define EP_TYPE_CONTROL		(_B0(EPTYPE1) | _B0(EPTYPE0) | _B0(EPDIR))
#define EP_TYPE_BULK_IN		(_B1(EPTYPE1) | _B0(EPTYPE0) | _B1(EPDIR))
#define EP_TYPE_BULK_OUT	(_B1(EPTYPE1) | _B0(EPTYPE0) | _B0(EPDIR))
#define EP_TYPE_INTERRUPT_IN	(_B1(EPTYPE1) | _B1(EPTYPE0) | _B1(EPDIR))
#define EP_TYPE_INTERRUPT_OUT	(_B1(EPTYPE1) | _B1(EPTYPE0) | _B0(EPDIR))
#define EP_TYPE_ISOCHRONOUS_IN	(_B0(EPTYPE1) | _B1(EPTYPE0) | _B1(EPDIR))
#define EP_TYPE_ISOCHRONOUS_OUT	(_B0(EPTYPE1) | _B1(EPTYPE0) | _B0(EPDIR))


#define EP_SINGLE_BUFFER	(_B0(EPBK1) | _B0(EPBK0) | _B1(ALLOC))
#define EP_DOUBLE_BUFFER	(_B0(EPBK1) | _B1(EPBK0) | _B1(ALLOC))

#define EP_SIZE( s )	((s) <=  8 ? (_B0(EPSIZE2) | _B0(EPSIZE1) | _B0(EPSIZE0)) : \
			((s) <= 16 ? (_B0(EPSIZE2) | _B0(EPSIZE1) | _B1(EPSIZE0)) : \
			((s) <= 32 ? (_B0(EPSIZE2) | _B1(EPSIZE1) | _B0(EPSIZE0)) : \
			/* 64 */     (_B0(EPSIZE2) | _B1(EPSIZE1) | _B1(EPSIZE0)))))

#define MAX_ENDPOINT		4

#define LSB( n )		(n & 0xFF)
#define MSB( n )		((n >> 8) & 0xFF)

#define LVAL( n )		LSB( n ),MSB( n )

#define USBLV( h, l )           (((uint16_t)(h) << 8) + (l))

// standard control endpoint request types

#define GET_STATUS		0
#define CLEAR_FEATURE		1
#define SET_FEATURE		3
#define SET_ADDRESS		5
#define GET_DESCRIPTOR		6
#define GET_CONFIGURATION	8
#define SET_CONFIGURATION	9
#define GET_INTERFACE		10
#define SET_INTERFACE		11

// HID (human interface device)

#define HID_GET_REPORT		1
#define HID_GET_PROTOCOL	3
#define HID_SET_REPORT		9
#define HID_SET_IDLE		10
#define HID_SET_PROTOCOL	11

#endif

//------------------------------------------------------------------------------
#endif
