// this file, clock, is for ntp related things. rtc related is in rtc file. (6-17-2021 ntp is no longer needed. code is in archive esp_server_4_1_qqq
//
// all the ntp web site returns is one number, the epoch, and the client calculates day of week, hour, minute, and second. 
/*
    
  2-26-2021 revised clock policy

    it's ok to poll ntp every minute. ubuntu checks every 32 seconds.

      if time was ever known then i have an epoch. forget about being able to sync to ntp and other esp and php precisely because when does that matter.

      don't use rtc except in the rare instance where web is down and esp resets and can't start my_epoch from npt. if that happens just let millis take over. a few seconds drift over 24 hours is no big deal.





  3-1-2021 day of year, midnight, gallons, etc.

    keep it in a file

    on boot if time is known compare doy to file doy

    at every second compare doy to old doy

    any time doy changes

      update the file 

      prune

      send gallons

        send gallons every ?? until acked
   */
/*

void testfunc(char* outStr){
  char str[10];
  for(int i=0; i < 10; ++i){
    outStr[i] = str[i];
  }
}
Called with

int main(){
  char myStr[10];
  testfunc(myStr);
  // myStr is now filled
}
*/

/*
void clock_management(){

  
      if dream or biz has EVER worked since boot assume time is known accurately enough to do everything. if they both failed and rtc works I suppose I should use it. this could be when major outage kills internet and power
      for a while, and presumably the rtc won't be brand new and will be ok due to battery backup. If it turns out that garbage is saved and uploaded when web is back to life I guess I have to go online and manually remove the
      bad data. 
    
   
  
  switch (clock_state){

    case 0:   // initial state, nothing known. this state will try dream and biz. if either works with low latency, my_epoch will be set and everything can be done normally
      break;
      
    case 1:   // checking rtc when time is known because dream or biz worked, so if rtc looks wrong it will be fixed
      break;
      
    case 2:   // checking rtc when biz and dream failed so time is not known. if rtc works it will be used to set my_epoch
      break;
      
    case 3:
      break;
      
    case 4:
      break;
      
    case 5:
      break;
      
    case 6:
      break;
      
    case 7:
      break;
  }
}

void parse_time(char * outStr,uint32_t start_ep,uint32_t end_ep){

    uint32_t secs = end_ep - start_ep;
    
    uint32_t days = secs / 86400; 

    secs = secs - days * 86400;
  
    uint32_t hrs = secs / 3600;

    secs = secs - hrs * 3600;

    uint32_t mins = secs / 60;

    sprintf(outStr,"%d days %d:%d",days,hrs,mins);
}

*/

void handle_doy_change(){ // this is called when rtc, ntp, or millis detects change in date. usually it's at midnight, but it could also be some other time if power was off, in which case rtc and ntp are accurate but millis is not.
      
      //int millen_result;

      //millen_result = get_both_server_millens(); // returns 1 if dream is ok, 2 if dream failed but biz ok, 0 if both failed      11-7-2021 this will sync my epoch to dream if dream is good and will sync rtc as well, no matter what it currently has
      //
      // 2-10-2022 this is a bad idea, I think. I had some weird errors that cannot be explained (see discussion in spiffs near the top of the file, above create_new_dc..., but this might have contributed. the problem is that this is called here
      // when the doy changed, but the doy is based on my_epoch, and this call could push my_epoch back to yesterday which seems like a disaster. the idea is to check server epoch once per day, but this is the worst place to do it.
      //
      // so set a state flag = check_server_millens_state
      // 2 means waiting for 2 am
      // if time > 2 then check. if success, set to 0. if fail, increment. if new value < 20 then ok, we will check again at that hour. otherwise set to 30 which should never trigger since there are not 30 hours in a day
      // on doy change set state to 2

      check_server_millens_state = 2; // wait for 2 am to check

      save_RS_to_xeep();

      reset_xbee_RS();
      


      
/*
      if (millen_result == 1){

        millen_result = calculate_clock_errors(); // returns 1 if rtc worked, 0 otherwise

        if (millen_result == 1){
    
          check_rtc_error_and_correct_if_needed();
    
          //check_my_epoch_error_and_correct_if_needed();
        
        } else {

          add_bad_news((char *) "rtc failed so rtc and my_epoch can't be fixed today");
          
        }
        
      } else {

        add_bad_news((char *) "dream epoch failed so rtc and my_epoch won't be checked and fixed today");
      }
     
*/
      uint32_t heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 

      add_routine_log((char *) "...");
      add_bad_news((char *) "...");
      add_log((char *) "...");
      
      sprintf(rwb_buf,"start handle_doy_change heap %d",heap);
      add_routine_log(rwb_buf);

      yesterday_master_cnt = master_pulse_cnt + offset_cnt;
      
      save_a_uint32_t((char *) "/yday_cnt",yesterday_master_cnt); // this is the only thing that changes at midnight
/*
      if (xbee_RS_state == 0){

        xbee_RS_state = 1;  // see notes in uart get_RS()
      }
  */    
//      if (time_is_known){
        
        prune_file_type((char *) "/tank3_new",2);
        prune_file_type((char *) "/air_new",2);   
        prune_file_type((char *) "/trans_1a",1);
        prune_file_type((char *) "/trans_2a",1);
        prune_file_type((char *) "/flowa",1);
        prune_file_type((char *) "/float",1);
  
        float_writes++;
        save_a_uint32_t((char *) "/float_wr",float_writes);
        
        flow_writes++;
        save_a_uint32_t((char *) "/flow_wr",flow_writes);
        
        tr1_writes++;
        save_a_uint32_t((char *) "/tr1_wr",tr1_writes);
        
        tr2_writes++;
        save_a_uint32_t((char *) "/tr2_wr",tr2_writes);
        
        tank_writes++;
        save_a_uint32_t((char *) "/tank_wr",tank_writes);

        
        air_writes++;
        save_a_uint32_t((char *) "/air_wr",air_writes);
        
  
        prune_pulse_file();  // this handles pulse_writes and counts yesterday_gallons, and updates datecode file, and starts timer to send to php. gallons_per_master_cnt is continously updated at 0x15 packets

        //send_flow_cutoff(dream_host); // this seldom changes, but if it does i need to update php

  //    }
      
      good_pings_today_dream = 0;
      bad_pings_today_dream = 0;

      
      good_pings_today_biz = 0;
      bad_pings_today_biz = 0;

      save_rpi_temp_to_xeep(); // saves rpi_today_max

      rpi_yesterday_max = rpi_today_max;
      rpi_yesterday_min = rpi_today_min; 

      rpi_today_max = 0;
      rpi_today_min = 255;

      sprintf(rwb_buf,"in handle_doy_change: rpi_today_max %d today_min %d",rpi_today_max,rpi_today_min);
      add_log(rwb_buf);
      
      heap = ESP.getFreeHeap();
      sprintf(rwb_buf,"end handle_doy_change heap %d",heap);
      add_routine_log(rwb_buf);

      if (in_charge_online == 1){
  
        // query biz and dream dst flags and fix them if they don't agree with what it should be
  
        uint32_t dst_now = is_dst_on_now();
  
        int ret;
        
        ret = read_php_number(dream_host,read_dst,1,0); // the last two args: type 0 normal dir, type 1 x10 dir     log_flag = 1 to log result
  
        if ((ret == 1) && (php_read_32 != dst_now)){
            
          write_php_number(dream_host,write_dst,1,dst_now); // 1 means x10 dir
  
          sprintf(rwb_buf,"dream dst_flag set to %d",dst_now);
          add_log(rwb_buf);
        }
/*  
        ret = read_php_number(biz_host,read_dst,1,0);
        
        if ((ret == 1) && (php_read_32 != dst_now)){
   
          write_php_number(biz_host,write_dst,1,dst_now); // 1 means x10 dir
  
          sprintf(rwb_buf,"biz dst_flag set to %d",dst_now);
          add_log(rwb_buf);
        }
        */


        
      }

}

void handle_seconds(uint32_t secs){ // this is called once per second


  uint32_t nowsecs = todays_seconds(); // i don't know what all calls this, but it detects doy change, so call it every second.

  old_clock_millis += secs * 1000;

  my_epoch += secs; // this is used by todays_seconds() to calculate the value. that function also handles day of year change, and i'm not sure it gets called regularly when I'm not messing with the web page, so call it now

  //ntp_epoch += secs;

  //locked_ntp_epoch += secs; // this is used to set rtc, and is only synced to ntp via manual button click
  locked_dream += secs;


/* don't do this here - too easy to miss a slot. do like tablet - on each packet calc the slot
 *  
  uint32_t secs33 = todays_seconds(); // added 3-2-2021 to ensure midnight is detected. i'm not sure todays_seconds() is called regularly by any code

  if ((secs33 % 60) == 0){

    calc_xeep_slot(); // this actually saves the current flow_adc in xeep. note that if esp is having trouble detecting every second then this will miss slots. 
  }
*/
  seconds_since_last_pulse += secs;

  //simple_ntp(1); // this only changes ntp_epoch every twenty minutes

  if ((nowsecs > 21600) && (nowsecs < 68400)){ // don't blink at night. 7 pm est, 8 pm dst, to about 5 am est, 6 am dst, so just use 7 pm and 6 am. the main point of blinking, sorta, is when i flash to see when it gets connected to wifi
                                                                // 7 pm = 19 * 3600 = 68400   6 am = 6 * 3600 = 21600
    
    toggle_leds();
  
  } else {
    
    digitalWrite(LED1pin, LOW);
    digitalWrite(LED2pin, LOW);
  }

  seconds_since_last_packet += secs;

  if ((seconds_since_last_packet > 300) && (stop_nagging_no_packets != 1)){

    sprintf(rwb_buf,"last packet %d",seconds_since_last_packet);
    add_bad_news(rwb_buf);

    stop_nagging_no_packets = 1;
  }

  if (seconds_since_last_packet > 300){ // ordinarily it should be about 60 seconds. when i had 100 here, the log had too many entries, so ignore the occasional single missed packet, and change this to 200 which would be 3 missed packets

    xbee_reset_seconds += secs;
    
    if (xbee_reset_seconds > 60){   // this happens when master is offline, so avoid filling log. it's ok to reset xbee every minute

      if (xbee_reset_flood == 0){
      
        xbee_reset_flood = 1; 

        add_bad_news((char *) "xbee reset");

      }

      xbee_reset_epoch = my_epoch;

      xbee_reset_flood_cnt++;

      xbee_reset_seconds = 0;

      xbee_reset_cnt += secs;

      digitalWrite(xbee_reset_pin, HIGH);
      delay(5);                             // spec says 90 usec. change this to 500 for testing. 
      digitalWrite(xbee_reset_pin, LOW);
    }
  } else {

    xbee_reset_seconds = 500; // this will make it reset immediately when packets are late, then it will resume the interval shown above
  }

  secs_since_good_ping_dream += secs;  
  secs_since_bad_ping_dream += secs; 


  secs_since_good_ping_biz += secs;  
  secs_since_bad_ping_biz += secs; 

  rpi_ack_secs += secs;

  if ((master_rtc_state >=3) && (master_rtc_state < 250)){

    master_rtc_state ++;
  }
/*
  if (xbee_RS_state == 4){

    xbee_RS_counter += secs;    // when i did master rtc state i used that variable to count seconds, which i should have done here

    if (xbee_RS_counter ==30){

      xbee_RS_state = 5;  // see notes in uart get_RS()
    }
  }
*/
  

  if (dream_dead != 0){  // 0 means the server is fine. i added this on 6-17-2021 after removing the ntp code from the next line, which looks like it would restart_wifi over and over when the servers are ok - i think the ntp code
                                              // must have prevented that whether or not ntp was being used, as long as it was not dead for more than 300 secs which is what ntp_secs_dead was compared to. HOWEVER that was another bug
                                              // because yesterday something was down for 2 hours and wifi was never reset, which i think was because of not using ntp and not realizing that i needed to remove ntp test from this section
      
    if ((my_epoch - dream_dead) > 300) {  // either wifi is dead or my isp is dead, so reset wifi
  
      restart_wifi_secs += secs;
  
      if (restart_wifi_secs > 600){ // it will restart immediately on the first apparent issue, but after that no need to do it very often; either it works or it doesn't, and tons of resets won't fix it better than one. remember this is 16 bits unsigned = 65535 max
  
        restart_wifi_secs = 0;
  
        restart_wifi_cnt += secs;
        
        restart_wifi();
      }
      
    } else {
  
      restart_wifi_secs = 30000; // force it to restart immediately next time everything dies
    }
}




  if (in_charge_online == 1){
  
    if(dream_dead != 0){  // 0 is good. if it's dead, this is the epoch when it died
  
      if (fake_dream_is_dead == 0){ // if this == 1, then if dream is "dead", we want it to stay dead so don't do new_ping til I click the button to stop fakeing which will then allow this to try to fix it
  
        if (dream_new_ping_cnt > 0) {
          dream_new_ping_cnt--;
        } else {
          dream_new_ping_cnt = 120; // ping every 2 minutes
        }
  
        if (dream_new_ping_cnt == 0){
          
          if (new_ping(dream_host) == 1){ // it looks like host is back online
  
            document_ack(1, dream_id);
            
            add_log((char *) "new ping dream ok");
            send_all_blocks(dream_id); // when this is done, either the host will be dead or will be ok, and everything has been handled
          } else {
            document_ack(0, dream_id);
  
          }
        }
      }
      
    } else { // dream seems ok
  
      ping_web_dream += secs;
    
      if (ping_web_dream > 320){ // this is about 5 minutes
    
        if (seconds_since_last_packet < ping_web_dream){ // don't ping web if xbee or master is dead, so web will show data is old
    
          try_ping(dream_id); // this handles the result, good or bad
        }
      }
  
      
    }
  
  } // if in_charge_online == 1



  //esp_seconds_since_last_ntp += secs;

  if (restart_wifi_flag > 0){

    restart_wifi_flag --;

    if (restart_wifi_flag == 0){

      restart_wifi();
    }
  }

  
}





boolean detect_second(){ 

  new_clock_millis = millis();

  the_millis = new_clock_millis - old_clock_millis; 

  if (the_millis > 1000){

    //old_clock_millis = new_clock_millis; // if the millis is several seconds due to half_sec_delay crap it will be handled 

    return true;

  } else {

    return false;
  }
}




int ask_host_for_current_millenium(int hostnum){    // new version repeats until latency is small or too many tries

  int ret = 0;

  for (int i=0; i<3; i++){
   
     if (hostnum == dream_id){ // dream
    
        ret = read_php_number(dream_host,read_millenium,1,1); // the last two args: type 0 normal dir, type 1 x10 dir     log_flag = 1 to log result.   the value read will be in php_read_32
        add_log((char *) "dream host");

      }
    
      
    
      if ((ret == 1) && (latency_millis < 2000)){ // good response. dream typically < 400 ms, biz about 1500
    
        sprintf(rwb_buf,"host millenium %d took %d milliseconds",php_read_32,latency_millis);
        add_log(rwb_buf);

        break;
        
      }

      delay(100);

      if (i==2){

        add_bad_news((char *) "failed to read millen. see normal log for info");

        ret = -1;
        
      }
  }

  return ret;
}





int calculate_clock_errors(){
    
    rtc_epoch = 0;

    int rtc_result;
    
    rtc_result = get_rtc_epoch(); // returns 1 if rtc worked, -1 otherwise

    if (rtc_result == 1){
  
      if (my_epoch > rtc_epoch){
        myerr = my_epoch - rtc_epoch;
      } else {
        myerr = rtc_epoch - my_epoch;
      }
      
      
      if (locked_dream > rtc_epoch){
        rtc_vs_dream = locked_dream - rtc_epoch;
      } else {
        rtc_vs_dream = rtc_epoch - locked_dream;
      }

      return 1;
      
    } else {

      return 0;
    }
}

void calc_yesterday_midnight_epoch(){

    yesterday_midnight_epoch = my_epoch - 86400 - (time_t)todays_seconds();
  
}

uint32_t current_hour;


uint32_t todays_seconds(){

  if (my_epoch < 1614584313){ // if rtc works this should never happen, and and i don't have to worry about crazy pruning or gallons screwups

      char buf[130];

      sprintf(buf,"todays seconds called with my_epoch %lld",my_epoch);
      add_bad_news(buf);

    return 1;
  }
  
    struct tm *ptm = gmtime ((time_t *)&my_epoch); 
    

    uint32_t save_year = currentYear;   // added 1-1-2022 because bar graph for gallons screwed up by using 2022 for dec 31
    
    currentYear = ptm->tm_year+1900;  // was used in construct_datecode, but i changed to yesterday_year 1-1-2022

    uint32_t save_doy = day_of_year;

    uint32_t the_secs = (((ptm->tm_hour * 60) + ptm->tm_min) * 60) + ptm->tm_sec;

    day_of_year = ptm->tm_yday + 1; // time_t is zero based, but i use 1 based for gallons url

    char buf[130];

    if (day_of_year != save_doy){



      sprintf(buf,"in todays_seconds(), day_of_year changed from %d to %d at %d seconds after midnight",save_doy,day_of_year,the_secs);
      add_routine_log(buf);

      yesterday_day_of_year = save_doy; // this is correct after esp has stored the value once, but will be wrong the first time through

      yesterday_year = save_year;
      
      save_a_uint32_t((char *) "/doy",day_of_year);
      
      handle_doy_change();
    }

    if (ptm->tm_hour == check_server_millens_state){

      if (trans_plus_flow > 0){

        add_log((char *) "skip millens since trans_plus_flow > 0");

        check_server_millens_state++;
        
        
      } else {
    
          sprintf(buf,"time to check millens, hour = %d",ptm->tm_hour);
          add_log(buf);
    
          int rett = get_both_server_millens(); // returns 1 if dream is ok, 2 if dream failed but biz ok, 0 if both failed      11-7-2021 this will sync my epoch to dream if dream is good and will sync rtc as well, no matter what it currently has
    
          if (rett == 1){ // dream was ok, and replied in less than 2 seconds, so my_epoch and rtc were synced to dream
    
            check_server_millens_state = 30; // wait for 2 am to check again. this will be set to 2 in doy change
            
          } else { 
    
            if (check_server_millens_state < 20){
    
              check_server_millens_state++;
              
            } else {
              
              check_server_millens_state = 30; // don't check near midnight because doy can have issues if my_epoch is set back behind midnight after we thought the day had already changed.
            }
    
            sprintf(buf,"millens fail, check again at hour %d",check_server_millens_state);
            add_bad_news(buf);
            
          }
      }

      

      
    }

    return the_secs;
}
