/* ds3231 library functions

  the library i am using - look in the libraries directory in /mtv/fake_home/arduino/libraries - this is part of the README:
        
      ## Description
      
      Arduino library for the DS3231 real-time clock (RTC). Abstracts functionality for clock reading, clock setting, and alarms for the DS3231 high-precision real-time clock. This is a splice of
      [Ayars'](http://hacks.ayars.org/2011/04/ds3231-real-time-clock.html) and [Jeelabs/Ladyada's](https://github.com/adafruit/RTClib) libraries.
      
      

      --- skip some stuff; this seems to be the main github

                
          There are many examples implemented where this library is used. You can find other examples from [Github-DS3231](https://github.com/NorthernWidget/DS3231/tree/master/examples)      

          
 */


/*
void get_all_at_once(){ // 6-7-2021 this is never called

    
//    if (scan_for_rtc(false) != 1){
  //    return;
  //  }

  
    //time_is_known = true; don't do this here. only set this flag by rtc in the one for epoch that is called in setup() if ntp is dead to set my_epoch
    
   
    RTClib myRTC;

    DateTime now = myRTC.now();

    
    
    sprintf(rwb_buf,"nowsec %d",now.second());   
                  
    add_log(rwb_buf);
    
    sprintf(rwb_buf,"nowhr %d",now.hour());   
                  
    add_log(rwb_buf);
    
    sprintf(rwb_buf,"nowmin %d",now.minute());   
                  
    add_log(rwb_buf);

}
*/


uint32_t get_rtc_epoch(){  

    if (scan_for_rtc() != 1){
      
      return -1;
    }

    
  RTClib myRTC;

  DateTime now = myRTC.now();   // this reads the whole clock properly so that internal buffers in the clock prevent rollover

  rtc_epoch = now.unixtime();

  return 1;
    
}



/*

void set_rtc_to_ntp2(){ 


    
    struct tm *nptm = gmtime ((time_t *)&locked_ntp_epoch); 
    
                                                  //    rtc range
  
    Clock.setSecond(nptm->tm_sec);                //    0-59        set second first, then you have to set all the rest within one second. scope test says it takes 3.2 msec to set these six registers
    Clock.setHour(nptm->tm_hour);                 //    0-23       
    Clock.setMinute(nptm->tm_min);                //    0-59
    Clock.setYear(nptm->tm_year + 1900 - 2000);   //    00-99
    Clock.setMonth(nptm->tm_mon + 1);             //    1-12       
    Clock.setDate(nptm->tm_mday);                 //    1-31

}
*/


void sync_rtc_to_epoch(time_t * epoch){ 


    /*
          tm_sec int seconds after the minute  0-61*
          tm_min  int minutes after the hour  0-59
          tm_hour int hours since midnight  0-23
          tm_mday int day of the month  1-31
          tm_mon  int months since January  0-11
          tm_year int years since 1900  
          tm_wday int days since Sunday 0-6
          tm_yday int days since January 1  0-365
          tm_isdst  int Daylight Saving Time flag 
     */
    
    struct tm *nptm = gmtime ((time_t *)epoch); 
    
                                                  //    rtc range
  
    Clock.setSecond(nptm->tm_sec);                //    0-59        set second first, then you have to set all the rest within one second. scope test says it takes 3.2 msec to set these six registers
    Clock.setHour(nptm->tm_hour);                 //    0-23       
    Clock.setMinute(nptm->tm_min);                //    0-59
    Clock.setYear(nptm->tm_year + 1900 - 2000);   //    00-99
    Clock.setMonth(nptm->tm_mon + 1);             //    1-12       
    Clock.setDate(nptm->tm_mday);                 //    1-31

}

/*

void sync_rtc_to_dream(){ 
    
    struct tm *nptm = gmtime ((time_t *)&locked_dream); 
    
                                                  //    rtc range
  
    Clock.setSecond(nptm->tm_sec);                //    0-59        set second first, then you have to set all the rest within one second. scope test says it takes 3.2 msec to set these six registers
    Clock.setHour(nptm->tm_hour);                 //    0-23       
    Clock.setMinute(nptm->tm_min);                //    0-59
    Clock.setYear(nptm->tm_year + 1900 - 2000);   //    00-99
    Clock.setMonth(nptm->tm_mon + 1);             //    1-12       
    Clock.setDate(nptm->tm_mday);                 //    1-31

}
*/

int scan_for_rtc(){
  
  int ret = 0;
  
  Wire.beginTransmission(0x68);
  
  if (Wire.endTransmission() == 0){
        
      ret = 1;

  } else {

     add_bad_news((char *) "rtc dead");

  }

  return ret;
}

void scan_i2c(){

  
  for (byte i = 8; i < 120; i++){
  
  
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0){
    
      sprintf(rwb_buf,"found 12c  %02X",i);  
      add_log(rwb_buf);
      
      //found 12c 0x57  xeep. web says it is 0x56 on some devices
      //found 12c 0x68  rtc

      delay(1);
      }
  }
}
