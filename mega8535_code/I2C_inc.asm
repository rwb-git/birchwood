
; AVR300.PDF is in futurlec/rwb/datatsheet/ and documents I2C_inc.asm

;.include "8535def.inc"

;.def r_16 = r16


;**** A P P L I C A T I O N   N O T E   A V R 3 0 0 ************************
;*
;* Title		: I2C (Single) Master Implementation
;* Version		: 1.0 (BETA)
;* Last updated		: 97.08.27
;* Target		: AT90Sxxxx (any AVR device)
;*
;* Support email	: avr@atmel.com
;*
;* DESCRIPTION
;* 	Basic routines for communicating with I2C slave devices. This
;*	"single" master implementation is limited to one bus master on the
;*	I2C bus. Most applications do not need the multimaster ability
;*	the I2C bus provides. A single master implementation uses, by far,
;*	less resources and is less XTAL frequency dependent.
;*
;*	Some features :
;*	* All interrupts are free, and can be used for other activities.
;*	* Supports normal and fast mode.
;*	* Supports both 7-bit and 10-bit addressing.
;*	* Supports the entire AVR microcontroller family.
;*
;*	Main I2C functions :
;*	'i2c_start' -		Issues a start condition and sends address
;*				and transfer direction.
;*	'i2c_rep_start' -	Issues a repeated start condition and sends
;*				address and transfer direction.
;*	'i2c_do_transfer' -	Sends or receives data depending on
;*				direction given in address/dir byte.
;*	'i2c_stop' -		Terminates the data transfer by issue a
;*				stop condition.
;*
;* USAGE
;*	Transfer formats is described in the AVR300 documentation.
;*	(An example is shown in the 'main' code).	
;*
;* NOTRES
;*	The I2C routines can be called either from non-interrupt or
;*	interrupt routines, not both.
;*
;* STATISTICS
;*	Code Size	: 81 words (maximum)
;*	Register Usage	: 4 High, 0 Low
;*	Interrupt Usage	: None
;*	Other Usage	: Uses two I/O pins on port c
;*	XTAL Range	: N/A
;*
;***************************************************************************

;**** Includes ****

;.include "1200def.inc"			; change if an other device is used

;**** Global I2C Constants ****

.equ	SCLP	= 3			; SCL Pin number (port c)
.equ	SDAP	= 2			; SDA Pin number (port c)

.equ	b_dir	= 0			; transfer direction bit in i2cadr

.equ	i2crd	= 1
.equ	i2cwr	= 0

;**** Global Register Variables ****

.def	i2cdelay= r16			; Delay loop variable
;.equ i2cdelay = r_16
.def	i2cdata	= r17			; I2C data transfer register
.def	i2cadr	= r18			; I2C address and direction register
.def	i2cstat	= r19			; I2C bus status register

;**** Interrupt Vectors ****

;	rjmp	RESET			; Reset handle
;	( rjmp	EXT_INT0 )		; ( IRQ0 handle )
;	( rjmp	TIM0_OVF )		; ( Timer 0 overflow handle )
;	( rjmp	ANA_COMP )		; ( Analog comparator handle )


;***************************************************************************
;*
;* FUNCTION
;*	i2c_hp_delay
;*	i2c_qp_delay
;*
;* DESCRIPTION
;*	hp - half i2c clock period delay (normal: 5.0us / fast: 1.3us)    rwb: avr300.pdf says normal 100khz, fast 400 khz
;*	qp - quarter i2c clock period delay (normal: 2.5us / fast: 0.6us)
;*
;*	SEE DOCUMENTATION !!!     AVR300.PDF is in futurlec/rwb/datatsheet/ and documents I2C_inc.asm
;*
;* USAGE
;*	no parameters
;*
;* RETURN
;*	none
;*
;***************************************************************************

i2c_hp_delay:
;
;  per avr300.pdf, 8 mhz 100 khz should use 12 and 6 here.  16 mhz nano should use 25.33 and half that. so
;  how the hell did nano rgb read the clock, or hot-water nano?

	ldi	i2cdelay,12
;	ldi	i2cdelay,2		;original value = 2, seems to be based on 2 mhz cpu clock
i2c_hp_delay_loop:
	dec	i2cdelay
	brne	i2c_hp_delay_loop
	ret

i2c_qp_delay:
	ldi	i2cdelay,6
;	ldi	i2cdelay,1		;original value = 1
i2c_qp_delay_loop:
	dec	i2cdelay
	brne	i2c_qp_delay_loop
	ret


;***************************************************************************
;*
;* FUNCTION
;*	i2c_rep_start
;*
;* DESCRIPTION
;*	Assert repeated start condition and sends slave address.
;*
;* USAGE
;*	i2cadr - Contains the slave address and transfer direction.
;*
;* RETURN
;*	Carry flag - Cleared if a slave responds to the address.
;*
;* NOTRE
;*	IMPORTANT! : This funtion must be directly followed by i2c_start.
;*
;***************************************************************************

i2c_rep_start:
	sbi	DDRc,SCLP		; force SCL low
	cbi	DDRc,SDAP		; release SDA
	rcall	i2c_hp_delay		; half period delay
	cbi	DDRc,SCLP		; release SCL
	rcall	i2c_qp_delay		; quarter period delay                 ; rwb 6-4-2024 i think all this does is make sure both lines are ready for a START; a repeated start comes in the middle of a session so
                                                                  ; i guess it's possible that you don't know. this safely raises SDA whil clock is low, and then raises clock so it's ready for the START that follows

;***************************************************************************
;*
;* FUNCTION
;*	i2c_start
;*
;* DESCRIPTION
;*	Generates start condition and sends slave address.
;*
;* USAGE
;*	i2cadr - Contains the slave address and transfer direction.
;*
;* RETURN
;*	Carry flag - Cleared if a slave responds to the address.
;*
;* NOTRE
;*	IMPORTANT! : This funtion must be directly followed by i2c_write.
;*
;***************************************************************************

i2c_start:				
	mov	i2cdata,i2cadr		; copy address to transmitt register
	sbi	DDRc,SDAP		; force SDA low
	rcall	i2c_qp_delay		; quarter period delay


;***************************************************************************
;*
;* FUNCTION
;*	i2c_write
;*
;* DESCRIPTION
;*	Writes data (one byte) to the I2C bus. Also used for sending
;*	the address.
;*
;* USAGE
;*	i2cdata - Contains data to be transmitted.
;*
;* RETURN
;*	Carry flag - Set if the slave respond transfer.
;*
;* NOTRE
;*	IMPORTANT! : This funtion must be directly followed by i2c_get_ack.
;*
;***************************************************************************

i2c_write:
	sec				; set carry flag
	rol	i2cdata			; shift in carry and out bit one
	rjmp	i2c_write_first
i2c_write_bit:
	lsl	i2cdata			; if transmit register empty
i2c_write_first:
	breq	i2c_get_ack		;	goto get acknowledge
	sbi	DDRc,SCLP		; force SCL low

	brcc	i2c_write_low		; if bit high
	nop				;	(equalize number of cycles)
	cbi	DDRc,SDAP		;	release SDA
	rjmp	i2c_write_high
i2c_write_low:				; else
	sbi	DDRc,SDAP		;	force SDA low
	rjmp	i2c_write_high		;	(equalize number of cycles)
i2c_write_high:
	rcall	i2c_hp_delay		; half period delay
	cbi	DDRc,SCLP		; release SCL
	rcall	i2c_hp_delay		; half period delay

	rjmp	i2c_write_bit


;***************************************************************************
;*
;* FUNCTION
;*	i2c_get_ack
;*
;* DESCRIPTION
;*	Get slave acknowledge response.
;*
;* USAGE
;*	(used only by i2c_write in this version)
;*
;* RETURN
;*	Carry flag - Cleared if a slave responds to a request.
;*
;***************************************************************************

i2c_get_ack:
	sbi	DDRc,SCLP		; force SCL low
	cbi	DDRc,SDAP		; release SDA
	rcall	i2c_hp_delay		; half period delay
	cbi	DDRc,SCLP		; release SCL

i2c_get_ack_wait:
	sbis	PINc,SCLP		; wait SCL high 
					;(In case wait states are inserted)
	rjmp	i2c_get_ack_wait

	clc				; clear carry flag
	sbic	PINc,SDAP		; if SDA is high
	sec				;	set carry flag
	rcall	i2c_hp_delay		; half period delay
	ret


;***************************************************************************
;*
;* FUNCTION
;*	i2c_do_transfer
;*
;* DESCRIPTION
;*	Executes a transfer on bus. This is only a combination of i2c_read
;*	and i2c_write for convenience.
;*
;* USAGE
;*	i2cadr - Must have the same direction as when i2c_start was called.
;*	see i2c_read and i2c_write for more information.
;*
;* RETURN
;*	(depends on type of transfer, read or write)
;*
;* NOTRE
;*	IMPORTANT! : This funtion must be directly followed by i2c_read.
;*
;***************************************************************************

i2c_do_transfer:
	sbrs	i2cadr,b_dir		; if dir = write
	rjmp	i2c_write		;	goto write data


;***************************************************************************
;*
;* FUNCTION
;*	i2c_read
;*
;* DESCRIPTION
;*	Reads data (one byte) from the I2C bus.
;*
;* USAGE
;*	Carry flag - 	If set no acknowledge is given to the slave
;*			indicating last read operation before a STOP.
;*			If cleared acknowledge is given to the slave
;*			indicating more data.
;*
;* RETURN
;*	i2cdata - Contains received data.
;*
;* NOTRE
;*	IMPORTANT! : This funtion must be directly followed by i2c_put_ack.
;*
;***************************************************************************

i2c_read:
	
   
   sbi	DDRc,SCLP		; 	force SCL low  <<<<<<<<<<< added by rwb 6-17-2019 for nano/xeep_test
	cbi	DDRc,SDAP		;	release SDA  <<<<<<<<<<< added by rwb 6-17-2019 for nano/xeep_test

                        ; 1-5-2021 this is still confusing. it appears that the atmel code simply had a bug
                        ; in that if master pulls sda low to ack for more read bytes, it is never released.
                        ; my tests today show that the clock can be ignored here and it still works, but since
                        ; this code has been used a lot and i'm still confused, leave it here (force scl lo)
                        ; since it cannot hurt, being as how it is immediately forced low in the following code.
                        ;
                        ; BUT, why not put it in the logical place, at end of ack? hmm, it STOPs no matter
                        ; where i put it down there. 


                        ; 6-27-2019 why did i do this? i think it was for sequential xeep read, which is where
                        ; master reads a byte and then instead of STOP, he ACKs to read another byte without having
                        ; to send address for each byte. when master ACKs, he holds sda low while clock is high. i
                        ; think maybe i had to do it here so that sda did not change while clock was high which is
                        ; how STOP (lo to hi SDA) and START (hi to lo SDA) are conveyed while clock is high. notre
                        ; that clock is forced low again in i2c_read_bit, so hopefully this additional code
                        ; will have no effect on old-fashioned read_xeep where address is sent each time. I use that
                        ; command to check first two bytes in each xeep page in rgb2019 so i think it's ok

	rol	i2cstat			; store acknowledge
					; (used by i2c_put_ack)
	ldi	i2cdata,0x01		; data = 0x01
i2c_read_bit:				; do
	sbi	DDRc,SCLP		; 	force SCL low
	rcall	i2c_hp_delay		;	half period delay

	cbi	DDRc,SCLP		;	release SCL
	rcall	i2c_hp_delay		;	half period delay

	clc				;	clear carry flag
	sbic	PINc,SDAP		;	if SDA is high
	sec				;		set carry flag

	rol	i2cdata			; 	store data bit
	brcc	i2c_read_bit		; while receive register not full


;***************************************************************************
;*
;* FUNCTION
;*	i2c_put_ack
;*
;* DESCRIPTION
;*	Put acknowledge.
;*
;* USAGE
;*	(used only by i2c_read in this version)
;*
;* RETURN
;*	none
;*
;***************************************************************************

i2c_put_ack:
	sbi	DDRc,SCLP		; force SCL low

	ror	i2cstat			; get status bit
	brcc	i2c_put_ack_low		; if bit low goto assert low
	cbi	DDRc,SDAP		;	release SDA
	rjmp	i2c_put_ack_high
i2c_put_ack_low:			; else
	sbi	DDRc,SDAP		;	force SDA low
i2c_put_ack_high:

	rcall	i2c_hp_delay		; half period delay

   
;	cbi	DDRc,SDAP		;	release SDA    <<<<<<<<<<<<< added by rwb 6-17-2019 for nano/xeep_test.asm <<<<<<<<<<<<<<<<<<<<<<
;	rcall	i2c_qp_delay		; <<<<<<<<<<<<< added by rwb 6-17-2019 for nano/xeep_test.asm <<<<<<<<<<<<<<<<<<<<<<

	cbi	DDRc,SCLP		; release SCL
i2c_put_ack_wait:
	sbis	PINc,SCLP		; wait SCL high
	rjmp	i2c_put_ack_wait
	rcall	i2c_hp_delay		; half period delay
	
  ; cbi	DDRc,SDAP		;	release SDA    <<<<<<<<<<<<< added by rwb 6-17-2019 for nano/xeep_test.asm <<<<<<<<<<<<<<<<<<<<<<
	
   ret


;***************************************************************************
;*
;* FUNCTION
;*	i2c_stop
;*
;* DESCRIPTION
;*	Assert stop condition.
;*
;* USAGE
;*	No parameters.
;*
;* RETURN
;*	None.
;*
;***************************************************************************

i2c_stop:
	sbi	DDRc,SCLP		; force SCL low
	sbi	DDRc,SDAP		; force SDA low
	rcall	i2c_hp_delay		; half period delay
	cbi	DDRc,SCLP		; release SCL
	rcall	i2c_qp_delay		; quarter period delay
	cbi	DDRc,SDAP		; release SDA
	rcall	i2c_hp_delay		; half period delay
	ret


;***************************************************************************
;*
;* FUNCTION
;*	i2c_init
;*
;* DESCRIPTION
;*	Initialization of the I2C bus interface.
;*
;* USAGE
;*	Call this function once to initialize the I2C bus. No parameters
;*	are required.
;*
;* RETURN
;*	None
;*
;* NOTRE
;*	PORTD and DDRD pins not used by the I2C bus interface will be
;*	set to Hi-Z (!).
;*
;* COMMENT
;*	This function can be combined with other PORTc initializations.
;*
;***************************************************************************

i2c_init:
	
	cbi	portc,sclp
	cbi	ddrc,sclp
	
	cbi	portc,sdap	
	cbi	ddrc,sdap
	ret

