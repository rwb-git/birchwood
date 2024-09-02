
#define who_knows 300   // the longest msg cannot exceed this. nothing is checked when adding entries, so this will reject anything longer, which is unlikely since i don't want log entries to wrap.

void show_a_log(String & ptr, uint32_t log_ptr, uint8_t log_wrap, char * the_log, uint32_t log_size){
         
    int ln = 0;

    int actual_ln = 0;
    
    uint32_t strt = 0;
    
    uint32_t fin = 0;
    
    char b2[who_knows]; 
    
    uint32_t search_ind = 0;
  
    if (log_wrap == 1){
      
      search_ind = log_ptr;
      strt = log_ptr;
    }
  
   char wrap[who_knows]; 

   ptr +="<br><br>";     

   int yield_cnt = 0;
   
   for (uint32_t i=0; i<log_size;i++){

    yield_cnt++;

    if (yield_cnt > 500){

      yield();

      yield_cnt = 0;
    }
  
      if (the_log[search_ind] == '\0'){
      
          fin = search_ind;
      
          if (fin > strt){ // this string is normal inside the buffer, not wrapped around the endpoint
        
              ln = fin - strt;

              if ((ln < (who_knows - 2)) && (ln > 2)){  // assume no msg will ever be this tiny - except when it wraps it can overwrite all but the end of an old message. is this the bug? i see ln = 2 and ln = 1 so far
                                                        // if ln = 1 what happens? that's just the null, right?
                                                        //
                                                        //  /0    a       b     c     d     /0
                                                        //  fin   start   s+1   s+2   s+3   fin, so ln here would be 16 - 12 = 4 which is correct
                                                        //  11    12      13     14   15    16    
                                                        //
                                                        //  /0    /0      a     b     c     d     /0
                                                        //  fin   start                                 in this case the copied string will start with a null, but i'm not sure this has happened yet
                                                        //
                                                        // ok i see one with bad_actual_ln = log size, so that could be an issue. this happens when two /0 are adjacent. fin = search_ind, process msg, strt = fin + 1, search_ind++, and if
                                                        // that is also /0 then strt = fin and fn is not > strt, so actual_ln = log_size
                                                        //
                                                        // so, if all the screwups are filtered out here, maybe that's ok, but should i consider this: if log is wrapped, add new msg and then inspect the next few chars, and
                                                        // simply change any /0 to something else, like a blank. or skip the searching and just fill the next 4 bytes with spaces; it will only affect the oldest message and
                                                        // doesn't seem like it could cause an issue.
              
                strncpy(b2,the_log + strt,ln); // if source is longer than destination, strncpy does NOT add null terminator so i have to add it
          
                b2[ln] = '\0';
                ptr += b2;
                ptr +="<br>";      
                
              } else {

                bad_log_cnt++;

                bad_ln = ln;
              }
      
          } else if (fin < strt) { // this string wraps around the end back to the start. also, skip entries that have zero length which happens sometimes when log wraps around

              actual_ln = log_size - strt + fin;

              if ((actual_ln < (who_knows - 2)) && (actual_ln > 2)){
              
                ln = log_size - strt;
                
                strncpy(b2,the_log + strt,ln); // if source is longer than destination, strncpy does NOT add null terminator so i have to add it
        
                b2[ln] = '\0';    // strcat needs this to know where this string ends
          
                ln = fin;
                
                strncpy(wrap,the_log,ln); // if source is longer than destination, strncpy does NOT add null terminator so i have to add it
          
                wrap[ln] = '\0';    // wrap[] has the ending and b2[] has the beginning
        
                strcat(b2,wrap);
                
                ptr += b2;
                ptr +="<br>"; 
                       
              } else {

                bad_log_cnt++;

                bad_actual_ln = actual_ln;
              }
          }
      
          strt = fin + 1;    
      
          if (strt >= log_size){
      
            strt = 0;
          }
      }
      
      search_ind++;
  
      if (search_ind >= log_size){
  
        search_ind = 0;
      }
   }
   ptr +="<br><br>"; 
}



void add_fl(char * msg){
  
  int sl = strlen(msg);
  
  for (int i=0;i<(sl+1);i++){ // add 1 for the null terminator

    fast_log[fast_log_ptr] = msg[i];
    
    fast_log_ptr++;
    
    if (fast_log_ptr >= log22_size){

      fast_log_wrap = 1;
      
      fast_log_ptr = 0; 
    } 
  }

  // if it wraps around, the overwritten entry can have zero length, which was causing frequent resets, but is now handled when displaying the log

}

void add_fast_log_0xAA(){

  char msg[30];

  sprintf(msg,"0xAA (%d)",fast_log_0xAA_cnt);

  add_fl(msg);
  
  fast_log_0xAA_cnt = 0;
}



void add_fast_log(char * msg){

  if (fast_log_0xAA_cnt > 0){

    add_fast_log_0xAA();
  }

  add_fl(msg);
}


void add_routine_log(char * msg){
     
  int sl = strlen(msg);
  
  for (int i=0;i<(sl+1);i++){ // add 1 for the null terminator

    routine_log[routine_log_ptr] = msg[i];
    
    routine_log_ptr++;
    
    if (routine_log_ptr >= routine_log_size){

      routine_log_wrap = 1;
      
      routine_log_ptr = 0; //buf_ind = 0;
    } 
  }

  add_fast_log(msg);
}

void log_bad_news_time2(){
     
  struct tm * ptm = gmtime ((time_t *) &my_epoch);

     
  sprintf(rwb_buf,"%02d:%02d:%02d",ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
  add_bad_news2(rwb_buf);
  
  
}



void log_time(){
     
  struct tm * ptm = gmtime ((time_t *) &my_epoch);

     
  sprintf(rwb_buf,"%02d:%02d:%02d",ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
  add_log(rwb_buf);
  
  
}


void log_routine_time(){
     
  struct tm * ptm = gmtime ((time_t *) &my_epoch);

     
  sprintf(rwb_buf,"%02d:%02d:%02d",ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
  add_routine_log(rwb_buf);
  
  
}

void add_bad_news(char * msg){
     
  add_bad_news2(msg);

  if (my_epoch > 0){
    
    log_bad_news_time2();
  }

  add_fast_log(msg);
}


void add_bad_news2(char * msg){
     
  int sl = strlen(msg);
  
  for (int i=0;i<(sl+1);i++){ // add 1 for the null terminator

    bad_news[bad_news_ptr] = msg[i];
    
    bad_news_ptr++;
    
    if (bad_news_ptr >= bad_news_log_size){

      bad_news_wrap = 1;
      
      bad_news_ptr = 0; 
    } 
  }
}

void add_log(char * msg){
     
  int sl = strlen(msg);
  
  for (int i=0;i<(sl+1);i++){ // add 1 for the null terminator

    log22[log22_ptr] = msg[i];
    
    log22_ptr++;
    
    if (log22_ptr >= log22_size){

      log22_wrap = 1;
      
      log22_ptr = 0; 
    } 
  }

  add_fast_log(msg);
}
