

void dream_died(){

  if (dream_dead == 0){ // ideally this routine should not be called if already dead, but it happened a lot when biz was flakey on-off-on over and over during a pulse train. this shouldn't hurt?
  
    dream_dead = my_epoch - 120; // move it back a minute or two to encompass recent events that need to be block sent later
    
    save_a_uint32_t((char *) "/dream_status",dream_dead);
  
    dream_new_ping_cnt = 120; // 3-22-2021 this was 1, which causes it to send all every second
  
    add_bad_news((char *) "dream died");  
  }
}



void dream_reborn(){

  dream_dead = 0; 
  
  save_a_uint32_t((char *) "/dream_status",dream_dead);

  add_bad_news((char *) "dream_born");
}




 

// rpi temperature last pkt 3  today max 141 min 138  yesterday max 140 min 136 max all time 143  last rpi ack seconds 122 
// 0D 66 7E AA 01 04 4C 4E    03    2E 00 D0 00 00 48

void process_rpi_temperature(uint8_t rpi){
  /*
        the issue is that rpi clock is not maintained, so rpi can reset the value it sends to master at any time during the day. worst case it happens in the hot afternoon, so right away it gets a high value and
        sends that to the master for 24 hours. ok, that actually only ruins the minimum value. in that worst case the rpi seems to send a zero once and then sends the true value alternating with 3

        however, no matter what time of day the rpi resets its value, the esp clock will be synced to the sun, and the high value in any 24 hours of esp time will still be the high rpi value for that period.

        i think i got confused because the rpi clock was off so far that the max never seemed to change even on a cool morning, which makes sense now.

        keep in mind that the rpi = 0 packet is a good way to see rpi clock drift; ideally it happens near midnight
   */


  uint32_t todays_secs_now = todays_seconds();

// it doesn't seem to send 0 every midnight. maybe master avr reset and sent the zero? keep this code, or at least check for zero and return so it's not the new minimum
  if (rpi == 0){
    
    return; // i'm not sure of this, but rpi might send zero once after it resets the value each "midnight" which can happen any time of the day as the rpi clock drifts, which apparently is pretty bad. i should change
            // the rpi code to use the master clock for midnight if it's not too much work.
  }

  

  if (rpi != last_rpi_not_3){   // this section logs whenever the actual rpi temperature changes

    if (rpi != 3){
      
      sprintf(rwb_buf,"r %d %d",rpi,last_rpi_not_3); 
      add_log(rwb_buf);
      log_time();

    }
  }
  
  rpi_temperature = rpi;

  // in this section, rpi_ack_secs will always be calculated. its purpose is to show me when rpi has crashed, so all this code has to do is show a large number if the value has stopped changing.
  //
  // if it's stuck on 3 then ack secs will grow because it's referenced to norm secs.
  //
  // if it's stuck on an actual temperature, then ack secs will grow until a 3 comes in

  // and i was confused when i thought the value should grow all the time; in that case it would be huge when 0x15 packets don't arrive between pulse blocks, which would be wrong. BUT THIS IS IN NORMAL PACKETS RETARD, which
  // doesn't matter and the point still stands, that the code is correct as is. if no packets arrive that's another issue. if packets are arriving normally and rpi is dead, the value will grow as it should

  if (rpi == 3){

    rpi_3_secs = todays_secs_now;

    if (todays_secs_now < rpi_norm_secs){ // midnight. if rpi has been dead for days I'm not sure what happens here
    //if ((todays_secs_now < 40000) && (rpi_norm_secs > 40000)){ // midnight. if rpi has been dead for days I'm not sure what happens here

      rpi_ack_secs = (86400 + todays_secs_now) - rpi_norm_secs; // midnight happened
      
    } else { // not midnight
    
      rpi_ack_secs = todays_secs_now - rpi_norm_secs;
    }
  
  } else {

    
    if (rpi < last_rpi_not_3){
  
      sprintf(rwb_buf,"rpi midnite? old %d new %d",last_rpi_not_3,rpi);
      add_routine_log(rwb_buf); 
      log_routine_time();

      rpi_today_min = rpi;
    }

    last_rpi_not_3 = rpi;
    /*
          3-27-2021 i want to detect rpi midnight when it resets the temperature it sends to avr, assuming rpi does that and not the avr, which i need to confirm. yes, rpi resets value to zero at midnight, and sends max value all day long. the other
          rpi files pipe_calls and spi_calls seem to relay the temperature, first with 0xA8 from python to C, then 0xD5 from C to avr, and avr stores it in float_one_s which looks like avr simply relays it.

          at esp midnight i reset rpi min to 255 so that's easy to catch but proves nothing. what i need to catch is when the value from avr/rpi drops. compare to last_rpi_not 3?
     */


    if (rpi < rpi_today_min){
  
      rpi_today_min = rpi;
    }
  
    if (rpi > rpi_today_max){
  
      rpi_today_max = rpi;
      
    }
    
    if (rpi > rpi_max){
  
      rpi_max = rpi;
    }
    
    rpi_norm_secs = todays_secs_now;

    if (todays_secs_now < rpi_3_secs){ // midnight. if rpi has been dead for days I'm not sure what happens here
      
    //if ((todays_secs_now < 40000) && (rpi_3_secs > 40000)){ // midnight. if rpi has been dead for days I'm not sure what happens here

      rpi_ack_secs = (86400 + todays_secs_now) - rpi_3_secs; // midnight happened
      
    } else { // not midnight
    
      rpi_ack_secs = todays_secs_now - rpi_3_secs;
    }
  //  rpi_ack_secs = todays_secs_now - rpi_3_secs;
  }

  if (first_rpi != 1){
    if ((rpi_ack_secs >= 500) && (bad_rpi_ack <= 500)){ // this is to detect dead rpi. smaller values fill the log with another issue, missed packets which can be dealt with elsewhere
      
      sprintf(rwb_buf,"large rpi ack %d",rpi_ack_secs);
      add_bad_news(rwb_buf);
    } 
  }

  bad_rpi_ack = rpi_ack_secs; // so if it stays bad, this will never be < 130. if it starts acting normally, this will allow another log entry. it will only suck if it is bad then good over and over

  first_rpi = 0; // don't log the two apparent errors on first packet
}
                




void show_status_bits(String & ptr){  // this is only called when computer page is loaded. keep it separate from the other routine that sends changes to web, and calculates trans_plus_flow to determine when to ignore tank level jitter


  // 0x01     0x02        0x04        0x08      0x10    0x20    0x40      0x80
  // 0 na     1 flow12    2 flow34    3 float   4 na    5 na    6 trans   7 na
  // per htmlbirchwood .php trans2 is flow12, 
  //
  // per mk_new_expanded_fake.php trans1 0x40   float 0x08    flow12=trans2 0x02    flow34 0x04   master 0x01   slave 0x10

  char buf[100];

  //uint8_t old_bit; //= old_status & 0x08;
  uint8_t new_bit = current_status & 0x08;

  
  if (new_bit){
    sprintf(buf," float ON ...");
    
  } else {
    sprintf(buf," float OFF ...");
  }
  ptr += buf;

  //old_bit = old_status & 0x04;
  new_bit = current_status & 0x04;
  
  
  if (new_bit){
    sprintf(buf," flow ON ...");

    //trans_plus_flow++;

  } else {
    sprintf(buf," flow OFF ...");

  }
  ptr += buf;


  
//  old_bit = old_status & 0x040;
  new_bit = current_status & 0x40;
  
  if (new_bit){
    sprintf(buf," trans1 ON ...");
    //trans_plus_flow++;

  } else {
    sprintf(buf," trans1 OFF ...");

  }
  ptr += buf;
 



  
  //old_bit = old_status & 0x02;
  new_bit = current_status & 0x02;
  

  if (new_bit){

    
    sprintf(buf," trans2 ON");
    
  
    
  } else {
    sprintf(buf," trans2 OFF");

  }

  ptr += buf;

  
}


void process_status_bits(){   // this is called on every packet. keep it separate from the other similar routine that is called to display my computer page


  // 0x01     0x02        0x04        0x08      0x10    0x20    0x40      0x80
  // 0 na     1 flow12    2 flow34    3 float   4 na    5 na    6 trans   7 na
  // per htmlbirchwood .php trans2 is flow12, 
  //
  // per mk_new_expanded_fake.php trans1 0x40   float 0x08    flow12=trans2 0x02    flow34 0x04   master 0x01   slave 0x10

  //char buf[100];

  uint8_t old_bit = old_status & 0x08;
  uint8_t new_bit = current_status & 0x08;

  trans_plus_flow = 0;
    
  
  if (new_bit){
    trans_plus_flow++;
    if (!old_bit){

      //fast_log_0xAA(); 

//      add_fast_log_0xAA();    // if > 0, this logs the count and resets it
      
      do_float_change(1);

      

//      new_pulse_block = 1;
/*      
      if (xbee_RS_state == 1){

        xbee_RS_state = 2;  // see notes in uart get_RS()
      }
*/
      
    }
    
  } else {
    
    if (old_bit){

      do_float_change(0);

      if (master_rtc_state == 1){

        master_rtc_state = 2; // waiting for 0xAA. 2-10-22, looks like state == 2 means float is off, but this state only lasts until next 0xAA packet

        add_log((char *) "master rtc state is 2, waiting for next 0xAA");
        log_time();
      }
/*
      if (xbee_RS_state == 2){

        xbee_RS_state = 3;  // see notes in uart get_RS()
      }
*/      
    }
  }

  old_bit = old_status & 0x04;
  new_bit = current_status & 0x04;
  
  
  if (new_bit){
    trans_plus_flow++;
    if (!old_bit){

      do_flow_change(1);
      
    }
  } else {
    
    if (old_bit){

      do_flow_change(0);
      
    }
  }
  
  old_bit = old_status & 0x040;
  new_bit = current_status & 0x40;
  
  if (new_bit){
    trans_plus_flow++;
    if (!old_bit){

      do_trans1_change(1);
      
      
    }
  } else {
    
    if (old_bit){

      do_trans1_change(0);
      
    }
  }
  
  old_bit = old_status & 0x02;
  new_bit = current_status & 0x02;
  

  if (new_bit){
    trans_plus_flow++;
    if (!old_bit){

      do_trans2_change(1);
      
      
    }
    
  } else {
    
    if (old_bit){

      do_trans2_change(0);
      
    }
  }
}

void try_ping(int hostnum){

  const char * host = dream_host;

  if (hostnum == dream_id){ // dream

    if(dream_dead != 0){  // 0 is good. if it's dead, this is the epoch when it died
      return;
    }

    host = dream_host;
  
  }



  http_ack = ping_the_web(host); // if the ack was good, it cleared ping_web. BUT REMEMBER if online is not enabled this will return 0, which will mean biz died or dream died, so don't do this if in_charge_online is not 1
  
/*      5-14-2021 i deleted all the re-sends at half second intervals because biz somehow seemed to stack all three up and wait 15 seconds to process them, and it wrecked the tank_level.txt file, so just send one and if the ack is bad, fall back on block send
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = ping_the_web(host);
  }
  
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = ping_the_web(host);
  }
*/
  document_ack(http_ack, hostnum);
  
  if (http_ack != 1){

    //http_fail = 1;

    if (hostnum == dream_id){ 

      secs_since_bad_ping_dream = 0; //, good_pings_today,bad_pings_today
      bad_pings_today_dream++;
        
      dream_died();     
      /*
    } else { // biz
      
      secs_since_bad_ping_biz = 0; //, good_pings_today,bad_pings_today
      bad_pings_today_biz++;
        
      biz_died();   
      */
    }
  } else { // good ping
    
    if (hostnum == dream_id){ 
   
        ping_web_dream = 0;
        secs_since_good_ping_dream = 0; //, secs_since_bad_ping, good_pings_today,bad_pings_today
        good_pings_today_dream++; //,bad_pings_today
     
      /*
    } else { // biz
        ping_web_biz = 0;
        secs_since_good_ping_biz = 0; //, secs_since_bad_ping, good_pings_today,bad_pings_today
        good_pings_today_biz++; //,bad_pings_today
     */
    }
     
  }
}

void delay_half_second(){

  for (int i=0;i<10;i++){

    delay(50);
    yield();
  }
}


int try_gallons(int hostnum){

  const char * host = dream_host;

 if (hostnum == dream_id){ // dream

    host = dream_host;

    if (try_gallons_dream_already_acked == 1){

      return 1;
    }
  /*
  } else { // biz

    host = biz_host;
    
    if (try_gallons_biz_already_acked == 1){

      return 1;
    }
    */
  }

  //delay_half_second();

  http_ack = send_gallons(host);

  if (http_ack == 1){

    if (hostnum == dream_id){

      try_gallons_dream_already_acked = 1;
//    } else {

  //    try_gallons_biz_already_acked = 1;
    }
  }
  
/*      5-14-2021 i deleted all the re-sends at half second intervals because biz somehow seemed to stack all three up and wait 15 seconds to process them, and it wrecked the tank_level.txt file, so just send one and if the ack is bad, fall back on block send  
  if (http_ack != 1){

    delay_half_second();
    http_ack = send_gallons(host);
  }
  
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = send_gallons(host);
  }
*/
  return handle_ack(http_ack,hostnum);
}


int handle_ack(int http_ack,int hostnum){

  document_ack(http_ack, hostnum);
  
  if (http_ack != 1){

    //http_fail = 1;

    if (hostnum == dream_id){ 
//add_bad_news((char *) "handle ack");
      dream_died();     
      
//    } else { // biz

  //    biz_died();   
    }

    return 0;
  } else {

    if (hostnum == dream_id){ 

      ping_web_dream = 0;

//    } else { // biz

  //    ping_web_biz = 0;
    }

    return 1;
    
  }
}

void try_a_change(int val,const char * chg_url,int hostnum){

  const char * host = dream_host;

  if (hostnum == dream_id){ // dream

    if(dream_dead != 0){  // 0 is good. if it's dead, this is the epoch when it died
      return;
    }

    host = dream_host;
 
  }


 // delay_half_second();
    

  http_ack = send_change(val,chg_url,host);

  sprintf(rwb_buf,"try a change ack %d",http_ack);
  add_fast_log(rwb_buf);
  
/*      5-14-2021 i deleted all the re-sends at half second intervals because biz somehow seemed to stack all three up and wait 15 seconds to process them, and it wrecked the tank_level.txt file, so just send one and if the ack is bad, fall back on block send
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = send_change(val,chg_url,host);
  }
  
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = send_change(val,chg_url,host);
  }

  */

  handle_ack(http_ack,hostnum);
}


void do_trans1_change(int val){

  write_type_3("/trans_1a",val);      // file type_number 3    state is 1 or 0

  tr1_writes++;

  save_a_uint32_t((char *) "/tr1_wr",tr1_writes);

  if (in_charge_online == 1){
  
    add_fast_log((char *) "try a change - trans1");
  
    try_a_change(val,trans1_chg,dream_id); // try_a_change delays(100) before sending
          
    //try_a_change(val,trans1_chg,biz_id); // try_a_change delays(100) before sending
  }

}

void do_trans2_change(int val){

  write_type_3("/trans_2a",val);      // file type_number 3    state is 1 or 0
  
  tr2_writes++;

  save_a_uint32_t((char *) "/tr2_wr",tr2_writes);

  if (in_charge_online == 1){
  
    add_fast_log((char *) "try a change - trans2");
  
    try_a_change(val,trans2_chg,dream_id);  // try_a_change delays(100) before sending
      
    //try_a_change(val,trans2_chg,biz_id); // try_a_change delays(100) before sending
  }

}

void do_flow_change(int val){
  
  write_type_3("/flowa",val);      // file type_number 3    state is 1 or 0;

  flow_writes++;

  save_a_uint32_t((char *) "/flow_wr",flow_writes);

  if (in_charge_online == 1){
  
    add_fast_log((char *) "try a change - flow");
    
    try_a_change(val,flow_chg,dream_id);  // try_a_change delays(100) before sending
      
    //try_a_change(val,flow_chg,biz_id); // try_a_change delays(100) before sending
  }

}

void send_all_blocks(int hostnum){

  // host has been dead and seems to be back online
  //
  // send blocks as long as ack is good
  //
  // don't set host as alive until all blocks are successful



  sprintf(rwb_buf,"send all"); //ream_is_dead);
  add_log(rwb_buf);
  log_time();

    uint32_t test_epoch = 0;
    
    if (hostnum == dream_id){

      test_epoch = dream_dead;
    }


    if (prepare_block("/trans_1a",1,test_epoch) > 0){
  
     if (try_a_block(trans1_blk,hostnum) != 1){
  
      return;
     }
    }

   delay_half_second();



    air_block = 1; // multiple values by 4 to go back to 10 bit adc   
    if (prepare_block("/air_new",2,test_epoch) > 0){
  
     if (try_a_block(air_blk,hostnum) != 1){
  
      return;
     }
    }
    air_block = 0;

    

   delay_half_second();

   if (prepare_block("/trans_2a",1,test_epoch) > 0){
  
     if (try_a_block(trans2_blk,hostnum) != 1){
  
      return;
     }
   }
   

   delay_half_second();

   if (prepare_block("/flowa",1,test_epoch) > 0){
  
     if (try_a_block(flow_blk,hostnum) != 1){
  
      return;
     }
   }
   
   delay_half_second();

   if (prepare_block("/float",1,test_epoch) > 0){
  
     if (try_a_block(float_blk,hostnum) != 1){
  
      return;
     }
   }
   
   delay_half_second();

   if (prepare_block("/tank3_new",2,test_epoch) > 0){
  
     if (try_a_block(tank_blk,hostnum) != 1){
  
      return;
     }
   }


   delay_half_second();

   if (prepare_block("/pulse",0,test_epoch) > 0){
  
     if (try_a_block(pulse_blk,hostnum) != 1){
  
      return;
     }
   }
   
   
  if (try_gallons(hostnum) != 1){ // this calls handle_ack when it's done
    
    add_log((char *) "try gallons fail?");
    
    return;
  }

  delay_half_second();

  // if we get here, everything worked and the host is ok
  
  if (hostnum == dream_id){ // dream

    dream_reborn();
      
    return;
    
  }
  
}



int try_a_block(const char * blk_url,int hostnum){
  
  const char * host = dream_host;

  if (hostnum == dream_id){ // dream

// this has to be tried while i still have the death epoch to know what records to send


//    if(dream_dead != 0){  // 0 is good. if it's dead, this is the epoch when it died
//      return 0;
//    }

    host = dream_host;
  
  }

  

  http_ack = send_block(blk_url,host);

  sprintf(rwb_buf,"try a block ack %d",http_ack);
  add_fast_log(rwb_buf);
  
/*      5-14-2021 i deleted all the re-sends at half second intervals because biz somehow seemed to stack all three up and wait 15 seconds to process them, and it wrecked the tank_level.txt file, so just send one and if the ack is bad, fall back on block send
  if (http_ack != 1){


    delay_half_second();
    
    http_ack = send_block(blk_url,host);
  }
  
  if (http_ack != 1){

    delay_half_second();
    
    http_ack = send_block(blk_url,host);
  }
*/
  document_ack(http_ack,hostnum);
//add_bad_news((char *) "try a block");
  return handle_ack(http_ack,hostnum);
}


void document_ack(int ack, int hostnum){


  if (hostnum == dream_id){
    if (ack == 1){
      last_good_dream_epoch = my_epoch;
      
    } else {
      last_bad_dream_epoch = my_epoch;
      bad_dream_cnt++;
    }
    /*
  } else {
    if (ack == 1){
      last_good_biz_epoch = my_epoch;
      
    } else {
      last_bad_biz_epoch = my_epoch;
      bad_biz_cnt++;
    }
    */
    
  }
}




void do_float_change(int val){
  
  write_type_3("/float",val);      // file type_number 4    state is 1 or 0;
  
  float_writes++;

  save_a_uint32_t((char *) "/float_wr",float_writes);

  if (in_charge_online == 1){
    
    add_fast_log((char *) "try a change - float");
    
    try_a_change(val,float_chg,dream_id);  // try_a_change delays(100) before sending
      
  //  try_a_change(val,float_chg,biz_id); // try_a_change delays(100) before sending
  }
  
}

void process_tank(){
  
  if (trans_plus_flow == 0){ // ignore jitter

    if (current_tank > last_tank_write){

      if ((current_tank - last_tank_write) < 3){    // when this was 4 the plot was ok, but was quite a bit different from old style. when it was 2 the plot sucked occasionally, lots of rapid up-down shit
        return;
      }
      
    }else {
  
      if ((last_tank_write - current_tank) < 3){
        return;
      }
    }
  }
    
  
    if (current_tank != last_tank_write){    // old_tank is read on boot and is set on each packet before getting new value from packet, in the uart code
  
      if ((my_epoch - last_tank_epoch) > 300){    // five minutes = 300 secs
  
        write_tank_new_style();
      
        last_tank_write = current_tank;       
              
        last_tank_epoch = my_epoch; 
      }
      

      if (in_charge_online == 1){
  
        add_fast_log((char *) "try a change - tank level");
        
        try_a_change(current_tank,tank_chg,dream_id);       // try_a_change delays(100) before sending 
    //    try_a_change(current_tank,tank_chg,biz_id);       // try_a_change delays(100) before sending
      }
    }
}



void process_air(uint8_t msb, uint8_t lsb){    
 
    current_air = (msb * 256 + lsb) / 4; // convert 10 bit to 8 bit. convert back to 10 bit f
  
    if (current_air != last_air_write){ // old_air){   

      if ((my_epoch - last_air_epoch) > 300){    // five minutes = 300 secs
  
        write_air_new_style();
      
        last_air_write = current_air;

        last_air_epoch = my_epoch; 
      }

      if (in_charge_online == 1){
  
        add_fast_log((char *) "try a change - air level");
        
        try_a_change(current_air * 4,air_chg,dream_id);       // try_a_change delays(100) before sending 
      //  try_a_change(current_air * 4,air_chg,biz_id);       // try_a_change delays(100) before sending
      }
    }
}




void process_0x15_pulse(){
  
/*    
      0x15 bytes
      
         start bytes
      
         11d data bytes not including ck
      
      1   0 obsolete
      
      2   reeiver which is also obsolete, right
      
      3   0x15
      
      4   pulse meter cnt msb
      5   another
      6   lsb
      
      7   msb secs since mid
      8   another
      9   lsb
      
      10   reset flags    bit 0 power on    bit 1 external reset pin    bit 2 brownout    bit 3 watchdog
      11   reset cnt
      
         ck  


   discussion of master reset in middle of pulse block:

    4-28-2021 pulse_master.asm looks at the line on reset, and ignores the state until it changes, so master resets should never send dupe pulses with different pulse cnt and/or different seconds since midnight.

      master seconds comes from master rtc so it should be ok for new pulses after the reset.

*/


  if (in_charge_0x15 == 1){   // 11-10-2021 this used to be after the repeat packet test so it only acked once. I moved it here because it seemed to not ack reliably. I also added the delay.

    if (in_charge_0xAA != 1){       // keep this delay for mega8535 but try deleting it for bench test with nano, hardwired rx tx. mega never had AB ack, so in charge AA should never be true for mega
       
       delay(200); // added 11-10-2021. too soon to be sure but it looks like this fixed the weirdness wherea pile of bytes was appended to the 0x16 packet
                   //
                   // also, did this help the master wdr reset issue? master reset count was 36 when i made this change in mid november 2021                
                   //
                   // 3-9-2024 bench test nano master. delete this delay and the AB ack delay for nano, but if this esp is acking AB that should mean nano is master, so, use that to 
                   // continue adding this delay if it seems that master is old box mega8535
    }

    ack_0x16_to_0x15();
  }
  
  uint32_t new_master_seconds = (uint32_t) uart_buff[9] + 256 * ((uint32_t) uart_buff[8] + 256 * (uint32_t) uart_buff[7]);

  if (new_master_seconds == master_seconds){

    return; // this is a repeat packet
  }  

  
/*    
    3-26-2021 this code has not been tested, so verify it with something like 50 gpm before using it.

    I decided to turn this block off because it almost never happens, and if jay gets a strong pump this code will suck. remember that the only thing this code blocks is if master resets while pulse line is high. if that's a thing then i need to 
    change master to save pulse state in rtc nvm and restore it after reset.

  // if this 0x15 would result in gpm > 80 just ignore it and assume it's noise or the master reset while the pulse line was high. in any case this "master_seconds" is considered to be bogus so stop here if gpm > 80, meaning seconds since
  // last 0x15 is < 75

  if ((my_epoch - pulse_epoch) < 75){

    float gpm22 = 6000.0 / ((float) (my_epoch - pulse_epoch));

    sprintf(rwb_buf,"pulse implies GPM %.2f. bad data or new pump?",gpm22);
    add_bad_news(rwb_buf);
    add_bad_news((char *) "remember that GPM > 80 is rejected");

    return;
  }
*/
  uint32_t todays_seconds_now = todays_seconds();  

  /*
   
        6-5-2021 one sorta simple way to avoid confusing android is to never set master clock between midnight and 2 am. 

            it also might make sense to avoid the two dst change days altogether if esp in in charge of websites. maybe that should be a rule: either android controls master and web sites, or esp does. i would need to
            change the android app to be able to tell it to leave master clock alone
            
        6-3-2021 handle master rtc and deal with dst

          as long as android tablet might ever be involved, master rtc has to use dst. esp and php will never use dst except for a simple flag to shift the legend in php (i think. if anything, it's not much more complex than this)

          so, if esp is in charge of master rtc, it will have to correct drift and also handle dst

          >>> don't try to do the 2 am bullshit; if android is involved and might go crazy on the web, let android also handle master rtc <<<

          so, this is for the scenario where esp handles master rtc and the web, but android might take over later on, so correcting for dst at midnight in the spring and in the fall will be ok, i think

          when dst is active, spring ahead, so master seconds since midnight will be mine + 3600 except for the last hour in the day, when my seconds will be near 86400 and master will be less than about one hour, 3600

          so do this:

            first, make functions to do the messy math that always pertains:

              uint32_t time_diff33(t1,t2){ // t1 > t2

                  if (t1 > 82000) and (t2 < 4000){   // don't use values close to 43200 because both t1 and t2 could straddle that at midday. this is for the situation where one is past midnight but the other is not
  
                    add 86400 to t2

                    if (t1 > t2){
  
                      diff = t1 - t2
                    
                    } else {

                      diff = t2 - t1;

                    }
                  
                  } else {
  
                    diff = t1 - t2 
                  }

                  return diff;
              }

              uint32_t time_diff(t1,t2){

                if (t1 > t2){

                  diff = time_diff33(t1,t2);

                } else { // t2 > t1

                  diff = time_diff33(t2,t1);

                }

                return diff
              }

              if dst is on {
  
                me += 3600
              }
  
              diff = time_diff(me,master); // order does not matter here. nor does it matter if one or the other is past midnight. this will return the number of seconds difference in sane terms
  
              if diff > too_much
  
                // esp will send time directly to master in terms of hours minutes seconds etc. so if hour < 11pm add an hour and send the mess. otherwise hold off.

              look at packet local box sends to master. see rx1284.asm

                i moved this discussion to xbee_send - send_master_rtc_packet()

              

            

               

   */

  master_seconds = new_master_seconds;
    
  save_a_uint32_t((char *) "/m_secs",master_seconds);

  if (todays_seconds_now > master_seconds){             // this can be confusing when dst is on; during most of the day it will display about 3600, but if the most recent pulse was between midnight and 1 am dst time (master time), it will show 86400 - 3600
                                                        //
                                                        // that situation is handled below by simply skipping some hours close to midnight. to fix this so that the info screen does not confuse me, if err > 82800 then err = 86400 - err. but, i also
                                                        // have to allow for clock difference between master rtc and esp clock, which should not be more than a few seconds, but to be safe allow 5 minutes: 86400 - one hour - five minutes = 82500

    master_err = todays_seconds_now - master_seconds;
    
  } else {

    master_err = master_seconds - todays_seconds_now;
  }

  if (master_err > 82500) { // pulse was between midnight and 1 am master time when dst is active, so fix the confusing value shown on info screen

    master_err = 86400 - master_err;
  }

  if (in_charge_rtc == 1){

    if ((todays_seconds() > 7200) && (todays_seconds() < 82000)){ // avoid first two hours so dst is not an issue because of the 2 am thing, and also avoid the last hour so i don't have to deal with master dst already in next day
      
      if (master_rtc_state == 0){
        
        // master rtc states 0:waiting    1:needs to be fixed, wait for float to turn off   2:float turned off, wait for 0xAA packet    3..14 or whatever:got 0xAA packet, wait 10 seconds    14 or whatever, try to set master rtc, go to state 0
  
        uint32_t actual_master_err;
  
        uint32_t fix_me = todays_seconds_now;
  
  /* 9-27-2021 I stopped using android tablet so let master rtc ignore dst, which will also eliminate the plot glitches on dst changes. If I need to use android along with esp I need to restore this block of code   
   
        if (is_dst_on_now() == 1){
  
          fix_me += 3600;
        }
  */
        if (fix_me > master_seconds){
  
            actual_master_err = fix_me - master_seconds;
           
        } else {
            actual_master_err = master_seconds - fix_me;
        }

        sprintf(rwb_buf,"m6666aster clock error %d  today secs (+dst if needed) %d master secs %d",actual_master_err,fix_me,master_seconds);
          add_fast_log(rwb_buf);
  
        if (actual_master_err > 20){
        //if (actual_master_err > 2){
  
          master_rtc_state = 1; // wait for float to turn off

          sprintf(rwb_buf,"master clock error %d  today secs (+dst if needed) %d master secs %d",actual_master_err,fix_me,master_seconds);
          add_log(rwb_buf);
  
          add_log((char *) "master rtc state is 1, waiting for float to turn off");
          log_time();
        }
      }
    }
  }

  uint32_t temp_pulse_cnt = (uint32_t) uart_buff[6] + 256 * ((uint32_t) uart_buff[5] + 256 * (uint32_t) uart_buff[4]);
  
  if (temp_pulse_cnt < master_pulse_cnt){ // == 1){ // master reset

    offset_cnt = offset_cnt + master_pulse_cnt;

    save_a_uint32_t((char *) "/offset_cnt",offset_cnt);
  }

  master_pulse_cnt = temp_pulse_cnt;
  
  save_a_uint32_t((char *) "/today_cnt",master_pulse_cnt);
  
  if (yesterday_master_cnt == 0){ // this is for new devices; don't delete this.

    yesterday_master_cnt = master_pulse_cnt; // on the first day this will give a low value instead of an enormously incorrect one
    
    save_a_uint32_t((char *) "/yday_cnt",yesterday_master_cnt);
  }

  gallons_per_master_cnt = master_pulse_cnt + offset_cnt - yesterday_master_cnt;

  uint8_t old_master_resets = master_resets; // it's just one byte, so uint8
  
  master_resets = uart_buff[11];

  if (old_master_resets != master_resets){

    save_a_uint32_t((char *) "/m_resets",(uint32_t) master_resets);

    sprintf(rwb_buf,"master resets old %d new %d",old_master_resets, master_resets);
    add_bad_news(rwb_buf);

    // 10   reset flags reset_flags    bit 0 power on    bit 1 external reset pin    bit 2 brownout    bit 3 watchdog


    if (uart_buff[10] & 0x01) {

      add_bad_news((char *) "power on reset");
    }
    
    if (uart_buff[10] & 0x02) {

      add_bad_news((char *) "external reset");
    }
    
    if (uart_buff[10] & 0x04) {

      add_bad_news((char *) "brownout reset");
    }
    
    if (uart_buff[10] & 0x08) {

      add_bad_news((char *) "watchdog reset");
    }

    //log_replay(); // log the timestamps and types for the last few packets before master reset

  }

  write_pulse_new_style((uint32_t)my_epoch);  // esp uses my_epoch to save pulses. esp only uses master rtc to detect dupe pulse packets. so, if esp is in charge and misses 0x15 packets it will affect gpm accuracy. i do this because it's too much trouble
                                    // to use master seconds since midnight and try to sync to any epoch. maybe that's not true, but i think i tried several approaches and settled on this.

  if (in_charge_online == 1){
    
    bool save_verbose = verbose1; // don't flood log with normal pulses
  
    verbose1 = false;
    
  
    if (prepare_block("/pulse",0,my_epoch - 600) > 0){  // 300 seconds means send all pulses from the last five minutes, which should be less than three. 4-26-2021 300 seconds = 5 minutes = 20 gpm. it's rare, but gpm can fall below this, 
                                                        // so send all from last 10 minutes. it only sends epoch for the first entry and after that it sends delta, and it's 2021 so stop fretting about tiny data packets moron.

       add_fast_log((char *) "try a block - pulses");
       
       try_a_block(pulse_blk,dream_id);
  
//       try_a_block(pulse_blk,biz_id);
  
    }

    verbose1 = save_verbose;
  }  
}

/*

void log_replay(){
  
    // replay_index points just beyond this packet, so log all the others in the list except this one. example, max = 5 and index = 3. this packet is in slot 2, so the preceding packets are in 1 (most recent), 0, 4, and 3 (oldest)
    // so i want to log 3 4 0 1, meaning start at current index and log max - 1 entries.

    int ri = replay_index;
    
    struct tm * begn; // = gmtime ((time_t *) &my_epoch);
    struct tm * endd;
  
    add_bad_news((char *) "replay packets before master reset");

    for (int i=0; i< (replay_max - 1); i++){
      
       begn = gmtime ((time_t *) &replay_begin[ri]);
       endd = gmtime ((time_t *) &replay_end[ri]);

       sprintf(rwb_buf,"replay %02X begin %02d:%02d:%02d   end %02d:%02d:%02d",replay_type[ri],begn->tm_hour,begn->tm_min,begn->tm_sec,endd->tm_hour,endd->tm_min,endd->tm_sec);

       add_bad_news2(rwb_buf); // add_bad_news2 won't timestamp all these entries

       ri++;

       if (ri >= replay_max){

        ri = 0;
       }
    }

    add_bad_news((char *) "end of replay");
}

*/

void count_gallons(){

  // on every pulse increment gallons and save to a file
  // on every reset load that file
  // assume pulse file writes applies to this one too
  // reset the count at midnight
  // send url at midnight. re-send every 5 minutes or whatever until ack unless web is dead then send daily?
  // in another file, keep the same format as php keeps and prune it every 40 days or whatever, so if i want to i can cut and paste it into the online file after printing it in show_file button

  

  
}
