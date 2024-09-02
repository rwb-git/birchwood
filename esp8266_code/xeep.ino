/*

    xeep layout, 4096 bytes:

      flow_adc for each minute of the day 0..1439

      xbee_RS for each day of the year including leap year 1440..1440 + 367 = 1440..1807  days are 1..366 so data starts in 1441 because i'm stupid

      rpi_temp for each day of the year including leap year 1808..2175



 
    per nano/nano_ds3231.asm

      xeep on ds3231 is 24C32   32k / 8 = 4k bytes from 0000 to 0FFF. hardwired address seems to be 111: 1010 1110, so the 7 bit address is 0x57 which agrees with i2c_scan

        C means 400 khz. my old xeep are 24LC256 LC means 1 Mhz. bytes = 256k/8 = 32k 0000 to 7FFF

    I want to save flow adc here and write every byte which is about 1 per minute

      24 x 60 = 1440d = 0x05A0 

      this almost fits the xeep 3 times: decimal xeep size is 4096. 3 x 1440 = 4320

      flow_adc per minute = 1440

      xbee RS daily for year 365

      rpi max temp daily for year 365   total so far 1440 + 2 x 365 = 2170, leaving 1926 unused

      
    //found 12c 0x68  rtc
    //found 12c 0x57  xeep. web says it is 0x56 on some devices. pdf says bits are 1010 a2 a1 a0 r/w. so the seven bit version with a2 a1 a0 = 111 is 101 0111 = 0x57. the 0x56 makes sense since you can do anything you like with a2 a1 a0,
    //                      so the range could be 0x50 .. 0x57.
                            // other info: AT24C32/64, 32K/64K SERIAL EEPROM: The 32K/64K is internally organized as 256 pages of 32 bytes each. Random word addressing requires a 12/13 bit data wordaddress
                            //
                            // BYTE WRITE: A write operation requires two 8-bit data word addresses following thedevice address word and acknowledgment. Upon receipt of this address, the EEPROMwill again respond with a zero and then clock in 
                            // the first 8-bit data word. Followingreceipt of the 8-bit data word, the EEPROM will output a zero and the addressingdevice, such as a microcontroller, must terminate the write sequence with a stop condi-tion. 
                            // At this time the EEPROM enters an internally-timed write cycle, tWR, to thenonvolatile memory. All inputs are disabled during this write cycle and the EEPROM willnot respond until the write is complete (refer to Figure 2).
                            //
                            // PAGE WRITE: The 32K/64K EEPROM is capable of 32-byte page writes.A page write is initiated the same way as a byte write, but the microcontroller does not send a stop condition after the first data word is clocked in. Instead, 
                            // after the EEPROM acknowledges receipt of the first data word, the microcontroller can transmit up to 31 more data words. The EEPROM will respond with a zero after each data word received.The microcontroller must terminate the 
                            // page write sequence with a stop condition (refer to Figure 3).The data word address lower 5 bits are internally incremented following the receipt of each data word. The higher data word address bits are not incremented, retaining 
                            // the memory page row location. When the word address, internally generated, reaches the page boundary, the following byte is placed at the beginning of the same page. If more than 32 data words are transmitted to the EEPROM, the 
                            // data word address will “rollover” and previous data will be overwritten.
                            // 
                            // Endurance: 1 Million Write Cycles –  Data Retention: 100 Years
                            //
                            // so, mine are 24C32: The AT24C32/64 provides 32,768/65,536 bits of serial electrically erasable and pro-grammable read only memory (EEPROM) organized as 4096/8192 words of 8 bitseach. 
                            //
                            // so 4096 bytes, not much, but could be useful as some sort of health check or backup to certain esp data, especially if there's anything I would want to transfer from one esp setup to another.
                            // 
                            //    for example, if i have uint32_t data to store there, only 1024 will fit if i stuff them into every byte.
                            //
                            // for reference, my old xeep is 256 / 8 = 32k bytes, or 8x larger


        Wire.beginTransmission(0x50); 
        Wire.write(address);
        Wire.write(data);
        Wire.endTransmission();    // stop transmitting
        
        
        Wire.beginTransmission(0x50); 
        Wire.send(targetAddress);
        Wire.endTransmission(); 
        Wire.requestFrom(0x50, 1);
        If(Wire.available()){
          byte data = Wire.receive();
        }
        Wire.endTransmission();

        
byte readI2CByte(byte data_addr){
  byte data = NULL;
  Wire.beginTransmission(ADDR);
  Wire.write(data_addr);
  Wire.endTransmission();
  Wire.requestFrom(ADDR, 1); //retrieve 1 returned byte
  delay(1);
  if(Wire.available()){
    data = Wire.read();
  }
  return data;

  

Wire.setClock(clockFrequency)
Parameters

clockFrequency: the value (in Hertz) of desired communication clock. Accepted values are 100000 (standard mode) and 400000 (fast mode). Some processors also support 10000 (low speed mode), 1000000 (fast mode plus)
and 3400000 (high speed mode). Please refer to the specific processor documentation to make sure the desired mode is supported. 
 */


void save_rpi_temp_to_xeep(){

  uint16_t slot = (uint16_t) (1808 + day_of_year); // ok, this saves yesterday's value in today's slot. if i use yesterday it will always be wrong if esp was reset that day, but this should be correct almost all of the time except for the day which does not matter
  
  uint8_t msb = (uint8_t)(slot / 256);

  uint8_t lsb = (uint8_t) (slot & 0x00FF);

  Wire.beginTransmission(0x57); 
  Wire.write(msb); // addr msb
  Wire.write(lsb);
  Wire.write(rpi_today_max);
  Wire.endTransmission();  

  delay(6); // let it finish writing. spec says 5 ms max  

  sprintf(rwb_buf,"save rpi temp %d at slot %d  msb %d  lsb %d",rpi_today_max,slot,msb,lsb);
  add_routine_log(rwb_buf);
  log_routine_time();
}



void save_RS_to_xeep(){ // range is 0x06 to 0x36  == 6 to 54. the little flat antennas are almost always 50, even when i wrapped it in aluminum foil; i don't know what is going on.

  uint16_t slot = (uint16_t) (1440 + day_of_year);
  
  uint8_t msb = (uint8_t)(slot / 256);

  uint8_t lsb = (uint8_t) (slot & 0x00FF);

  Wire.beginTransmission(0x57); 
  Wire.write(msb); // addr msb
  Wire.write(lsb);
  Wire.write(min_xbee_RS);
  Wire.endTransmission();  

  delay(6); // let it finish writing. spec says 5 ms max  

  sprintf(rwb_buf,"save RS %d at slot %d  msb %d  lsb %d",min_xbee_RS,slot,msb,lsb);
  add_routine_log(rwb_buf);
  log_routine_time();
}


void prepare_rpi_temp(){ 

  // rpi_temp for each day of the year including leap year 1808..2175     1808 is day 1   + 365 = 2173 = day 366 same as 1 + 365 = 366
  //
  // eventually either here or in php i need to start with the last data saved and go to the next slot and start there, go to the end, then begin at 1808 and go through the last data saved
  //
  //    save rpi temp 131 at slot 1947 msb 7 lsb 155    <- this is from log on day 139, so that is for day 138: 1808 + 138 = 1946 but that says 1947 - ok, notes above explain that i screwed up and it saves yesterday's value in today's slot
  //
  //    so, here i want to start at 1808 + doy right now + 1 and continue through 2173, then start with 1809 and go through 1808 + doy
  //
  //    wire can do 128 so get 128 + 128 + 110 = 366 values

  uint8_t msb,lsb;

  uint16_t addr1, addr_now, addr2;

  addr1 = 1808;
  addr2 = addr1 + 365; // ignore leap year shit

  addr_now = addr1; // + day_of_year + 1; // i store yesterday's value just after midnight, at this address - 1;

  

  if (addr_now > addr2) {

    addr_now = addr1;
  }

  msb = (uint8_t)(addr_now / 256);

  lsb = (uint8_t) (addr_now & 0x00FF);

  sprintf(rwb_buf,"prepare_rpi_temp; just send the data as is in xeep order starting at %d msb %02X lsb %02X",addr_now,msb,lsb);
  add_log(rwb_buf);

  
  
  uint8_t data[367];
  int cnt = 0;

  //uint32_t milstart = millis();

  Wire.beginTransmission(0x57);
  Wire.write(msb); // addr msb
  Wire.write(lsb);
  Wire.endTransmission();
  
  for (int i=0;i<2;i++){
    
      Wire.requestFrom(0x57, 128);  // wire seems to limit to 128 so get (2 x 128) + 110 = 366

      while (Wire.available() && (cnt < 366)){
        data[cnt] = Wire.read();

        yield();

        cnt++;
      }
  }

  Wire.requestFrom(0x57, 110); 

    while (Wire.available() && (cnt < 366)){
      data[cnt] = Wire.read();
      yield();

      cnt++;
    }
  
  Wire.endTransmission();

  char bb[20];

  sprintf(block,"%02X ",data[0]);
  //add_log(block);

  for (int i=1;i<366;i++){  // block has blocksize bytes, which at this moment is 4400. this uses 3 bytes per data, so 3 x 366 = 1098
    yield();

    sprintf(bb,"%02X ",data[i]);
    //add_log(bb);
    strcat(block,bb);
  }
}



void prepare_xbee_RS(){ 

  // xbee_RS for each day of the year including leap year 1440..1440 + 367 = 1440..1807  days are 1..366 so data starts in 1441 because i'm stupid
 
  uint8_t msb,lsb;

  uint16_t addr1, addr_now, addr2;

  addr1 = 1441;
  addr2 = addr1 + 365; // ignore leap year shit

  addr_now = addr1; // + day_of_year + 1; // i store yesterday's value just after midnight, at this address - 1;

  if (addr_now > addr2) {

    addr_now = addr1;
  }

  msb = (uint8_t)(addr_now / 256);

  lsb = (uint8_t) (addr_now & 0x00FF);

  sprintf(rwb_buf,"prepare_xbee_RS; just send the data as is in xeep order starting at %d msb %02X lsb %02X",addr_now,msb,lsb);
  add_log(rwb_buf);
  
  uint8_t data[367];
  int cnt = 0;

  //uint32_t milstart = millis();

  Wire.beginTransmission(0x57);
  Wire.write(msb); // addr msb
  Wire.write(lsb);
  Wire.endTransmission();
  
  for (int i=0;i<2;i++){
    
      Wire.requestFrom(0x57, 128);  // wire seems to limit to 128 so get (2 x 128) + 110 = 366

      while (Wire.available() && (cnt < 366)){
        data[cnt] = Wire.read();

        yield();

        cnt++;
      }
  }

  Wire.requestFrom(0x57, 110); 

    while (Wire.available() && (cnt < 366)){
      data[cnt] = Wire.read();
      yield();

      cnt++;
    }
  
  Wire.endTransmission();

  char bb[20];

  sprintf(block,"%02X ",data[0]);
  //add_log(block);

  for (int i=1;i<366;i++){  // block has blocksize bytes, which at this moment is 4400. this uses 3 bytes per data, so 3 x 366 = 1098
    yield();

    sprintf(bb,"%02X ",data[i]);
    //add_log(bb);
    strcat(block,bb);
  }
}



void prepare_flow_adc(){ 

  uint8_t data[1440];
  int cnt = 0;

  //uint32_t milstart = millis();

  Wire.beginTransmission(0x57);
  Wire.write(0); // addr msb
  Wire.write(0);
  Wire.endTransmission();
  for (int i=0;i<11;i++){
      Wire.requestFrom(0x57, 128);  // wire seems to limit to 128 so get (11 x 128) + 32 = 1440

      while (Wire.available() && (cnt < 1440)){
        data[cnt] = Wire.read();

        yield();

        cnt++;
      }
  }

  Wire.requestFrom(0x57, 32); 

    while (Wire.available() && (cnt < 1440)){
      data[cnt] = Wire.read();
      yield();

      cnt++;
    }
  
  Wire.endTransmission();

  char bb[20];

  sprintf(block,"%02X ",data[0]);
  //add_log(block);

  for (int i=1;i<1440;i++){ // 1440 x 3 = 4320... x 4 = 5760
    yield();

    sprintf(bb,"%02X ",data[i]);
    //add_log(bb);
    strcat(block,bb);
  }
}



void write_xeep_slot(){

  // two bytes msb and lsb for slot per minute. 24 x 60 = 1440

  uint8_t msb = (uint8_t)(xeep_slot / 256);

  uint8_t lsb = (uint8_t) (xeep_slot & 0x00FF);

  if (flow_adc > 0){
  
      Wire.beginTransmission(0x57); 
      Wire.write(msb); // addr msb
      Wire.write(lsb);
      Wire.write(flow_adc);
      Wire.endTransmission();  
  
      delay(6); // let it finish writing. spec says 5 ms max    
  }   
 }






 void test_write_xeep(){

  // write 0..FF and repeat

  uint8_t val = 0x23;

  //Wire.setClock(100000);
  
  for (byte i=0;i<40;i++){
    //for (int i=0;i<0x0FFF;i++){

    Wire.beginTransmission(0x57); 
    Wire.write(0x04); // addr msb
    Wire.write(i);
    Wire.write(val);
    Wire.endTransmission();  

    yield();
    delay(6); // let it finish writing. spec says 5 ms max

  }

 }
uint8_t read_xeep_byte(uint8_t msb,uint8_t lsb){
  
    uint8_t val = 0;
    //Wire.setClock(100000);


      Wire.beginTransmission(0x57);
      Wire.write(msb); // addr msb
      Wire.write(lsb);
      Wire.endTransmission();
      Wire.requestFrom(0x57, 1); //retrieve 1 returned byte
      //delay(5);
      if(Wire.available()){
        val = Wire.read();
      }
      Wire.endTransmission();

      return val;
  
     
    
}

void read_1440_bytes(){

  uint8_t data[1440];
  int cnt = 0;

  uint32_t milstart = millis();

      Wire.beginTransmission(0x57);
      Wire.write(0); // addr msb
      Wire.write(0);
      Wire.endTransmission();
      for (int i=0;i<11;i++){
          Wire.requestFrom(0x57, 128);  // when i used 1440 it consistenly read 128 in 13 msec, so i changed to 11 x 128 + 32 more. pdf says eep pages are 32 bytes so that's not the issue. looks like wire limitation
                                        // this setup reads 1440 in 139 msec, which is fine if it actually steps through properly. i need to verify.
          //delay(5);
    
          while (Wire.available() && (cnt < 1440)){
            data[cnt] = Wire.read();
            yield();
    
            cnt++;
          }
      }

      Wire.requestFrom(0x57, 32); 
        //delay(5);
  
        while (Wire.available() && (cnt < 1440)){
          data[cnt] = Wire.read();
          yield();
  
          cnt++;
        }
      
      Wire.endTransmission();

      sprintf(rwb_buf,"read %d in %lu msec",cnt,millis() -  milstart);
      add_log(rwb_buf);

      // so far today it has written a bunch up to 0x0392

      add_log((char *) "dump 0x0375...");

      sprintf(rwb_buf,"%02X ",data[0x0375]);

      char bb[4];
      
      for (int i=0; i<60;i++){
        sprintf(bb,"%02X ",data[0x0375 + i]);
        strcat(rwb_buf,bb);
      }

      add_log(rwb_buf);



      add_log((char *) "dump 0x0000...");

      sprintf(rwb_buf,"%02X ",data[0x0000]);
      
      for (int i=0; i<60;i++){
        sprintf(bb,"%02X ",data[0x0000 + i]);
        strcat(rwb_buf,bb);
      }

      add_log(rwb_buf);

      


      add_log((char *) "dump 0x03F0...");

      sprintf(rwb_buf,"%02X ",data[0x03F0]);
      
      for (int i=0; i<60;i++){
        sprintf(bb,"%02X ",data[0x03F0 + i]);
        strcat(rwb_buf,bb);
      }

      add_log(rwb_buf);

  
}

  void test_read_xeep(byte msb){

    uint8_t val = 0;
    //Wire.setClock(100000);
    for(byte i=0;i<6;i++){

            
  //    byte readI2CByte(byte data_addr){
    //    byte data = NULL;
        Wire.beginTransmission(0x57);
        Wire.write(msb); // addr msb
        Wire.write(i);
        Wire.endTransmission();
        Wire.requestFrom(0x57, 1); //retrieve 1 returned byte
        delay(5);
        if(Wire.available()){
          val = Wire.read();
        }
        Wire.endTransmission();
  
            /*
      Wire.beginTransmission(0x57); 
      Wire.write(1); // new name is write...Wire.send(i);
      Wire.endTransmission(); 
      Wire.requestFrom(0x57, 1);
      if(Wire.available()){
        val = Wire.read(); // new name is read... Wire.receive();
      }
      Wire.endTransmission();
*/
      sprintf(rwb_buf,"ad %02X %02X",i,val);

      add_log(rwb_buf);
    }
  }
