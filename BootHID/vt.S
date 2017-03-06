.extern __init, __bad_interrupt, __vector_10, __vector_11, __vector_12
.global __vector_default, exit
.section .vectors.bl,"ax",@progbits

/* micro-jumptable, we are using just reset and USB vectors */
exit:
__vector_default:
	jmp	__init
#if __CD_NOT_IN_VT
	jmp	__bad_interrupt		/*  4 */
	jmp	__bad_interrupt		/*  8 */
	jmp	__bad_interrupt		/* 12 */
	jmp	__bad_interrupt		/* 16 */
	jmp	__bad_interrupt		/* 20 */
	jmp	__bad_interrupt		/* 24 */
	jmp	__bad_interrupt		/* 28 */
	jmp	__bad_interrupt		/* 32 */
	jmp	__bad_interrupt		/* 36 */
#else
.global config_descriptor

config_descriptor:
	.byte	9, 2			; CD bLength, bDescriptorType
	.word	9+9+9+7			; wTotalLength
	.byte	1, 1			; bNumInterfaces, bConfigurationValue
	.byte	0, 0xC0			; iConfiguration, bmAttributes
	.byte	50, 9			; bMaxPower, ID bLength
	.byte	4, 0			; bDescriptorType, bInterfaceNumber
	.byte	0, 1			; bAlternateSetting, bNumEndpoints
	.byte	3, 0			; bInterfaceClass, bInterfaceSubClass
	.byte	0, 0			; bInterfaceProtocol, iInterface
	.byte	9, 0x21			; HD bLength, bDescriptorType
	.word	0x0111			; bcdHID
	.byte	0, 1			; bCountryCode, bNumDescriptors
	.byte	0x22			; bDescriptorType
	.word	33			; wDescriptorLength
	.byte	7			; ED bLength
	.byte	5, 0x81			; bDescriptorType, bmAddress
	.byte	3			; bmAttributes
	.word	64			; wPacketSize
	.byte	200			; bPollInterval
	.word	0
#endif
#if defined(__AVR_ATmega16U4__) || defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB647__) || defined(__AVR_AT90USB1287__)
	jmp	__vector_10
	rjmp	__vector_11
#elif defined(__AVR_AT90USB82__) || defined(__AVR_AT90USB162__) || defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega32U2__)
	jmp	__bad_interrupt
	jmp	__vector_11
	rjmp	__vector_12
#else
 #error "Unsupported Device"
#endif
