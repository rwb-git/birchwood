
/*
  SPIFFS/LittleFS.open() and dir.openFile() functions return a File object. This object supports all the functions of Stream, so you can use readBytes, findUntil, parseInt, println, and all other Stream methods.

  There are also some functions which are specific to File object.

  

  stream can parseInt() and parseFloat() which might be cool for things like storing time fix float

  --- c_str() ---
        
        const char* c_str() 
        
        Returns a pointer to a const char * array that contains a null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
        
        This array includes the same sequence of characters that make up the value of the string object plus an additional terminating null-character ('\0') at the end.

   --- unused blocks was about 380 initially for LittleFS and SPIFFS ----------


2021045_010_2021046_015_2021047_020_2021048_025_2021049_030_

 */

void report_malloc_result(){

    uint32_t heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 

    sprintf(rwb_buf,"heap after malloc %u",heap);
    
    add_fast_log(rwb_buf);
  
}

void report_malloc_fail(int num){

    uint32_t heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 

    sprintf(rwb_buf,"malloc fail to get %d heap is %u",num,heap);
    
    add_bad_news(rwb_buf);

    

  
}


void clear_flags_bit(uint8_t the_bit){
  
/*
      test, set, clear bit k:

        uint32_t one_32 = 1;

        test bit k: (n >> k) & one_32    if bit k is set this returns 1, else it returns 0
        
        set bit k: n |= one_32 << k

        clear bit k: n &= ~(one_32 << k);

*/

  uint32_t flags = read_a_uint32_t((char *) "/online"); // returns 0 if it fails to read a value

  uint32_t one_32 = 1;
  
  flags &= ~(one_32 << the_bit);

  save_a_uint32_t((char *) "/online",flags);

}


void set_flags_bit(uint8_t the_bit){
  
  uint32_t flags = read_a_uint32_t((char *) "/online"); // returns 0 if it fails to read a value

  uint32_t one_32 = 1;
  
  flags |= one_32 << the_bit;

  save_a_uint32_t((char *) "/online",flags);

}



uint8_t read_flags_bit(uint8_t the_bit){
  
  uint32_t flags = read_a_uint32_t((char *) "/online"); // returns 0 if it fails to read a value

  uint32_t one_32 = 1;
  
  return (flags >> the_bit) & one_32;

}



int prepare_block(const char * fn,int field2,time_t test_epoch){    // this looks heap-safe since it checks to make sure it does not overflow block which is statically allocated unlike test_read_new_file which uses malloc
  /*
        8-27-2022

          test prepare block with air. block send typically is when dream dies for a while then comes back and esp knows epoch of last good ack, and sends from that point.

          to fake that, i need to stop sending for a while but keep putting in local file - one button to do this. save the epoch

          then, another button that uses that saved epoch to prepare and send a block. send_all_blocks calls prepare_a_block, then try_a_block which calls send_block

          24 hours now that I only write air every 5 minutes: 10 digits for decimal epoch, space, 2 digits value, space = 14 chars per item: 24 x 60 / 5 x 14 = 4032 DON'T FORGET THAT I ONLY SEND THE FIRST EPOCH, AND
          AFTER THAT it's just the delta, so the block is going to be way smaller than that number; instead of 10 digits, more like 3: 300 secs = 5 mins, 999 secs = 16 mins, so no big deal. whew. php converts the deltas
          to millens which is why the files are full of huge numbers.
   */

  char two[10];
  char four[10];
  
  int num_read = 0;

  uint32_t preceding = 0;

  uint32_t current = 0;
  
  int field1 = 8;

  char element[20];
  
  //char debug[50];

  int first = 1;

  int skip = 1;

  int found = 0;

  uint32_t yesterday = (uint32_t) my_epoch - 86400; // actually this is "24 hours ago"

  uint32_t the_test;

  block[0] = '\0'; // old notes said log error if prepare_block found no records the log would keep listing the old size, so do this to "clear" the block

  if (yesterday < (uint32_t)test_epoch){

    the_test = (uint32_t)test_epoch; // use the larger epoch. we only want to send events that happened after host died, and not anything older than 24 hours ago
    
  } else {

    the_test = yesterday;
  }

  //the_test = 0;

  if(LittleFS.begin()){
  
    if (LittleFS.exists(fn)){ 

      File f = LittleFS.open(fn,"r");
     
      if (f){

        filesize = f.size();
        
        num_recs = filesize / (long)(field1 + field2);
            
        for (int jj=0;jj<num_recs;jj++){

          if (strlen(block) > (blocksize - 30)){

            sprintf(rwb_buf,"block is full: %d",strlen(block));
            add_log(rwb_buf);

            break;
          }
          
          if (f.available() >= (field1 + field2)){
          
            num_read = f.readBytes(four,field1);

            if (num_read == field1){
              four[field1] = '\0';
              

              current = (uint32_t)strtol(four, NULL, 16);

              if (current < the_test) {

                skip = 1;
              
              } else {

                found++;

                skip = 0;
  
                if (first == 1){
  
                  first = 0;
  
                  sprintf(block,"%d ",current + 18000); // add 5 hours
                  
                } else {
                  
                  sprintf(element,"%d ",current - preceding);  // delta does not need five hours
                  
                  strcat(block,element);
                }
  
                preceding = current; //(uint32_t)strtol(four, NULL, 16);
              }
            }

            if (field2 > 0){
              
              num_read = f.readBytes(two,field2);
  
              if (skip == 0){
    
                if (num_read == field2){
                  two[field2] = '\0';
    
                  int jk = (uint8_t)strtol(two, NULL, 16);   // 8-27-2022 before adding air this was uint8_t. this writes a decimal value %d so I think this should still work for all types.

                  if (air_block == 1){

                    jk = jk * 4; // I store 8 bit values here, but php uses 10 and is hard to undo that decision
                    
                  }
                  
                  sprintf(element,"%d ",jk);
    
                  strcat(block,element);
                }
              }
            }
          }
        }

        f.close();

        if (strlen(block) > 3){

          if (verbose1) {
            sprintf(rwb_buf,"block size %d",strlen(block));
            add_log(rwb_buf);
          }

          return found;
        }
      }
    }
  }
  return 0;
}



void read_datecode_file(char * buf){  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------

  if(LittleFS.begin()){
    
    if (LittleFS.exists("/datecodes")){
    
      File f = LittleFS.open("/datecodes","r");
  
      if (f){
          
        String s = f.readStringUntil('\n');

        strcpy(buf,s.c_str());
        
        f.close();
      }
    }
  }  
}


void parse_datecode(){  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------

  
  /*
       if *stringp is NULL, the strsep() function returns NULL and does
       nothing else.  Otherwise, this function finds the first token in
       the string *stringp, that is delimited by one of the bytes in the
       string delim.  This token is terminated by overwriting the
       delimiter with a null byte ('\0'), and *stringp is updated to
       point past the token.  In case no delimiter was found, the token
       is taken to be the entire string *stringp, and *stringp is made
       NULL.

RETURN VALUE         top

       The strsep() function returns a pointer to the token, that is, it
       returns the original value of *stringp.


       The strdup() function returns a pointer to a new string which is
       a duplicate of the string s.  Memory for the new string is
       obtained with malloc(3), and can be freed with free(3).

   */

  char dc[200];

  read_datecode_file(dc);

  //int cnt = 0;
  
  char * paych = strdup(dc); // strdup copies dc, and needs to be freed when done. why did i do it like this in avrdude_cmdline:    char * paych = strdup(&dc[0]);  online seems to say it should be (dc) like this line does. keep track of heap over time...
                             // but note that paych worked correctly in strlen() which means that the string at &dc[0] was properly used. I guess it looks for the null terminator lol what else could it do. anyway, i tested this code
                             // with free disabled and heap start dropped every time. with free enabled, heap start does not change (this was with the other parse that shows it on the web page
   
  char * save_paych = paych; // save so i can free, which requires original pointer value, and strsep alters paych

  char * token;

  byte index = 0;

  int raptr = 0;




  //char buf[100];

  code_cnt = 0;

  if (strlen(paych) > 0){

    //int len1 = strlen(paych);
    //int codes = len1 / 12;
    
    token = strsep(&paych,"_");

    while (strlen(token) > 0){

      yield();

      if (index == 0){

        index = 1;

        years2[raptr] = atoi(token);
        
        sprintf(rwb_buf,"es2 %d %d",years2[raptr],raptr);   
                    
        add_log(rwb_buf);
        
      } else {

        gals2[raptr] = atoi(token);

        raptr++;

        code_cnt++;

        index = 0;
      }
      
      token = strsep(&paych,"_");

      if ((token) == NULL){ //"\0"){

        break;
      }
      
    }
  }

  free(save_paych);
  
}


void try_to_clean_up_datecode(){

  //int save = day_of_year;
  yesterday_day_of_year = 55;
  

  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  yesterday_day_of_year=56;

  
  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  yesterday_day_of_year=57;

  
  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  yesterday_day_of_year=58;


  
  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  yesterday_day_of_year=59;


  
  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  yesterday_day_of_year=60;


  
  yesterday_gallons = 40 + day_of_year;

  create_new_dc_and_update_datecode_file();

  
  read_datecode_file(rwb_buf);
  add_log(rwb_buf);

  //day_of_year = save;
}


void construct_datecode(char * dest){  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------

  uint32_t uptime = (uint32_t)(my_epoch - boot_epoch);

// this is done in todays_seconds which is called every second  currentYear = nptm->tm_year+1900;   // used to set rtc

  if ((gallons_per_master_cnt > yesterday_gallons) && (uptime > 86400)){  // master count needs a day to be valid. note that this doesn't exactly work correctly if esp gets reset briefly, but when esp is down for days this can send stupid high value

    sprintf(dest,"%d%03d_%03d_",yesterday_year,yesterday_day_of_year,gallons_per_master_cnt);   // used to use currentYear before 1-1-2022
    sprintf(rwb_buf,"new datecode %s based on master cnt",dest);
    
  } else {

    sprintf(dest,"%d%03d_%03d_",yesterday_year,yesterday_day_of_year,yesterday_gallons);  // used to use currentYear before 1-1-2022
    sprintf(rwb_buf,"new datecode %s based on 0x15 packet cnt",dest);
  }
   
   add_routine_log(rwb_buf);
}


void add_fake_datecode(){  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------

  sprintf(rwb_buf,"add fake bef %d",day_of_year);   
                    
  add_log(rwb_buf);

  day_of_year++;
  yesterday_gallons +=5;
  
  sprintf(rwb_buf,"add fake aft %d",day_of_year);   
                    
  add_log(rwb_buf);
  
  create_new_dc_and_update_datecode_file();

}

void create_new_dc_and_update_datecode_file(){  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------


  sprintf(rwb_buf,"begin add new datecode");   
              
  add_routine_log(rwb_buf);


  parse_datecode();

  int strt;
  
  if (code_cnt > 2){   // i want 5 codes when I'm done, and we will add one, so we want four from the old. array slots 0 1 2 3 4 5 6 7
                  //                                                                                         cnt  1 2 3 4 5 6 7 8
                  //
                  // so, if there were 7 there, we want the last 4 so we want 3 4 5 6   start at 3 = cnt - 4
                  //                   6                                      2 3 4 5            2         4
                  //                   5                                      1 2 3 4            1         4
                  //                   4                                      0 1 2 3 = all of them if cnt < 5

    
    strt = code_cnt - 2;
    
  } else {

    strt = 0;
  }

  char buf2[32];
  char buf[256];



  for (int i=strt; i<code_cnt; i++){

    if (i == strt){
      
        sprintf(rwb_buf,"old datecode %d   %d",years2[i],gals2[i]);
        add_routine_log(rwb_buf);

        sprintf(buf,"%d_%03d_",years2[i],gals2[i]); // NOTE the first goes in different buffer
        
    } else {

        sprintf(rwb_buf," old datecode %d   %d",years2[i],gals2[i]);
        add_routine_log(rwb_buf);      
        
        sprintf(buf2,"%d_%03d_",years2[i],gals2[i]); 
  
        strcat(buf,buf2);
    }
  }

  // add the new datecode

  char dest[51];
  construct_datecode(dest);
  
  strcat(buf,dest);
  
  if(LittleFS.begin()){

    File f = LittleFS.open("/datecodes","w"); 
    
    if (f){
      
      f.print(buf);
      
      f.close();
    }
  }
  
  sprintf(rwb_buf,"end of add new datecode");    
  add_routine_log(rwb_buf);

}




void test_read_pulse_file(char * fn){     // file type_number 2  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------

//  char str32[100];
 
  char five[9];

//  String s,filename;

//  sprintf(str32,"%s",fn);

//  filename = str32;
  
  int num_read = 0;

  int stop_now = 0;

  if(LittleFS.begin()){
  
    if (LittleFS.exists(fn)){

      File f = LittleFS.open(fn,"r");
     
      if (f){

        filesize = f.size();

        num_recs = filesize / 8; // each record should be "ttttt" = time

        if (num_recs > 0){
      
              times = (uint32_t *)  malloc(filesize);
      
              if (times == NULL){ // malloc failure
                      
                sprintf(rwb_buf,"malloc test_read_pulse_file %s, times",fn);
                
                add_bad_news(rwb_buf);
                
                report_malloc_fail(filesize);
                
                stop_now = 1;
              } else {
      
                report_malloc_result();
              }
      
              if (stop_now == 0){ // first test
                            
                  need_free_times = 1;
              
                  for (int jj=0;jj<num_recs;jj++){
                    
                    if (f.available() >= 8){
                    
                      num_read = f.readBytes(five,8);
          
                      if (num_read == 8){
                        five[8] = '\0';
                        times[jj] = (uint32_t)strtol(five, NULL, 16);
                      }
                    }
                  }
      
              } // first test

          } // num_recs > 0

        f.close();
      }
    }
  }
}


void test_read_new_file(char * fn,int field1,int field2){       // this is only called on boot to get most recent value, and when I look at files. i think this was crashing the heap when air was writing every minute. so how does prepare_block handle large file
                                                                        //
                                                                        // ok, prepare_block checks for room in block which is statically allocated, and fails safely unlike this proc which crashes the heap. i need to check malloc here, and also maybe not store
                                                                        // so much data. air does not need 10 bits in the file which is only used when internet is down and I have to send a block. just remember that php expects 10 bits so if I save 8 I need to
                                                                        // convert back to 10 for send block.

/*
    these files are packed without any field separators. 

    all timestamps are 8 bytes = true millenium

    field 2 for things like transfer are one byte, 1 or 0

    field 2 for tank is 0..100

    air is 10 bits max val 1023 so two bytes?



    8-27-2022

        I don't think I actually understood what I did here originally but I got it to work. 

        I think field2 = 1 means read one character, like a 1 or a 0 for trans1, and field2 = 2 for tank means hex values like 0x64 which is 100, the largest value for tank

        so if that is all correct, air, which is 10 bits will have field2 = 3, for max value 0x3FF; actual value for 40 psi is ideally 0x1AD so it will definitely use 3 chars.
 */

  log_heap();
 
  char two[10];
  char four[10];

  num_recs = 0; // added 3-11-2024 since it shows garbage if the file is empty; it seems to use values from the last file that had content, and then looks
                // in times and values which don't exist and have been de allocated, I think. 
  
  int num_read = 0;

  int stop_now = 0;

  if(LittleFS.begin()){
  
    if (LittleFS.exists(fn)){ 

      File f = LittleFS.open(fn,"r");
     
      if (f){

        filesize = f.size();    // filesize is an integer so this will screw up if file > 32768. air has been fucked in show file after 12+ hours. air stores 11 bytes every minute. 24 x 60 x 11 = 15840
                                //
                                // STUPID look at heap; typically 13000 so how can you put 15840 in there. for that matter, how close does pulse come to crashing out, or tank? right now pulse is 1240 and tank is 2260, with normal activity
                                // tank has about 12 hours of constants so that leaves 12 x 60 x 10 = 7200 bytes, so maybe there are a lot of minute pairs where it does not change.

        int big_num = filesize / (field1 + field2); 

        int times_malloc = field1 * big_num;

        if (big_num > 0){
      
              times = (uint32_t *) malloc(times_malloc); // filesize / (long)(field1 + field2)); // for air 8 * size / (8 + 3)      8 * 15840 = 126720  sparkfun says arduino int is 16 bits so I need to use longs here
                                                                                                                                    // i'm confused. this huge number is not relevant. air = 11 bytes per minute x 24 hrs x 60 = 15840 and i've forgotten how
                                                                                                                                    // pruning works so assume 48 hrs might be stored = 31680 which still fits in signed integer. longs don't do any harm, but 
                                                                                                                                    // don't seem to be needed. show_arrays() is fucking up with air, but I don't think integer overflow is the problem                                                                                                                              
              if (times == NULL){ // malloc failure
                                
                sprintf(rwb_buf,"malloc test_read_new_file %s, times",fn);
                
                add_bad_news(rwb_buf);
      
                report_malloc_fail(times_malloc);
                
                stop_now = 1;
              }
      
              if (stop_now == 0){ // first test
                                    
                      need_free_times = 1;
              
                      int val_malloc = field2 * big_num;
      
                      values = (uint8_t *) malloc(val_malloc); // always one byte for 1 0 100 whatever                  
              
                      if (values == NULL){
                                        
                        sprintf(rwb_buf,"malloc test_read_new_file %s, values",fn);
                        
                        add_bad_news(rwb_buf);
          
                        report_malloc_fail(val_malloc);
          
                        free(times);
          
                        stop_now = 1;
                      } else {
      
                        report_malloc_result();
                      }
          
                      if (stop_now == 0){ // second test
                            
                            need_free_values = 1;
                            
                            num_recs = big_num; // filesize / (field1 + field2);
                                                      
                            sprintf(rwb_buf,"filesize %d big_num %d num_recs %d",filesize,big_num,num_recs); // try both %l and %ld - do they mean the same thing? %l did not print anything
                            add_fast_log(rwb_buf);   
                                          
                            sprintf(rwb_buf,"times malloc %d values malloc %d total %d",times_malloc, val_malloc, times_malloc + val_malloc); // try both %l and %ld - do they mean the same thing? %l did not print anything
                            add_fast_log(rwb_buf);   
                                              
                            for (int jj=0;jj<num_recs;jj++){          
                              
                              if (f.available() >= (field1 + field2)){
                              
                                num_read = f.readBytes(four,field1);
                    
                                if (num_read == field1){
                                  four[field1] = '\0';
                                  times[jj] = (uint32_t)strtol(four, NULL, 16);
                                }
                                
                                num_read = f.readBytes(two,field2);
                    
                                if (num_read == field2){
            
                                  two[field2] = '\0';
      
                                  values[jj] = (uint8_t)strtol(two, NULL, 16);  // always one byte 1 0 100 whatever                                          
                                }
                              }
                            }
                    
                            if (num_recs > 0){
                              
                              final_state = values[num_recs - 1]; // final state is uint8_t
                              final_time = times[num_recs - 1];
                            }
                      } // second test
      
              } // first test
          
          } // big_num > 0
          
        f.close();
      }
    }
  }

  if (num_recs > 0){
    
    sprintf(rwb_buf,"final time %08X, final state %02X",final_time,final_state); // try both %l and %ld - do they mean the same thing? %l did not print anything
    add_fast_log(rwb_buf);   

    
    sprintf(rwb_buf,"times final size of contents %d", num_recs * field1);
    add_fast_log(rwb_buf);   

    
    sprintf(rwb_buf,"values final size of contents %d", num_recs * field2);
    add_fast_log(rwb_buf);   
  }

  // don't free before showing the data   handle_malloc();
  
  //log_heap();
}



void prune_pulse_file(){     // file type_number 2

  char str32[100];
 
  char five[65];

  yesterday_gallons = 0;

  calc_yesterday_midnight_epoch();
  
  uint32_t today_midnight_epoch = (uint32_t) yesterday_midnight_epoch + 86400;
  
  int num_read = 0;

  uint32_t time_val;

  int good_cnt = 0;

  int pruned_cnt = 0;

  int stop_now = 0;

  if(LittleFS.begin()){
  
    if (LittleFS.exists("/pulse")){

      File f = LittleFS.open("/pulse","r");
     
      if (f){

        filesize = f.size();

        if (filesize > 0){
      
              times = (uint32_t *) malloc(filesize);
      
              if (times == NULL){ // malloc failure
                                
                sprintf(rwb_buf,"malloc prune_pulse_file, times");
                
                add_bad_news(rwb_buf);
      
                report_malloc_fail(filesize);
                
                stop_now = 1;
              }
      
              if (stop_now == 0){ // first test      
                    
                    need_free_times = 1;
            
                    num_recs = filesize / 8; // each record is 8 byte epoch
            
                    for (int jj=0;jj<num_recs;jj++){
                      
                      if (f.available() >= 8){
                      
                        num_read = f.readBytes(five,8);
            
                        if (num_read == 8){
                          
                          five[8] = '\0';
                          
                          time_val = (uint32_t)strtol(five, NULL, 16);
            
                          if ((time_val >= (uint32_t)yesterday_midnight_epoch) && (time_val < (uint32_t)today_midnight_epoch)){
            
                            yesterday_gallons++;
                          }
            
                          if (time_val > (uint32_t)my_epoch - 86400){    // no matter what time of day we are pruning, after gallons are counted we can discard all pulses more than 24 hours old
                            
                            times[good_cnt] = time_val; // epoch stays the same
            
                            good_cnt++;  
                          
                          } else {
            
                            pruned_cnt++;
                          }
                        }
                      }
                    }
            
                    if (yesterday_day_of_year > 0){
                      
                      create_new_dc_and_update_datecode_file();
                      
                      if (in_charge_online == 1){                                                
              
                        try_gallons_dream_already_acked = 0; 
              
                        if(dream_dead == 0){  // 0 is good. if it's dead, this is the epoch when it died
                          
                          try_gallons(dream_id);  // this sends the datecode string, so don't do this if yesterday_day_of_year is wrong (0)
                        }
                    
                      }
                    }
      
              } // first test
          
          } // filesize > 0
          
        f.close();
      }

      f = LittleFS.open("/pulse","w"); 
     
      if (f){

        for (int i=0; i<good_cnt; i++){ 

          sprintf(str32,"%08X",times[i]);
 
          f.print(str32);       
        }
        
        f.close();

        pulse_writes++;

        save_a_uint32_t((char *) "/pulse_wr",pulse_writes);

        sprintf(rwb_buf,"pruned %d pulses at %d secs",pruned_cnt,todays_seconds());   
        add_routine_log(rwb_buf);
        
      } 
    }
  }
  
  handle_malloc();
}





void prune_file_type(char * fn,int field2){   // field2 = 1 for things like trans which are one char 1 or 0, and 2 for tank and air which need 2 chars = one byte = 0..256

  /*
                    at midnight, I think, I prune data older than 24 hours, meaning that I start each day with 24 hours of data, and I add during the day, so 48 hours is stored just before midnight pruning.

                    so, how big can tank or air be? air stores 8 + 3 = 11 bytes per record. 48 x 60 x 11 = 31680. heap typically is 12000 - 13000 (yikes. after show files, one is stuck at 8000). 

                    so assuming I can fix that 8000 error, 12000 / 11 = 1090 records, or if I store 8 bits 12000 / 10 = 1200 records. 48 x 60 = 2880, so if i store every 3 minutes 48 x 60 / 3 = 960 records x 11 = 10560.

                    just store every 5 minutes, 8 bits. air is 20 to 40 anyway. tank is 100 max. don't go crazy. 48 x 60 / 5 * 10 bytes = 5760 bytes.
   */


  sprintf(rwb_buf,"pruning %s at %d secs",fn,todays_seconds());   
  add_routine_log(rwb_buf);

  int field1 = 8; 

  char str32[100];
 
  char two[20];
  char four[20];

  int num_read = 0;

  int good_one = 0;

  uint32_t time_val;

  int good_cnt = 0;

  int pruned_cnt = 0;

  int stop_now = 0;

  if(LittleFS.begin()){
  
    if (LittleFS.exists(fn)){

      File f = LittleFS.open(fn,"r");
     
      if (f){

        filesize = f.size();

        int big_num = filesize / (field1 + field2);

        if (big_num > 0){
      
              times = (uint32_t *) malloc(field1 * big_num); // filesize / (long)(field1 + field2)); // for air 8 * size / (8 + 3)      8 * 15840 = 126720  sparkfun says arduino int is 16 bits so I need to use longs here
      
              if (times == NULL){ // malloc failure
                                
                sprintf(rwb_buf,"malloc prune_file_type %s, times",fn);
                
                add_bad_news(rwb_buf);
      
                report_malloc_fail(field1 * big_num);
                
                stop_now = 1;
              }
      
              if (stop_now == 0){ // first test
                       
                  need_free_times = 1;
      
                  values = (uint8_t *) malloc(field2 * big_num);      // values is always one byte, whether it's trans 1 0 or tank 0..100 air 0..??              
      
                  if (values == NULL){
                      
                    sprintf(rwb_buf,"malloc prune_file_type %s, values",fn);
                    
                    add_bad_news(rwb_buf);
      
                    report_malloc_fail(field2 * big_num);
      
                    free(times);
      
                    stop_now = 1;
                  }
      
                  if (stop_now == 0){ // second test
                    
                      need_free_values = 1;
                      
                      num_recs = big_num; // filesize / (field1 + field2);  
                  
                      for (int jj=0;jj<num_recs;jj++){
                        
                        if (f.available() >= (field1 + field2)){
                        
                          num_read = f.readBytes(four,field1);
              
                          if (num_read == field1){
                            
                            four[field1] = '\0';
                            
                            time_val = (uint32_t)strtol(four, NULL, 16);
                            
                            if (time_val > ((uint32_t)my_epoch - 86400)){   // except for gallons count (pulses) which is midnight based, this prune operation is ok at any time of day - we never need more than the last 24 hours of flow, float, etc. 
                
                              good_one = 1; // once we get past the first one they should all be good, but whatever
                              
                              times[good_cnt] = time_val;
                              
                            }else{
              
                              pruned_cnt++;
                              
                              good_one = 0;
                            }
                          }
                          
                          num_read = f.readBytes(two,field2);
              
                          if (good_one == 1){
                            
                            if (num_read == field2){
                              
                              two[field2] = '\0';
      
                              values[good_cnt] = (uint8_t)strtol(two, NULL, 16); // always one byte, 0, 1, or 0..100 or whatever                                
                            }
              
                            good_cnt++;
                          }
                        }
                      }
                  } // second test
              } // first test 
        } // big_num > 0

        f.close();
      }

      f = LittleFS.open(fn,"w"); 
     
      if (f){

        for (int i=0; i<good_cnt; i++){  // need = 40 so skip 0..39. last one is 64 index 63

          switch(field2){                    
                   
            case 1:
              sprintf(str32,"%08X%01X",times[i],values[i]);   // simple states 0 and 1 use one character

              break;
              
            case 2:
              sprintf(str32,"%08X%02X",times[i],values[i]);   // air, tank 0..100d = 0x00..0x64 two chars

              break;
          }
 
          f.print(str32);       
        }
        
        f.close();

        sprintf(rwb_buf,"pruned %d",pruned_cnt);   
        add_routine_log(rwb_buf);
        
      } 
    }
  }

  handle_malloc();
}



void delete_file(){

  if(LittleFS.begin()){
  
    if (LittleFS.exists("/crap")){
    
      LittleFS.remove("/crap"); 
                        
    }

    
    if (LittleFS.exists("/pulse_set")){
    
      LittleFS.remove("/pulse_set"); 
                        
    }
  
  }
  
}



void write_tank_new_style(){      // file type_number 1

  // php saves milenium level milenium level... so do that here
  //
  // todays_seconds() is uint32_t which is 4 bytes, but the values here will never exceed 3 bytes - but wait. tank trans and flow will be saved as minutes

  if (last_new_style_tank == current_tank){
    return;
  }


//sprintf(rwb_buf,"last %02X cur %02X %d tpf %d",last_new_style_tank,current_tank,current_tank,trans_plus_flow);   
//               last 62 cur 64 100 tpf 1
//               0123456789012345678901234 56789
//add_log(rwb_buf);

  last_new_style_tank = current_tank;

  // simple_my_epoch();
  
  char str1[100];
  sprintf(str1,"%08X%02X",(uint32_t) my_epoch,current_tank); // tank is one byte %02X, which puts two characters in the file. max is 100d which is 0x64 and fits in one byte

  if(LittleFS.begin()){

    File f = LittleFS.open("/tank3_new","a"); 
                      
    if (f){
      
      f.print(str1);
      
      f.close();
    
      tank_writes++;

      save_a_uint32_t((char *) "/tank_wr",tank_writes);
    }
  }
  
}



void reset_air_new_style(){
  


  last_new_style_air = current_air;

  sprintf(rwb_buf,"%08X%02X reset the file",(uint32_t) my_epoch,current_air);
  add_routine_log(rwb_buf);
  
  char str1[100];
  sprintf(str1,"%08X%02X",(uint32_t) my_epoch,current_air);

  if(LittleFS.begin()){

    //LittleFS.remove("/air_new");  this is not needed - "w" will restart the file

    File f = LittleFS.open("/air_new","w");     // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< write instead of append since file was filling up with garbage
                      
    if (f){
      
      f.print(str1);
      
      f.close();
    
      air_writes++;

      save_a_uint32_t((char *) "/air_wr",air_writes);
    }
  }
  
}





void write_air_new_style(){      // file type_number 1

  // php saves milenium level milenium level... so do that here
  //
  // todays_seconds() is uint32_t which is 4 bytes, but the values here will never exceed 3 bytes - but wait. tank trans and flow will be saved as minutes

  if (last_new_style_air == current_air){
    return;
  }


  last_new_style_air = current_air;

//  sprintf(rwb_buf,"%08X%02X",my_epoch,current_air);
//  add_routine_log(rwb_buf);
  
  char str1[100];
  sprintf(str1,"%08X%02X",(uint32_t) my_epoch,current_air);    // air is maximum of 3 characters 0x3FF so %03X is correct here.

  if(LittleFS.begin()){

    File f = LittleFS.open("/air_new","a"); 
                      
    if (f){
      
      f.print(str1);
      
      f.close();
    
      air_writes++;

      save_a_uint32_t((char *) "/air_wr",air_writes);
    }
  }
  
}




void fix_float_file(){  

  char str1[100];
  
  // simple_my_epoch();
  
  sprintf(str1,"%08X%01X",(uint32_t) my_epoch - 3600,1);

  if(LittleFS.begin()){

    File f = LittleFS.open("/float","w");     //<<<<<<<<<<<<< use "w" to erase the old contents

    if (f){
      
      f.print(str1);
      
      f.close();
    }
  } 
}



void write_type_3(const char fn[50], uint8_t state){      // file type_number 3    state is 1 or 0

  char str1[100];
  
  sprintf(str1,"%08X%01X",(uint32_t) my_epoch,state);    // 8-27-2022 I think this writes 1 hex character for state, like 1 or 0, not 01 00
  
  if(LittleFS.begin()){

    File f = LittleFS.open(fn,"a"); 

    if (f){
      
      f.print(str1);
      
      f.close();
    }
  } 
}


void write_pulse_new_style(uint32_t epoch){ 

  char str1[100];

  sprintf(str1,"%08X",epoch); 
  
  if(LittleFS.begin()){

    File f = LittleFS.open("/pulse","a"); 
                      
    if (f){
      
      f.print(str1);
      
      f.close();
      
        pulse_writes++;

        save_a_uint32_t((char *) "/pulse_wr",pulse_writes);
    }
  }
}


/* 
   
    6-9-2023  old version before I tried to "fix" missed pulses. how are they missed? most likely a reset? since box 4 never resets while testing the ultrasonic depth probe, why not do that first (eliminate RS check in shed box).
              it's hard to imagine any other way that pulses are missed that would not cause it to happen more frequently. typically it is several days without a miss. at any rate this is the plan if they are still missed:

                  keep track of last two pulse epoch. calc the gpm for those two. calc the gpm for this one. if this one is about 1/2 gpm of the previous one, add an epoch halfway and save both. set a flag to not do this test the 
                  next time because the actual flow might have simply dropped. in that case, this "fix" will delay it a bit and add 100 gallons incorrectly. ideally I would wait for the next pulse to avoid this, but I'm not sure
                  i can stuff the extra pulse in at a later time without having to add a php mess. why not try the RS fix first, since the shed box has been resetting several times per day. ok, compare the reset times

                    the missed pulse was at 2 pm yesterday

                    recent resets per bad news log:
                                            
                        00:00:00                      midnight at start of yesterday
                        master resets old 37 new 38
                        06:56:35
                        watchdog reset
                        06:56:35
                        master resets old 38 new 39
                        07:19:09
                        watchdog reset
                        07:19:09
                        bad cks 2
                        11:06:06
                        bad ck
                        11:06:06
                        bad ck
                        13:03:04
                        master resets old 39 new 40
                        13:03:08                      13:03 is 1:02 so spring ahead 2:03 pm <<<<<<<<<<<<<<<< this looks suspicious. try the RS fix before changing anything here
                        watchdog reset
                        13:03:08
                        bad cks 2
                        15:57:56
                        bad ck
                        15:57:56
                        bad ck
                        17:49:02
                        master resets old 40 new 41   21:24 is 9:24 EST but I'm on DST so 10:24 (spring ahead, fall back)
                        21:24:33
                        watchdog reset
                        21:24:33
                        dream died
                        23:17:59
                        dream_born
                        23:20:17                      midnight at start of today
 
void write_pulse_new_style(uint32_t epoch){ 

  char str1[100];

  sprintf(str1,"%08X",epoch); 
  
  if(LittleFS.begin()){

    File f = LittleFS.open("/pulse","a"); 
                      
    if (f){
      
      f.print(str1);
      
      f.close();
      
        pulse_writes++;

        save_a_uint32_t((char *) "/pulse_wr",pulse_writes);
    }
  }
}

*/


void read_dir(){

  if(!LittleFS.begin()){

    return;
  }
  
  Dir dir = LittleFS.openDir("/");

  int size1;
  
  strings_found = 0;

  file_tries++;

  while (dir.next()){

    if (strings_found < maxfiles){

      yield();
     
      File f = dir.openFile("r");
  
      if (f){
 
        size1 = f.size();
        
        sprintf(filenames[strings_found],"%s %d",dir.fileName().c_str(),f.size()); // this string has to be less than filestringsize. worst one so far is tank3_new 1596   which is 12 chars so put 18 as limit
                    
        strings_found++;

        if (strings_found >= maxfiles){
          
          sprintf(rwb_buf,"filenames array full"); 
          add_log(rwb_buf);
        }
    
        if (size1 < (filestringsize - 5)){
          
          read_file(f);
        }
      }
    }
  }
}



void read_file(File f){

    if (f){

      String s = f.readStringUntil('\n');
      strcpy(filenames[strings_found],s.c_str());
      strings_found++;
      
      f.close();
    }  
}



/*

Second, there is a limit of 32 chars in total for filenames. One '\0' char is reserved for C string termination, so that leaves us with 31 usable characters.

Combined, that means it is advised to keep filenames short and not use deeply nested directories, as the full path of each file (including directories, '/' characters, base name, dot and extension) has to be 31 chars at a 
maximum. For example, the filename /website/images/bird_thumbnail.jpg is 34 chars and will cause some problems if used, for example in exists() or in case another file starts with the same first 31 characters.



* spiffs can only write to free pages
* pages in a block become free when a block is erased
* spiffs must always have two blocks of free sectors

my info says block is 8192 page is 256 so that's 32 pages in a block.

my settings file with 16 bit and 32 bit reset counts used 1004 bytes, but that might include other overhead. I need to save some simulated data files, like daily files and see what happens. also, see how
crazy it is if I save a daily file 288 times like I plan on. 




The bad news is that LittleFS has been deprecated by Espressif (the designers of ESP8266/ESP32) because it was not resilient enough. And relatively slow, too.

The replacement LITTLEFS is even power-interruption resilient and the File System will not be corrupted if the power disappears during a write operation. Allegedly. Even if one file gets corrupted, the rest are safe.

Espressif state: "LittleFS is the original filesystem and is ideal for space and RAM constrained applications that utilize many small files and care about static and dynamic wear levelling and don’t 
need true directory support. Filesystem overhead on the flash is minimal as well."

The also say: "LittleFS is recently added and focuses on higher performance and directory support, but has higher filesystem and per-file overhead (4K minimum vs. LittleFS’ 256 byte minimum file allocation unit)."

So for memory-lean ESP8266s you might want to stick with LittleFS but for newer chips with a larger memory use the LITTLEFS implementation. Note that you CANNOT mix-and-match.

shit. which should I use. switch to littleFS and forget about it. that line about spiffs wear leveling is worrisome - does it imply that littleFS sucks at that? anyway, it looked like I had plenty of space in LittleFS, so littleFS should be ok...

LittleFS with settings file and the formatcomplete file: total 3121152 used 16384   unused 3104768     blocksize 8192    page 256   unused blocks 379

write a dummy file containing this string "asasdfffASASDFASF12345". no change to stats. wtf. print the dir list



dummy size 22  (hmm, file has 22 chars)
file open for read ok
asasdfffASASDFASF12345


formatComplete.txt size 10 (file has 9 chars?)
file open for read ok
some text


settings size 9
file open for read ok
147 147 


Opens a file. path should be an absolute path starting with a slash (e.g. /dir/filename.txt). mode is a string specifying access mode. It can be one of “r”, “w”, “a”, “r+”, “w+”, “a+”. Meaning of these modes is the same as for fopen C function.

r      Open text file for reading.  The stream is positioned at the
       beginning of the file.

r+     Open for reading and writing.  The stream is positioned at the
       beginning of the file.

w      Truncate file to zero length or create text file for writing.
       The stream is positioned at the beginning of the file.

w+     Open for reading and writing.  The file is created if it does
       not exist, otherwise it is truncated.  The stream is
       positioned at the beginning of the file.

a      Open for appending (writing at end of file).  The file is
       created if it does not exist.  The stream is positioned at the
       end of the file.

a+     Open for reading and appending (writing at end of file).  The
       file is created if it does not exist.  The initial file
       position for reading is at the beginning of the file, but
       output is always appended to the end of the file.



2-7-2021 using LittleFS I appended a lot to dummy file, maybe 180 chars. the web page showed some errors, probably because a buffer was overflowed. and then it wrecked everything. i removed all file ops and it still would not flash and run. I erased all 
the flash and it still would not load and run. but the original file in esp_server_3 did load and run, so I'm back here. I don't think this incriminates LittleFS yet, because buffer overflows are stupid


LittleFS.rename(oldname,newname);

*/

void rename_dummy_file(){

  
  String s = " MORE";
  
  if(LittleFS.begin()){
  
    if (LittleFS.exists("/dummy")){

      LittleFS.rename("/dummy","/smarty");
      
    } else if (LittleFS.exists("/smarty")){
      
      LittleFS.rename("/smarty","/dummy");
    }
  }
}


void append_dummy_file(){

  String s = " MORE";
  
  if(LittleFS.begin()){

    File f = LittleFS.open("/dummy","a");  
                      //  0123456789012345678901
    //FILE * fp = (FILE *) &f;

    if (f){
 
      char buf[300];

      strcpy(buf,s.c_str());
      
      f.print(buf);
      
      f.close();
    }
  } 
}


void write_dummy_file(){


  String s = "asasdfffASASDFASF12345";

  
  if(LittleFS.begin()){


    File f = LittleFS.open("/dummy","w");  
                      //  0123456789012345678901

    //FILE * fp = (FILE *) &f;

    if (f){
 
      char buf[300];

      strcpy(buf,s.c_str());
      
      f.print(buf);
      
      f.close();
    }
  }
}



void check_spiffs_format(){
       
  if(LittleFS.begin()){

    if (!LittleFS.exists("/settings")){

      format_fs();
    }
  }
}



void format_fs(){
     
  if(LittleFS.begin()){
        
    LittleFS.format();

    save_settings();
  }
}



uint32_t read_a_uint32_t(char * fn){

  if(LittleFS.begin()){
    
    if (LittleFS.exists(fn)){
    
      File f = LittleFS.open(fn,"r");
  
      if (f){
          
        String s = f.readStringUntil('\n');
        
        return atoi(s.c_str());
        
        f.close();
      }
    }
  }  
  return 0;
}


void save_a_uint32_t(char * fn,uint32_t val){
  
  if(LittleFS.begin()){

    File f = LittleFS.open(fn,"w");
                     
    if (f){
 
      char buf[30];
      sprintf(buf,"%u\n",val); // 9-9-2022 this was %d but i think 4 bytes is long in arduino
      
      f.print(buf);
      
      f.close();
    }
  }
}


void save_settings(){
  
  if(LittleFS.begin()){

    File f = LittleFS.open("/settings","w");  // avoid long filenames -see warning above. in this case, the so-called directory is not a real directory and is just stupid. <- true for spiffs, but LittleFS has actual directories which I probably won't use
                      //  0123456789012345678901
    if (f){
 
      char buf[300];
      sprintf(buf,"%i %u \n",reset_cnt,reset_cnt_32); // needs final " " or app says not an integer.  these are uint16_t and uint32_t
      /*
      %hu Unsigned Integer (short)                    uint8_t? 
      
      %i  Unsigned integer                            uint16_t
      
      %l or %ld or %li  Long
      %lf Double
      %Lf Long double
      
      %lu Unsigned int or unsigned long               uint32_t
      */
      f.print(buf);
      
      f.close();
    }

    read_reset_cnt();
  }
}


void read_reset_cnt(){


      File f = LittleFS.open("/settings","r");  // r w a r+ w+ a+ 

      if (f){
        
        String s = f.readStringUntil('\n');

        f.close();

        parse_settings(s);
      }  
}

void parse_settings(String s){

  int cnt = 0;
  
  char * paych = strdup(&s[0]); // strdup copies pay, and needs to be freed when done
   
  char * save_paych = paych; // save so i can free, which requires original pointer value, and strsep alters paych

  char * token;

  //uint16_t data;

  if (strlen(paych) > 0){

    token = strsep(&paych," ");

    while (strlen(token) > 0){


      yield();

      uint16_t u16;
      uint32_t u32;
      //uint8_t u8;
      
      switch(cnt){
        case 0:
        
          u16 = get_uint16(token);

          reset_cnt = u16;

          break;

          
        case 1:
        
          u32 = get_uint32(token);

          reset_cnt_32 = u32;

          break;

      }
      cnt++;
      
      token = strsep(&paych," ");

      if ((token) == NULL){ //"\0"){

        break;
      }
      
    }
  }

  free(save_paych);
}


void try_to_read_reset_cnt_file(){
  
  if(LittleFS.begin()){

  } else {

   

    return;
  }

  read_reset_cnt();
  
}

/*
 * 

FSInfo fs_info;
LittleFS.info(fs_info);
or LittleFS.info(fs_info);

Fills FSInfo structure with information about the file system. Returns true if successful, false otherwise.
Filesystem information structure

struct FSInfo {
    size_t totalBytes;
    size_t usedBytes;
    size_t blockSize;
    size_t pageSize;
    size_t maxOpenFiles;
    size_t maxPathLength;
};


 */

void get_spiffs_info(){

  FSInfo fs_info;


  if(LittleFS.begin()){

  } else {

    

    return;
  }
  
  LittleFS.info(fs_info);

  spiffs_total = fs_info.totalBytes;
  spiffs_used = fs_info.usedBytes;
  spiffs_block = fs_info.blockSize;
  spiffs_page = fs_info.pageSize;
  
//spiffs total bytes 3121152 used 98304 unused 3022848 blocksize 8192 pagesize 256 unused blocks 369 records 229 size 1374        this was 2-14-2021 on LittleFS

  
}


uint16_t get_uint8(char * token){

  uint8_t ret = atoi(token); 

  return ret;
  
}

uint16_t get_uint16(char * token){

  uint16_t ret = atoi(token); 

  return ret;
  
}
//(int)strtol(hexstring, NULL, 16);



uint32_t get_uint32(char * token){

  uint32_t ret = atoi(token); 

  return ret;
  
}


uint32_t get_uint32_from_hex(char * token){

  uint32_t ret = (uint32_t)strtol(token,NULL,16); 

  return ret;
  
}



uint8_t get_uint8_from_hex(char * token){

  uint8_t ret = (uint8_t)strtol(token,NULL,16); 

  return ret;
  
}
