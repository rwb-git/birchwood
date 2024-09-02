
int is_dst_on_now(){ // return 1 if dst is being used now, 0 otherwise. 

  // get epoch when dst starts in march of this year. ignore the 2 am or whatever. just do it at midnight since this flag only affects the x10 legend display which will probably not be viewed between midnight and 2 am

  uint32_t save_it = test_dst_epoch; // save it so that the test web page can check future years. otherwise this is not used
  
  test_dst_epoch = my_epoch;

  march();

  uint32_t dst_start = test_dst_epoch;

  // get epoch when dst ends in november of this year

  test_dst_epoch = my_epoch;

  november();

  uint32_t dst_end = test_dst_epoch;

  test_dst_epoch = save_it;
  
  // if current epoch is between those then return 1, else return 0

  if (my_epoch >= dst_start){

      if (my_epoch < dst_end){ // this used to be <= but dst ends on that day so shouldn't it be <? in 2021 it showed 1 for nov 7 but dst ended at midnight the morning of nov 7 so it should say 0

        return 1;
      }
  }

  return 0;
  
}

void march(){     // remember unix 32 bit time ends jan 2038

  // begins second sunday in march at 2 am
  // ends first sunday in november at 2 am
  //
  //In 2021, DST begins on March 14 and ends on Nov. 7  will begin again on March 13, 2022
  
    struct tm *nptm = gmtime ((time_t *)&test_dst_epoch); 

    // first, set month to november and date to 1 and see what wday results

    nptm->tm_mon = 2;
    nptm->tm_mday = 8; // looking for 2nd sunday so skip first week

    test_dst_epoch = mktime(nptm);

    nptm = gmtime((time_t *) &test_dst_epoch);

    if (nptm->tm_wday != 0){

      nptm->tm_mday += 7 - nptm->tm_wday;
    }

    test_dst_epoch = mktime(nptm);

}



void november(){

  // begins second sunday in march at 2 am
  // ends first sunday in november at 2 am
  //
  //In 2021, DST begins on March 14 and ends on Nov. 7  will begin again on March 13, 2022


  // find first sunday in november of this year

  
    struct tm *nptm = gmtime ((time_t *)&test_dst_epoch); 

    // first, set month to november and date to 1 and see what wday results

    nptm->tm_mon = 10;
    nptm->tm_mday = 1;

    test_dst_epoch = mktime(nptm);

    nptm = gmtime((time_t *) &test_dst_epoch);

    if (nptm->tm_wday != 0){

      nptm->tm_mday += 7 - nptm->tm_wday;

      //int fix = 7 - dd; // example: day is 4 and needs to be zero, so add 7-4 to date, or 3. if day is 1, add 7-1 = 6 days
    }

    test_dst_epoch = mktime(nptm);

    
    //struct tm *nptm = gmtime ((time_t *)&test_dst_epoch); 
    /*
    Clock.setSecond(nptm->tm_sec);                //    0-59        set second first, then you have to set all the rest within one second. scope test says it takes 3.2 msec to set these six registers
    Clock.setHour(nptm->tm_hour);                 //    0-23       
    Clock.setMinute(nptm->tm_min);                //    0-59
    Clock.setYear(nptm->tm_year + 1900 - 2000);   //    00-99
    Clock.setMonth(nptm->tm_mon + 1);             //    1-12  this is 1-12 because I add 1 over there. the struct itself returns 0..11 so when i use 2 for march and 10 for november it should be correct     
    Clock.setDate(nptm->tm_mday);                 //    1-31
        
    int tm_sec  seconds after the minute – [0, 61] (until C99)[0, 60] (since C99)[note 1]
    int tm_min  minutes after the hour – [0, 59]
    int tm_hour hours since midnight – [0, 23]
    int tm_mday day of the month – [1, 31]
    int tm_mon  months since January – [0, 11]
    int tm_year years since 1900
    int tm_wday days since Sunday – [0, 6]
    int tm_yday days since January 1 – [0, 365]
  */

  
  //sprintf(rwb_buf,"test_dst_epoch %d<br><br> %d-%d-%d %d:%d:%d ..... dst flag %d ... day of week %d",test_dst_epoch,nptm->tm_mon+1,nptm->tm_mday,nptm->tm_year+1900,nptm->tm_hour,nptm->tm_min,nptm->tm_sec,nptm->tm_isdst,nptm->tm_wday);
  

  
    /*
        tm_sec int seconds after the minute  0-61*
        tm_min  int minutes after the hour  0-59
        tm_hour int hours since midnight  0-23
        tm_mday int day of the month  1-31
        tm_mon  int months since January  0-11
        tm_year int years since 1900  
        tm_wday int days since Sunday 0-6
        tm_yday int days since January 1  0-365
        tm_isdst  int Daylight Saving Time flag    <<<  test this first. ok, that works with systems like linux that have locales and env vars. i could install time and timezone libraries and it supposedly works here, but for now skip
                                                        all that and see what kind of brute force code might work. for a particular year, there's an epoch where dst begins and one where it ends, so figure out how to calculate that and the
                                                        rest is easy.

                                                        consider this: set year, set month to november, set date to 1. if that is sunday then done. otherwise increment date until sunday. or just do the math directly. similar for second sunday in march


        
        <ctime>
        time_t
        
        Time type
        
        Alias of a fundamental arithmetic type capable of representing times, as those returned by function time.
        
        For historical reasons, it is generally implemented as an integral value representing the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC (i.e., a unix timestamp). Although libraries may implement this type using alternative time representations.
        
        Portable programs should not use values of this type directly, but always rely on calls to elements of the standard library to translate them to portable types.


        >>> note that i don't use time_t in general for my epoch values, but use uint32_t and cast it like this:

          struct tm *nptm = gmtime ((time_t *)&locked_ntp_epoch); 



        // gmtime example 
        #include <stdio.h>      // puts, printf 
        #include <time.h>       // time_t, struct tm, time, gmtime 
        
        #define MST (-7)
        #define UTC (0)
        #define CCT (+8)
        
        int main ()
        {
          time_t rawtime;
          struct tm * ptm;
        
          time ( &rawtime );
        
          ptm = gmtime ( &rawtime );
        
          puts ("Current time around the World:");
          printf ("Phoenix, AZ (U.S.) :  %2d:%02d\n", (ptm->tm_hour+MST)%24, ptm->tm_min);
          printf ("Reykjavik (Iceland) : %2d:%02d\n", (ptm->tm_hour+UTC)%24, ptm->tm_min);
          printf ("Beijing (China) :     %2d:%02d\n", (ptm->tm_hour+CCT)%24, ptm->tm_min);
        
          return 0;

        

        // mktime example: weekday calculator 
        
        #include <stdio.h>      // printf, scanf 
        #include <time.h>       // time_t, struct tm, time, mktime 
        
        int main ()
        {
          time_t rawtime;
          struct tm * timeinfo;
          int year, month ,day;
          const char * weekday[] = { "Sunday", "Monday",
                                     "Tuesday", "Wednesday",
                                     "Thursday", "Friday", "Saturday"};
        
          // prompt user for date 
          printf ("Enter year: "); fflush(stdout); scanf ("%d",&year);
          printf ("Enter month: "); fflush(stdout); scanf ("%d",&month);
          printf ("Enter day: "); fflush(stdout); scanf ("%d",&day);
        
          // get current timeinfo and modify it to the user's choice 
          time ( &rawtime );
          timeinfo = localtime ( &rawtime );
          timeinfo->tm_year = year - 1900;
          timeinfo->tm_mon = month - 1;
          timeinfo->tm_mday = day;
        
          // call mktime: timeinfo->tm_wday will be set 
          mktime ( timeinfo );
        
          printf ("That day is a %s.\n", weekday[timeinfo->tm_wday]);
        
          return 0;
     */
}
