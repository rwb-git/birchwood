
.define def_box4


.include "m8535def.inc"

.equ unit_id_f = 3
.equ pause_seconds_f = 60       ; this is hardcoded in load flash routine. if i use xbee_rs.c to change, i have to redo
                                 ; after flashing master, since that clears internal eeprom
.equ tank_empty_f = 66
.equ tank_full_f = 243
.equ auto_turnoff_mins_f = 60
.equ flow_1_2_cutoff_f = 102
.equ flow_3_4_cutoff_f = 102
.equ one_sec = 0x98
				               ; $01 (bit 0)
.equ flow_1_2_bit = 1		; $02 (bit 1) 
.equ flow_3_4_bit = 2		; $04 (bit 2) 
.equ float_bit = 3		   ; $08 (bit 3) 
.equ overflow_bit = 4		; $10 (bit 4) 
				               ; $20 (bit 5)
.equ transfer_pump_bit = 6 ; $40 (bit 6) 
				               ; $80 (bit 7)



.equ echo_id = 0xDD                    ; obsolete shit 3-30-2019

;--------------------------------------------------------------------------------------








.cseg

.org $0000
	rjmp RESET      ;Reset handle
.org	aciaddr		;analog comparator interrupt vector = $0010
	rjmp	ac_int	
			
.ORG 0x011	;reset org to 0x011 since unit_id.inc stores constants in hi memory			


.include "I2C_inc.asm"                 ; does not have any wdr. it has at least one infinite loop waiting for a pin to
                                       ; change, but if that fails I need watchdog to reset the avr so wdr would be a
                                       ; mistake there unless i put a timeout counter
.include "avr200.asm"	;multiply and divide routines
.include "avr204.asm"	;BCD conversion routines




.equ five_msec = 0x9c                  ; 8mhz
.equ fiftyfive_msec = 0x06b7           ; 8mhz


;-------------------------------------------------
set_ff_timer:           ; set fork flag for 10 seconds, since this is mega, which takes about 8, 8535 takes about 12


   ser r21                             ; ser sets all bits. r21 = 0xFF
   sts fork_flag,r21
	
	ldi	zl_r30,low(ff_timer)
	ldi	zh_r31,high(ff_timer)
	rcall	copy_timer_0		
	
	ldi r16,10
	clr r17
	
	ldi	zl_r30,low(ff_timer)
	ldi	zh_r31,high(ff_timer)
	rcall	add_to_time_block



   ret
;------------------------------
   

;---------------------
toggle_leds:

      rcall reset_timer_1_flash_leds


      lds r29,led_toggle
      cpi r29,1
      breq ret_a197

      ldi r29,1
      sts led_toggle,r29

		rcall	green_led_on
		rcall	yellow_led_off
		ret
ret_a197:

      clr r29
      sts led_toggle,r29
		rcall	green_led_off
		rcall	yellow_led_on
		ret
;-------------------------------------------------------	




flash_leds:	;flash leds to show that uC is functioning

   lds r16,pulse_state_cnt
   cpi r16,200
   breq flash48
   ret
flash48:

		rcall 	read_timer_1	;uses r25,r26
		andi	t1_h_r26,0x10	;10-22-2002: was 0x10
		brne	ret_197
      ret
		
ret_197:

	rcall	reset_timer_1_64	;uses r16
   lds r26,led_toggle
   cpi r26,1
   breq line4059

   ldi r26,1
   sts led_toggle,r26

		rcall	green_led_on
		rcall	yellow_led_off
		ret
line4059:
   clr r26
   sts led_toggle,r26
		rcall	green_led_off
		rcall	yellow_led_on
		ret


;---------------------------
init_watchdog:
   
   in	r29,mcucsr	;this register = 0 if a watchdog reset has occurred
   andi r29,0b11110000        ; clear lower 4 flags
   out mcucsr,r29

	wdr			;reset watchdog
	
	set			;set T
	in r29, WDTCR
	ori r29, (1<<WDCE)|(1<<WDE)
	out WDTCR, r29                         ; 1 CYCLE
	in r29, WDTCR                          ; 1 cycle
	ori r29, (1<<Wdp0)|(1<<Wdp1)|(1<<wdp2)	; 1 cycle   set prescaler for 2048 cycles, a little over 2 seconds from 3 to 5 volts
                                          ; default is 000 = 16.3 msec
   andi r29,0b11101111                    ; 1 cycle clear wdce
	out WDTCR, r29                         ; 1 cycle


	ret
	
;*********************************************************************************************
pause_one_tick:		;uses r16,r25,r26,r29

      
      rcall	reset_timer_1_64	;uses r16

test_hi_byte3a:	
		rcall	read_timer_1	;uses r25,r26
		ldi	r29,high (fiftyfive_msec) 
		cp	t1_h_r26,r29
		brne	test_hi_byte3a	;hi bytes should test equal 

test_lo_byte3a:	
		rcall	read_timer_1	;uses r25,r26
		ldi	r29,low (fiftyfive_msec)
		cp	t1_l_r25,r29
		brlo	test_lo_byte3a	;branch as long as low byte is lower 
		ret
;*********************************************************************************************
pause_five_msec:	
                              ; .equ five_msec = 0x9c
                              ; mega 0.005 x 8e6/256 = 156.25 d = 0x9C ok


		rcall	reset_timer_1_64	;uses r16

test_hi_byte3:	
		rcall	read_timer_1	;uses r25,r26
		ldi	r29,high (five_msec) ;5 ms = 0x00c0 at 2.4576 Mhz with prescale = 64
		cp	t1_h_r26,r29
		brne	test_hi_byte3	;hi bytes should test equal 

test_lo_byte3:	
		rcall	read_timer_1	;uses r25,r26
		ldi	r29,low (five_msec)
		cp	t1_l_r25,r29
		brlo	test_lo_byte3	;branch as long as low byte is lower 
		ret
	
;----------------------------------

poll_u2:

   sbis ucsra,rxc

   rjmp clear_c_13

   sec
   ret

clear_c_13:

   clc

   ret




;******************************
uart_send_xor_r3:

   ;wdr ; 2017b
   sbis ucsra,udre
   rjmp uart_send_xor_r3

   out udr,r16

   eor r3,r16

   ret




;******************************
uart_send:

   ;wdr ; 2017b
   sbis ucsra,udre
   rjmp uart_send

   out udr,r16

   ret


;*********************************

reset_uart:          ; i keep getting extra bytes, so try to flush the hard way

   clr r16
   out ucsrb,r16
   ldi r16,(1<<rxen)|(1<<txen)
   out ucsrb,r16

   ret


;*********************************
init_uart: ;THIS IS FROM MEGA; 8535 has some differences in receive buffer reading etc.

   ; xbee xsc pro is low start bit, 8 data, lsb first, one stop bit high, idle high
   ;  defaults: 9600 baud, parity 0, 

   ldi r16,51
   ldi r17,0

   out ubrrh,r17
   out ubrrl,r16

   ldi r16,(1<<rxen)|(1<<txen)
   out ucsrb,r16

   ; 8 data, 2 stop bits; lol stop bits must not mean anything. xbee expects 1 and i'm sending 2
   ; and it works fine
   ;ldi r16,(1<<ursel)|(1<<usbs)|(3<<ucsz0)
   
   ; 8 data, 1 stop bit  (1 or 2 stop bits seems to work fine with xbee
   ldi r16,(1<<ursel)|(3<<ucsz0)
   out ucsrc,r16



   ret


;****************************
uart_receive:

   clr r29
   clr r30
   clr r31
   clr r21
uart_receive22:
   ;wdr ; 2017b
   sbic ucsra,rxc
   rjmp data_here
   adiw r30,1
   cpi r31,0xff         ;0x05 worked, 0x04 failed, so use 0x10, 9600 baud, 8mhz avr

   brne uart_receive22
   inc r29
   brne uart_receive22

data_here:
   ;wdr ; 2017b
   in r21,udr
   rcall uart_buffer

;   lds r22,uart_cnt
;   inc r22
;   sts uart_cnt,r22
   ;rcall pause_9600
line2148:
   ret


;****************************************
init_timer_2:

; timer 2 not used by xbee as of 5-12-2012






   ldi r29,0b01111111   ;on, prescale 1024, fast pwm, set oco on compare match
   out tccr2,r29
   ldi r29,0x7f         ; 0xff is low all the time; 0xfe is tiny pulse; 0x00 is high all but tiny low pulse
   out ocr2,r29
   ret


;****************************
uart_get_char:

   clr r30
   clr r31
   clr r21
   clr r23              ; set r23 = 0xff if byte is received before timeout
uart_receive22aab:
   adiw r30,1

   cpi r31,0x0A         ; 0x05 worked, 0x04 failed, so use 0x10, 9600 baud, 8mhz avr
                        ; 8 mhz mega; xbee pro xsc factory settings
   breq line2148aab

   sbis ucsra,rxc
   rjmp uart_receive22aab

   in r21,udr
   rcall uart_buffer

   ldi r23,0xFF

line2148aab:
   
   ret



;****************************
one_secc:
 
   wdr ; 2017b
   clr r17
   clr r18
   clr r19
line2056:
   ;wdr ; 2017b
   sbis ucsra,rxc
   rjmp line2057
   sts r17_cnt,r17      ; typically 0x0f to 0x11, usually 0x10, for success, so test for 0x12 down there
   ret
line2057:
   inc r19
   cpi r19,0
   brne line2056

   inc r18
   cpi r18,0
   brne line2056

   inc r17
   cpi r17,0x12         ; success is usually by 0x10, occasionally 0x11, 8mhz avr, 9600 baud
   brne line2056

   sts r17_cnt,r17
   ret

;************************
baud_delay:

line2048:
   ;wdr ; 2017b

   sbis ucsra,rxc
   rjmp line2048
   in r21,udr
   rcall uart_buffer
   sts uart_1,r21 

   ret

;---------------------------
clear_uart:
   ;wdr ; 2017b

   sbis ucsra,rxc
   ret
   in r21,udr
   rcall uart_buffer
  ; rcall pause_9600
   rjmp clear_uart




;****************************************
;init_timer_2:

; timer 2 not used by xbee as of 5-12-2012





;
;   ldi r29,0b01111111   ;on, prescale 1024, fast pwm, set oco on compare match
;   out tccr2,r29
;   ldi r29,0x7f         ; 0xff is low all the time; 0xfe is tiny pulse; 0x00 is high all but tiny low pulse
;   out ocr2,r29
;   ret


;****************************************
;init_timer_0:
;
;   ldi r29,0b01111101   ;on, prescale 1024, fast pwm, set oco on compare match
;   out tccr0,r29
;   ldi r29,0x7f         ; 0xff is low all the time; 0xfe is tiny pulse; 0x00 is high all but tiny low pulse
;   out ocr0,r29
;   ret
;****************************

;*********************************************************************************************

;init_timer_2:
;		
;		ldi	r19,beep_frequency ;beep 'frequency': lower values = higher pitch
;		out	ocr2,r19
;		
;		ret		
;*********************************************************************************************

init_timer_1:
		ldi	r29,0x04
                           ; 0x04 = 256 prescale per mega pdf
      
		out	tccr1b,r29
		
		
		
		ret
;*********************************************************************************************

init_timer_0:
		
	ldi	r29,$06		; timer 0 input is pin pb0
                     ; 0x06 = 0b000110 = external clock on t0 falling edge per mega pdf
		out	tccr0,r29
		
		ret		
;*********************************************************************************************

;----------------------------------------------------------------

reset_timer_1_flash_leds:
;  timer 1 16 bit
;
;     prescale 1 8 64 256 1024
;
;     8e6/1024/256 = 30 hz from the low byte. so i can get anything i like if i choose to test both bytes
;
;        let it count up to N in the upper byte and then reset the count and toggle the leds
;
;        count    1     2     3     4     5     6     7     8     9     10
;
;        hz       30    15    10    7.5   6     5     etc
;
;
		ldi	r16,0b00000101	      ; clk / 1024 for mega
      
		out	tccr1b,r16


reset_timer_1_count:
		ldi	r16,0x00	;start counter at 0
		out	tcnt1h,r16
		out	tcnt1l,r16

   ret
;----------------------------------------------------------------

reset_timer_1_64:		;uses r16

		ldi	r16,0x04	   ; mega prescale 256 confirmed pdf
                        ; 8e6 / 256 = 31250 hz
		out	tccr1b,r16



		ldi	r16,0x00	;start counter at 0
		out	tcnt1h,r16
		out	tcnt1l,r16
		ret
;*********************************************************************************************
reset_timer_1_1:		;uses r16

      ; if wgm13:0 = 0 counting is always up
      ;
      ; tccr1a bits 1 and 0 wgm11 wgm10      do i ever set tccr1a? default 0x00
      ;
      ; tccr1b bits 4 and 3 wgm13 wgm12      i set these to 0
      ;
      ; so it always counts up

		ldi	r16,0x01	;prescale 1; confirmed pdf mega
					
		out	tccr1b,r16

		ldi	r16,0x00	;start counter at 0
		out	tcnt1h,r16
		out	tcnt1l,r16
		ret

;*********************************************************************************************








;--------------------- include_mega.asm ends here -----------------------------------------------------------------------------------------------
;--------------------- include_mega.asm ends here -----------------------------------------------------------------------------------------------
;--------------------- include_mega.asm ends here -----------------------------------------------------------------------------------------------












.def xword = r26
.def yword = r28
.def zword = r30

.equ term_port = portd
.equ term_ddr = ddrd
.equ term_pin = pind

.equ term2_port = porta
.equ term2_ddr = ddra
.equ term2_pin = pina

.equ pump_relay = pd6
.equ float_input = pd6		;this pin will be low when float is on

				
.equ depth_probe_input = pa0
;.equ flow_1_2_input = pa1
.equ air_pin = pa1
.equ flow_3_4_input = pa2
.equ depth_pulse = pa3
.equ pulse_sensor_line = pa5
.equ pulse_meter_line = pa6


.equ timer_0_clock_pb0 = pb0

.equ ss = pb4
.equ mosi = pb5
.equ miso = pb6		
.equ sck = pb7


.equ send_now_pushbutton=pd2	         ; black pushbutton: press to send prompt packet, or if
				                           ; master, send one now


;
;.ifdef def_box8 box 8 is nano                       ;--------------------------------------------------------------------------------
;.equ transfer_pump_input = pa4
;.equ trans2_pin = pc1
;.equ xbee_reset = pd7                  ; 8-11-2022 box 8 gutted to use for nano - arduino ide code
;.equ xbee_port = portd
;.equ xbee_ddr = ddrd
;.equ yellow_led = pd3
;.equ green_led = pd4
;.endif
;;
.ifdef def_box6                      ;------------------------------------------------------------------------------
.equ transfer_pump_input = pd5       ; slave screw 
.equ trans2_pin = pd4                ; master screw
.equ trans_port = portd
.equ trans_ddr = ddrd
.equ trans_pins = pind
.equ xbee_reset = pb3                ; new vero board with 3 headers so i did not use pc, but i need to add mstr slv 
.equ xbee_port = portb               ; to pc so screws can be used for air or other
.equ xbee_ddr = ddrb
.equ yellow_led = pc7
.equ green_led = pc1
.endif


.ifdef def_box5                      ;------------------------------------------------------------------------------
.equ transfer_pump_input = pc0       ; slave screw
.equ trans2_pin = pc1                ; master screw
.equ trans_port = portc
.equ trans_ddr = ddrc
.equ trans_pins = pinc
.equ xbee_reset = pd5                ; new vero board with 3 headers so i did not use pc, but i need to add mstr slv 
.equ xbee_port = portd               ; to pc so screws can be used for air or other
.equ xbee_ddr = ddrd
.equ yellow_led = pd3
.equ green_led = pd4
.endif

	
.ifdef def_box1                      ;------------------------------------------------------------------------------
.equ transfer_pump_input = pd5
.equ trans2_pin = pd4
.equ trans_port = portd
.equ trans_ddr = ddrd
.equ trans_pins = pind
.equ xbee_reset = pb3                ; original board wire wrap?
.equ xbee_port = portb
.equ xbee_ddr = ddrb
.equ yellow_led = pc7
.equ green_led = pc1
.endif

		
	
.ifdef def_box3                      ;------------------------------------------------------------------------------
.equ transfer_pump_input = pd5
.equ trans2_pin = pd4
.equ trans_port = portd
.equ trans_ddr = ddrd
.equ trans_pins = pind
.equ xbee_reset = pb3                ; original board wire wrap
.equ xbee_port = portb
.equ xbee_ddr = ddrb
.equ yellow_led = pc7
.equ green_led = pc1
.endif

			
	
.ifdef def_box4                      ;------------------------------------------------------------------------------
.equ transfer_pump_input = pd5
.equ trans2_pin = pd4
.equ trans_port = portd
.equ trans_ddr = ddrd
.equ trans_pins = pind
.equ xbee_reset = pc0                ; original board wire wrap
.equ xbee_port = portc
.equ xbee_ddr = ddrc
.equ yellow_led = pc7
.equ green_led = pc1
.endif

					


;.else                                  ;---------------------------------------------------------------------------------
;
;.equ xbee_reset = pb3      ; box 2 uses this pb3 see box_2_final.asm; box3 uses this confirmed by test 8-3-2017
;.equ xbee_port = portb
;.equ xbee_ddr = ddrb
;
;.equ yellow_led = pc7		;port c line 7 drives yellow led = beep
;.equ green_led = pc1		;port c line 1 drives green led = valid startbit
;
;.endif
;
;pc2 and pc3 are defined as I2C sda and scl in I2C_inc.asm file

				
			
.equ spi_loop_count_limit = 0xff	 ; was 0xa0 before usb

.equ slave_id = 4
.equ master_id = 1
				
.equ prompt_id = 0x8c		;use this id to send prompt packet to master, requesting

.def packet_ptr_l_r0 = r0	
.def packet_ptr_h_r1 = r1	


.def i2c_data_r3 = r3

.def xeep_l_r6 = r6		
.def xeep_h_r7 = r7		

;r13	R13 IS USED PERMANENTLY TO STORE FLAGS...DO NOT USE THIS REGISTER
;	bit 0: 1 = need to echo packet when time in 0xa8..0xab is reached
;	bit 1: 1 = packet was transfered to pc; see pause_check_pc routine


;I2C uses r16, r17, r18, r19
				
.def byte_to_send_r21 = r21	

.def write_eep_byte_r22 = r22

.def loop_ctr_r23 = r23
			
				

.def in_bits_r24 = r24	
				

				
.def t1_l_r25 = r25			
.def t1_h_r26 = r26		;r25 and r26 are used to store count from 16 bit timer 1

.def xl_r26 = r26		;(X) = r26 and r27
.def xh_r27 = r27

.def yl_r28 = r28		;(Y) = r28 and r29
.def yh_r29 = r29		


.def zl_r30 = r30		;(Z) = r30 and r31
.def zh_r31 = r31



;*********************************************************************************************
;sram usage	sram is 0x60..0x025f = 512 bytes
;
;
;	use 'assembler options' - 'create LST file' to see line numbers
;		
;		stack pointer starts at 0x025f and counts downward. how much space
;		should I allow for stack?
		


.equ my_sram_start = 0x60
; begin_awk_here        this is a flag for script audit_sram


.DSEG


.org SRAM_START

packet_bytes:                                      .BYTE 0   
sender_id:                                         .BYTE 1   
receiver_id:                                       .BYTE 1   
status_byte:                                       .BYTE 1   
level_byte:                                        .BYTE 1   

flow_1_2:                                          .BYTE 1   
flow_3_4:                                          .BYTE 1   
check_high:                                        .BYTE 1   
check_low:                                         .BYTE 1   

check_high_temp:                                   .BYTE 1   
check_low_temp:                                    .BYTE 1   
spi_loop_count_l:                                  .BYTE 1   
spi_loop_count_h:                                  .BYTE 1   
reset_flags:                                       .BYTE 1   

packet_address_l:                                  .BYTE 1   
packet_address_h:                                  .BYTE 1   
set_rtc_data:                                      .BYTE 8   
total_seconds:                                     .BYTE 5   
rollover_flag:                                     .BYTE 1   

status_flag_7f:                                    .BYTE 1   
last_master_time:                                  .BYTE 4   
last_slave_time:                                   .BYTE 4   
rtc_data:                                          .BYTE 0   
rtc_seconds:                                       .BYTE 1   

rtc_minutes:                                       .BYTE 1   
rtc_hours:                                         .BYTE 1   
rtc_day:                                           .BYTE 1   
rtc_date:                                          .BYTE 1   
rtc_month:                                         .BYTE 1   

rtc_year:                                          .BYTE 1   
rtc_control:                                       .BYTE 1   
rtc_verify:                                        .BYTE 9   
unit_id:                                           .BYTE 1   
last_send_time:                                    .BYTE 4   

calc_result:                                       .BYTE 4   
next_send_time:                                    .BYTE 4   
auto_shut_off_time:                                .BYTE 4   
main_loop_counter:                                 .BYTE 1   
sender_id_s:                                       .BYTE 1   

receiver_id_s:                                     .BYTE 1   
status_byte_s:                                     .BYTE 1   
level_byte_s:                                      .BYTE 1   
float_one_s:                                       .BYTE 1   
float_two_s:                                       .BYTE 1   

send_check_high:                                   .BYTE 1   
send_check_low:                                    .BYTE 1   
pause_seconds_l:                                   .BYTE 1   
pause_seconds_h:                                   .BYTE 1   
flow_1_2_cutoff:                                   .BYTE 1   

flow_3_4_cutoff:                                   .BYTE 1   
new_flow_1_2_cutoff:                               .BYTE 1   
new_flow_3_4_cutoff:                               .BYTE 1   
overflow_cutoff:                                   .BYTE 1   
tank_empty:                                        .BYTE 1   

tank_full:                                         .BYTE 1   
debug_temp:                                        .BYTE 16  
auto_turnoff_mins:                                 .BYTE 1   

dum_flag2:                                         .BYTE 1   
adc0_loops:                                        .BYTE 1   
adc0_maxl:                                         .BYTE 1   
adc0_maxh:                                         .BYTE 1   
adc0_minl:                                         .BYTE 1   

adc0_minh:                                         .BYTE 1   
current_adc:                                       .BYTE 1   
adc1_maxl:                                         .BYTE 1   
oneshot:                                           .BYTE 8   
button_count:                                      .BYTE 8   

button_num:                                        .BYTE 1   
last_button:                                       .BYTE 1   
adc0_val:                                          .BYTE 1   
adc1_val:                                          .BYTE 1   
new_pause_seconds_h:                               .BYTE 1   

new_pause_seconds_l:                               .BYTE 1   
uart_1:                                            .BYTE 1   
uart_2:                                            .BYTE 1   
uart_3:                                            .BYTE 1   
uart_4:                                            .BYTE 1   

uart_5:                                            .BYTE 1   
at_cnt:                                            .BYTE 1   
r17_cnt:                                           .BYTE 1   
uart_s1:                                           .BYTE 1   
uart_s2:                                           .BYTE 1   

uart_s3:                                           .BYTE 1   
uart_s4:                                           .BYTE 1   
pkt_cnt:                                           .BYTE 1   
start_bit_cnt:                                     .BYTE 1   

pkt_buf:                                          .BYTE  0   

pb_sender:                                        .BYTE  1   
pb_receiver:                                      .BYTE  1   
pb_echo_flag:                                     .BYTE  0   


pb_packet_type:                                   .BYTE  14


at_on_fail:                                        .BYTE 1   
u_buf:                                             .BYTE 16  
u_buf_ptr:                                         .BYTE 1   
reset_cnt:                                         .BYTE 1   
xbee_reset_time:                                   .BYTE 2   

xbee_reset_cnt:                                    .BYTE 2   
currentRS:                                         .BYTE 1   
current_0x16_RS:                                   .BYTE 1   
masterRS:                                          .BYTE 1   
slaveRS:                                           .BYTE 1   
rtcbatv:                                           .BYTE 1   

usbtiny_2313_reset_cnt:                            .BYTE 1   
info_req_rec:                                      .BYTE 1   
info_req_rec_xbee:                                 .BYTE 1   
box_id:                                            .BYTE 1   
uart_receive_cnt:                                  .BYTE 1   

receiver:                                          .BYTE 1   
xeep_already_written:                              .BYTE 1   
sender_id_check:                                   .BYTE 1   
seconds_since_midnite:                             .BYTE 3   
pulse_meter_cnt:                                   .BYTE 3   

pulse_secs:                                        .BYTE 1   
pulse_state:                                       .BYTE 1   
pulse_state_cnt:                                   .BYTE 1   
pulse_ack_cnt:                                     .BYTE 1   
led_secs_cnt:                                      .BYTE 4   

abort_cnt:                                         .BYTE 1   
led_toggle:                                        .BYTE 1   
ff_timer:                                          .BYTE 1   
fork_flag:                                         .BYTE 1   

air_val:                                           .BYTE 2
depth_adc:                                         .BYTE 1   

pending_0x36_flag:                                 .BYTE 1   

pending_pulse_flag:                                .BYTE 1   
pending_float_flag:                                .BYTE 1   

fake_pulse_minute:                                 .BYTE 1   

control_byte:                                      .BYTE 1   

eight_bytes_SH_SL:                                 .BYTE 1   

sram_scratchpad_start:                             .BYTE 256 
last_sram:                                         .BYTE 1

; end_awk_here        this is a flag for script audit_sram


.CSEG

;*********************************************************************************************
;nvram usage:	nvram is 0x08..0x3f - what is this? rtc stuff?

;*********************************************************************************************
;internal eep usage - not used before usb. usb xeep data transfer is too slow, so to use the
;4-byte command, which, ironically, looks too fast, lol, i'm thinking about a two step process:
;
;send command to avr to prepare a block of bytes (eep is 512, so use that), and tell me when
;its ready, like a packet ready flag. then have a tight loop that is fast enough to keep up
;with the 2313 request - note that old 8535 is 8mhz and the 2313 is 12 mhz - but, since he hand coded
;the spi stuff, instead of using the bus, shouldn't i be able to keep up easily, if it's a real
;tight loop?
;
;  i use the first 256 bytes for other stuff. store some permanent variables in next group

.equ flow_1_2_eep                   = 0x0101
.equ flow_3_4_eep                   = flow_1_2_eep                + 1
.equ pause_seconds_h_eep            = flow_3_4_eep                + 1
.equ pause_seconds_l_eep            = pause_seconds_h_eep         + 1
.equ reset_cnt_eep                  = pause_seconds_l_eep         + 1
.equ control_byte_eep               = reset_cnt_eep               + 1
;
;
;
;*********************************************************************************************
;xeep usage
;
;24lc256 = 256/8 = 32768 bytes = 0000..7fff
;
;packet storage: 24 hrs * 12 packets * 3 bytes = 864 bytes per day * 7 days = 6048d = 0x17a0
;bytes stored are status, level, datestamp: datestamp = date of month in upper 5 bits. lower 3 bits
;are cleared to indicate valid  packets from units 1, 2, and 4
;
;0x0000..0x179f = basic packet storage for 7 days
;
;  sunday   0000  035F
;  mon      0360  06BF
;  tue      06C0  0A1F
;  wed      0A20  0D7F
;  thu      0D80  10DF
;  fri      10E0  143F
;  sat      1440  179F
;
;  to calculate the time a particular 256 byte page is on, do this:
;  first, calculate minutes since midnite sunday am, divided by 5, multiplied by 3, to see how many bytes to this 
;  minute in time: for example, 5:20 am saturday morning:
;  smtwtf = six full days = 6 x 24 x 60 / 5 x 3 = 5184d = 0x1440, like it says in the list above
;  add (5 x 60 + 20)/5*3 = 192d = 0x00C0 + 1440 = 1500, meaning right on the boundary of a page
;
;  example 2: 12:25 saturday noon = (12 x 60 + 25) /5 x 3 = 1bf + 1440 = 15FF, again, right smack on a page boundary
;
;adc byte storage: 

					;1728 = 24 hours x 12 packets/hour x 6 bytes/packet
					;so 1728 bytes are stored each day
					
					;				decimal addrh range
					;
					;sunday 	         17a0...1e5f	23..30
					;monday           1e60...251f	30..37
					;tuesday          2520...2bdf	37..43
					;wednesday        2be0...329f	43..50
					;thursday         32a0...395f	50..57
					;friday           3960...401f	57..64
					;saturday         4020...46df	64..70
;0x17a0..0x46df = adc byte storage
;					
;xeep remaining after packet and adc storage = 0x3920 = 14624d
;
;0x46e0..0x46ff = UNUSED
;
;daily summary:
.equ daily_block_size = 8
;store float minutes on (2 bytes), transfer pump minutes on (2 bytes), year, and checksum: 
;	8 bytes * 366 days = 2928d = 0xb70
;
;0x4700..0x0526f =  daily summary storage
.equ daily_summary_loc = 0x4700
;this needs to be verified by checking final version of daily summary
;
;0x5270..0x7fff = UNUSED = 11663d = 0x2d8f
;*********************************************************************************************
;timer usage:
;
;timer 0: eternal clock uses timer 0 to count 1 sec pulses from rtc. this will fail if
;rtc fails, or during power outages.
;
;timer 1: read adc, flash leds, pause 5 msec, pause one bit, pause one half bit
;
;timer 2: wait one cycle, beep off, beep on
;*********************************************************************************************
init_ports:		;uses no regs


      sbi	xbee_ddr,xbee_reset      ; 
		sbi	xbee_port,xbee_reset	


	rcall	brief_pause		;not sure if this is necessary


	cbi	term_ddr,float_input	;input from float (master mode)
	sbi	term_port,float_input	;enable internal pullup
	
	cbi	trans_ddr,trans2_pin
	sbi	trans_port,trans2_pin	;enable pullup for transfer pump input line
	
   cbi	trans_ddr,transfer_pump_input
	sbi	trans_port,transfer_pump_input	;enable pullup for transfer pump input line
						;see documentation: the 5vdc adapter drives
						;the base of an open collector npn transistor


; do i need to init adc lines?
;
;	cbi	term2_ddr,flow_1_2_input
;	sbi	term2_port,flow_1_2_input	;enable pullup
;	
;	cbi	term2_ddr,flow_3_4_input
;	cbi	term2_port,flow_3_4_input	;disable pullup; adc reads this line

	sbi	term2_ddr,depth_pulse		;this output energizes depth probe
	cbi	term2_port,depth_pulse		;turn probe off by lowering line
	
	sbi	term2_ddr,pulse_sensor_line	;this output energizes water sensors
	sbi	term2_port,pulse_sensor_line	;turn sensor power off by RAISING line to pnp 
	
;	cbi	term2_ddr,depth_probe_input
;	cbi	term2_port,depth_probe_input	;disable pullup
	
	cbi	term2_ddr,pulse_meter_line    ; input
	sbi	term2_port,pulse_meter_line   ; enable pullup
	

;port d
		
		cbi	ddrd,send_now_pushbutton	;input line
		sbi	portd,send_now_pushbutton	;enable pullup

	cbi ddrd,0         ; uart rxd

	sbi ddrd,1         ; uart txd
  

.ifdef def_box8                        ; def box_8

      sbi	ddrd,yellow_led		;yellow led is an output
		sbi	portd,yellow_led	;raise line to turn led off
		
		sbi	ddrd,green_led		;green led is an output
		sbi	portd,green_led		;raise line to turn led off
	
.endif

.ifdef def_box6

      sbi	ddrc,yellow_led		;yellow led is an output
		sbi	portc,yellow_led	;raise line to turn led off
		
		sbi	ddrc,green_led		;green led is an output
		sbi	portc,green_led		;raise line to turn led off
	
.endif

.ifdef def_box5

      sbi	ddrd,yellow_led		;yellow led is an output
		sbi	portd,yellow_led	;raise line to turn led off
		
		sbi	ddrd,green_led		;green led is an output
		sbi	portd,green_led		;raise line to turn led off
	
.endif

.ifdef def_box3
		
		sbi	ddrc,yellow_led		;yellow led is an output
		sbi	portc,yellow_led	;raise line to turn led off
		
		sbi	ddrc,green_led		;green led is an output
		sbi	portc,green_led		;raise line to turn led off
	
.endif


.ifdef def_box4
		
		sbi	ddrc,yellow_led		;yellow led is an output
		sbi	portc,yellow_led	;raise line to turn led off
		
		sbi	ddrc,green_led		;green led is an output
		sbi	portc,green_led		;raise line to turn led off
	
.endif

.ifdef def_box1
		
		sbi	ddrc,yellow_led		;yellow led is an output
		sbi	portc,yellow_led	;raise line to turn led off
		
		sbi	ddrc,green_led		;green led is an output
		sbi	portc,green_led		;raise line to turn led off
	
.endif
		

		cbi	ddrb,timer_0_clock_pb0	;input
		sbi	portb,timer_0_clock_pb0	;enable pullup	
							
	
	
		sbi	xbee_ddr,xbee_reset   ;ddrb,pb3       ;xbee reset		;output
		ret


;******************************************** xbee begins here

at_off_menu_1:

   rcall xbee_off_at_mode


   ret

;*************************

at_cmd:

   sts uart_s1,r21
   sts uart_s2,r22
   ;sts menu_num,r23
   rcall send_at
   rcall uart_rec_4

   ret


;****************************

atsh_menu_16:
   ldi r21,'S'
   ldi r22,'H'
   ldi r23,16

   rcall at_cmd
   ret

;****************************

atsl_menu_17:
   ldi r21,'S'
   ldi r22,'L'
   ldi r23,17

   rcall at_cmd
   ret

;************************

set_pkt_buf_to_cc:            ; this was just to find a bug. erase if not needed anymore, with spi code 0x76

   push r29
   push r28

	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

   ldi r21,16
   mov r2,r21
   clr r22
   sts u_buf_ptr,r22

   ldi r22,0xCC

line1263bc:

   st Y+,r22

   dec r2
   brne line1263bc

   pop r28
   pop r29
   ret


;************************
clear_pkt_buf:

   push r29
   push r28

	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

   ldi r21,16
   mov r2,r21
   clr r22
   sts u_buf_ptr,r22

line1263bcc:

   st Y+,r22

   dec r2
   brne line1263bcc

   pop r28
   pop r29
   ret

;********************


clear_uart_buffer:

   push r29
   push r28

	ldi	r29,high(u_buf) 
	ldi	r28,low(u_buf)	

   ldi r21,16
   mov r2,r21
   clr r22
   sts u_buf_ptr,r22

line1263b:

   st Y+,r22

   dec r2
   brne line1263b

   pop r28
   pop r29
   ret
;**************************

send_start_bytes:

   ldi r21,0x1B               ; was 0x0B but 8535 s/n 003  was getting some at 00 and 01

line1392:

   ldi r16,'&'
   rcall uart_send
 
   dec r21
   brne line1392
 
   ret

;******************
atrs:



   ldi r21,'R'
   ldi r22,'S'
   sts uart_s1,r21
   sts uart_s2,r22
   rcall send_at_2
   rcall uart_rec_2_no_lcd

   ret


;********************************

ascii2hex:                 ; input one ascii numerical digit in r21
                           ; output hex 0..F in r21
                           ; if value is not valid ascii, 0 is returned. it would be easy to 
                           ; return a flag, like FF, but i don't need that now, and 0 is a better failure mode
                           ;
                           ; 0..9 = 0x30..0x39, so subtract 0x30 to convert hex ascii to numerical 0..9
                           ;
                           ; A..F = 0x41..0x46 so subtract 0x41 then add 0x0A or just subtract 0x37
   cpi r21,0x30
   brlo line1457

   cpi r21,0x40
   brsh line1460
                           ;0x30..0x39
   subi r21,0x30

   ret
line1460:                  ; val > 0x40

   cpi r21,0x47
   brsh line1457

   subi r21,0x37
   ;subi r21,0x41
   ret
line1457:

   clr r21
   ret


;**************************
get_rs_byte:               ; returns RS byte in r21

   rcall get_at_rs         ; rs is in uart buff, like 3331 for 0x31, 0r 3241 for 0x2A
                           ; 0..9 = 0x30..0x39    a..f = 0x41..0x46
                           ; 
                           ; if < 0x40, val - 0x30
                           ;
                           ; if > 0x41, (val -0x41) + 0x0a
                           ;
                           ; msb << 4, or msb * 16, or swap nibbles = swap reg

	ldi	r29,high(u_buf) 
	ldi	r28,low(u_buf)	

   ld r21,Y+               ; msb
   ld r20,Y                ; lsb

   rcall ascii2hex

   mov r19,r21
   mov r21,r20

   rcall ascii2hex
   
   swap r19                ; msb x 16
   add r21,r19
   ret

;**********************

get_at_rs:
   
   rcall at_on
   lds r21,at_on_fail
   cpi r21,0xFF
   breq line1226

   rcall clear_uart_buffer
   rcall atrs
   rcall at_off
   ret
line1226:                     ; fail

   ret
;*************************

process_set_rtc_xbee:          

; packet format
;
; 3 new style bytes - sender xbee sn, receiver xbee sn, packet type
; if packet type is 0xBB, sender and receiver is not important. master unit will accept this packet from anyone
; actually, all units will accept this packet, but it only has any effect on master


	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

	ldi	zl_r30,low(set_rtc_data)         ; store bytes in sram 0x72..0x79
	ldi	zh_r31,high(set_rtc_data)
   
   adiw r28,3                             ; skip 3 new-style bytes = sender,receiver,type
	
   ldi r17,8

loop1826:

   ld r21,Y+
   st z+,r21

   dec r17

   brne loop1826

   rcall	init_clock3	;copy from sram to rtc registers
	ret


;********************************


xbee_send_rtc:              

   ; tablet, pypipes, new_probe, or whatever has already used existing code to set rtc. or maybe I decided to let
   ; master box send this command on a schedule. at any rate, the rtc in this box is assumed to be accurate.
   ;
   ; so, all we need to do here is read our rtc and send that exact data to any listeners, right?


   rcall record_clock_in_sram_verify   ; 7 bytes at rtc_data. we will also send 1hz flag

   rcall send_start_bytes
   
   ldi r16,11                          ; number of data bytes. does not include checsum
                                       ; count the number of calls to uart_send_xor_r3
   rcall uart_send
  
   clr r3

   ldi r16,0                           ; obsolete, was xbee_unit_id
   rcall uart_send_xor_r3
 
   ldi r16,0x7E                        ; receiver = second data byte - send this to all listeners 
   rcall uart_send_xor_r3
 
   ldi r16,0x92                        ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply
                                       ; search for packet_type_list to see used values
   rcall uart_send_xor_r3              ; uses r3, r16

	ldi	zl_r30,low(rtc_data)          ; load pointer to sram 0x0090 = (Z) pointer
	ldi	zh_r31,high(rtc_data)          ; load pointer to sram 0x0090 = (Z) pointer

   ldi r17,7                           ; send 7 rtc bytes then send 1hz flag

loop1839:

   ld r21,z+

   rcall bin2bcd8                       ; uses r21, r22

   mov r16,r21

   rcall uart_send_xor_r3              ; uses r3, r16

   dec r17

   brne loop1839

	; send_spi_sleep(0x10,sleeper);

   ldi r16,0x10                        ; flag to enable 1hz
   rcall uart_send_xor_r3              ; uses r3, r16

   mov r16,r3                          ; checksum
   rcall uart_send
   ret
 

;********************************

xbee_send_8_old_style:              ; call this with old style packet at sender_id_s...
                                    ; this will add bytes to beginning and end, but will send those original
                                    ; eight bytes as the data


   .ifdef no_xbee

   ret

   .endif


   rcall clear_pkt_buf

   rcall send_start_bytes
   
   ldi r16,13                       ; number of data bytes. does not include checsum; pkt_buf is 16 bytes
   
   rcall uart_send
  
   clr r3

	lds	r16,flow_3_4_cutoff           ; this will send cutoff to tablet and rpi
   ;ldi r16,0                           ; obsolete, was xbee_unit_id
   rcall uart_send_xor_r3




   lds r16,depth_adc                 ; 8-12-2022 i never use this 0x7E so put depth adc here

   ;lds r16,receiver                 ; receiver = second data byte
   rcall uart_send_xor_r3
 
   ; fake_master sends 0x33 instead of 0xAA

   .ifdef fake_packets

   ldi r16,0x33                     ;  xbee_packet_type xbee_packet_type_list packet_type_list
   .else

   ldi r16,0xAA                     ;  xbee_packet_type xbee_packet_type_list packet_type_list
   .endif

                                    ;  0x1B ask for info - master or slave id
                                    ;  0x1C ask for info - xbee sn
                                    ;  0x1D reply to 0x1B or 0x1C
                                    ;  0x92 send rtc to all listeners
                                    ;  0xAA old style radio packets, all eight ridiculous bytes
                                    ;  0xBB proxy poll request. xbee debugging, wherein i ask xbee 2 to poll xbee 3 
                                    ;  0xCC set flow threshold/cutoff values update - msg for master
                                    ;  0xDD ask master what current threshold/cutoff values are
                                    ;  0xDE master responds with cutoff values
                                    ;  0xEE xbee echo reply - xbee testing
                                    ;  0xFF xbee echo request - xbee testing
   rcall uart_send_xor_r3
	

      ldi	zl_r30,sender_id_s
		clr	zh_r31			


next_byte2:		
		ld 	r16,z+	;receiver, then four data bytes
		;ld 	byte_to_send_r21,z+	;receiver, then four data bytes
		
      rcall uart_send_xor_r3
		;rcall	send_byte		;uses r16,r19,r20,r21,r23,r25,r26	

		cpi	zl_r30,send_check_low+1		
		
		brne	next_byte2

;      this sends 8 bytes = sender, receiver, status, tank level,  float one   float two    ck hi     ck lo
;
;      ck hi and low can be used for anything. probably sender and receiver too
;
;      float one is actually rpi temperature
;
;      float two is flow adc




   lds r16,air_val+1                     ; was master RS. this is air val msb
   rcall uart_send_xor_r3
   
   
   lds r16,air_val                     ; 8-12-2022 put air pressure here since slave RS is never used
   ;lds r16,slaveRS
   rcall uart_send_xor_r3


   mov r16,r3             ; checksum
   rcall uart_send
  	

   lds r21,unit_id
   cpi r21,master_id          ; master clears slaveRS after sending. no need for any other box to clear anything here
   brne line1674

   clr r21
   sts slaveRS,r21

line1674:
		ret

 
;*****************************

send_at_2:

   rcall clear_uart

   ldi r16,'A'
   rcall uart_send

   ldi r16,'T'
   rcall uart_send

   lds r16,uart_s1
   rcall uart_send

   lds r16,uart_s2
   rcall uart_send

   ldi r16,0x0D
   rcall uart_send
   
   clr r22
   sts uart_1,r22
   sts uart_2,r22

   rcall one_secc
 
   ret


;******************************************
send_at:


  
   rcall clear_uart

   ldi r16,'A'
   rcall uart_send

   ldi r16,'T'
   rcall uart_send

   lds r16,uart_s1
   rcall uart_send

   lds r16,uart_s2
   rcall uart_send

   ldi r16,0x0D
   rcall uart_send
   
   clr r22
   sts uart_1,r22
   sts uart_2,r22

   rcall one_secc
 
   ret
;***************************************

uart_rec_2_no_lcd:

   rcall uart_receive
   sts uart_1,r21

   rcall uart_receive
   sts uart_2,r21

   ret

;***************************

uart_rec_5_no_lcd:

   rcall uart_receive
   sts uart_1,r21

   rcall uart_receive
   sts uart_2,r21

   rcall uart_receive
   sts uart_3,r21

   rcall uart_receive
   sts uart_4,r21
   
   rcall uart_receive
   sts uart_5,r21

   ret

;****************************

uart_rec_4:

   rcall uart_rec_5_no_lcd

   ret


;******************
;***************************************************
;****************************
one_secc_no_check:

   clr r17
   clr r18
   clr r19
line2056c:
   wdr                 ; not needed by sn 003 with or without usb tiny attached
                        ; fast guard time was enabled during the test
                        ; BUT IS NEEDED FOR xbee_rs to read sn, so leave it here
   inc r19
   cpi r19,0
   brne line2056c

   inc r18
   cpi r18,0
   brne line2056c

   inc r17
   cpi r17,0x12         ; 0x06 works ok on old box, but increaed to get read sn to work
   ;cpi r17,0x06         ; success is usually by 0x10, occasionally 0x11, 8mhz avr, 9600 baud
                        ; mega 8mhz was 0x13
                        ; 0x13 x 2.4576/8 = 6
   brne line2056c

   ret

;***************************

xbee_at_mode:     ; wait one sec, send +++, wait one se

   rcall at_on
   lds r21,at_on_fail
   cpi r21,0xFF
   breq liney1226

line2058:
    ret

liney1226:
   ret
;**************************

at_on:     ; wait one sec, send +++, wait one se

   lds r22,at_cnt
   inc r22
   sts at_cnt,r22
   rcall one_secc_no_check
   rcall clear_uart

   ldi r16,'+'
   rcall uart_send

   ldi r16,'+'
   rcall uart_send

   ldi r16,'+'
   rcall uart_send
   clr r22
   sts uart_1,r22
   sts uart_2,r22
   sts uart_3,r22

   rcall one_secc             ; this returns when sbis usr,rxc = data here,  or timeout

   rcall uart_receive         ; 0x4F = 'O'     and this also waits for data here, with wdr for infinite wait if xbee fails
   sts uart_1,r21

   cpi r21,0x4F

   rcall uart_receive         ; 0x4B = 'K'
   sts uart_2,r21

   cpi r21,0x4B
   
   rcall uart_receive         ; 0x0D = <CR>
   sts uart_3,r21

   rcall uart_rec_5_no_lcd
   cpi r21,0x0D
   
   ret

;THIS CAN NEVER HAPPEN
;THIS CAN NEVER HAPPEN
;THIS CAN NEVER HAPPEN
;THIS CAN NEVER HAPPEN
line1892:                     ; fail - bad response
;THIS CAN NEVER HAPPEN
;THIS CAN NEVER HAPPEN


   ldi r21,0xff
   sts at_on_fail,r21

   ret


;***************************
;***************************


at_off:      
xbee_off_at_mode:     

   ldi r16,'A'
   rcall uart_send

   ldi r16,'T'
   rcall uart_send

   ldi r16,'C'
   rcall uart_send
   ldi r16,'N'
   rcall uart_send
   ldi r16,0x0D
   rcall uart_send

   rcall uart_rec_5_no_lcd
   ;rcall clear_uart
   
   ret

;*********************

;*************************

uart_buffer:


   cpi r21,0x0d            ; ignore <CR> since i get lots of extras
   breq line2323

   cpi r21,'&'             ; ignore start bytes
   breq line2323

   rjmp line2344

line2323:

   ret

line2344:

   push r27
   push r28
   push r29

	ldi	r29,high(u_buf) 
	ldi	r28,low(u_buf)	

   lds r27,u_buf_ptr
   add r28,r27

   clr r27
   adc r29,r27

   st Y,r21

   lds r27,u_buf_ptr
   cpi r27,0x0F

   brsh line2264

   inc r27
   sts u_buf_ptr,r27

line2264:

   pop r29
   pop r28
   pop r27

   ret



;*********************


timed_out:



   ret

;****************************
poll_uart:

   rcall poll_u2

   brcs new_char24

   ret

new_char24:

   clr r21
   sts start_bit_cnt,r21

   in r21,udr

   rcall uart_buffer

   cpi r21,'&'
   breq line3129


   ret
   ;;;;;rjmp echo_char             ; just a ret

line3129:                     ; got startbyte '&'; several are sent, so check for first non - '&' 
   

;awadr ;2017

   lds r21,start_bit_cnt
   inc r21
   sts start_bit_cnt,r21

   rcall uart_get_char

   cpi r23,0xff
   breq line8130j

   rjmp timed_out
line8130j:
   
   cpi r21,'&'
   brne line3129a
   
   rjmp line3129

line3129a:

   clr r3               ; checksum
   sts pkt_cnt,r21      ; number of data bytes, not including checksum

	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

line8131:

;awadr ;2017
   rcall uart_get_char

   cpi r23,0xff
   breq line8132

   rjmp timed_out

line8132:
   eor r3,r21

   st Y+,r21

   lds r23,pkt_cnt
   dec r23
   sts pkt_cnt,r23
   brne line8131

   rcall uart_get_char     ; get checksum
   cpi r23,0xff
   breq line8133

   rjmp timed_out

line8133:

   eor r3,r21              ; if the result of this is 0, packet is probably good

   mov r21,r3
   cpi r21,0
   breq line2586           ; SO KEEP PROCESSING lol. i suppose xbee never sends me garbage? (fixed)

   ret

line2586:

   rcall set_xbee_reset_time

;awadr ;2017
;   rcall get_rs_byte               ; returns RS byte in r21      6-9-2023 delete this RS call to see if it fixes wdr reset issue
;
;   sts currentRS,r21
;
;   sts current_0x16_RS,r21


   
   lds r21,pb_echo_flag                ; xbee_packet_type = this byte is also pb_packet_type. 0xff means echo request
                                       ; search for packet_type_list to see used values
line1912:;-------------------------------------
   
   
   .ifdef fake_packets
   
   cpi r21,0x33                        ; normal old-style radio packet
   
   .else

   cpi r21,0xAA                        ; normal old-style radio packet
   
   .endif

   brne line1926


   lds r21,pb_packet_type + 1          ; this is old style sender
   cpi r21,master_id
   brne line2304

   lds r21,currentRS
   sts masterRS,r21                    ; every box keeps the most recent rs for packets from slave and master 
   rjmp line2305

line2304:

   cpi r21,slave_id
   brne line2305

   lds r21,currentRS
   sts slaveRS,r21

line2305:
;awadr ;2017
   rcall process_old_style_packet

   ret

line1926:;------------------------------------

   cpi r21,0x55   
   brne cbline2012c

   rcall process_control_byte

   ret
cbline2012c:;---------------------------------------


   cpi r21,0xCC         ; all units do this, in case i switch boxes 
   brne line2012c

   rcall process_flow_cutoff_update

   ret
line2012c:;---------------------------------------


   cpi r21,0x92         ; set rtc
   brne line2012c2

   rcall process_set_rtc_xbee

   ret
line2012c2:;---------------------------------------

   .ifdef fake_packets
   
   cpi r21,0x36         ; 0x16 acks 0x15 packet which is pulse meter cnt       pulse_meter  pulse_meter_cnt
   
   .else

   cpi r21,0x16         ; 0x16 acks 0x15 packet which is pulse meter cnt       pulse_meter  pulse_meter_cnt

   .endif

   brne line60104sw

   rjmp process_0x16                   ; rjmp uses its ret
   

line60104sw: ;----------------------------
;
;
;   cpi r21,0x26         ; 0x16 acks 0x15 packet which is pulse meter cnt       pulse_meter  pulse_meter_cnt
;   brne line6004sw
;
;   rjmp process_0x16                   ; rjmp uses its ret
;   
;
;line6004sw: ;----------------------------


   cpi r21,0xDD         ; only master responds to this request
   brne line2012dc

   lds r29,unit_id
   cpi r29,master_id
   breq line2251
   ret
line2251:

   rcall master_cutoff_values

   ret
line2012dc:;-------------------------------------

line2270:;-------------------------------

   ret

 
;***********************

master_cutoff_values:         ; master sends this 0xDE reply if it receives 0xDD packet


   rcall send_start_bytes
   
   ldi r16,0x07             ; number of data bytes. does not include checsum; pkt_buf is 16 bytes
   
   rcall uart_send
  
   clr r3

;.equ pkt_buf               = start_bit_cnt            + 1         ; packet buffer 16 bytes
;.equ pb_sender             = pkt_buf                  + 0
;.equ pb_receiver           = pb_sender                + 1
;.equ pb_echo_flag          = pb_receiver              + 1
;.equ pb_packet_type        = pb_echo_flag             + 0


   ldi r16,0                           ; obsolete, was xbee_unit_id
   rcall uart_send_xor_r3
 
   lds r16,pb_sender        ; receiver = second data byte
   rcall uart_send_xor_r3
 
   ldi r16,0xDE             ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply
                            ; search for packet_type_list to see used values
   rcall uart_send_xor_r3
 

   lds r16,flow_1_2_cutoff
   rcall uart_send_xor_r3
  
   lds r16,flow_3_4_cutoff
   rcall uart_send_xor_r3
  
   lds r16,pause_seconds_h
   rcall uart_send_xor_r3

   lds r16,pause_seconds_l
   rcall uart_send_xor_r3
 
   mov r16,r3             ; checksum
   rcall uart_send
   ret
 

;*********************************

ask_master_cutoff_values:         ;  0xDD ask for master 0xDE reply cutoff values


   rcall send_start_bytes
   
   ldi r16,0x03             ; number of data bytes. does not include checsum; pkt_buf is 16 bytes
   
   rcall uart_send
  
   clr r3

;.equ pkt_buf               = start_bit_cnt            + 1         ; packet buffer 16 bytes
;.equ pb_sender             = pkt_buf                  + 0
;.equ pb_receiver           = pb_sender                + 1
;.equ pb_echo_flag          = pb_receiver              + 1
;.equ pb_packet_type        = pb_echo_flag             + 0


   ldi r16,0                           ; obsolete, was xbee_unit_id  
   rcall uart_send_xor_r3
 
   lds r16,pb_sender        ; receiver = second data byte
   rcall uart_send_xor_r3
 
   ldi r16,0xDD             ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply
                            ; search for packet_type_list to see used values
   rcall uart_send_xor_r3
 
 
   mov r16,r3             ; checksum
   rcall uart_send
   ret
 

;***********************

send_master_cutoff_values:       ; 0xCC packet sent to all listeners, but mainly affects master


   rcall send_start_bytes
   
   ldi r16,0x07             ; number of data bytes. does not include checsum; pkt_buf is 16 bytes
   
   rcall uart_send
  
   clr r3

;.equ pkt_buf               = start_bit_cnt            + 1         ; packet buffer 16 bytes
;.equ pb_sender             = pkt_buf                  + 0
;.equ pb_receiver           = pb_sender                + 1
;.equ pb_echo_flag          = pb_receiver              + 1
;.equ pb_packet_type        = pb_echo_flag             + 0


   ldi r16,0                           ; obsolete, was xbee_unit_id  
   rcall uart_send_xor_r3
 
   ;;;lds r16,pb_sender        ; receiver = second data byte
   ldi r16,0x7E                ; receiver = second data byte - send this to all listeners 
   rcall uart_send_xor_r3
 
   ldi r16,0xCC             ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply
                            ; search for packet_type_list to see used values
   rcall uart_send_xor_r3
 

   lds r16,new_flow_1_2_cutoff
   rcall uart_send_xor_r3
  
   lds r16,new_flow_3_4_cutoff
   rcall uart_send_xor_r3
  
   lds r16,new_pause_seconds_h
   rcall uart_send_xor_r3
   lds r16,new_pause_seconds_l
   rcall uart_send_xor_r3
 
   mov r16,r3             ; checksum
   rcall uart_send
   ret
 

;**************************

process_flow_cutoff_update:      ; do this if any box receives 0xCC packet

; packet format
;
; 3 new style bytes - sender xbee sn, receiver xbee sn, packet type
; if packet type is 0xBB, sender and receiver is not important. master unit will accept this packet from anyone
; actually, all units will accept this packet, but it only has any effect on master
;
; byte 4 = flow 1 2 cutoff
; byte 5 = flow 3 4 cutoff


	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

   adiw r28,3                       ; skip 3 new-style bytes = sender,receiver,type

   ld r21,Y+
   sts flow_1_2_cutoff,r21
   ld r21,Y+
   sts flow_3_4_cutoff,r21
 
   ld r21,Y+
   sts pause_seconds_h,r21
   
   ld r21,Y+
   sts pause_seconds_l,r21
   
   ; copy to eep so these values take precedence over flash on reset

      ;  write_eep:	
		;  write byte in write_eep_byte_r22 to internal eep at addrh = r30   addrl = zh_r31 <- wrong

   ldi r30,low(flow_1_2_eep)
   ldi r31,high(flow_1_2_eep)
   lds r22,flow_1_2_cutoff
   rcall write_eep


   ldi r30,low(flow_3_4_eep)
   ldi r31,high(flow_3_4_eep)
   lds r22,flow_3_4_cutoff
   rcall write_eep


   ldi r30,low(pause_seconds_h_eep)
   ldi r31,high(pause_seconds_h_eep)
   lds r22,pause_seconds_h
   rcall write_eep


   ldi r30,low(pause_seconds_l_eep)
   ldi r31,high(pause_seconds_l_eep)
   lds r22,pause_seconds_l
   rcall write_eep



   ret

;***********************


;**************************

process_control_byte:

; packet format
;
; 3 new style bytes - sender xbee sn, receiver xbee sn, packet type
; if packet type is 0xBB, sender and receiver is not important. master unit will accept this packet from anyone
; actually, all units will accept this packet, but it only has any effect on master
;
; byte 4 = flow 1 2 cutoff
; byte 5 = flow 3 4 cutoff


	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

   adiw r28,3                       ; skip 3 new-style bytes = sender,receiver,type

   ld r21,Y+
   sts control_byte,r21

;   ld r21,Y+
;   sts flow_3_4_cutoff,r21
; 
;   ld r21,Y+
;   sts pause_seconds_h,r21
;   
;   ld r21,Y+
;   sts pause_seconds_l,r21
   
   ; copy to eep so these values take precedence over flash on reset

      ;  write_eep:	
		;  write byte in write_eep_byte_r22 to internal eep at addrh = r30   addrl = zh_r31 <- wrong

   ldi r30,low(control_byte_eep)
   ldi r31,high(control_byte_eep)
   lds r22,control_byte
   rcall write_eep

   ret

;***********************

process_old_style_packet:


	ldi	r29,high(pkt_buf) 
	ldi	r28,low(pkt_buf)	

   adiw r28,3

	ldi	r31,high(packet_bytes) 
	ldi	r30,low(packet_bytes)	

   ldi r22,8

line8131p:

   ld r21,Y+
   st Z+,r21

   dec r22
   brne line8131p


   rcall process_packet
   ret


;------------------------------------

new_flash_leds:                        ; flash once per second to verify rtc 1 hz

   ldi r16,1
   and r16,r19                         ; r19 has seconds count

   brne state3485
   
   rcall yellow_led_off
   rcall green_led_on
   ret

state3485:

   rcall yellow_led_on
   rcall green_led_off

   ret


;*********************************************************************************************
ac_int:			;analog comparator jumps here on interrupt
			reti
;*********************************************************************************************
timer_0_check:		;instead of using interrupts, which might corrupt i2c transfer,
			;process this timer by checking it periodically. since it counts
			;once per second, i should always be able to set a flag when the
			;count exceeds 128 (0x0080). then, i should be able to catch the
			;timer when it rolls over but before it reaches 128 again. (128 
			;seconds is a little over 2 minutes.) 
			
			;notre that this routine is not processed while sending or
			;receiving packets, which takes several seconds. BUT, the only
			;time this routine acts is at rollover, which occurs every 
			;256 seconds, or 4.27 minutes. and, the rollover flag ensures
			;that i have 2.13 minutes to catch the error.
		
   in r19,tcnt0

   lds r29,led_secs_cnt
   cp r19,r29
   breq old_code_1134

   sts led_secs_cnt,r19
  	
   lds r29,control_byte       ; 0x99 on    0x88 off      
   cpi r29,0x99
   brne  dead88ff

   rcall new_flash_leds       
dead88ff:


;	rcall increment_usb_dead_counter ; if count is too high, this will reset 2313

old_code_1134:

   lds r19,unit_id
   cpi r19,master_id
   brne line3857
   
   in	r19,tcnt0
   
   lds r29,pulse_secs
   cp r29,r19

   breq line3857

   sts pulse_secs, r19

   lds r16,pulse_ack_cnt               ; incremented once per second
   inc r16
   sts pulse_ack_cnt, r16

   rcall check_pulse_meter             ; check once per second
	
line3857:

		
	in	r19,tcnt0
	cpi	r19,0x80
	brlo	dont_flag_yet
	
	ldi	r19,0xff
	sts	rollover_flag,r19	;rollover_flag = 0xff when timer 0 has passed 0x80
	ret
dont_flag_yet:
	lds	r19,rollover_flag
	cpi	r19,0xff	;if timer 0 is below 0x80 and rollover_flag = 0xff then
				;timer 0 has rolled over and 0x7a.. needs to be incremented
	breq	inc_timer_0_cnt
	ret
inc_timer_0_cnt:
	clr	r19
	sts	rollover_flag,r19	;clear flag
	
	ldi	yl_r28,total_seconds
	clr	yh_r29		;(Y) points to data at 0x007a..0x007d
	
loop262:	
	ld	r19,y
	inc	r19
	st	y+,r19

	brne	exit263		;if this byte rolled over to zero, inc next byte
	
	cpi	yl_r28,total_seconds+4
	brlo	loop262
exit263:
	ret

;*********************************************************************************************
perform_adc:	;channel address is in r29: 0..7; conversion loop counter is in r25,r26    read_adc
		
	out 	admux,r29
	
	rcall	reset_timer_1_1
	sbi	adcsr,adsc	;set start conversion bit

adc_loop:

	sbis	adcsr,adif
	rjmp	adc_loop	

	rcall	read_timer_1
	
	rcall	reset_timer_1_64
	sbi	adcsr,adif	;clear bit by writing a '1' to it
	
	ret
	
;*********************************************************************************************
perform_8_bit_adc:		;call with address (0..7) in r29. 8 bit result is returned in
				            ;r29

	rcall	perform_adc
	
	in	r29,adcl
	in	r30,adch
	
	lsr	r30		;divide 10 bit result by 4 to get 8 bit value
	ror	r29
	
	lsr	r30
	ror	r29
	
	ret
;*********************************************************************************************
init_anacomp:
		ret
;*********************************************************************************************
init_adc:	

	ldi	r29,0x05
	out	adcsr,r29	;set prescale to 32
				;at 2.4576 mhz, prescale 32 = 76.8 khz
				;"   "      "      "     16 = 153 khz
				
	sbi	adcsr,aden	;set adc enable bit			
	
	ldi	r29,0x00
	rcall	perform_adc	;then perform one conversion to initialize adc
		
	ret
			
;*********************************************************************************************
load_flash_constants: ; this doesnt load from flash anymore, but some are loaded from internal eep

	ldi	r30,master_id
	sts	unit_id,r30


   ldi r29,low(pause_seconds_h_eep)
   mov r0,r29
   ldi r29,high(pause_seconds_h_eep)
   mov r1,r29
   rcall read_eep2
   mov r29,r0

   cpi r29,0xFF                  ; after flash, eep is all 0xff
   breq line2781w

   sts pause_seconds_h,r29

   ldi r29,low(pause_seconds_l_eep)
   mov r0,r29
   ldi r29,high(pause_seconds_l_eep)
   mov r1,r29
   rcall read_eep2
   mov r29,r0
   
   cpi r29,0xFF                  ; after flash, eep is all 0xff
   breq line2781w
   
   sts pause_seconds_l,r29

   rjmp line2782w

line2781w:


   ldi r29,0x18                        ; change pause seconds to 280d = 0x0118
	sts	pause_seconds_l,r29
	
   ldi r29,0x01
	sts	pause_seconds_h,r29	
line2782w:

	
   ldi r29,tank_empty_f
	
	sts	tank_empty,r29	
	
	ldi r29, tank_full_f
	sts	tank_full,r29			
	
   ldi r29,auto_turnoff_mins_f
	
	sts	auto_turnoff_mins,r29		

   ; 5-15-2011 added xbee packet 0xCC to write cutoff values to eep. if eep has non-zero values, use them.

   ldi r29,low(flow_1_2_eep)
   mov r0,r29
   ldi r29,high(flow_1_2_eep)
   mov r1,r29
   rcall read_eep2
   mov r29,r0
   cpi r29,0
   breq line2781

   cpi r29,0xFF                  ; after flash, eep is all 0xff
   breq line2781

   sts flow_1_2_cutoff,r29

   ldi r29,low(flow_3_4_eep)
   mov r0,r29
   ldi r29,high(flow_3_4_eep)
   mov r1,r29
   rcall read_eep2
   mov r29,r0
   
   cpi r29,0
   breq line2781
   
   cpi r29,0xFF                  ; after flash, eep is all 0xff
   breq line2781
   
   sts flow_3_4_cutoff,r29

   rjmp line2782

line2781:
	
	ldi r29,flow_1_2_cutoff_f
	sts	flow_1_2_cutoff,r29			
	
	
	
	ldi r29,flow_3_4_cutoff_f
	sts	flow_3_4_cutoff,r29			
line2782:
	





   ldi r29,low(control_byte_eep)
   mov r0,r29
   ldi r29,high(control_byte_eep)
   mov r1,r29
   rcall read_eep2
   mov r29,r0
   
   cpi r29,0xFF                  ; after flash, eep is all 0xff
   breq cbline2781w
   
   sts control_byte,r29

   rjmp cbline2782w

cbline2781w:


   ldi r29,0x99                        ; 0x99 on    0x88 off      
	sts	control_byte,r29
	
cbline2782w:

	





	ret
;*********************************************************************************************

set_yellow_led_per_r4:				;preserves r29; uses r4
		
		push	r29
		ldi	r29,1
		cpse	r29,r4			;anacomp interrupt sets r4 = 01	
		
		
		rjmp	led_off
		
		rcall	yellow_led_on		;uses no regs
		pop	r29
		ret
		
led_off:	rcall	yellow_led_off		;uses no regs
		pop 	r29
		ret		

;*********************************************************************************************

read_timer_1:		;uses r25,r26
		in	t1_l_r25,tcnt1l
		in	t1_h_r26,tcnt1h

		ret
;*********************************************************************************************
inc_xeep_addr:		
	inc	xeep_l_r6
	brne	lined682a		;use brne because inc sets z, but not carry flag
					;so, if r6 did not roll over, it does not = 0
					;so do not inc r7
	inc	xeep_h_r7
lined682a:		
	ret		
;*********************************************************************************************
write_xeep_inc:				;uses r3, 6, 7, 16, 17, 18, 19 25, 26, 29
					; writes r3 to r7h:r6l
	rcall 	write_xeep
	rcall	inc_xeep_addr
	ret

;*********************************************************************************************
read_xeep_inc:	;uses r3, r6, r7, r16, r17, r18, r19

	rcall	read_xeep
inc_r6_r7:
	inc	r6
	brne	line1067
	inc	r7
line1067:
	ret
			
;*********************************************************************************************
	
send_packet:	;uses r8,r16,r17,r18,r19,r20,r21,r23,r25,r26,r30,r31

		;call this routine with sender id,receiver id and 
		;four data bytes stored in sram at sender_id_s...
		
		rcall	green_led_on
		rcall	yellow_led_on
		
		
		ldi	zl_r30,sender_id_s
		clr	zh_r31
		rcall	calc_checksum		;uses r8,r16,r17,r18,r30,r31

		lds	r16,check_high_temp	;copy calculated checksum to loc following 
		sts	send_check_high,r16	;packet data bytes
		
		lds	r16,check_high_temp+1
		sts	send_check_low,r16

      rcall xbee_send_8_old_style


		ret

;*********************************************************************************************



.ifdef def_box8                        ; def box_8

		
yellow_led_on:		;uses no regs


		cbi	portd,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portd,yellow_led	;raise line to turn led off
		ret


green_led_on:		;uses no regs
		cbi	portd,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portd,green_led		;raise line to turn led off
		ret

.endif


.ifdef def_box6
		
yellow_led_on:		;uses no regs

		cbi	portc,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portc,yellow_led	;raise line to turn led off
		ret

green_led_on:		;uses no regs
		cbi	portc,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portc,green_led		;raise line to turn led off
		ret

.endif



.ifdef def_box5

		
yellow_led_on:		;uses no regs


		cbi	portd,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portd,yellow_led	;raise line to turn led off
		ret


green_led_on:		;uses no regs
		cbi	portd,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portd,green_led		;raise line to turn led off
		ret

.endif

.ifdef def_box3
	
yellow_led_on:		;uses no regs


		cbi	portc,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portc,yellow_led	;raise line to turn led off
		ret


green_led_on:		;uses no regs
		cbi	portc,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portc,green_led		;raise line to turn led off
		ret

.endif



.ifdef def_box4
	
yellow_led_on:		;uses no regs


		cbi	portc,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portc,yellow_led	;raise line to turn led off
		ret


green_led_on:		;uses no regs
		cbi	portc,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portc,green_led		;raise line to turn led off
		ret

.endif


.ifdef def_box1
	
yellow_led_on:		;uses no regs


		cbi	portc,yellow_led	;lower line to turn led on
		ret

yellow_led_off:		;uses no regs
		sbi	portc,yellow_led	;raise line to turn led off
		ret


green_led_on:		;uses no regs
		cbi	portc,green_led		;lower line to turn led on
		ret

green_led_off:		;uses no regs
		sbi	portc,green_led		;raise line to turn led off
		ret



.endif
;*********************************************************************************************		

wait_for_eewe:		;uses no regs
wait_213:	sbic	eecr,eewe
		rjmp	wait_213
		ret

;*********************************************************************************************		
read_eep2:		; input addrl = r0, addrh = r1; out data is in r0
		rcall	wait_for_eewe		;wait for eeprom write enable

		out	eearl,r0		
		
		out	eearh,r1
		
		ldi	r29,0x01
		out	eecr,r29		;set eere
		
wait_2642:	sbic	eecr,eere		;wait for eere to be cleared
		rjmp	wait_2642
		
		in	r0,eedr	
r17_ok2:	ret

		
;*********************************************************************************************		
write_eep:	
		;write byte in write_eep_byte_r22 to internal eep at addrh = r30   addrl = zh_r31
      ; 5-15-2011 ADDL IS IN R30, isn't it?

		rcall	wait_for_eewe		;wait for eeprom write enable
		
		cli				;an interrupt in the following block
						;will cause eep write to fail

		out	eearl,zl_r30
		out	eearh,zh_r31
		
		out	eedr,write_eep_byte_r22
		
		ldi	r29,4			;write 1 to eemwe, 0 to eewe
		ldi	r28,2			;write 1 to eewe

		out	eecr,r29
		out	eecr,r28
		
		sei
		
		ret
;*********************************************************************************************		
calc_byte_sum:	
		;uses r9,r18,r30,r31
;intel check		;calc sum on 7 bytes in sram at (Z)		
		;calc sum on 6 bytes in sram at (Z)
		;return with zero flag set if all bytes = zero
		
		clr	r18	;loop counter		
loop591a:		
		ld	r9,z+
		tst	r9
		brne	packet_ok
				
		inc	r18
;intel check			cpi	r18,7
		cpi	r18,6
		
		brne	loop591a
		sez		;all bytes were zero, so set zero flag
		
packet_ok:			;zero flag is clear if we branched here
		ret
		
;*********************************************************************************************		
calc_checksum:	
		;uses r8,r9,r16,r17,r18,r30,r31
		;calc checksum on 6 bytes in sram at (Z)
		;NOTE THAT ANSWER IS NOT STORED IN FOLLOWING TWO BYTES. IT IS STORED
		;AT CHECK_HIgh_TEMP AND NEXT BYTE
		
		
		clr	r16	;store checksum low r16, hi r17
		clr	r17
		clr	r8	;use r8 =0 for simple adding
		clr	r18	;loop counter
		
loop591:		
		ld	r9,z+
		
		add	r16,r9		;lsb is r16
		adc	r17,r8		;msb is r17
		
		inc	r18
		cpi	r18,6
		
		brne	loop591
		
		ldi	zl_r30,check_high_temp
		clr	zh_r31
		
		st	z+,r17		;store msb first
		st	z+,r16		;then lsb following msb
		
		ret

;*********************************************************************************************		

flag_pc:	;lower line briefly to tell pc a valid packet has arrived

	ldi	r16,0xff-0x02
	and	r13,r16		;clear bit 1 to see if pc gets packet


   ret

;*********************************************************************************************		

wait_for_rollover:	;uses (Y) in timer_0_check
			;
			;if timer 0 = 255, wait for rollover. otherwise, it will be safe
			;for at least one second to read all sram contents, since they
			;only change at the end of the second where timer = 255
				
	rcall	timer_0_check	;uses (Y) to update timer_0 counters
		
	in	r29,tcnt0
	cpi	r29,0xff	;wait for tcnt <> 255 since rollover is about to occur
	breq 	wait_for_rollover_2
	ret
wait_for_rollover_2:
	rcall	timer_0_check
	lds	r29,rollover_flag	;have to wait for this flag to clear to be certain
					;that sram counter is updated
	cpi	r29,0xff
	breq	wait_for_rollover_2
	ret

;*********************************************************************************************		
calc_elapsed_time:	;calcs elapsed time for count stored at (z); return in 0xa4..0xa7
	rcall	wait_for_rollover	;uses (Y)
	
	ldi	yl_r28,total_seconds	;(Y) points to timer data
	clr	yh_r29
	
	ldi	xl_r26,calc_result	;(X) points to result table
	clr	xh_r27
	
	in	r19,tcnt0	
	ld	r18,z+	
	sub	r19,r18	
	rol	r20			;save carry
	st	x+,r19
	
loop713:
	ld	r18,z+
	ld	r19,y+
	ror	r20			;restore carry
	sbc	r19,r18
	rol	r20			;save carry, since cpi may alter it
	st	x+,r19
	cpi	xl_r26,calc_result+4
	brne	loop713
	
	ret	
;*********************************************************************************************		
calc_remaining_time:	;calcs remaining time for count stored at (z); return in 0xa4..0xa7
	rcall	wait_for_rollover	;uses (Y)
	
	ldi	yl_r28,total_seconds	;(Y) points to timer data
	clr	yh_r29
	
	ldi	xl_r26,calc_result	;(X) points to result table
	clr	xh_r27
	
	in	r19,tcnt0	
	ld	r18,z+	
	sub	r18,r19	
	rol	r20			;save carry
	st	x+,r18
	
loop713a:
	ld	r18,z+
	ld	r19,y+
	ror	r20			;restore carry
	sbc	r18,r19
	rol	r20			;save carry, since cpi may alter it
	st	x+,r18
	cpi	xl_r26,calc_result+4
	brne	loop713a
	
	ret		
	
;*********************************************************************************************		
copy_timer_0:		;copies timer 0 cnt and sram 0x7a..0x7c to (Z)

	rcall	wait_for_rollover	;uses (Y)
	
	rcall	timer_0_check	;update timer_0 counters
		
	in	r19,tcnt0
	cpi	r19,0xff	;wait for tcnt <> 255 since rollover is about to occur
	breq 	copy_timer_0
	
	st	z+,r19
	
	ldi	yl_r28,total_seconds
	clr	yh_r29
loop693:
	ld	r19,y+
	st	z+,r19
	cpi	yl_r28,total_seconds+3
	brne	loop693
	
	ret
		
;*********************************************************************************************		

mult_r29_by_864:		;this decrements r29 first, then multiplies by 864 = 0x360
				
				;uses r16,17,18,19,20,21,22,29
	dec	r29

	ldi	mc16uL,0x60		;multiply (day-1) by 864 = 0x0360
					
					;864 = 24 hours x 12 packets/hour x 3 bytes/packet
					;so 864 bytes are stored each day
					
					;sunday 0..863 = 0..35f
					;monday          360..6bf
					;tuesday         6c0..a1f
					;wednesday       a20..d7f
					;thursday        d80..10df
					;friday          10e0..143f
					;saturday        1440..179f 	
					
					;xeep 24lc256 = 256/8 = 32768 bytes = 0000..7fff
					;xeep 24lc64 = 64/8 = 8192 bytes = 0000..1fff
	ldi	mc16uH,0x03
	mov	mp16uL,r29
	ldi	mp16uH,0
	
	rcall	mpy16u			;result: m16u3(r21):m16u2(r20):m16u1(r19):m16u0(r18)	
	ret	
;*********************************************************************************************			
calc_address_for_3_bytes:
	
	rcall	record_clock_in_sram_verify
		;store second, minutes, hours, day, and date at 0x90,2,3,4
	
	lds	r29,rtc_day		;load day 1..7
	cpi	r29,0
	breq	day_0
	
	rcall	mult_r29_by_864		;NOTE THAT R29 IS DECREMENTED BEFORE MULTIPLICATION
		
	mov	r8,m16u0
	mov	r9,m16u1
	rjmp	line586
		
day_0:
	clr	r8
	clr	r9
line586:


	lds	mc8u,rtc_hours		;get hour number
	ldi	mp8u,36			;multiply by 36 bytes per hour
	rcall	mpy8u			;result: m8uH:m8uL 
	
	add	r8,m8ul
	adc	r9,m8uh			;add to tally in r8,r9

	lds	dd8u,rtc_minutes	;get minutes
	ldi	dv8u,5			;divide by 5 minutes per packet
	rcall	div8u
	
	mov	mc8u,dres8u
	ldi	mp8u,3
	rcall	mpy8u			;multiply by 3 bytes per packet
	
	add	r8,m8ul
	adc	r9,m8uh			;add to tally in r8,r9

	mov	xeep_h_r7,r9
	mov	xeep_l_r6,r8
	
	sts	packet_address_l,r8	;save packet address to send to pc
	sts	packet_address_h,r9
		
	ret	
	
		
;*********************************************************************************************		
do_date_stuff:	
	
	rcall	read_xeep		;read current id byte; z already points there
					;
					;
					;to avoid having to ever erase xeep:
					;
					;first, load the id/date byte and look at the date
					;
					;if the date is wrong, then fill id nibble with 1s
					;since this is the first write to this packet block
					;today
					
	lds	r29,rtc_date		;retrieve date byte
	lsl	r29			;shift left 3 bits, clearing lowest to zeros
	lsl	r29
	lsl	r29
	
	mov	r28,i2c_data_r3		;get old date/id byte just read from xeep
	andi	r28,0xf8		;clear lower three bits
	
	cp	r28,r29			;compare dates 
	breq	already_fixed		;branch if dates already match
	
	ldi	r29,0xff		;otherwise reset all bits to 1s
	mov	i2c_data_r3,r29		;save revised byte 
   
already_fixed:	

	ret
	
;*********************************************************************************************		
do_date_stuff_2:	
	
	lds	r29,rtc_date		;load date to use as date stamp in upper nibble
					;note that date has been converted from bcd to binary
					;but that is not a problem as long as i remember lol
	
	lsl	r29			;shift left 3 bits, filling with zeros
	lsl	r29
	lsl	r29
	ori	r29,0x07		;set lower 3 bits to ones
	
	and	i2c_data_r3,r29		;'mov' date to idbyte by anding
	
	rcall	write_xeep		;store id byte (in r3) in xeep at r6,r7
	
	ret
	
;*********************************************************************************************		
save_2_data_bytes:	

	ldi	loop_ctr_r23,0x01	;loop_ctr_r23 counts sram registers 
	
loop627a:
	ld	r29,z+			;retrieve a packet byte from sram
	mov	i2c_data_r3,r29
		
	rcall	write_xeep_inc		;store a packet byte (in r3) in xeep at r6,r7

	inc	loop_ctr_r23
	
	cpi	loop_ctr_r23,0x03	;store 2 data bytes first
	brne 	loop627a
	
	ret	
;*********************************************************************************************		
save_zero_byte_in_xeep:	;writes 0 byte to xeep at r6:r7. increments r6:r7	

	clr	r29
	mov	i2c_data_r3,r29
		
	rcall	write_xeep_inc		;store a packet byte (in r3) in xeep at r6,r7
	
	ret		
;*********************************************************************************************		
save_1_data_byte:	;writes byte at (Z) to xeep at r6:r7. increments (Z) and r6:r7	

	ld	r29,z+			;retrieve a packet byte from sram
	mov	i2c_data_r3,r29
		
	rcall	write_xeep_inc		;store a packet byte (in r3) in xeep at r6,r7

	
	ret			
;*********************************************************************************************		
save_5_data_bytes:	

	ldi	loop_ctr_r23,0x01	;loop_ctr_r23 counts sram registers 
	
loop627b:
	ld	r29,z+			;retrieve a packet byte from sram
	mov	i2c_data_r3,r29
		
	rcall	write_xeep_inc		;store a packet byte (in r3) in xeep at r6,r7

	inc	loop_ctr_r23
	
	cpi	loop_ctr_r23,0x06	
	brne 	loop627b
	
	ret	
	
;****************************************************
check_xeep_already_written:

	rcall	calc_address_for_3_bytes         ; puts xeep addr in r6:r7

   rcall inc_xeep_addr	 ; skip 2 data bytes - point to datestamp	
   rcall inc_xeep_addr  		
	
	rcall	read_xeep		
						
   mov r21,r3              ; save dstestamp 
	lds	r29,sender_id_check

   and r29,r21             ; if this sender is already saved, the bit is clear, so AND == 0
   breq  already_id

   ldi r29,0x00
   sts xeep_already_written,r29     ; set flag == not written
   ret

already_id:

	lds	r29,rtc_date		;retrieve date byte
	lsl	r29			;shift left 3 bits, clearing lowest to zeros
	lsl	r29
	lsl	r29
	
	mov	r28,i2c_data_r3		;get old date/id byte just read from xeep
	andi	r28,0xf8		;clear lower three bits
	
	cp	r28,r29			;compare dates 
	breq	already_fixed3		;branch if dates already match

   ldi r29,0x00
   sts xeep_already_written,r29     ; set flag == not written
   ret

already_fixed3:
   ldi r29,0xFF
   sts xeep_already_written,r29     ; set flag == written already
   ret
;*********************************************************************************************		
save_master_data_in_xeep:	;this saves packets that master SENDS, since master generates
				;all of the data.	

	rcall	calc_address_for_3_bytes
			
	ldi	zl_r30,sender_id_s+2	;point z to first data byte (skip 2 id bytes)
	clr	zh_r31
	
	rcall	save_2_data_bytes	;status byte, then tank level (0..100) are stored first
	
	rcall	do_date_stuff
	
	lds	r29,unit_id		;master sent this packet
	
	com	r29			;invert bits, so that id bit can be 'anded'
					;note that this only works correctly for unit ids
					;that consist of one '1' bit in the lowest 3 bits,
					;which would be units 1 (master), 2, and 4 (slave)
	
	and	i2c_data_r3,r29		;clear appropriate id bit

	rcall	do_date_stuff_2
	
	ret


;*********************************************************************************************		

process_packet:	;packet is temporarily stored in sram at 0x60..0x67
		;
		;if checksum is ok, and sender/receiver = 1-4 or 4-1 then store
		;two data bytes (status and tank level) plus sender id and date stamp
		;in xeep
		;
		;actually, the only valid packet i want to ignore is X 1 0 0 0 0 0 (-x-1)
		;so if checksum = -x-1 ignore the packet. all other valid packets should
		;contain normal data. or, just ignore packets from 'prompt_id'
		

	ldi	zl_r30,packet_bytes
	clr	zh_r31
	rcall	calc_byte_sum	;skip packet if all zeroes = noise
	brne	ok_1296
	rjmp	inc_invalid_total
ok_1296:
	ldi	zl_r30,packet_bytes
	clr	zh_r31
	rcall	calc_checksum	;uses r8,r16,r17,r18,r30,r31

	lds	r29,check_high_temp
	lds	r28,check_high
	cp	r28,r29
	breq	ok_607
	rjmp	save_bad_peak_count	;bad checksum hi byte
ok_607:
	lds	r29,check_low_temp
	lds	r28,check_low
	cp	r28,r29
	breq	ok_613
	rjmp	save_bad_peak_count	;bad checksum lo byte
ok_613:
	lds	r28,sender_id
	
	lds	r29,receiver_id		;get receiver id
	lds	r28,unit_id		;get this unit's id	

	cp	r28,r29			;was this packet sent to this unit?
	brne	flag_pc_1		;if not, skip following block
	
	
	rcall	master_code
flag_pc_1:
		 	
   lds r19,control_byte       ; 0x99 on    0x88 off      
   cpi r19,0x99
   brne  dead88cc


	rcall	flash_leds_good_packet
dead88cc:

	ret

save_bad_peak_count:			;checksum was bad. save peak counts
inc_invalid_total:	
	
	ret


;*********************************************************************************************
	
clear_sram:		;sram is 0x60..0x25f
			;
			;don't clear stack: stop at 0x250
	
	
	lds	r17,reset_flags	;save this
	
	lds	r18,reset_cnt     	; save this
   lds   r19,xbee_reset_cnt
   lds   r20,xbee_reset_cnt + 1
   lds   r21,usbtiny_2313_reset_cnt

   ldi	zl_r30,0x60
	clr	zh_r31
	
	clr	r16
loop1287:
	st	z+,r16
	cpi	zl_r30,0
	brne	loop1287
	
	
loop1288:
	st	z+,r16
	cpi	zl_r30,0
	brne	loop1288

	
loop1289:
	st	z+,r16
	cpi	zl_r30,0x50	;stop at 0x250
	brne	loop1289
	
	sts	reset_flags,r17	;restore this

	sts	reset_cnt,r18	   ; restore this

   sts   xbee_reset_cnt,r19
   sts   xbee_reset_cnt + 1,r20

   ldi r29,0x01
   ;ldi r29,0x20
   sts uart_receive_cnt,r29

   sts usbtiny_2313_reset_cnt,r21
	ret
;*********************************************************************************************
read_clock_current_byte:	;reads next rtc byte. addr is automatically incremented
	cli	
	
	ldi	i2cadr,$d0+i2crd	; Set device address and read
	rcall	i2c_rep_start		; Send repeated start condition and address

	sec				; Set no acknowledge (read is followed by a stop condition)
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata

	rcall	i2c_stop		; Send stop condition - releases bus
	sei
	
	ret
;*********************************************************************************************
read_clock:	;reads rtc  at r7 and returns byte in r3; uses r16,r17,r18,r19
	
	cli	
	ldi	i2cadr,$d0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,xeep_h_r7	; Write word address
	rcall	i2c_do_transfer		; Execute transfer
	
	ldi	i2cadr,$d0+i2crd	; Set device address and read
	rcall	i2c_rep_start		; Send repeated start condition and address

	sec				; Set no acknowledge (read is followed by a stop condition)
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata

	rcall	i2c_stop		; Send stop condition - releases bus
	sei
	
	ret	
;*********************************************************************************************

record_clock_in_sram:		;save 0,1,2,3,4 clock registers in sram at 0x90..0x94
				;after converting packed bcd to binary format

	ldi	zl_r30,rtc_data	;load pointer to sram 0x0090 = (Z) pointer
	clr	zh_r31


	clr	loop_ctr_r23	;loop_ctr_r23 counts clock registers from $00 to $07
	
loop950:

	mov	xeep_h_r7,loop_ctr_r23  ;read clock register using xeep_h_r7 = addr
	
	rcall	read_clock		;read a clock register: data returned in i2c_data_r3

	mov	fbcdh,i2c_data_r3
	rcall	bcd2bin8
		
	st	z+,tbin		;save in sram using zl_r30,zh_r31 pointer	

	inc	loop_ctr_r23
	
	cpi	loop_ctr_r23,0x08
	brne 	loop950

	ret
	
;*********************************************************************************************
record_clock_in_sram_verify:

	
	rcall record_clock_in_sram	;move these to 0x98..
	
	
	ldi	zl_r30,rtc_data	;load pointer to sram 0x0090 = (Z) pointer
	clr	zh_r31
	
	ldi	xl_r26,rtc_verify	;also point to 0x98
	clr	xh_r27
loop989:	
	ld	r29,z+
	st	x+,r29
	
	cpi	zl_r30,rtc_data+5
	brne	loop989
		
	rcall record_clock_in_sram	;leave these at 0x91.. and compare
	
	ldi	zl_r30,rtc_data	;load pointer to sram 0x0090 = (Z) pointer
	clr	zh_r31
	
	ldi	xl_r26,rtc_verify	;also point to 0x98
	clr	xh_r27
loop999:	
	ld	r29,z+
	ld	r28,x+
	
	cp	r28,r29
	brne 	record_clock_in_sram_verify   ;infinite loop until hr, min, sec, etc. match
	
	cpi	zl_r30,rtc_data+5
	brne	loop999	
	
	
	ret

;*********************************************************************************************

init_clock3:	
;this procedure copies 7 bytes from sram at $72..$79 into rtc registers 0..7
;to set time, date, and format

	
	ldi	zl_r30,set_rtc_data	;z will point to sram at $72..$79
	ldi	zh_r31,0x00
	
	ldi	r20,0x00		;r20 will point to rtc registers

loop748:		
	ld	r21,z+			;get a byte from sram
	
line773:
	rcall	write_i2c_rtc		;put data byte in r21, address in r20
					; uses r16,r17,r18,r19,r20,r21
	inc	r20
	cpi	r20,0x08
	brne 	loop748

	ret

;*********************************************************************************************

write_xeep:				;writes byte in i2c_data_r3 to addrh xeep_h_r7, 
					;addrl xeep_l_r6 in ext eep
			
	cli		
	ldi	i2cadr,$A0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,xeep_h_r7	; Write word address high(0x00)
	rcall	i2c_do_transfer		; Execute transfer
	
	mov	i2cdata,xeep_l_r6	; Write word address low(0x00)
	rcall	i2c_do_transfer		; Execute transfer

	mov	i2cdata,i2c_data_r3	; Set write data to 01010101b
	rcall	i2c_do_transfer		; Execute transfer

	rcall	i2c_stop		; Send stop condition
	
	rcall	pause_five_msec		;ext eep needs this for write operations. it might
	;be an improvement to change this to check for ack, since the xeep will not ack
	;until the write cycle is complete.
	sei
	ret


;*********************************************************************************************

write_xeep_page:			;writes byte in i2c_data_r3 to addrh xeep_h_r7, 
					;addrl xeep_l_r6 in ext eep
	cli				
	ldi	i2cadr,$A0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,xeep_h_r7	; Write word address high(0x00)
	rcall	i2c_do_transfer		; Execute transfer
	
	mov	i2cdata,xeep_l_r6	; Write word address low(0x00)
	rcall	i2c_do_transfer		; Execute transfer
	
	ldi	r20,0x00
loop1566:	

	mov	i2cdata,i2c_data_r3	; Set write data to 01010101b
	rcall	i2c_do_transfer		; Execute transfer
	inc	r20
	cpi	r20,0x20
	brne	loop1566

	rcall	i2c_stop		; Send stop condition
	
	rcall	pause_five_msec		;ext eep needs this for write operations
	sei
	ret


;*********************************************************************************************
read_xeep:				;uses r3, r6, r7, r16, r17, r18, r19
					;returns byte in i2c_data_r3, read from addrh 
					;xeep_h_r7, addrl xeep_l_r6 in ext eep
	cli
	ldi	i2cadr,$A0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,xeep_h_r7	; Write word address high
	rcall	i2c_do_transfer		; Execute transfer
	
	mov	i2cdata,xeep_l_r6	; Write word address low
	rcall	i2c_do_transfer		; Execute transfer

	ldi	i2cadr,$A0+i2crd	; Set device address and read
	rcall	i2c_rep_start		; Send repeated start condition and address

	sec				; Set no acknowledge (read is followed by a stop condition)
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata

	rcall	i2c_stop		; Send stop condition - releases bus
	sei
	ret

;*********************************************************************************************
read_xeep_current_byte:			;returns byte in i2c_data_r3, read from current
					;address 
	cli
	ldi	i2cadr,$A0+i2crd	; Set device address and read
	rcall	i2c_rep_start		; Send repeated start condition and address

	sec				; Set no acknowledge (read is followed by a stop condition)
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata

	rcall	i2c_stop		; Send stop condition - releases bus
	sei
	ret

;*********************************************************************************************
read_xeep_start:			;returns byte in i2c_data_r3, read from addrh 
					;xeep_h_r7, addrl xeep_l_r6 in ext eep
	cli
	ldi	i2cadr,$A0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,xeep_h_r7	; Write word address high
	rcall	i2c_do_transfer		; Execute transfer
	
	mov	i2cdata,xeep_l_r6	; Write word address low
	rcall	i2c_do_transfer		; Execute transfer
	
	ldi	i2cadr,$A0+i2crd	; Set device address and read
	rcall	i2c_rep_start		; Send repeated start condition and address
	sei
	ret
	
read_xeep_byte:
	cli
	ldi	i2cadr,$A0+i2crd	; Set device address and read
	clc				; Set to acknowledge all but final byte
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata
	sei
	ret
read_xeep_stop:	;note that this also reads the final byte 
	cli
	sec				; Set no acknowledge for final byte
	rcall	i2c_do_transfer		; Execute transfer (read)
	mov	i2c_data_r3,i2cdata

	rcall	i2c_stop		; Send stop condition - releases bus
	sei
	ret

;*********************************************************************************************
erase_xeep:	;write 0xff to xeep
		;
		;24lc64 eep ends at 0x1fff
		;24lc256 eep ends at 0x7fff
	
	clr	zl_r30
	clr	zh_r31
	ldi	r16,0xff
	mov	r3,r16
loop1583:
	wdr
	mov	r7,zh_r31
	mov	r6,zl_r30

	rcall	write_xeep_page		;writes byte in i2c_data_r3 to addrh xeep_h_r7, 
					;addrl xeep_l_r6 in ext eep

	adiw	zl_r30,0x20
	cpi	zh_r31,0x80
	brne	loop1583
	
	ret
;*********************************************************************************************
init_spi:
	sbi 	spcr,spe
	cbi 	spcr,cpha ;  my original was cbi. usb is too.
	cbi 	spcr,dord
	cbi 	spcr,cpol
	cbi 	spcr,mstr
	
	sbi 	ddrb,pb6	;configure miso as an output

;	sbic	usb_pin,usb_select	;if this pin is low, we are using usb, so skip next line
;	sbi 	portb,spi_ready	;raise line = busy
	
	sbi	portb,mosi	;enable pullup, to avoid false requests when pc
				;is off or disconnected
	
;	ldi 	r29,0xCC
	ldi 	r29,0xAB   ; changed to confirm new code was working
	out 	spdr,r29
	ret
;*********************************************************************************************
brief_pause:			;this loads r29 with 6, then decrements to zero. total
				;pause for this routine, not including the call is:
				;ldi: 1
				;dec 6 to 0 = 6 x 1 = 6
				;brne: branch 5 times: 5 x 2 + 1 = 11
				;ret 4
				;total: 22 / 2.4576mhz = 9 micro seconds
	ldi	r29,0x06
wait1189:
	dec	r29
	brne	wait1189
	ret


;*********************************************************************************************

get_spi_byte_r2: ;read_spi_r2 get_spi_r2 read_byte_r2

	add	r2,r29		;adds r29 to checksum in r2

get_spi_byte:	;puts r29 on spi, then gets next byte from spi and returns it in r29
	out	spdr,r29
	
wait1209:
	
	sbis	spsr,spif

	rjmp	wait1209

	in	r29,spsr
	in 	r29,spdr
	
	ret
		
;*********************************************************************************************
send_checksum_2:;checksum is in r2. it has not been negated.	
	
	neg 	r2
	mov	r29,r2
	rcall	get_spi_byte
	ret
;*********************************************************************************************

process_usb:

	cpi 	r29,0x11	;check for 'reset 8535' command
	brne 	exit1230c2
	

	;infinite loop - watchdog will reset 8535

reset_now:
	rjmp reset_now
		
exit1230c2:;------------------------------------------------------------	










	cpi 	r29,0x19    ; send 0xCC new cutoff threshold to master
	brne 	exit_5147

	rcall	get_spi_byte_r2	
   sts   new_flow_1_2_cutoff,r29

	rcall	get_spi_byte_r2	
   sts   new_flow_3_4_cutoff,r29
  
	rcall	get_spi_byte_r2	
   sts   new_pause_seconds_h,r29

	rcall	get_spi_byte_r2	
   sts   new_pause_seconds_l,r29
  
	rcall	get_spi_byte_r2	   ; sanity byte 1
   cpi r29,0xF3
   brne line5157
	
   rcall	get_spi_byte_r2	   ; sanity byte 2
   cpi r29,0x3F
   brne line5157

   rcall send_master_cutoff_values       ; 0xCC packet sent to all listeners, but mainly affects master

line5157:
   ret

exit_5147:;----------------------------------------


	cpi 	r29,0x1A    ; send 0xDD ask what is cutoff threshold to master
	brne 	exit_5157d

   rcall ask_master_cutoff_values         ;  0xDD ask for master 0xDE reply cutoff values

   ret

exit_5157d:;----------------------

   cpi r29,0x1f
   brne line4444

   ldi r29,0x5d              ; // ack 0x1f msg = 0x5d 0x4a
   rcall	get_spi_byte_r2
   

   ldi r29,0x4a               ;// ack 0x1f msg 
   rcall	get_spi_byte_r2
   

   rjmp set_ff_timer   ; set fork flag

line4444:


;---------------------------------------------

   cpi r29,0x20

   brne line4424

   ldi r29,0x31               ;// ack 0x20 msg = 0x31 0xf8
   rcall	get_spi_byte_r2
   

   ldi r29,0xf8            
   rcall	get_spi_byte_r2
   

   clr r29
   sts fork_flag,r29

   ret

line4424:





	cpi 	r29,0x40	;was 40 before usb.. dummy command 0x40 just acks 0x42, 0x42
	brne 	exit1230
	
	
	ldi	r29,0x55                      ; 4-21-2019 was 0x42		;send start code to pc
	rcall	get_spi_byte_r2	
	
	ldi	r29,0x65                      ; was 0x42		;send completion code to pc
	rcall	get_spi_byte_r2
		

	ret
	
exit1230:;------------------------------------------------------------	


exit1232vc:;------------------------------------------------------------
;----------------------------------------------------------------------
	cpi	r29,0x66	;was 66 before usb. check for "read rtc hms" command = 0xa6
	brne 	exit1231
	
	rcall	record_clock_in_sram_verify	;copy clock registers into sram 0x90..0x97
	
	ldi	zl_r30,rtc_data
	clr	zh_r31
	
	clr	r2		;checksum
	
loop1264:

	ld	r29,z+		;get a byte
	
	rcall	get_spi_byte_r2
	
	cpi	zl_r30,rtc_data+7
	brne	loop1264
	
	lds	r29,reset_flags	;get reset status byte
	rcall	get_spi_byte_r2	;send reset byte to pc
	
	lds	r29,reset_cnt
	rcall	get_spi_byte_r2

	rcall	get_spi_byte_r2
	rcall	get_spi_byte_r2
	rcall	get_spi_byte_r2
	rcall	get_spi_byte_r2
	rcall	get_spi_byte_r2
	
	rcall	send_checksum_2
	ret
	
	rjmp	loop1264
exit1231:;------------------------------------------------------------

	cpi	r29,0x77	;was 77 before usb. check for "read int eep" command. note that this proc
				;only reads the lower 256 bytes, and uses simplified looping
				;based on that assumption
	brne	exit1283a
	
	clr	r0		;r0 and r1 point to byte in internal eep
	clr	r1
	clr	r2		;r2 = checksum
	
loop1265a:	
	push 	r0
	rcall	read_eep2	; input addrl = r0, addrh = r1; out data is in r0	
	
;	add	r2,r0		;add to checksum

	mov	r29,r0		;move byte to send into r29
	rcall	get_spi_byte_r2	;send/receive one byte
	
	pop 	r0
	
	inc	r0
	brne	loop1265a	;send 256 bytes
	
	rcall	send_checksum_2
	
	ret			;send 256 bytes each time, followed by one byte checksum

exit1283a:;------------------------------------------------------------


   cpi r29,0x78            ; check for  clr pkt_buf
   brne line2949ssf
   
   rcall clear_pkt_buf
   ret
line2949ssf:;-----------------------------------





   cpi r29,0x79            ; check for  read local RS
   brne line2949q
   
   rcall get_at_rs

   ret

line2949q: ;-------------------------------


	cpi	r29,0x7A	                     ; read last eep page. added 1-9-2019 so usbasp can read eep in new_probe
   
	brne	exit_7A
	
	clr	r0		;r0 and r1 point to byte in internal eep


	ldi	r29,high(EEPROMEND)

   mov r1,r29

	clr	r2		;r2 = checksum
	
loop_7A:	
	push 	r0
	rcall	read_eep2	; input addrl = r0, addrh = r1; out data is in r0	

   wdr 

;	add	r2,r0		;add to checksum

	mov	r29,r0		;move byte to send into r29
	rcall	get_spi_byte_r2	;send/receive one byte
	
	pop 	r0
	
	inc	r0
	brne	loop_7A	;send 256 bytes
	
	rcall	send_checksum_2
	
	ret			;send 256 bytes each time, followed by one byte checksum

exit_7A:;------------------------------------------------------------



        cpi	r29,0x86	;check for read_spi_threshold_values
        brne	exit0x86
 
 	clr	r2
 
 	lds	r29,flow_1_2_cutoff
 	rcall	get_spi_byte_r2
 	
 	lds	r29,flow_3_4_cutoff
 	rcall	get_spi_byte_r2
 	
 	lds	r29,overflow_cutoff
 	rcall	get_spi_byte_r2 	 	
		
	rcall	send_checksum_2
  
        ret

exit0x86:;---------------------------------------------------




	cpi	r29,0x88	;check for "fill int eep" command. note that this only
				;fills 0..0xff, which is lower half of eep
	brne	exit1284
	
	
	ldi	zl_r30,0
	ldi	zh_r31,0
loop1318:
	mov	write_eep_byte_r22,zl_r30	
	rcall	write_eep	;write byte in write_eep_byte_r22 to int eep at (Z)	
	
	adiw	zl_r30,1

	cpi	zl_r30,0xff
	brne	loop1318
	ret
exit1284:;------------------------------------------------------------



	cpi	r29,0x92	;check for 'prepare xeep copy xeep to eeprom' command
	brne	exit_92

	rcall	get_spi_byte	; get addrh for xeep page to be read
	
	mov	xeep_h_r7,r29		;xeep address r6,r7
	clr	r6
	
	;clr	r2		;checksum
	
	ldi	zl_r30,low(sram_scratchpad_start)			; sram pointer
	ldi	zh_r31,high(sram_scratchpad_start)
loop3918:

	; read a byte from xeep	
	
	rcall	read_xeep	;read first byte after setting xeep address pointer

	inc	xeep_l_r6	;256 bytes are always sent, so ptrh is ignored here	

	; write byte to sram
	st	z+,r3

	cpi	zl_r30,low(sram_scratchpad_start)
	brne	loop3918

	clr r29

poll_5036:
	push r29		;i'm using this as loop counter, to re-send 0x51 lots of times

	ldi r29, 0x51 ; tell pc eep is ready to read. sometimes it misses 0x51, so send it a few times
	rcall	get_spi_byte_r2	

	wdr				;maybe this is un-necessary

	cpi r29,0x93	;pc should send this to read sram as soon as it gets 0x51 
	brne not_yet_5032

	pop r29			;fix the stack before we leave

	rjmp read_sram_5032	;that proc will ret

not_yet_5032:

	pop r29		;retrieve the loop counter

	inc r29

	cpi r29,0x10		;try this many times. we are repeating just because of data collisions, i think; pc is
						;polling the crap out of us, and sometimes misses 0x51 if i just send it one time. since
						;it usually sees it, i could probably loop 3-4 times here and eliminate the problem,
						;but what does it hurt to loop 64 times? hmmm. is there a scenario where this causes
						;some other problem and makes pc wait too long? hmmm. i think i'll go back to 16 
	brne poll_5036

	;if we get here, something probably failed, maybe not, but pc can still send 0x77 to read eeprom

	ret
exit_92:;------------------------------------------------------------



;;;;;;;;;;; see cmd 0x77 above - read lower 256 bytes - but i can't use that because it is
; too slow
	cpi	r29,0x93	;check for 'read xeep data from eeprom' command
	brne	exit_93

read_sram_5032:	
	
	ldi	zl_r30,low(sram_scratchpad_start)			; sram pointer
	ldi	zh_r31,high(sram_scratchpad_start)
	
	clr	r2		;r2 = checksum
	
line4652:	
loop1265aq:	

	ld	r29,z+		;load byte from sram
	rcall	get_spi_byte_r2	;send/receive one byte

	cpi zl_r30,low(sram_scratchpad_start)
	
	brne	loop1265aq	;send 256 bytes
	
	rjmp	send_checksum_2 ;use its ret
	
	ret
exit_93:;------------------------------------------------------------



	cpi	r29,0x99	;check for "read ext eep" command. note that this
				;proc always returns 256 bytes, and uses a simplified
				;address and loop counter based on the assumption that
				;the lower address starts at 0 and ends at 0xff
	brne	exit1334
	
	rcall	get_spi_byte
	mov	xeep_h_r7,r29	

	clr	r6
	
	clr	r2		;checksum
	
	rcall	read_xeep	;read first byte after setting xeep address pointer
	mov	r29,i2c_data_r3
	rcall	get_spi_byte_r2	
	inc	xeep_l_r6	;256 bytes are always sent, so ptrh is ignored here	
	
loop1335a:	

	rcall 	read_xeep_current_byte	;returns byte in i2c_data_r3, read from current addr
				
	
	mov	r29,i2c_data_r3
	rcall	get_spi_byte_r2
					
	inc	xeep_l_r6	;256 bytes are always sent, so ptrh is ignored here

	brne	loop1335a	
	
	rcall	send_checksum_2

	ret
	
exit1334:;------------------------------------------------------------

   cpi	r29,0x9c	;check for 'read last 256 bytes of sram' command      read sram      read_sram 0x9C
	brne	exit229c	;use this to see how large stack should be
		
	clr	r2		;r2 = checksum
	
	ldi	zl_r30,0x60	;send 256 bytes: 0x0160..0x025f

	ldi	zh_r31,0x01

sloop1265aq:	

	ld	r29,z+		;load byte from sram
	rcall	get_spi_byte_r2	;send/receive one byte

	cpi zl_r30,0x60
	
	brne	sloop1265aq	;send 256 bytes
	
	rjmp	send_checksum_2 ;use its ret
	



exit229c:;------------------------------------------------------------
	cpi	r29,0x9d	;check for 'clear last 256 bytes of sram' command
	brne	exit229d	;use this to see how large stack should be
	
	ldi	r29,0x00	;set sram = 0x00
	lds	r28,last_sram	;stop at highest sram variable
	
	in	zl_r30,spl	;clear 0x0160..stack pointer

	in	zh_r31,sph
	
loop1265fd:	
	
	st	z,r29
	sbiw	zl_r30,0x01

	cp	zl_r30,r28
;	cpi	zl_r30,0x5f
	
	brne	loop1265fd
	
	ret
	
exit229d:;------------------------------------------------------------






   cpi r29,0xD1                        ; C got 0xC1 so it needs seconds since midnight
   brne not_D1

	clr r16                             ; i think this is safe because usb will get this byte
   sts pending_pulse_flag,r16
   rjmp send_secs

not_D1:

;-----------------------------------------


   cpi r29,0xD2                        ; C got 0xC2 so it needs seconds since midnight
   brne not_D2

	clr r16                             ; i think this is safe because usb will get this byte
   sts pending_float_flag,r16
   rjmp send_secs
	
not_D2:

;-----------------------------------------


   cpi r29,0xD3                        ; C got 0xC3 so it needs seconds since midnight
   brne not_D3

	clr r16                             ; i think this is safe because usb will get this byte
   sts pending_float_flag,r16
   rjmp send_secs

not_D3:

;-----------------------------------------


   cpi r29,0xD4                        ; reset_cnt = 0
   brne not_D4

	clr r16                       
   sts reset_cnt,r16
   
   rcall write_reset_eep

   ret

not_D4:

;-----------------------------------------



   cpi r29,0xD5                        ; rpi temperature
   brne not_D5

	rcall	get_spi_byte

;   sts rpi_temperature,r29
   sts float_one_s,r29

   ret

not_D5:

;-----------------------------------------


   cpi r29,0xD0                        ; C got 0xC4 so it needs seconds since midnight
   brne not_D0

	clr r16                             ; i think this is safe because usb will get this byte
   sts pending_pulse_flag,r16

send_secs:

	clr	r2		                        ; checksum
 
   lds r29,seconds_since_midnite       ; msb
	rcall	get_spi_byte_r2	
	
   lds r29,seconds_since_midnite + 1
	rcall	get_spi_byte_r2	
   
   lds r29,seconds_since_midnite + 2
	rcall	get_spi_byte_r2	

   lds r29,current_0x16_RS
	rcall	get_spi_byte_r2	

	rcall	send_checksum_2                           ; 13

   ret

not_D0:

;-----------------------------------------

	cpi	r29,0xdd	;check for "read new packet" command  0xDD
	breq	line6643
	rjmp	exit1335
line6643:
	clr	r2		;checksum

	ldi	r16,0x02
	or	r13,r16		;set bit 1 to tell pause_check_pc that pc got packet

	
	lds	xeep_h_r7,packet_address_h
	mov	r29,xeep_h_r7
	rcall	get_spi_byte_r2	;send addrh             first byte = 1
	
	lds 	r6,packet_address_l
	mov	r29,xeep_l_r6
	rcall	get_spi_byte_r2	;send addrl	            2


   lds r29,sender_id_s + 2   

	rcall	get_spi_byte_r2	                        ; 3


   lds r29,sender_id_s + 3 
	
   rcall	get_spi_byte_r2	                        ; 4

	;  xbee skips xeep write sometimes, so just use xeep for datestamp, and use most recent packet for depth and status
	rcall 	read_xeep	      ; returns byte in i2c_data_r3, read from addrh 
				                  ; xeep_h_r7, addrl xeep_l_r6 in ext ee			
	mov	r29,i2c_data_r3
	rcall get_spi_byte_r2                           ; 5

	ldi	r29,1                ;  sender_id	RPI NEEDS THIS 
	rcall get_spi_byte_r2                           ; 6
	
	lds	r29,depth_adc             ; depth adc 8-bit, was receiver_id. i want to calibrate plot, but how to tell rpi
	rcall get_spi_byte_r2                           ; 7       what i did in php?

   lds r29,float_one_s                 
	rcall get_spi_byte_r2               ; rpi_temperature   8


   lds r29, air_val+1                 ; 8-12-2022 10 bit air pressure msb
   rcall get_spi_byte_r2               ;     9
	
   lds r29,air_val                   ; 8-12-2022 
                                       ; 
	rcall get_spi_byte_r2                           ; 10
	

;  5-5-2019 i changed my mind on this. put cutoff in xbee_sender since that is obsolete and this will
;  send cutoff to rpi and to tablet. but wait. 0xAA is not sent to rpi, so put it in both places

	lds	r29,flow_3_4_cutoff
	rcall get_spi_byte_r2                           ; 11

   lds r29,float_two_s                             ; this is flow 3 4, called dummy 3 in spi calls
	rcall get_spi_byte_r2                           ; 12
	
	
	rcall	send_checksum_2                           ; 13

	
	ret
	
exit1335:;------------------------------------------------------------




	cpi	r29,0xbb	;check for "set rtc hms" command
	brne 	exit1241a
	
	
	ldi	zl_r30,set_rtc_data ;store bytes in sram 0x72..0x79
	ldi	zh_r31,0x00
	
loop1624a:

	rcall	get_spi_byte
	st	z+,r29
	
	cpi	zl_r30,set_rtc_data+8
	brne	loop1624a
	rcall	init_clock3	;copy from sram to rtc registers
	ret

exit1241a:;------------------------------------------------------------

	cpi	r29,0xbd	;read rtc registers and nvram contents
	brne	exit1736
	
	clr	r7		;r7 = address
	clr	r3		;r3 = i2c data read
	clr	r2		;r2 = checksum
	
	
	rcall	read_clock	;reads @r7 and returns byte in r3
	
	mov	r29,r3
	rcall	get_spi_byte_r2
	inc	r7	

loop1749:	
	rcall	read_clock_current_byte	;reads next byte 
	
	mov	r29,r3
	rcall	get_spi_byte_r2
	inc	r7
	ldi	r16,0x40	;final nvram is 0x3f
	cp	r7,r16
	brne	loop1749
	
	rcall	send_checksum_2	
	
	ret


exit1736:;--------------------------------------------------------------



	cpi	r29,0xE1	;check for "read t0 count" command; 
				;
	brne	exit1338
	
	
	clr	r2		;checksum
	
	in	r29,tcnt0

	rcall	get_spi_byte_r2		;.....................byte 1
	
	ldi	zl_r30,total_seconds	;timer count is in 0x7a..0x7d; flag is in rollover_flag
	clr	zh_r31
loop1438:
	ld	r29,z+

	rcall	get_spi_byte_r2			; byte 2 3 4 5 - 4-28-2011 - this loooks wrong - sends 5 bytes here, so it
                                 ; also sends the next byte, rollover flag
	
	cpi	zl_r30,total_seconds+5
	brne	loop1438		;................	
	
	ldi	zl_r30,last_master_time		;time since valid unit 1 
	clr	zh_r31
	rcall	calc_elapsed_time
	ldi	zl_r30,calc_result		;calc answer is at 0xa4..	
	clr	zh_r31
loop1439:
	ld	r29,z+

	rcall	get_spi_byte_r2		;byte 6 7 8 9
	
	cpi	zl_r30,calc_result+4
	brne	loop1439		;.................		
	

	ldi	zl_r30,last_slave_time	;time since last valid 4 packet
	clr	zh_r31
	rcall	calc_elapsed_time		;z = 0x84: unit 4	
	ldi	zl_r30,calc_result		;calc answer is at 0xa4..		
	clr	zh_r31

loop143a:
	ld	r29,z+

	rcall	get_spi_byte_r2		; byt 10 11 12 13
	
	cpi	zl_r30,calc_result+4
	brne	loop143a		;.....................
	
	
	
	ldi	zl_r30,next_send_time	;time to send next packet
	clr	zh_r31

loop143b:
	ld	r29,z+

	rcall	get_spi_byte_r2		; 14 15 16 17
	
	cpi	zl_r30,next_send_time+4
	brne	loop143b		;.....................
	
	
	
	mov	r29,r13			;send flags register r13

	rcall	get_spi_byte_r2		;18
	
	in	r29,sph			;send stack pointer
	rcall	get_spi_byte_r2		;19
	
	in	r29,spl
	rcall	get_spi_byte_r2		;20
	
	lds	r29,spi_loop_count_l	;send most recent spi loop count
	rcall	get_spi_byte_r2		;21
	
	lds	r29,spi_loop_count_h
	rcall	get_spi_byte_r2		;22
	
	rcall	send_checksum_2			;23
	
	ret
	
exit1338:;------------------------------------------------------------
	
	

	cpi	r29,0xE2	;check for "read auto shutoff time" command; 
				;
	brne	exit1338a
	
	
	clr	r2		;checksum
	
	ldi	zl_r30,auto_shut_off_time 
	clr	zh_r31
loop1438a:
	ld	r29,z+

	rcall	get_spi_byte_r2
	
	cpi	zl_r30,auto_shut_off_time+4
	brne	loop1438a		;...............
	
	ldi	zl_r30,auto_shut_off_time
	clr	zh_r31
	rcall	calc_remaining_time
	ldi	zl_r30,calc_result		;calc answer is at 0xa4..	
	clr	zh_r31
loop1439a:
	ld	r29,z+

	rcall	get_spi_byte_r2
	
	cpi	zl_r30,calc_result+4
	brne	loop1439a		;.................		
	
	lds	r29,status_flag_7f		;bit 0 is pump on/off
	

	rcall	get_spi_byte_r2
	
	rcall	send_checksum_2	
	
	ret
	
exit1338a:;------------------------------------------------------------



	cpi	r29,0xee	;check for "read unit_id" command
	brne	ddexit1337
	
	clr	r2		;checksum
	
	lds	r29,unit_id

	rcall	get_spi_byte_r2	;send addrh
		
	rcall	send_checksum_2

	ret
	
ddexit1337:;------------------------------------------------------------



	cpi	r29,0xee	;check for "read unit_id" command
	brne	exit1337
	
	clr	r2		;checksum
	
	lds	r29,unit_id

	rcall	get_spi_byte_r2	;send addrh
		
	rcall	send_checksum_2

	ret
	
exit1337:;------------------------------------------------------------

        cpi	r29,0xff	;check for resync spi command
        brne	exit3094
        
        ldi	r29,0x18	;ack = 18
        rcall	get_spi_byte
        ret

exit3094:;------------------------------------------------------------



	cpi	r29,0xf0	;check for "read adc" command; command = 0xf0..0xf7
				;lower nibble = adc channel to be read
	brlo	exit1336
;NOTE THAT THIS BRANCH IS BRLO, SO THAT READ ADC COMMAND CAN BE USED TO INDICATE
;WHICH ADC CHANNEL IS TO BE READ: F0 = CHANNEL 0, F1 = CHANNEL 1,...THEREFORE, TO
;USE F8..FF, THOSE COMMANDS MUST BE TRAPPED BEFORE THIS SECTION. COMMANDS LOWER THAN
;0XF0 CAN BE TRAPPED ABOVE OR BELOW THIS SECTION	
	
	clr	r2		;checksum
	
	andi	r29,0x07	;clear upper bits to leave adc address in lower 3 bits
	rcall	perform_adc
	
	in	r29,adcl

	rcall	get_spi_byte_r2	;send lsb of adc conversion
	
	in	r29,adch

	rcall	get_spi_byte_r2	;send upper 2 bits of conversion
	
	mov	r29,r25		;send lsb of loop counter

	rcall	get_spi_byte_r2
	
	mov	r29,r26		;send msb of loop counter

	rcall	get_spi_byte_r2
	
	rcall	send_checksum_2	
	
	ret
	
exit1336:;------------------------------------------------------------


; reply to invalid spi request = 0x13 was 0x11, but that is 'reset 8535', and i worry about echos
	ldi	r29,0x13
	rcall	get_spi_byte  ; deleted this for usb, since it seemed to send phony bytes after each
							; real set of byes - i might have fixed that - i think it might have
							; been caused by me messing with mosi line, which i stopped doing in
							; usb, but still do in old pascal parallel port code
	ret	

;*********************************************************************************************

check_spi_usb:		;check_usb check usb
	
	sbis	spsr,spif	; if a byte is here, process it

   ret

	in	r29,spsr
	in 	r29,spdr

	cpi r29,0x91
	breq skip_it ; 0x91 is pc polling us, so ack 71

	cpi r29,0x71	;is spi echoing our byte? if so, usb might be hung
	brne line4637

	rjmp is_usb_ok

line4637:

;--------- 4-1-2019 can PC ever actually echo byte from avr? ----------
;
; pc is spi master so no bytes are transferred unless pc says so.
;
; busy avr can echo pc bytes as I have seen, but all this avr "is spi echoing" is probably unnecessary


	cpi r29,0x75	;is spi echoing our byte? if so, usb might be hung
	brne line4638

	cpi r29,0xC0	;is spi echoing our byte? if so, usb might be hung
	brne line4638

	cpi r29,0xC1	;is spi echoing our byte? if so, usb might be hung
	brne line4638

	cpi r29,0xC2	;is spi echoing our byte? if so, usb might be hung
	brne line4638

	cpi r29,0xC3	;is spi echoing our byte? if so, usb might be hung
	brne line4638

	rjmp is_usb_ok

line4638:

	rcall process_usb	; this calls get_spi_byte_r2 which puts r29 in spdr, so we ack the request 
	ret					; and don't do anything else here

skip_it:	;just a routine 0x91 byte from pc, which only means pc is alive and well
	
	sbrc	r13,1	   		               ; if bit 1 is clear, we have a new packet for pc
	rjmp 	handle_new_rpi_flags          ; do this if no new packet
	
	ldi 	r28,0x75		; do this if new packet is ready
	rjmp	send_it

handle_new_rpi_flags:

   lds r28,pending_0x36_flag
   cpi r28,0
   breq nothing_new36

; clear flag when C asks for data
;
	clr r16                             ; i think this is safe because usb will get this byte
   sts pending_0x36_flag,r16
   
   rjmp	send_it

nothing_new36:


   lds r28,pending_pulse_flag
   cpi r28,0
   breq nothing_new2

; clear flag when C asks for data
;
;	clr r16                             ; i think this is safe because usb will get this byte
;   sts pending_pulse_flag,r16
   
   rjmp	send_it

nothing_new2:

   lds r28,pending_float_flag
   cpi r28,0
   breq nothing_new

;	clr r16                             ; i think this is safe because usb will get this byte
;   sts pending_float_flag,r16
   
   rjmp	send_it





nothing_new:			; 0x71 is our routine reply to 0x91 polling, telling pc we are ok and nothing
	ldi r28,0x71		; needs to be done by pc

send_it:
	out spdr,r28
done_with_usb:
	ret


is_usb_ok: 		; count loops where we don't get a usb byte. if too many go bye, we need to reset
				   ; the 2313 to see if that is the problem

	;rcall increment_usb_dead_counter ; if count is too high, this will reset 2313
	ret
;*********************************************************************************************

master_code:	;stuff master does upon receipt of valid packet addressed to master_id

					
;check to see if this is a 'prompt' packet, asking master to immediately send latest info
;at present, this packet is defined as X 1 0 0 0 0 0 checksum, where X = prompt_id

	lds	r16,sender_id
	cpi	r16,prompt_id
	brne	not_prompt_packet
	
	lds	r16,status_byte		
	tst	r16
	brne	not_prompt_packet
	
	lds	r16,level_byte
	tst	r16
	brne	not_prompt_packet
	
	lds	r16,flow_1_2
	tst	r16
	brne	not_prompt_packet
	
	lds	r16,flow_3_4
	tst	r16
	brne	not_prompt_packet
	
	rcall	send_packet_in_five_seconds
						
not_prompt_packet:

   ; this is new for xbee. when master sends, he sets next time for 60 secs. but if slave replies, he
   ; sets normal send time, which is 300 secs at this moment.

	lds	r16,sender_id
	cpi	r16,slave_id
	brne	not_from_slave


	lds	r16,pause_seconds_l	;r16 is lsb of delay time (seconds) before sending 
	lds	r17,pause_seconds_h	;r17 is msb of delay time
	rcall	set_send_time_in_0xa8	;add r16,r17 to current time
	


not_from_slave:
	ret
;*********************************************************************************************
add_to_time_block:	;adds r16,r17 to 4 bytes stored at (Z)

	ld	r18,z		;add r16 to low byte
	add	r18,r16
	st	z+,r18
	
	ld	r18,z		;add r17 to next byte
	adc	r18,r17
	st	z+,r18
	
	brcc	exit1696	;if no carry, we are done
	
	ld	r18,z
	inc	r18
	st	z+,r18
	
	brne	exit1696	;'inc' does not set carry, so check for zero rollover
	
	ld	r18,z
	inc	r18
	st	z+,r18
exit1696:
	ret

;*********************************************************************************************
set_xbee_reset_time:		;this adds six minutes to current time
				;to indicate when xbee should be reset if no packet is received. if
            ; any valid packet is heard from anyone, it means xbee is ok.
				;
				;nrote that this uses CURRENT time, not time packet was 
				;received, so, for example, if you want to wait 5 seconds
				;between packets, use the value 5 here. 

	ldi	zl_r30,low(xbee_reset_time)
	ldi	zh_r31,high(xbee_reset_time)
	rcall	copy_timer_0	;copies timer 0 cnt and sram 0x7a..0x7c to (Z)	

   ldi r16,0x68
   ldi r17,0x01            ; 6 minutes = 360d secs = 0x0168 secs
	
	ldi	zl_r30,low(xbee_reset_time)
	ldi	zh_r31,high(xbee_reset_time)
	rcall	add_to_time_block

	ret



;*********************************************************************************************
set_send_time_in_0xa8:		;this adds low byte r16 and high byte r17 to current time
				;to indicate when next packet should be sent. r16 and r17
				;are delay time in seconds
				;
				;nrote that this uses CURRENT time, not time packet was 
				;received, so, for example, if you want to wait 5 seconds
				;between packets, use the value 5 here. 

	push	r16		;save delta values
	push	r17
	
	ldi	zl_r30,next_send_time
	clr	zh_r31
	rcall	copy_timer_0	;copies timer 0 cnt and sram 0x7a..0x7c to (Z)	
	
	pop	r17
	pop	r16
	
	ldi	zl_r30,next_send_time
	clr	zh_r31	
	rcall	add_to_time_block

	ret
;*********************************************************************************************
set_auto_turnoff_time:
	
				;multiply auto_turnoff_mins by 60 to get seconds
	lds	mc8u,auto_turnoff_mins
	ldi	mp8u,60		;multiply by 60
	rcall	mpy8u		;result: m8uH:m8uL 
	
	push	m8uH		;save multiplication results
	push	m8uL
	
	ldi	zl_r30,auto_shut_off_time
	clr	zh_r31
	rcall	copy_timer_0	;copies timer 0 cnt and sram 0x7a..0x7c to (Z)	
	
	pop	r16		;move multiplication results into r16,r17
	pop	r17
	
	
	ldi	zl_r30,auto_shut_off_time
	clr	zh_r31	
	rcall	add_to_time_block	

	ret

;*********************************************************************************************
write_i2c_rtc:	;put data byte in r21, address in r20; uses r16,r17,r18,r19,r20,r21
	cli
	ldi	i2cadr,$d0+i2cwr	; Set device address and write
	rcall	i2c_start		; Send start condition and address

	mov	i2cdata,r20		; Write word address
	rcall	i2c_do_transfer		; Execute transfer
		
	mov	i2cdata,r21
	rcall	i2c_do_transfer		; Execute transfer

	rcall	i2c_stop		; Send stop condition
	sei
	ret
;*********************************************************************************************
test_nvram_2:
	ldi	r20,0x08	;first nvram address = 0x08
loop2235:
	mov	r21,r20		;write address to ram: contents of location 'n' = n
	rcall	write_i2c_rtc
	inc	r20
	cpi	r20,0x40	;final address is 0x3f
	brne	loop2235
	
	ret
;*********************************************************************************************
init_rtc:	;set rtc square wave output to 1 hz and enable oscillator

	clr	r7		;r7 = address = 0 = seconds byte\oscillator enable bit
;	clr	r3		;r3 = i2c data read IS THIS REALLY NECESSARY?
		
	rcall	read_clock	;reads @r7 and returns byte in r3
	
	mov	r21,r3		;r21 = data byte to write
	sbrs	r21,7		;oscillator is already enabled if bit 7 is clear
	rjmp	set_control_byte
	andi	r21,0b01111111	;clear bit 7 to enable rtc oscillator
	clr	r20		;rtc addr = byte 0
	rcall	write_i2c_rtc
	
set_control_byte:
	ldi	r21,0x10		;data byte is in r21
	ldi	r20,0x07		;control byte address is 0x07
	
	rcall	write_i2c_rtc
	ret
						
;*********************************************************************************************
check_timer:		;checks time at (X) and sets carry if time has been reached
			;
			;compare current seconds count to timer such as 'send next packet'
			;
			;set (X) to point to time being checked
			;
			;if time to 'do something' has been reached, set carry
	
	rcall	wait_for_rollover	;uses (Y) to check for tcnt=0xff
	
	ldi	zl_r30,total_seconds	;(Z) points to current seconds count				
	clr	zh_r31
	
	in	r16,tcnt0
	ld	r17,x+
	sub	r17,r16
	rol	r18		;save carry

loop1810:

	ld	r16,z+
	ld	r17,x+
	
	ror	r18		;restore carry
	sbc	r17,r16
	rol	r18		;save carry
	cpi	zl_r30,total_seconds+3
	brne	loop1810
	
	ror	r18		;restore carry
	
	ret				


;*************************************
check_xbee_reset_time:
      		;compare current seconds count to send time
				;if time to send has been reached, set carry
				;
				;since carry 'set' means send, i need to subtract
				;current time from time to send. as long as current time
				;has not exceeded time to send, carry will be cleared
	
	ldi	xl_r26,low(xbee_reset_time)	;(X) points to time 
	ldi   r27,high(xbee_reset_time)
	
	rcall	check_timer	;checks time at (X) and sets carry if time has been reached

   brcc   line6878

   rcall reset_xbee

   rcall set_xbee_reset_time

line6878:


	ret


;*********************************************************************************************
check_send_time:		;compare current seconds count to send time
				;if time to send has been reached, set carry
				;
				;since carry 'set' means send, i need to subtract
				;current time from time to send. as long as current time
				;has not exceeded time to send, carry will be cleared
	
	ldi	xl_r26,next_send_time	;(X) points to time to send packet
	clr	xh_r27	
	
	rcall	check_timer	;checks time at (X) and sets carry if time has been reached
		
	ret
	
;*********************************************************************************************
unit_specific_tasks:		;slave check auto-shutoff timer, etc.

	
	rcall	check_send_packet_status
	
	ret

;*********************************************************************************************	
send_packet_in_five_seconds:	
	ldi	r16,0x05		;r16 is lsb of delay time (seconds) before sending 
	clr	r17			;r17 is msb of delay time
	rcall	set_send_time_in_0xa8	;add r16,r17 to current time
	ret	
;*********************************************************************************************	
		
;*********************************************************************************************
sensors_on:

	cbi	term2_port,pulse_sensor_line	;turn sensor power on by LOWERING line to pnp 
	ret
;*********************************************************************************************
sensors_off:

	sbi	term2_port,pulse_sensor_line	;turn sensor power off by RAISING line to pnp 
	ret
;*********************************************************************************************
depth_on:

	sbi	term2_port,depth_pulse	;turn depth power on by raising line to npn
	ret
;*********************************************************************************************
depth_off:

	cbi	term2_port,depth_pulse	;turn depth power off by lowering line to npn
	ret	
;*********************************************************************************************


.ifdef fake_sensors

check_float:

   lds r16,rtc_hours

   andi r16,1

   breq line5710

   ; pretend float is on

   ldi r29,243
   sts float_two_s,r29        ; new for xbee: send flow adc in packet


	set				;set T 

   lds	r16,status_byte_s
	bld	r16,float_bit		;use T to set/clear float bit
	sts	status_byte_s,r16
	
	lds	r16,status_byte_s
	bld	r16,flow_3_4_bit
	sts	status_byte_s,r16

   clt				;clear T 
	
	lds	r16,status_byte_s
	bld	r16,flow_1_2_bit   ;transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16


	lds	r16,status_byte_s
	bld	r16,transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16

   ret

finish_up:

line5710:

   ldi r29,14
   sts float_two_s,r29        ; new for xbee: send flow adc in packet

	clt 

   lds	r16,status_byte_s
	bld	r16,float_bit		;use T to set/clear float bit
	sts	status_byte_s,r16
	
	lds	r16,status_byte_s
	bld	r16,flow_3_4_bit
	sts	status_byte_s,r16

   set 
	
	lds	r16,status_byte_s
	bld	r16,flow_1_2_bit   ;transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16


	lds	r16,status_byte_s
	bld	r16,transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16


  
   ret

;---------------------------------------


check_adc_sensors:	;read all adc lines, but skip unused lines. update status byte
	

   lds r16,rtc_hours
   andi r16,1

   breq increasing

   lds r16,rtc_minutes
   ldi r17,70
   sub r17,r16

	sts	level_byte_s,r17

   ret


increasing:

   lds r16,rtc_minutes
   ldi r17,10
   add r16,r17

	sts	level_byte_s,r16

   ret

;-------------------
;
check_transfer_pump2:	;line is inverted: 0 = on, 5v = off

	ret

;*********************************************************************************************
check_transfer_pump:	;line is inverted: 0 = on, 5v = off
	
	ret

.else                                  ; fake sensors is not defined 


;------------------------------------------


check_adc_sensors:	;read all adc lines, but skip unused lines. update status byte
	
	rcall	depth_on

   wdr

;   8-24-2022  very close to 1/2 second on; 9 x 1/18 = 9/18 = 1/2

	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick	
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
			
   wdr
;
;	ldi	r29,depth_probe_input
;	rcall	perform_8_bit_adc
;;	
;;	 
;;   ldi r31,32
;;
;;   clr r28
;;   clr r27
;;
;;lloop4383:
;;
;;	sbi	adcsr,adsc	;set start conversion bit
;;
;;lair_adc_loop:
;;
;;	sbis	adcsr,adif
;;	rjmp	lair_adc_loop	
;;	
;;	sbi	adcsr,adif	;clear bit by writing a '1' to it
;;	
;;	in	r29,adcl
;;	in	r30,adch
;;   
;;   add r27,r29
;;   adc r28,r30
;;
;;   dec r31
;;   brne lloop4383
;;
;;   ldi r31,7                           ; 32 values were read, so shift 7 to get 8 bit
;;
;;lloop4855:
;;
;;	lsr	r28
;;	ror	r27
;;
;;   dec r31
;;   brne lloop4855
;;
;;   ;sts air_val,r27                     ; lsb
;;  ; sts air_val+1,r28
;
;
;   push r29
;
;	rcall	depth_off
;
;
;	
;   wdr
;
;	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
;	rcall	pause_one_tick
;	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
;	rcall	pause_one_tick	
;	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
;	rcall	pause_one_tick
;	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
;	rcall	pause_one_tick
;	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
;	
;	pop	r29
;
; ;  mov r29,r27
;




	ldi	r29,depth_probe_input
	out 	admux,r29
	
	 
   ldi r31,32

   clr r28
   clr r27

lloop4383:

	sbi	adcsr,adsc	;set start conversion bit

lair_adc_loop:

	sbis	adcsr,adif
	rjmp	lair_adc_loop	
	
	sbi	adcsr,adif	;clear bit by writing a '1' to it
	
	in	r29,adcl
	in	r30,adch
   
   add r27,r29
   adc r28,r30

   dec r31
   brne lloop4383

   ldi r31,7                           ; 32 values were read, so shift 7 to get 8 bit

lloop4855:

	lsr	r28
	ror	r27

   dec r31
   brne lloop4855



   wdr


   mov r29,r27

   sts depth_adc,r29                   ; i can do the math later and calibrate easily, especially if
                                       ; i wait until php. rpi will be a pain to keep synced.

	lds	r17,tank_full
	cp	r17,r29			;is tank_full<current value?
	
	brcc	depth_value_ok_2
	
	lds	r29,tank_full		;adc value is too high, so set tank level to full
	lds	r17,tank_empty
	rjmp	line4146
depth_value_ok_2:
	lds	r17,tank_empty	;adc-tank_empty

	cp	r29,r17
	brcc	line4146
	
	lds	r29,tank_empty		;adc value is low, so set tank level to empty
		
line4146:	
			
	sub	r29,r17
	

	ldi	mc8u,0x64	;multiply by 100
	mov	mp8u,r29
	rcall	mpy8u		;product is in m8uL(r17):m8uH(r18) (r16,r17,r18,r19 are used)
	
	
	lds	r19,tank_full
	lds	r16,tank_empty
	sub	r19,r16		;tank_full-tank_empty
	
	mov	dd16uH,m8uH	;mov r15,r18
	mov	dd16uL,m8uL	;mov r16,r17
	
	mov	dv16ul,r19	;mov r18,r19
	
	clr	dv16uH		;clr r19
	
	rcall	div16u		;answer is in dres16uH(r17):dres16uL(r16) (r14..r20 are used)

	sts	level_byte_s,dres16uL
	


	;----------------------------- air pressure --------------------------
   
   
   ldi	r29,air_pin
	
  	out 	admux,r29
   
   ldi r31,32

   clr r28
   clr r27

loop4383:

	sbi	adcsr,adsc	;set start conversion bit

air_adc_loop:

	sbis	adcsr,adif
	rjmp	air_adc_loop	
	
	sbi	adcsr,adif	;clear bit by writing a '1' to it
	
	in	r29,adcl
	in	r30,adch
   
   add r27,r29
   adc r28,r30

   dec r31
   brne loop4383

   ldi r31,5                           ; 32 values were read, so shift 5 bits. result is 10 bit

loop4855:

	lsr	r28
	ror	r27

   dec r31
   brne loop4855

   sts air_val,r27                     ; lsb
   sts air_val+1,r28
	
   

   ; the sensor is 0-100 psi. 0 psi = 0.5v, 50 psi = 2.5v, 100 psi = 4.5 v so
   ;
   ;     read volts
   ;
   ;     subtract 0.5v:  256 x 0.5 / 5 = 25
   ;
   ;     multiply volts x 100 psi / 4 volts: 4 volts = 256 x 4 / 5 = 205
   ;
   ;        (adc - 25) x 100 / 205 = psi     100/205 = 20/41
   ;
   ;     assume max will be 60 psi: 60 x 4/100 + 0.5 = 2,9v
   ;
   ;     2.9 x 256 / 5 = 149d so i have to use 16 bit math
   ;
   ;     spreadsheet says 20 psi is about 66d, 40 psi is about 111. 111-66 = 45, or about 2 counts per psi
   ;
   ;     -------
   ;
   ;     read adc
   ;
   ;     subtract 25d
   ;
   ;     mutiply x 20d
   ;
   ;     divide by 41d
   ;
   ;
	









	rcall	depth_off
	
   wdr

	rcall	sensors_on
	wdr

	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick
	rcall	pause_one_tick		;pause about 1/18 sec = 55 milliseconds
	rcall	pause_one_tick			

	ldi	r29,flow_3_4_input
	rcall	perform_8_bit_adc

   sts float_two_s,r29        ; new for xbee: send flow adc in packet
	
	lds	r16,flow_3_4_cutoff
	cp	r29,r16
	brlo	flow_3_4_is_on
	
   rcall flow_is_off                   ; check for transition
	clt				;use T to clear bit in status byte
	rjmp	line4043a
flow_3_4_is_on:
   rcall flow_is_on                   ; check for transition
	set				;use T to set bit in status byte
line4043a:
	lds	r16,status_byte_s
	bld	r16,flow_3_4_bit
	sts	status_byte_s,r16

	rcall	sensors_off

   wdr

   ret

;-------------------

check_transfer_pump2:	;line is inverted: 0 = on, 5v = off
	push	zh
	push	zl
	
	lds	r16,status_byte_s	
	
	sbrc	r16,flow_1_2_bit   ;transfer_pump_bit
	rjmp	pump_was_already_on2	;status bit was hi
	
;pump was off (pin was hi, status bit was low), so sample it several times to see 
;if it has changed	
	
	
	ldi	zh,0x00
	clr	zl
loop_3263a2:
	sbic	trans_pins,trans2_pin      ;transfer_pump_input	;if pin is still hi, exit 

	rjmp	exit_46982		;false alarm
	
;pin is low, meaning pump just turned on	
	
	adiw	zword,1
	brne	loop_3263a2
	
	set				;set T 
	rjmp	finish_up_a2
		
	
pump_was_already_on2:			;pump was on (pin was high, status byte bit was '1')
	clr	zl
	ldi	zh,0x00
	
loop_3264a2:
	sbis	trans_pins,trans2_pin      ;transfer_pump_input	;if pin is still low, exit 

	rjmp	exit_46982		;false alarm
	adiw	zword,1
	brne	loop_3264a2
	
	clt				;clear T 
finish_up_a2:
	lds	r16,status_byte_s
	bld	r16,flow_1_2_bit   ;transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16
exit_46982:
	pop	zl
	pop	zh
	
	ret






;*********************************************************************************************
check_transfer_pump:	;line is inverted: 0 = on, 5v = off
	push	zh
	push	zl
	
	lds	r16,status_byte_s	
	
	sbrc	r16,transfer_pump_bit
	rjmp	pump_was_already_on	;status bit was hi
	
;pump was off (pin was hi, status bit was low), so sample it several times to see 
;if it has changed	
	
	
	ldi	zh,0x00
	clr	zl
;	ldi	r16,0xff		;sample this many times to eliminate noise
loop_3263a:
	sbic	trans_pins,transfer_pump_input	;if pin is still hi, exit 

	rjmp	exit_4698		;false alarm
	
;pin is low, meaning pump just turned on	
;	dec	r16
;	brne	loop_3263a
	
	adiw	zword,1
	brne	loop_3263a
	
	set				;set T 
	rjmp	finish_up_a
		
	
pump_was_already_on:			;pump was on (pin was high, status byte bit was '1')
	clr	zl
	ldi	zh,0x00
	
;	ldi	r16,0xff		;sample this many times to eliminate noise
loop_3264a:
	sbis	trans_pins,transfer_pump_input	;if pin is still low, exit 

	rjmp	exit_4698		;false alarm
;	dec	r16
;	brne	loop_3264a
	adiw	zword,1
	brne	loop_3264a
	
	clt				;clear T 
finish_up_a:
	lds	r16,status_byte_s
	bld	r16,transfer_pump_bit	;use T to set/clear bit
	sts	status_byte_s,r16
exit_4698:
	pop	zl
	pop	zh
	
	ret
	
;*********************************************************************************************

check_float:
	push	zh
	push	zl
	
	lds	r16,status_byte_s	;check previous float state: NOTE THAT STATUS BYTE
					;FLOAT BIT = 1 MEANS FLOAT IS ON, TURN PUMP ON, BUT
					;INPUT PIN FROM FLOAT IS LOW WHEN FLOAT IS 'ON'
	sbrc	r16,float_bit
	rjmp	float_was_already_on
;float was off (pin was high), so sample it several times to see if it has changed	
	clr	zl
	ldi	zh,0x00
loop_3263:
	sbic	term_pin,float_input	;if pin is high, exit 

	rjmp	exit_4778				;false alarm
	adiw	zword,1
	brne	loop_3263
	
   ldi r16,0xC2
   sts pending_float_flag,r16            ; this will go on spi bus next time

	set				;set T 


	rjmp	finish_up
		
	
float_was_already_on:			;float was on (pin was low, status byte bit was '1')
	
	clr	zl
	ldi	zh,0x00
loop_3264:
	sbis	term_pin,float_input	;if pin is low, exit 

	rjmp	exit_4778		;false alarm
	adiw	zword,1
	brne	loop_3264
	
   ldi r16,0xC3
   sts pending_float_flag,r16            ; this will go on spi bus next time

	clt				;clear T 
finish_up:
	lds	r16,status_byte_s
	bld	r16,float_bit		;use T to set/clear float bit
	sts	status_byte_s,r16
	
	rcall	send_packet_in_five_seconds	;float state has just changed, so send now
exit_4778:
	pop	zl
	pop	zh
	ret	

.endif                                 ; fake sensors not defined

;*********************************************************************************************
check_black_pushbutton:
	sbic	pind,send_now_pushbutton	;if black pushbutton is pressed, send one now
	ret
	
	
	rcall	send_packet_in_five_seconds	
	ret

;*********************************************************************************************

check_send_packet_status:	;this routine is only used by master

	
	rcall	check_send_time	;returns carry 'set' if time to send has been reached
	brcs	send_now_3268
	
	rcall	check_float                   ; this gets checked a lot, and should be checked before first packet, i believe
	ret
	
send_now_3268:	
	;set time for next packet to be sent
   ; before xbee, this added pause_seconds_l , _h. with xbee, i changed to this:
   ;  set pause to 60 secs. if slave replies, set pause to pause_seconds_l,h
   ;  pause secs looks like 0x012c = 300d
   ;  later i reduced it to 280d = 0x118

   ldi r16,60
   clr r17

	;lds	r16,pause_seconds_l	;r16 is lsb of delay time (seconds) before sending 
	;lds	r17,pause_seconds_h	;r17 is msb of delay time
	rcall	set_send_time_in_0xa8	;add r16,r17 to current time
	
	lds	r16,unit_id
	sts	sender_id_s,r16		;store my id in outgoing packet
	sts	sender_id,r16		;this is to tell pc that i sent this packet
	
					
	ldi	r16,slave_id
	sts	receiver_id_s,r16	;master always sends to slave
	sts	receiver_id,r16		;this is to tell pc where packet was sent
	
	rcall	check_adc_sensors	;read overflow, flow, depth, and float_adc values
	rcall	check_transfer_pump
	rcall	check_transfer_pump2

;   ldi r16,0x71
;   sts sender_id_s + 2,r16
;
;   ldi r16,0x53
;   sts sender_id_s + 3,r16
	
	rcall	send_packet

	rcall	flag_pc				;attempt to send packet to pc


	ldi r29,master_id
	sts   sender_id_check,r29
   rcall check_xeep_already_written

   lds r29,xeep_already_written
   cpi r29,0xFF
   brne line4758a
    	
   lds r19,control_byte       ; 0x99 on    0x88 off      
   cpi r19,0x99
   brne  dead88dd


	rcall	flash_leds_xeep_already_written
dead88dd:

   ret

line4758a:

	rcall	save_master_data_in_xeep	;save status byte and depth 
	
;	rcall	flag_pc				;attempt to send packet to pc
	
not_yet_1826:
	
	ret	



;*********************************************************************************************
init_send_times:
					;set time for first packet to be sent
	rcall	send_packet_in_five_seconds
	
	ret	


;*************************

reset_xbee:

   cbi xbee_port,xbee_reset
   ;cbi portb,pb3

 
   ldi r26,1
   lds r25,xbee_reset_cnt
   add r25,r26
   sts xbee_reset_cnt,r25

   clr r26
   lds r25,xbee_reset_cnt + 1
   adc r25,r26
   sts xbee_reset_cnt + 1,r25

   ;wait 90 usec

   rcall pause_one_tick		;uses r16,r25,r26,r29
   ;sbi portb,pb3
   sbi xbee_port,xbee_reset

   ret

; ---------------------------------

init_pulse_meter:  ; get state after reset then wait for transition

	sbic	pina,pa6                      ; low pin? set bit...bit is opposite of pin

	rjmp high4778				; pin is high

   ; pin is low

	set				;set T 
	rjmp	ppfinish_up
	

high4778:

	clt				;clear T 

ppfinish_up:
	
   lds	r16,pulse_state
	bld	r16,float_bit		;use T to set/clear float bit
	sts	pulse_state,r16
	
	ret	

;------------------------------------------------
	
;-------------------------

calc_seconds_since_midnite:

   ;.equ rtc_data = last_slave_time + 4
   ;.equ rtc_seconds = rtc_data + 0
   ;
   ;.equ rtc_minutes = rtc_data + 1
   ;.equ rtc_hours = rtc_minutes + 1
   ;.equ rtc_day = rtc_hours + 1
   ;.equ rtc_date = rtc_day + 1
   ;.equ rtc_month = rtc_date + 1
   ;.equ rtc_year = rtc_month + 1
   ;.equ rtc_control = rtc_year + 1


   rcall record_clock_in_sram_verify

   ; need 3 bytes since 24 x 60 x 60 = 86400
   ;
   ; minutes = hrs * 60 + minutes = 1440 = 2 bytes
   ;
   ; seconds = minutes * 60 = 3 bytes
	
	lds	mc8u,rtc_hours                ; mc8u = r16   mp8u = r17 m8uh = r18   m8ul r17
	ldi	mp8u,60		                  ; multiply by 60
	rcall	mpy8u		                     ; result: m8uH:m8uL 
	
;	push	m8uH		                     ; save multiplication results
;	push	m8uL

   lds r19,rtc_minutes
   add r17,r19

   clr r19
   adc r18,r19                         ; minutes is in r18:r17
   
   mov r19,r18
   mov r18,r17                         ; minutes is in r19:r18
   
   ;.def	mc16uL	=r16		;multiplicand low byte
   ;.def	mc16uH	=r17		;multiplicand high byte
   ;.def	mp16uL	=r18		;multiplier low byte
   ;.def	mp16uH	=r19		;multiplier high byte
   ;.def	m16u0	=r18		;result byte 0 (LSB)
   ;.def	m16u1	=r19		;result byte 1
   ;.def	m16u2	=r20		;result byte 2
   ;.def	m16u3	=r21		;result byte 3 (MSB)
   ;.def	mcnt16u	=r22		;loop counter
   ;
   ;;***** Code
   ;
   ;mpy16u:	clr	m16u3		;clear 2 highest bytes of result

   ldi r16,60
   clr r17

	rcall	mpy16u			;result: m16u3(r21):m16u2(r20):m16u1(r19):m16u0(r18)	

   lds r17,rtc_seconds
   add r18,r17

   clr r17

   adc r19,r17
   adc r20,r17                         ; total fits in 3 bytes

   sts seconds_since_midnite,r20       ; msb
   sts seconds_since_midnite+1,r19
   sts seconds_since_midnite+2,r18     ; lsb
   
   ret



;----------------------------

process_0x16:  ; 0x16 means a box received 0x15 pulse meter and acked 0x16

   ldi r16,200
   sts pulse_state_cnt,r16             ; pulse state cnt is the flag that causes 0x15 to be sent; 200 means "don't send"


   ldi r16,0x36
   sts pending_0x36_flag,r16            ; this will go on spi bus next time


;   rcall flash_leds_good_packet
   ret



 

;;****************************

send_xbee_0x15:

   lds r16,pulse_state_cnt             ; pulse state cnt is the flag that causes 0x15 to be sent; 200 means "don't send"

   ldi r17,1

   and r17,r16

   breq green31
	
   rcall yellow_led_on
   rcall green_led_off

   rjmp line3157
green31:

   rcall green_led_on
   rcall yellow_led_off
	
line3157:

   inc r16
   cpi r16,12

   brlo cont3167

   ldi r16,200

   sts pulse_state_cnt,r16             ; pulse state cnt is the flag that causes 0x15 to be sent; 200 means "don't send"
   ret

cont3167:
   
   sts pulse_state_cnt,r16

   
   rcall send_start_bytes
	
   ldi r16,11             ; number of data bytes. does not include checsum; pkt_buf is 16 bytes
   
   rcall uart_send
  
   clr r3

;.equ pkt_buf               = start_bit_cnt            + 1         ; packet buffer 16 bytes
;.equ pb_sender             = pkt_buf                  + 0
;.equ pb_receiver           = pb_sender                + 1
;.equ pb_echo_flag          = pb_receiver              + 1
;.equ pb_packet_type        = pb_echo_flag             + 0

   ldi r16,0                           ; obsolete, was xbee_unit_id
   rcall uart_send_xor_r3
 
   lds r16,pb_sender        ; receiver = second data byte
   rcall uart_send_xor_r3

   ; fake_master sends 0x25 instead of 0x15

   .ifdef fake_packets

   ldi r16,0x35             ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply

   .else

   ldi r16,0x15             ; xbee_packet_type echo flag = 3rd byte. 0 no echo; 0xee = echo reply

   .endif

                            ; search for packet_type_list to see used values
   rcall uart_send_xor_r3
 
   lds r16,pulse_meter_cnt             ; msb
   rcall uart_send_xor_r3
  
   lds r16,pulse_meter_cnt + 1
   rcall uart_send_xor_r3
  
   lds r16,pulse_meter_cnt + 2
   rcall uart_send_xor_r3
   
 
   lds r16,seconds_since_midnite       ; msb
   rcall uart_send_xor_r3
	
   lds r16,seconds_since_midnite + 1
   rcall uart_send_xor_r3
   
   lds r16,seconds_since_midnite + 2
   rcall uart_send_xor_r3
   
   lds r16,reset_flags
   rcall uart_send_xor_r3
   
   lds r16,reset_cnt                   ; reset_cnt is one byte 
   rcall uart_send_xor_r3
   
   mov r16,r3             ; checksum
   rcall uart_send

   ret
 
;------------------------------
   
handle_0x15_state:

   lds r16,pulse_state_cnt             ; counts 0x15 pkts and is the flag that causes 0x15 to be sent; 200 means "don't send"

   cpi r16,200

   breq ret10345

   lds r16,pulse_ack_cnt               ; counts seconds

   cpi r16,5                           ; when this was 3 during bench test sometimes box 12 would send two
                                       ; packets to the tablet 4 or 5 seconds apart, implying that the fake master
                                       ; should have waited a moment more. that might have been a collision
                                       ; between fake maste and real master, but bump this from 3 to 5 anyway

   brlo ret10345

   clr r16

   sts pulse_ack_cnt,r16

   rcall send_xbee_0x15
   
ret10345:
   ret

;------------------------------------------------

.ifdef fake_sensors                    ; software pulses

check_pulse_meter:

   ; if minutes > 40 or less than 20, turn on one minute, turn off next minute

   lds r16,rtc_minutes
   cpi r16,20

   brlo pulse6536

   cpi r16,40

   brlo ret6536

pulse6536:

   lds r17,fake_pulse_minute
   cp r16,r17

   breq ret6536

   sts fake_pulse_minute,r16

   andi r16,1

   breq pulse_on_6536

   rcall calc_seconds_since_midnite    ; saves  seconds_since_midnite and pulse_date
   ldi r16,0xC1
   sts pending_pulse_flag,r16            ; this will go on spi bus next time

   ret

pulse_on_6536:

   rcall calc_seconds_since_midnite    ; saves  seconds_since_midnite and pulse_date
   ldi r16,0xC0
   sts pending_pulse_flag,r16            ; this will go on spi bus next time

   clr r16
   sts pulse_state_cnt,r16             ; pulse state cnt is the flag that causes 0x15 to be sent
   sts pulse_ack_cnt,r16


ret6536:

   ret

.else                                  ; real sensor or board that simulates them

check_pulse_meter:

	push	zh
	push	zl
	
	lds	r16,pulse_state               ; status_byte_s	;check previous float state: NOTE THAT STATUS BYTE
					                        ; FLOAT BIT = 1 MEANS FLOAT IS ON, TURN PUMP ON, BUT
					                        ; INPUT PIN FROM FLOAT IS LOW WHEN FLOAT IS 'ON'

	sbrc	r16,float_bit                 ;; ok to use this for pulse

	rjmp	pfloat_was_already_on

	clr	zl
	ldi	zh,0x00
ploop_3263:
	sbic	pina,pa6                      ; if pin is still high, exit but if it is clear keep checking

	rjmp	pexit_4778				         ; false alarm
	adiw	zword,1
	brne	ploop_3263
	
   rcall calc_seconds_since_midnite    ; saves  seconds_since_midnite and pulse_date

   ldi r16,0xC0
   sts pending_pulse_flag,r16            ; this will go on spi bus next time

   clr r16
   sts pulse_state_cnt,r16             ; pulse state cnt is the flag that causes 0x15 to be sent
   sts pulse_ack_cnt,r16

   lds r16,pulse_meter_cnt + 2
   inc r16
   sts pulse_meter_cnt + 2,r16
   brne acnt33

   lds r16,pulse_meter_cnt + 1
   inc r16
   sts pulse_meter_cnt + 1,r16
   brne acnt33
   
	
   lds r16,pulse_meter_cnt             ; msb
   inc r16
   sts pulse_meter_cnt,r16

acnt33:
	
   set				;set T 
	rjmp	pfinish_up
	
pfloat_was_already_on:			         ; float was on (pin was low, status byte bit was '1')
	
	clr	zl
	ldi	zh,0x00
ploop_3264:
	sbis	pina,pa6                      ; if pin is still high, exit but if it is clear keep checking

	rjmp	pexit_4778		               ; false alarm
	adiw	zword,1
	brne	ploop_3264
	
   ldi r16,0xC1
   sts pending_pulse_flag,r16            ; this will go on spi bus next time
	
   clt				                     ; clear T 
pfinish_up:
	lds	r16,pulse_state               
	bld	r16,float_bit		            ; use T to set/clear float bit
	sts	pulse_state,r16
	
pexit_4778:
	pop	zl
	pop	zh
	ret	

.endif

;------------------------------

flow_is_off:                           ; called every loop if flow is off

   lds r16,status_byte_s               ; flow bit is set if flow was already on

   bst r16,flow_3_4_bit                ; copy bit to T

   brtc no_change_839                  ; branch if bit was already clear

   ; flow just turned off so add elapsed to tally in pulse_elapsed
   
   ; added this line since box4 seems to include flow-off in total. without this line it
   ; seems that the total should be lower instead of higher since t1 would = now
;   

no_change_839:
   
   ret

;------------------------------

flow_is_on:

   lds r16,status_byte_s               ; flow bit is set if flow was already on

   bst r16,flow_3_4_bit                ; copy bit to T

   brts no_change_839                  ; branch if bit was already set

   ; flow just turned on so save seconds since midnite and date

   
   ret


;------------------------------


flash_leds_good_packet:	;flash leds five times to show that packet was valid

; these work for 2,476 and 8 mhz because the timers are set differently

		cli
		clr	r29
		clr	r30
		rcall	reset_timer_1_64
		wdr
loop_640:
		rcall 	read_timer_1	;uses r25,r26
		andi	t1_h_r26,0x20
		breq	ret_197a
		
		clr	r30
		rcall	green_led_on
		rcall	yellow_led_on
		rjmp	line_647
ret_197a:
		tst	r30
		brne	line_647
		wdr
		inc	r29
		inc	r30
		rcall	green_led_off
		rcall	yellow_led_off
line_647:
		cpi	r29,5
		brne	loop_640
		sei
		ret


;*********************************************************************************************
flash_leds_xeep_already_written:

; these work for 2,476 and 8 mhz because the timers are set differently

      cli
		clr	r29
		clr	r30
		rcall	reset_timer_1_64
		wdr
loop_6403:
		rcall 	read_timer_1	;uses r25,r26
		andi	t1_h_r26,0x20
		breq	ret_197a3
		
		clr	r30
		rcall	green_led_off
		rcall	yellow_led_on
		rjmp	line_6473
ret_197a3:
		tst	r30
		brne	line_6473
		wdr
		inc	r29
		inc	r30
		rcall	green_led_on
		rcall	yellow_led_off
line_6473:
		cpi	r29,3
		brne	loop_6403
		sei
		ret


   
   
;*********************************************************************************************

flash_leds_reset:	

; these work for 2,476 and 8 mhz because the timers are set differently

		rcall	yellow_led_off
		cli
		clr	r29
		clr	r30
		rcall	reset_timer_1_64
		wdr
loop_640r:
		rcall 	read_timer_1	;uses r25,r26
		andi	t1_h_r26,0x10   ; 8535
		breq	ret_197ar
		
		clr	r30
		rcall	green_led_on
;		rcall	yellow_led_on
		rjmp	line_647r
ret_197ar:
		tst	r30
		brne	line_647r
		wdr
		inc	r29
		inc	r30
		rcall	green_led_off
;		rcall	yellow_led_off
line_647r:
		cpi	r29,7
		brne	loop_640r
		sei
		ret





;*********************************************************************************************
check_reset_cause:
	in	r29,mcusr	;this register = 0 if a watchdog reset has occurred
	sts	reset_flags,r29
	
	ret
	
;-------------------------------------------------
check_ff_timer:
	
	ldi	xl_r26,low(ff_timer)	
	ldi	xh_r27,high(ff_timer)	
	
	rcall	check_timer	;checks time at (X) and sets carry if time has been reached
	
   brcc line7805

   clr r26
   sts fork_flag,r26

line7805:

   ret
;--------------------------------------------------

write_reset_eep:

   ldi r30,low(reset_cnt_eep)
   ldi r31,high(reset_cnt_eep)
   lds r22,reset_cnt
   rcall write_eep

   ret

;------------------------------

read_reset_eep:

   ldi r29,low(reset_cnt_eep)
   mov r0,r29
   ldi r29,high(reset_cnt_eep)
   mov r1,r29
   rcall read_eep2

   sts reset_cnt,r0

   ret


;*********************************************************************************************

RESET:
	ldi	r16,high(RAMEND) 
	out	SPH,r16	         
	ldi	r16,low(RAMEND)	 
	out	SPL,r16
	

   rcall read_reset_eep
	
	rcall	check_reset_cause
	rcall	init_watchdog
	
   lds r29,reset_cnt
   inc r29
   sts reset_cnt,r29

   rcall write_reset_eep

	rcall	clear_sram	
	
	rcall	init_ports			


;;   sbi ddrd ,pd3
;;   sbi ddrd ,pd4
;;
;;   sbi portd,pd3
;;   sbi portd,pd4
;   rcall yellow_led_on
;   rcall green_led_on
;
;
;lineshit:
;
;   wdr
;
;   rjmp lineshit
;


   rcall	load_flash_constants	
	rcall	init_timer_1
	rcall	init_timer_0
 
; this was using pin pd7 for beep or some shit?   rcall	init_timer_2
   rcall reset_xbee
	rcall	i2c_init
	rcall	init_spi
	rcall	init_adc
	rcall	init_rtc
   rcall init_uart

	
	clr	r13
	clr	r4
	
	ldi	r16,0x02                      ; 4-11-2019 rpi was asking for packet immediately and it was blank
	or	r13,r16		                     ; set bit 1 so rpi won't ask for packet on reset
	
   rcall	init_send_times		;set send time for first packet

   rcall set_xbee_reset_time		;this adds six minutes to current time

   
   ldi r21,0x7E                        ; this is obsolete. it's always 0x7E because I never use set_receiver
   sts receiver,r21


	clr	r29
	sts	main_loop_counter,r29
   
   rcall init_pulse_meter

;ldi r19,0x88
;sts control_byte,r19


   lds r19,control_byte       ; 0x99 on    0x88 off      
   cpi r19,0x99
   brne  dead88ee


   rcall flash_leds_reset        ; confirm reset... need disticnct pattern
dead88ee:

   ldi r16,200
   sts pulse_state_cnt,r16             ; pulse state cnt is the flag that causes 0x15 to be sent; 200 means "don't send"

	rcall	calc_address_for_3_bytes         ; puts xeep addr in r6:r7. added 4-11-2019 because first packet had zeros
                                          ; for xeep addr which confused rpi. later I fixed r13 which was the actual
                                          ; bug that was telling rpi packet was ready on reset, so this can be deleted
                                          ; i think

main_loop:



;sleep:
;
;   rcall poll_uart                     ; need this to wake up
;   lds r21,control_byte
;   cpi r21,0x88
;   brne awake
;
;   wdr
;
;   rjmp sleep
;
;
;
;
;awake:

   lds r21,fork_flag
   tst r21
   brne line7830

   rcall poll_uart
	
   rcall check_xbee_reset_time
	
line7830:

	lds	r29,main_loop_counter	            ; this counter is used to ensure that
	inc	r29			                        ; the radio line is checked as frequently as
	sts	main_loop_counter,r29	            ; possible, and minimum time is wasted checking
	cpi	r29,$10			                     ; to see if pc needs spi bus.
	brne	not_this_time		                  ;
	clr	r29                                 ; 11-7-2013 i drastically speeded up check hi line
	sts	main_loop_counter,r29               ; so this counter can probably be increased after i test with pc
	

	
not_this_time:
	
   lds r29,control_byte       ; 0x99 on    0x88 off      
   cpi r29,0x99
   brne  dead88

	rcall check_spi_usb			;note that usb is checked way more often than parallel port

dead88:

   lds r21,fork_flag
   
   tst r21                    ; tst "ands" r21 with itself.

   brne line7853              ; ignore radios while pypipes reads stuff

   rcall poll_uart
   
   rcall	unit_specific_tasks	;slave auto turn off; send/echo; 
   rcall poll_uart

	rcall	timer_0_check		;catch 0xff rollover and increment counter
   rcall poll_uart

   rcall poll_uart
   
   rcall handle_0x15_state

	rcall	check_black_pushbutton
	
   rjmp line7880

line7853:

   rcall check_ff_timer
line7880:

   wdr
	
	rjmp	main_loop


