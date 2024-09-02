
uint8_t convert_binary_to_bcd(uint8_t binary){

  // binary range 00d to 99d = 0x00 to 0x63

  uint8_t tens = binary / 10; 

  return tens * 16 + (binary - tens * 10);
}

void test_bin2bcd(){

  uint8_t bcd;
  
  for (int i=0; i<100; i++){

    bcd = convert_binary_to_bcd(i);

    sprintf(rwb_buf,"in %d %02X out %02X",i,i,bcd);
    add_fast_log(rwb_buf);    // 10-5-2021 this was add_log, but it was flooding both regular log and fast log when the bug was calling this procedure, which is not actually needed anymore
    
  }
}

void send_master_rtc_packet(){

  // how to deal with dst discussion is in data - process_0x15_pulse(), because that's where i get access to master rtc. does it belong in clock, rtc, data, here, or where?
  
  /*
   * 
   * 
                0B 07 7E 92 24 36 08 07   25   09    21    10    EB sent 9:35 am sept 25 2021. this is DST, so the time sent is EST, one hour less, since I'm ignoring dst everywhere now except for online legend.
                            ss mm hh day  date month year  1 hz  ck


                0B 07 7E 92 22 07 07 06 04 06 21 10 FC    android sent this at about 7:12 am dst friday 6-4-2021. all the rtc values except the 1 hz flag are in BCD, so the time was probably 7:7:22
                
    rtc string: 0B 07 7E 92 46 37 11 06 04 06 21 10 BE at about 10:37:49 esp time, 11:37:49 dst. bcd looks good

    
                0B 07 7E 92 53 41 12 06 04 06 21 10 DE android about 12:42
    rtc string: 0B 07 7E 92 27 42 12 06 04 06 21 10 A9 esp about 12:42



                0B  11d
                07  sender
                7E  receiver
                92  msg type = receiver is to set rtc
                22  secs
                07  mins
                07  hours: this is 7 am dst; hour range is 0..23
                06  day = friday? s1 m2 t3 w4 t5 f6 yes, friday is 6 if sunday is 1                                 
                04  date
                06  month
                21  year
                10  1 hz flag
                FC  ck. confirm in spreadsheet /mtv/fake_home/xor.ods

                xbee_send_rtc:

                  start bytes

                  11d

                  clear ck. xor all following bytes with ck

                  sender id

                  0x7E

                  0x92

                  send 7 rtc bytes then 1 hz flag. convert binary to bcd before sending. xor bytes with ck. 1 hz flag is 0x10 sent as 0x10, no bcd conversion for it

                    hey wait a damn minute. can't i just send these straight from my rtc, without worrying about range or bcd? umm, no, because arduino lib converts everything to human decimal
                        
                                                                      //    rtc range
                      
                        Clock.setSecond(nptm->tm_sec);                //    0-59        set second first, then you have to set all the rest within one second. scope test says it takes 3.2 msec to set these six registers
                        Clock.setHour(nptm->tm_hour);                 //    0-23       
                        Clock.setMinute(nptm->tm_min);                //    0-59
                        Clock.setYear(nptm->tm_year + 1900 - 2000);   //    00-99
                        Clock.setMonth(nptm->tm_mon + 1);             //    1-12       
                        Clock.setDate(nptm->tm_mday);                 //    1-31


                    secs    0..59 so use my_epoch
                    mins    0..59 so use my_epoch
                    hrs     so use my_epoch
                    day     this is day of week and is used by master, i think, to store xeep, so it matters. 
                    
                            tm_wday int days since Sunday 0-6

                            rtc day of week is arbitrary, i think, and i believe i use 1 for sunday, 7 for saturday

                            so that implies my_epoch day + 1
                    date    use my_epoch
                    month   use my_epoch month + 1
                    year    use my_epoch year - 100? does this matter?

                  send ck
            */

    // create a string of the hex values and add to log
    uint8_t bytes[20];
    
    bytes[0] = 0x0B;
    /*          0B  11d
                07  sender
                7E  receiver
                92  msg type = receiver is to set rtc
                22  secs
                07  mins
                07  hours: this is 7 am dst; hour range is 0..23
                06  day = friday? s1 m2 t3 w4 t5 f6 yes, friday is 6 if sunday is 1                                 
                04  date
                06  month
                21  year
                10  1 hz flag
                FC  ck. confirm in spreadsheet /mtv/fake_home/xor.ods   */
    bytes[1] = 0x07;
    bytes[2] = 0x7E;
    bytes[3] = 0x92;

    is_dst_on_now(); // this screws something up if i do it down there. how?

    struct tm *ptm = gmtime ((time_t *)&my_epoch); 
    
    bytes[4] = ptm->tm_sec;
    bytes[5] = ptm->tm_min;
    bytes[6] = ptm->tm_hour;

/* 9-24-2021 I stopped using android tablet so let master rtc ignore dst, which will also eliminate the plot glitches on dst changes. If I need to use android along with esp I need to restore this block of code

    if (saveit == 1){

      bytes[6] += 1;
      
      if (bytes[6] >= 24){
        bytes[6] -= 24;
      }
    }

*/
    
    bytes[7] = ptm->tm_wday + 1;
    bytes[8] = ptm->tm_mday;
    bytes[9] = ptm->tm_mon + 1;
    bytes[10] = ptm->tm_year + 1900 - 2000;
    bytes[11] = 0x10; // 1 hz flag

    for (int i=4;i<=10;i++){

      bytes[i] = convert_binary_to_bcd(bytes[i]);
    }

    uint8_t ck = 0;

    for (int i=1;i<=11;i++){

      ck = ck ^ bytes[i];
    }
    
    bytes[12] = ck;

    /*
     *   
                  sprintf(block,"%d ",current + 18000); // add 5 hours
                  
                } else {
                  
                  sprintf(element,"%d ",current - preceding);  // delta does not need five hours
                  
                  strcat(block,element);
                }

     */
     char element[20];
  
     sprintf(rwb_buf,"rtc string: %02X ",bytes[0]); 
        
     for (int i=1;i<=12;i++){
        
        sprintf(element,"%02X ",bytes[i]);  
        
        strcat(rwb_buf,element);
      }

      add_log(rwb_buf);

      if (in_charge_rtc == 1){

        do_start_bits();

        for (int i=0; i<=12; i++){
          Serial.write(bytes[i]);
        }
      }
      
}



void ack_0xAB_to_0xAA(){


  do_start_bits();
  
  Serial.write(0x03);
  Serial.write(0x07);
  Serial.write(0x00);
  Serial.write(0xAB);
  Serial.write(0xAC); // checksum verified in spreadsheet checksum.ods
}

void ack_0x16_to_0x15(){

  /*   
      0B 00 00 15 00 01 55 00 76 1F 08 03 23
      
      03 07 00 16 11
      
      0D 66 7E AA 01 04 0C 52 03 2D 00 93 00 00 54 
   */

  do_start_bits();
  
  Serial.write(0x03);
  Serial.write(0x07);
  Serial.write(0x00);
  Serial.write(0x16);
  Serial.write(0x11); // checksum verified in spreadsheet checksum.ods
}


void send_master_cutoff_values(){  // from xbee/avr_files/1284/rx1284.asm

  /*

   i used new_probe and usbasp to send, using xbee menu i sent 102 102 60 for the three values = cutoff, cutoff, xbee pause seconds, i think
   
      0D 96 7E AA 01 04 00 4A 03 F9 01 4B 00 00 BD
      
      07 07 7E CC 66 66 00 3C 89    this was sent to master: 102d 102d 60d = 0x66 0x66 0x00 0x3C. 

        ck includes 7E CC 66 66 00 3C and is xor operation

        07  number of datas
        07  xbee unit id
        7E  receiver id
        CC  msg type
        66  flow 12 
        66  flow 34
        00  pause seconds high byte
        3C  pause seconds low byte
        89  checksum is xor of all datas xbee unit id .. pause low bytes. libre calc spreadsheet confirms 07 xor 7e xor cc xor 66 xor 66 xor 00 xor 3c = 0x89
        
      
      03 07 96 DD 4C    rx1284.asm says 0xDD is "ask master cutoff values"
      
      07 00 07 DE 66 66 00 3C E5    rx1284.asm says 0xDE is "master responds with cutoff values". pulse_master.asm says send this: 0x07 0x00 pb_sender 0xDE flow12 flow34 pause_h pause_l ck
      
      0D 66 7E AA 01 04 00 4A 71 F9 01 B9 00 00 CD    cutoff is second byte, and was changed from 0x96 up in the first 0xAA to 0x66 here, so it worked.

      test 0xDD, looks good:
      
        0D 66 7E AA 01 04 00 4A 03 F9 01 4B 00 00 4D
        
        03 07 96 DD 4C
        
        07 00 07 DE 66 66 00 3C E5 

      Serial.write(val)
      Serial.write(str)
      Serial.write(buf, len)
      Parameters
      
      Serial: serial port object. See the list of available serial ports for each board on the Serial main page.
      val: a value to send as a single byte.
      str: a string to send as a series of bytes.
      buf: an array to send as a series of bytes.
      len: the number of bytes to be sent from the array.
      Returns
      
      write() will return the number of bytes written, though reading that number is optional. Data type: size_t.

   */

  // send start bytes = send '&' 0x1B times

  do_start_bits();

  // send 07= number of data bytes not including ck

  Serial.write(0x07);

  // send 1 xbee unit id = is this obsolete? use 0x07

  Serial.write(0x07);

  // send 2 0x7E = receiver ID; is this obsolete? use 0x7E

  Serial.write(0x7E);

  // send 3 0xCC = msg type

  Serial.write(0xCC);

  // send 4 flow 1 2 cutoff = is this obsolete? use same value as flow 34

  Serial.write(temporary_flow_cutoff);

  // send 5 flow 3 4 cutoff = the value

  Serial.write(temporary_flow_cutoff);

  // send 6 pause seconds h = is this obsolete? use 0

  Serial.write(0);

  // send 7 pause seconds l = use 60d

  Serial.write(60);

  // send checksum

  uint8_t ck = 0x07 ^ 0x7E;

  ck = ck ^ 0xCC;

  ck = ck ^ temporary_flow_cutoff;
  ck = ck ^ temporary_flow_cutoff;

  ck = ck ^ 0;
  ck = ck ^ 60;

  Serial.write(ck);
}

void do_start_bits(){

  for (int i=0;i<27;i++){ // send 27 = 0x1B start bytes

    Serial.write('&');
  }
}

void ask_master_cutoff_values(){

  do_start_bits();
  
  //03 07 96 DD 4C    rx1284.asm says 0xDD is "ask master cutoff values"
  
  Serial.write(0x03);
  Serial.write(0x07);
  Serial.write(0x96);
  Serial.write(0xDD);
  Serial.write(0x4C);
  
}





void send_control_byte(uint8_t payload){  // 0x99 on, 0x88 off

/*  
                simple packet that tells box to do nothing at all. infinite wdr loop. packet is 0x55 similar to send master cutoff values

                    off    0x07 0x07 0x7E 0x55 0x88 0x88 0x88 0x88 ck

                    on     0x07 0x07 0x7E 0x55 0x99 0x99 0x99 0x99 ck
*/                    

  // send start bytes = send '&' 0x1B times

  do_start_bits();

  // send 07= number of data bytes not including ck

  Serial.write(0x07);

  // send 1 xbee unit id = is this obsolete? use 0x07

  Serial.write(0x07);

  // send 2 0x7E = receiver ID; is this obsolete? use 0x7E

  Serial.write(0x7E);

  // send 3 0xCC = msg type

  Serial.write(0x55);

  // send 4 flow 1 2 cutoff = is this obsolete? use same value as flow 34

  Serial.write(payload);

  Serial.write(payload);

  Serial.write(payload);

  Serial.write(payload);

  // send checksum

  uint8_t ck = 0x07 ^ 0x7E;

  ck = ck ^ 0x55;

  ck = ck ^ payload;
  ck = ck ^ payload;
  ck = ck ^ payload;
  ck = ck ^ payload;
  

  Serial.write(ck);

  // awake 07 07 7E 55 99 99 99 99 2C   confirmed in spreadsheet and online calc. same ck because it comes from 01 7E 55, and then when you xor twice with same new arg it returns old value, so 88 88 88 88 and 99 99 99 99 don't do anything except return 2C
  // sleep 07 07 7E 55 88 88 88 88 2C 
}
