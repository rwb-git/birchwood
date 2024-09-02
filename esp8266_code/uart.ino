// disconnect D8 = txd2 = GPIO 15 during flashing. rx d7 gpio 13 is ok. something about gpio 15 associated with flash during boot...

// pulse_master.asm include_mega.asm says xbee uart is low start bit,  8 data,  lsb first,   one stop bit high,   idle high,   9600 baud,   parity 0

// garbage quite often follows 0x16 ack from local box to 0x15 pulse packet: 
//
//    03 07 00 16 11      the garbage:  2B 2B 2B 41 54 52 53 0D 41 54 43 4E 0D 0D 32 39 0D 4F 4B 0D 39 0D 4F 4B 0D now convert to ascii o
//                                      +  +  +  A  T  R  S  CR A  T  C  N  CR CR sp  ' CR O  K  CR '  CR O  K  CR  so this looks like a box asking for RS; why it's being broadcast is a mystery, unless it fails to
//                                      enter AT mode, or it's supposed to send it?


void init_uart(){

// Both Serial and Serial1 objects support 5, 6, 7, 8 data bits, odd (O), even (E), and no (N) parity, and 1 or 2 stop bits. To set the desired mode, call Serial.begin(baudrate, SERIAL_8N1), Serial.begin(baudrate, SERIAL_6E2), etc.
// Default configuration mode is SERIAL_8N1. Possibilities are SERIAL_[5678][NEO][12]. Example: SERIAL_8N1 means 8bits No parity 1 stop bit.

  Serial.begin(9600);

  for (int i=0; i<5; i++){
    
    delay_half_second();
  }

//  Serial.swap();    // swap enables the second uart on D7 rx gpio 13   and D8  tx gpio 15. but, gpio 15 (tx) has to be allowed to be pulled low during reset or it will not boot (it looks for an sd card if gpio 15 is hi). so I either need a crappy
                      // circuit that somehow detects every reset, or I have to use the other uart that is used by usb to flash.
                      //
                      // ok, using the normal uart I have to disconnect just rx to flash, but then with usb still connected I can re-connect both to xbee and it works so far. it might fuck up if an xbee packet arrives while programming, so 
                      // try this: leave both connected to the level shifter and remove power from xbee while programming. yeah, this looks like the best approach. xbee seems to work with usb connected.

  
}



void save_in_big_buff(uint8_t inchar){
          
  big_uart_buff[big_uart_ptr] = inchar;
  big_uart_ptr++;

  if (big_uart_ptr >= big_buff_size){

    big_uart_ptr = 0; // just rollover and fill from the beginning
  }
}

uint8_t bad_ck_state;

uint32_t bad_ck_cnt;

uint32_t bad_ck_epoch;

void handle_bad_ck(){
  
  /*
        log the first one. but sometimes xbee blows up and sends a blizzard, so count them as long as they are pouring in, and log one summary, then start over.

        state 0 waiting for bad ck

        state 1 received a bad ck in the last second

        state 2?
   */

   switch (bad_ck_state){

      case 0:

        bad_ck_epoch = my_epoch;

        add_bad_news((char *) "bad ck"); // log this one because it might be the only one

        bad_ck_state = 1;

        bad_ck_cnt = 1;
        
        break;

      case 1:

        if ((my_epoch - bad_ck_epoch) < 2){

          bad_ck_epoch = my_epoch;

          bad_ck_cnt++;
          
        } else {

          bad_ck_state = 0;

          if (bad_ck_cnt > 1){

            sprintf(rwb_buf,"bad cks %d",bad_ck_cnt);
            add_bad_news(rwb_buf);
          }
        }

        break;
   }
}


void poll_uart(){ // stackoverflow says tx and rx buffers are 128 bytes so if that's true I should never miss a packet as long as xbee receives it.

  uint8_t inchar;

  if (Serial.available() > 0){    // start bit is 0x26 = 38d = &; lots of those are sent, so skip them

    if (uart_ptr < 1000){ // this should never happen, since this ptr is reset after every packet, and the worst case screwup would try to get 256 bytes, so after a few packets this would get fixed.
      
      inchar = (uint8_t)Serial.read();
/*
      if (my_millis_ra_ptr < (my_millis_ra_size - 1)){

        if (my_millis_ra_ptr > 0){    // at 9600 baud, it's roughly 9600 / 10 = 960 bytes per second, or about 1 ms per byte, i think. this data agrees with that. so, if two bytes gap is more than 100 ms i will assume it's garbage
          new_millis = millis();
          my_millis_ra[my_millis_ra_ptr] = new_millis - old_millis;
          old_millis = new_millis; 
        } else {
          my_millis_ra[my_millis_ra_ptr] = millis();  // why did I do this.
        }
        

        my_millis_ra_ptr++;
      }
*/



        //    3-9-2024 it appears that esp only uses start_bits to separate packets in the big buffer???
        if ((start_bit_cnt < 5) && (inchar == 38)){ // don't save all of them so I can see more packets. when it was < 3 it was not separating packets properly in the display. they seem to have been processed properly otherwise. ok, there's another issue
                                                    // where somehow all the startbytes are missing:  
                                                    // 26 26 26 26 0B 00 00 15 00 07 7C 00 C2 B8 08 08 14 
                                                    // 26 26 26 26 0B 00 00 15 00 07 7C 00 C2 B8 08 08 14 03 00 00 16 16 2B 2B 2B 41 54 52 53 0D 41 54 43 4E 0D
                                                    //                 1  2  3  4  5  6  7  8  9 10 11 CK NO SB  2 3  what is this shit? don't forget the weirdness wher I seem to be getting local box RS check, which shouldn'g go out should it?
                                                     
                                                    // 26 26 26 26 26 0D 66 7E AA 01 04 4C 44 03 2E 00 C6 00 00 54 

                                                                                                        
                                                    // garbage quite often follows 0x16 ack from local box to 0x15 pulse packet: 
                                                    //
                                                    //    03 07 00 16 11      the garbage:  2B 2B 2B 41 54 52 53 0D 41 54 43 4E 0D 0D 32 39 0D 4F 4B 0D 39 0D 4F 4B 0D now convert to ascii o
                                                    //                                      +  +  +  A  T  R  S  CR A  T  C  N  CR CR sp  ' CR O  K  CR '  CR O  K  CR  so this looks like a box asking for RS; why it's being broadcast is a mystery, unless it fails to
                                                    //                                      enter AT mode, or it's supposed to send it?

          save_in_big_buff(inchar);
        }

      if ((millis() - old_uart_millis) > 7){ // at 9600 baud it takes about 1 ms per byte, so if it's 100 ms since the last one, start over. when this was 100 it would occasionally concatenate packets around 0x15. 

        // this byte is not related to the previous one, so restart packet
     
        uart_ptr = 0;
      
        start_bit_cnt = 0;
      
        for (int i=0;i<uart_buff_size;i++){
          uart_buff[i] = 0;
        }
        done = 1;
        ck = 0;
      }

      old_uart_millis = millis();
      

      if ((inchar == 38) && (done == 1)){  // 38d = 0x26
        
        start_bit_cnt++;      // master sends 0x1B start bits = 27d.   3-9-2024 it appears that esp only uses start_bits to separate packets in the big buffer???
      
      } else { // not a start bit, so process it
        
          if (done == 1){ // first non-start bit so reset stuff
            done = 0;
            ck = 0;
            uart_ptr = 0;
            expected = 0;

//            replay_begin[replay_index] = my_epoch; // this will be ignored if ck is bad

            /*
                  4-14-2021

                    it looks like master wdr during pulse train, so i want to log a bunch of preceding stuff when master reset of any type is detected, but it could be particularly helpful in locating wdr cause.

                    so, keep a rotating list of packet type (0xAA, 0x15, 0x16, 0xFF = unknown) with start and stop timestamp for each. the last 5 should be more than enough.

                    call it replay_type[], replay_begin[], replay_end[], replay_index
             */
  
          }


          //                                                             the loop counter in the avr code sends this block       1          2         3    4    5    6          7      8
          //
          //                                                                                     number   send       rec   type                       stat tank rpi  flow34
          // packet from master (which is not same as the one from local box to android tablet): 0D       66         7E    0xAA  sender_id  receiver  data data data data       ck     ck   master_RS  slaveRS  ck
          //                                                                  uart buff index    0        1          2     3     4          5         6    7    8    9          10     11   12         13       14 
          //                                                                                              this is flow cutoff in byte 1, rpi temperature in byte 8, flow adc in byte 9
          //
          // 8-20-2022 changes to add 10 bit air and depth 8 bit adc instead of avr doing the math                   2 = depth adc                                                          air msb    air lsb       
          //
          //
          // 8-11-2022 I think these bytes are not used, and I can use one for air pressure: 2 4 5 10 11
          //
          // 15 bytes are sent. first byte is 0D = 13 data bytes, those are followed by one checkbyte. in current esp listing the two RS values are always zero.ck10 and ck11 have varying values, ck10 usually or always 01, but that is not used here
          //
          // pulse_master.asm code is messy for the old data bytes which include receiver byte 5, ck10 and ck11 so use these for new stuff: 2 = receiver, 4 = sender id, and maybe the two RS values. so use sender_id byte 4 for air pressure
          // 
          // look at flow adc and how it's handled by php - nope, that's sent only in a huge pile when I click a button. it has to be handle like tank level. see process_tank() which calls try_a_change() and go from there. try_a_change calls
          // send_change which puts the byte with tank_chang.php, so I need air_change.php
          //
          // make a new php page since the old one is long enough. new page just shows transfer blocks and air graph.

/*                            
                  use these for future expansion = XX   YY can be used but involves messy asm code  PP will be used for air pressure. put depth adc in DD so I can calibrate in php. how to sync rpi?

                  actually, no need to deal with the messy loop if I can safely put data in places like sender_id_s without some other code overwriting it. 

                                                      msb lsb
                        DD    YY YY             YY YY PP PP            
                  0D 66 7E AA 01 04 00 64 93 DA 01 D6 00 00 4D
                  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
                  
                  0D 66 7E AA 01 04 00 64 03 DB 01 47 00 00 4D
                  
                  0D 66 7E AA 01 04 00 64 93 DD 01 D9 00 00 45
                  
                  0D 66 7E AA 01 04 00 64 03 D6 01 42 00 00 45
                  
                  0D 66 7E AA 01 04 00 64 93 DA 01 D6 00 00 4D
                  
                  0D 66 7E AA 01 04 00 64 03 DC 01 48 00 00 45
                  
                  0D 66 7E AA 01 04 00 64 93 D9 01 D5 00 00 4D
                  
                  0D 66 7E AA 01 04 00 64 03 DA 01 46 00 00 4D
                  
                  0D 66 7E AA 01 04 00 64 93 DB 01 D7 00 00 4D
                  
                  0D 66 7E AA 01 04 00 64 03 DF 01 4B 00 00 45
                  
                  0D 66 7E AA 01 04 00 64 93 D2 01 CE 00 00 5D 
*/
          if (uart_ptr < uart_buff_size){
            uart_buff[uart_ptr] = inchar;
          }
          
          save_in_big_buff(inchar);
          
          
          if (uart_ptr == 0){
          
              expected = inchar; // the first byte of a packet is the number of data bytes that this packet should have. following the "expected" bytes will be the checksum
              ck = 0;
          
          } else if (uart_ptr == (expected + 1)){ // this should be the checksum

              done = 1;
              save_ck = inchar;
              save_my_ck = ck;
              
              if (ck != inchar){ // ignore packets with bad ck, and don't include in "unknown" count either. 

                bad_ck++;

                handle_bad_ck();

              } else {  // good checksum, so process this packet

                    seconds_since_last_packet = 0;

                    stop_nagging_no_packets = 0;

                    good_ck++;

/*                    replay_end[replay_index] = my_epoch;
                    
                    replay_type[replay_index] = uart_buff[3];

                    replay_index++;

                    if (replay_index >= replay_max){

                      replay_index = 0;;
                    }

                    //log_replay(); // delete this once the code works
*/                    

                    if (unknown_packet_flood == 1){

                      if ((my_epoch - unknown_packet_epoch) > 300){ // none for five minutes so log the total

                        if (unknown_packets > 1){ // the first one was already logged
                        
                          sprintf(rwb_buf,"%d more unknown packets were received",unknown_packets - 1);
                          
                          add_bad_news(rwb_buf);
                        }
                        
                        unknown_packet_flood = 0;

                        unknown_packets = 0;
                      }
                    }

                    if (xbee_reset_flood == 1){

                      if ((my_epoch - xbee_reset_epoch) > 300){ // none for five minutes so log the total

                        if (xbee_reset_flood_cnt > 1){ // the first one was already logged
                        
                          sprintf(rwb_buf,"%d more xbee resets",xbee_reset_flood_cnt - 1);
                          
                          add_bad_news(rwb_buf);
                        }
                        
                        xbee_reset_flood = 0;

                        xbee_reset_flood_cnt = 0;
                      }
                    }

                    uint16_t new_slot;

                    switch(uart_buff[3]){
                      
                      case 0xAA:
      
                          packets_0xAA++;

                          fast_log_0xAA_cnt++;

                          if (in_charge_0xAA == 1){       // when i had this near the end it took forever for nano to get this ack, so put it here and see...

//                            delay(200);                     // why delay at all? with this delay, bench test said 9 retries. with no delay, retries = 0

                            ack_0xAB_to_0xAA();
                          }
/*                                              
                          if (xbee_RS_state == 3){
                    
                            xbee_RS_state = 4;  // see notes in uart get_RS()

                            xbee_RS_counter = 0;
                          }
*/
                          if (master_rtc_state == 2){

                            master_rtc_state = 3;

                            add_log((char *) "master rtc state is 3, count a few seconds then send packet");
                            log_time();
                          }
          
                          //old_tank = current_tank;
          
                          if (first_packet != 1){
                            old_status = current_status;
                          }
                          
                          current_tank = uart_buff[7];
                          current_status = uart_buff[6];
          
                          if (first_packet == 1){ 
          
                            if (force_status_write == 1){ // force all status bits to toggle so all files get written; this is a way to test file write code or whatever
                              
                              old_status = ~current_status; // ~ flips all the bits
                              
                            }
                            
                            first_packet = 0;     
                          }

                          //add_fast_log((char *) "0xAA");
                            
                          process_status_bits();
          
                          process_tank();                 // new style local files. sends packet to php

                          old_air = current_air;

                          process_air(uart_buff[12],uart_buff[13]); // air pressure msb lsb  
          
                          process_rpi_temperature(uart_buff[8]);

                          flow_adc = uart_buff[9];

                          new_slot =  (1440.0 * (float)todays_seconds() / 86400.0);

                          // it never ends. i think i need to add 5 hours here so that php on UTC will plot it correctly using UTC. for example, 7 am UTC is 2 am EST and 3 am DST. ignore dst. add 5 hours so that something that happens
                          // ...wait a minute. do i have to do anything. changing ntp from utc to est does the 5 hour shift. try it as is and add or subtract 5 hours here if needed after looking at the plot in a few days

                          if (new_slot != xeep_slot){

                            if (new_slot > (xeep_slot + 1)){ // fill skipped slots, but ignore wrap around screwup

                              for (uint16_t i=xeep_slot+1;i<=new_slot;i++){

                                xeep_slot = i;

                                write_xeep_slot();

                              }
                            } else {

                              xeep_slot = new_slot;

                              write_xeep_slot();
                            }
                          }
                          
                          flow_cutoff = uart_buff[1];

                          get_RS(0);   // 9-11-2022 if uart buffer is 128 bytes it should be safe to do this on every 0xAA packet.



                        //  save_RS_to_xeep();

                          break;
                        
                        case 0x15:
      
                          packets_0x15++;
          
                          seconds_since_last_pulse = 0;
                          
                          add_fast_log((char *) "0x15");

                          process_0x15_pulse();
          
                          count_gallons();
                          break;

                        case 0x16:

                          add_fast_log((char *) "0x16");
      
                          packets_0x16++;

                          break;

                        case 0xDD:  // ask master cutoff
                        case 0xDE:  // master responds cutoff
                        case 0xCC:  // send master cutoff
                        case 0x92:  // send rtc to all listeners

                          break;

                        case 0x39:    // sewer
                        
                          get_RS(1);   // 9-11-2022 if uart buffer is 128 bytes it should be safe to do this on every 0xAA packet.

                          break;

                        default:  // packet has correct checksum but type is unknown. this should not happen since i've included case statements for all expected packets, i think.
                                  // sometimes i get a lot in a few seconds and it fills the log with useless entries. maybe only log the first one and then count them for X minutes. if there are none for Y minutes reset everything and log the total
                  
                          if (unknown_packet_flood == 0){
                          
                            unknown_packet_flood = 1; 
      
                            add_bad_news((char *) "unknown packet");

                          }

                          unknown_packet_epoch = my_epoch;

                          unknown_packets++;

                          total_unknown_packets++;

                          break;
                          
                    } // switch uart_buff[3] = msg type
                    
                }       
           }
           
           if ((uart_ptr > 0) && (uart_ptr < (expected + 1))){

              ck = ck ^ inchar;   // checksum does not include first byte = number of datas, and does not include final byte, which is ck from master
              
           }
           
        uart_ptr++;
        uart_cnt++;
      }      
    }
  }
}




/*
  
  
  
 
String            UARTrcvData;                            // AUTO
char              UARTrcvBuffer;                          // AUTO
int               UARTrcvCount;                           // AUTO
 
 
 
 
void UARTreceive() { // ----------------------------------------------------------------
  UARTrcvCount = 0;
  UARTrcvBuffer == 'N';
  UARTrcvData = "";

  while (UARTrcvBuffer != 'F' && UARTrcvCount < RcvCount) {
    while (Serial.available() > 0) {
      UARTrcvCount = UARTrcvCount + 1;
      UARTrcvBuffer = (char)Serial.read();
      if (UARTrcvBuffer != 'F') {
        UARTrcvData += UARTrcvBuffer;
      }
    }
  }
}

*/

/*
 * 

Serial object works much the same way as on a regular Arduino. Apart from hardware FIFO (128 bytes for TX and RX) HardwareSerial has additional 256-byte TX and RX buffers. 
Both transmit and receive is interrupt-driven. Write and read functions only block the sketch execution when the respective FIFO/buffers are full/empty.

Serial uses UART0, which is mapped to pins GPIO1 (TX) and GPIO3 (RX). Serial may be remapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin. Calling swap again maps UART0 back to GPIO1 and GPIO3.

Serial1 uses UART1, TX pin is GPIO2. UART1 can not be used to receive data because normally it's RX pin is occupied for flash chip connection. To use Serial1, call Serial1.begin(baudrate).

If Serial1 is not used and Serial is not swapped - TX for UART0 can be mapped to GPIO2 instead by calling Serial.set_tx(2) after Serial.begin or directly with Serial.begin(baud, config, mode, 2).





Both Serial and Serial1 objects support 5, 6, 7, 8 data bits, odd (O), even (E), and no (N) parity, and 1 or 2 stop bits. To set the desired mode, call Serial.begin(baudrate, SERIAL_8N1), Serial.begin(baudrate, SERIAL_6E2), etc.












Serial swap has nothing to do with Serial1. The ESP8266 has two hardware UARTs: UART0 on pins 1 and 3 (TX0 and RX0 resp.), and UART1 on pins 2 and 8 (TX1 and RX1 resp.).
UART0 also has hardware flow control on pins 15 and 13 (RTS0 and CTS0 resp.). These two pins can also be used as alternative TX0 and RX0 pins. That's what Serial.swap() does.






The ESP8266 has two hardware UARTS (Serial ports):
UART0 on pins 1 and 3 (TX0 and RX0 resp.), and UART1 on pins 2 and 8 (TX1 and RX1 resp.), however, GPIO8 is used to connect the flash chip. This means that UART1 can only transmit data.

UART0 also has hardware flow control on pins 15 and 13 (RTS0 and CTS0 resp.). These two pins can also be used as alternative TX0 and RX0 pins.






The Serial object works much the same way as on a regular Arduino. Apart from the hardware FIFO (128 bytes for TX and RX), Serial has an additional customizable 256-byte RX buffer. The size of this software buffer can be changed by the user. 
It is suggested to use a bigger size at higher receive speeds.

The ::setRxBufferSize(size_t size) method changes the RX buffer size as needed. This should be called before ::begin(). The size argument should be at least large enough to hold all data received before reading.

For transmit-only operation, the 256-byte RX buffer can be switched off to save RAM by passing mode SERIAL_TX_ONLY to Serial.begin(). Other modes are SERIAL_RX_ONLY and SERIAL_FULL (the default).

Receive is interrupt-driven, but transmit polls and busy-waits. Blocking behavior is as follows: The ::write() call does not block if the number of bytes fits in the current space available in the TX FIFO.
The call blocks if the TX FIFO is full and waits until there is room before writing more bytes into it, until all bytes are written. In other words, when the call returns, all bytes have been written to the TX FIFO,
but that doesn't mean that all bytes have been sent out through the serial line yet. The ::read() call doesn't block, not even if there are no bytes available for reading. The ::readBytes() call blocks until the number of
bytes read complies with the number of bytes required by the argument passed in. The ::flush() call blocks waiting for the TX FIFO to be empty before returning. It is recommended to call this to make sure all bytes have been
sent before doing configuration changes on the serial port (e.g. changing baudrate) or doing a board reset.

Serial uses UART0, which is mapped to pins GPIO1 (TX) and GPIO3 (RX). Serial may be remapped to GPIO15 (TX) and GPIO13 (RX) by calling Serial.swap() after Serial.begin. Calling swap again maps UART0 back to GPIO1 and GPIO3.

Serial1 uses UART1, TX pin is GPIO2. UART1 can not be used to receive data because normally it's RX pin is occupied for flash chip connection. To use Serial1, call Serial1.begin(baudrate).

If Serial1 is not used and Serial is not swapped - TX for UART0 can be mapped to GPIO2 instead by calling Serial.set_tx(2) after Serial.begin or directly with Serial.begin(baud, config, mode, 2).



Both Serial and Serial1 objects support 5, 6, 7, 8 data bits, odd (O), even (E), and no (N) parity, and 1 or 2 stop bits. To set the desired mode, call Serial.begin(baudrate, SERIAL_8N1), Serial.begin(baudrate, SERIAL_6E2), etc.
Default configuration mode is SERIAL_8N1. Possibilities are SERIAL_[5678][NEO][12]. Example: SERIAL_8N1 means 8bits No parity 1 stop bit.

A new method has been implemented on both Serial and Serial1 to get current baud rate setting. To get the current baud rate, call Serial.baudRate(), Serial1.baudRate(). Return a int of current speed. For example

// Set Baud rate to 57600
Serial.begin(57600);

// Get current baud rate
int br = Serial.baudRate();

// Will print "Serial is 57600 bps"
Serial.printf("Serial is %d bps", br);

Serial and Serial1 objects are both instances of the HardwareSerial class.
This is also done for official ESP8266 Software Serial library, see this pull request.
Note that this implementation is only for ESP8266 based boards, and will not works with other Arduino boards.

To detect an unknown baudrate of data coming into Serial use Serial.detectBaudrate(time_t timeoutMillis). This method tries to detect the baudrate for a maximum of timeoutMillis ms. It returns zero if no baudrate was detected, 
or the detected baudrate otherwise. The detectBaudrate() function may be called before Serial.begin() is called, because it does not need the receive buffer nor the SerialConfig parameters.

The uart can not detect other parameters like number of start- or stopbits, number of data bits or parity.

The detection itself does not change the baudrate, after detection it should be set as usual using Serial.begin(detectedBaudrate).

Detection is very fast, it takes only a few incoming bytes.

SerialDetectBaudrate.ino is a full example of usage.
 */

 
uint8_t convert_hex_chars_to_number(uint8_t msb, uint8_t lsb){

  // 48 = 0, 49 1, etc. 65 = A...

  uint8_t lval, val;

  if (lsb < 64){

    lval = lsb - 48;
  } else {

    lval = (lsb - 65) + 10;
  }

  if (msb < 64){

    val = msb - 48;
  } else {

    val = (msb - 65) + 10;
  }

  return (val * 16) + lval;
}

 void get_RS(int type){

  xbee_RS = 0;

 /*
     a few hours of 205 on desk in back room, 9-11-2022 temperature about 73, overcast.
     
        15 00000 25 00000 35 00002 45 00000
        16 00000 26 00000 36 00003 46 00000
        17 00000 27 00000 37 00006 47 00000
        18 00000 28 00000 38 00001 48 00000
        19 00000 29 00000 39 00006 49 00000
        20 00000 30 00000 40 00023 50 00000
        21 00000 31 00000 41 00026 51 00000
        22 00000 32 00000 42 00008 52 00000
        23 00000 33 00000 43 00000 53 00000
        24 00000 34 00001 44 00000 54 00000
   */

  uint32_t mil = millis();
  uint8_t inbyte;

  while((millis() - mil) < 1000){   // might not matter, but you have to wait 1 sec before +++, which i think means you have been sending stuff, but anyway
    yield();
  }

  Serial.write('+');
  Serial.write('+');
  Serial.write('+');

  mil = millis();

  while((millis() - mil) < 1000) { 
  
    yield();
  }  

  int count = 0;
  int fail = 0;

   while (Serial.available() > 0){  

      inbyte = (uint8_t)Serial.read();

      if (count == 0){

        if (inbyte != 0x4F){

          if ((inbyte != 0x26) && (inbyte != 0x2B)){    // ignore & and +
          
            sprintf(rwb_buf,"RS expected 0x4F but got %02X",inbyte);      // I get lots of 0x26 here, which is &, so maybe it's start bits. ignore 0x26 here. i also get a few 0x2B = +, so ignore those too. hell, why not ignore all the crap as long as it usually works.
                                                                          // fast log will show all the successes.
            add_bad_news(rwb_buf);
          }

          fail = 1;
          break;
        }
      }
      
      if (count == 1){
      
        if (inbyte != 0x4B){
          
          sprintf(rwb_buf,"RS expected 0x4B but got %02X",inbyte);
          add_bad_news(rwb_buf);
          
          fail = 1;
          break;
        }
      }

      if (count == 2){

        if (inbyte != 0x0D){
          
          sprintf(rwb_buf,"RS expected 0x0D but got %02X",inbyte);
          add_bad_news(rwb_buf);
          
          fail = 1;
          break;
        }
      }

      count++;
            
      yield();
   }

  if (fail == 0){
      
      Serial.write('A');
      Serial.write('T');
    
      Serial.write('R');
      Serial.write('S');      
      Serial.write(0x0D);
    
      int done = 0;
    
      uint8_t lsb = 0;
      uint8_t  msb = 0;
    
      mil = millis();
    
      while (done == 0){
        
        yield();
    
        if ((millis() - mil) > 1500){
    
          done = 1; // give up waiting for xbee
        }
    
        while (Serial.available() > 0){  
        
              yield();
              
              inbyte = (uint8_t)Serial.read();
        
              if (count == 3){
                msb = inbyte;
              }
        
              if (count == 4){
        
                lsb = inbyte;
              }
        
              if (count == 5){
        
                if (inbyte == 0x0D){
                          
                  xbee_RS = convert_hex_chars_to_number(msb,lsb); 
                                   
                  sprintf(rwb_buf,"chars %c %c RS %hu",msb, lsb, xbee_RS);
                  add_fast_log(rwb_buf);

                  //uint8_t RS_type = 0; // 0 0xAA, 1 0x39
                  
                  //#define xbee_histo_size 40

                  //uint16_t xbee_histo[xbee_histo_size + 1]; // save a count of RS for values 15..54

                  int ind = (int)xbee_RS - 15; // 15 goes in slot 0, 54 - 15 goes in slot 39

                  if (type == 0){ // 0 0xAA, 1 0x39
    
                      if ((ind >= 0) && (ind < xbee_histo_size)){
    
                        xbee_histo[ind] = xbee_histo[ind] + 1;
                      }
    
                      if (xbee_RS > 5){
    
                        if (xbee_RS < min_xbee_RS){
    
                          min_xbee_RS = xbee_RS;
                        }
    
                        if (xbee_RS > max_xbee_RS){
    
                          max_xbee_RS = xbee_RS;
                        }
                      }
                    } else { // type = 1
    
                      if ((ind >= 0) && (ind < xbee_histo_size)){
    
                        xbee_histo_39[ind] = xbee_histo_39[ind] + 1;
                      }
    
                      if (xbee_RS > 5){
    
                        if (xbee_RS < min_xbee_RS_39){
    
                          min_xbee_RS_39 = xbee_RS;
                        }
    
                        if (xbee_RS > max_xbee_RS_39){
    
                          max_xbee_RS_39 = xbee_RS;
                        }
                      }
                    } // else type == 1                  
                  
                } else { // if inbyte == 0x0D

                  uint8_t dud = convert_hex_chars_to_number(msb, lsb);

                  sprintf(rwb_buf,"RS expected 0x0D but got %02X",inbyte);
                  add_bad_news(rwb_buf);                                    
                  
                  sprintf(rwb_buf,"chars %c %c val %hu",msb, lsb, dud);
                  add_bad_news(rwb_buf);                              
                }
                
              }
        
              count++;  

          }
       }
     
    Serial.write('A');        // this is not necessary, but it puts xbee back in normal mode instead of it waiting to timeout
    Serial.write('T');
    
    Serial.write('C');
    Serial.write('N');
    Serial.write(0x0D);
  }    
 }


 
