/*
    i compared php biz values and they were all within 2 seconds. this is the url sends event and php uses their own timestamp. i did this in a spreasheet (/mtv/fake_home/timestamps.ods) using this formula:

      = (CO20-5*3600)/(60*60*24) + 25569  where C020 is the unix timestamp

      and then format date and select the option that shows the whole date and time. alternatively i could have figured out a way to convert my local shit back to unix epoch, or simply go back to the old code that
      dumps files raw, but this was quick and works



      dead ack policy:

        keep a flag biz_ok and dream_ok

        if biz does not ack an event set the flag and save the epoch, but reduce the epoch to make sure it is before the event timestamp that has to be re-sent.

          save this epoch in a file so it will tell me on reset that events were missed

          don't send any new events to biz

          every ?? minutes send something to test biz. do not use ping for this in case one slips through because it will update the site to think data is fresh

          when biz appears to be alive send blocks for everything but limited to events that happened after the last good ack

            or if the code is simple, send one or two events prior to the dead ack just to make sure, and this simplifies any worries about what exact epoch to use

            if biz acks all the blocks and appears to be updated, change the file to make it clear that all is ok. like set the epoch value to 5.

          on reset check that file to see what mode we are in, regular events or waiting for site to come back to life

          gallons are a bit different? or maybe not. if biz does not ack gallons after second try, set biz to dead and do the normal procedure described here. when a site comes back online send the gallons string along with
          any event blocks. it's simpler to just do this than to try to have separate code for gallons. a dead site is a dead site. 

          maybe on all http sends that fail, pause a tiny bit and resend a few times before deciding a site is dead. or not, because the above plan to only send blocks that start after the site died will not
          send much at all if the site is only down for a brief period.



  GET return data

     HTTPClient http;

      String serverPath = serverName + "?temperature=24.37";
      
      http.begin(serverPath.c_str());
      
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
     
        String payload = http.getString();

        
        or this? nah, this is a user function that calls http.getString()...String sensorReadings = httpGETRequest(serverName);
        
     } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      
      http.end();

      
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);



            // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {

        // get lenght of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();

        // create buffer for read
        uint8_t buff[128] = { 0 };

#if 0
        // with API
        Serial.println(http.getString());
#else
        // or "by hand"

        // get tcp stream
        WiFiClient * stream = &client;

        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // read up to 128 byte
          int c = stream->readBytes(buff, std::min((size_t)len, sizeof(buff)));
          Serial.printf("readBytes: %d\n", c);
          if (!c) {
            Serial.println("read timeout");
          }

          // write it to Serial
          Serial.write(buff, c);

          if (len > 0) {
            len -= c;
          }
        }
#endif

        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

   discussing another problem, guy said this: If this code is running every 20 seconds why stop and restart the HTTP connection? You could put the http.begin in the start loop and just check there is still an http connection in the loop if you wanted. 


Status Code   Meaning
200   OK: the request was successful
303   See Other: used to redirect to a different URI, after a POST request, for instance
400   Bad Request: the server couldn't understand the request, because the syntax was incorrect
401   Unauthorized: user authentication is required
403   Forbidden: the server refuses to execute the request, authorization won't help
404   Not Found: the requested URI was not found
500   Internal Server Error: The server encountered an unexpected condition and couldn't fulfill the request


    
    const char* dream_host = "http://www.fork20.xyz/birchwood/";   //http://www.fork20.xyz/birchwood/water2_new_fake.php
    
    const char* x10 = "x10_test_esp8266/";
    
    400 Bad Request
        The server cannot or will not process the request due to an apparent client error (e.g., malformed request syntax, size too large, invalid request message framing, or deceptive request routing).[31]
    401 Unauthorized (RFC 7235)
        Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided. The response must include a WWW-Authenticate header field containing a challenge applicable to the
        requested resource. See Basic access authentication and Digest access authentication.[32] 401 semantically means "unauthorised",[33] the user does not have valid authentication credentials for the target resource.
        Note: Some sites incorrectly issue HTTP 401 when an IP address is banned from the website (usually the website domain) and that specific address is refused permission to access a website.[citation needed]
    402 Payment Required
        Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, as proposed, for example, by GNU Taler,[34] but that has not yet 
        happened, and this code is not widely used. Google Developers API uses this status if a particular developer has exceeded the daily limit on requests.[35] Sipgate uses this code if an account does not have 
        sufficient funds to start a call.[36] Shopify uses this code when the store has not paid their fees and is temporarily disabled.[37] Stripe uses this code for failed payments where parameters were correct, for example blocked fraudulent payments.[38]
    403 Forbidden
        The request contained valid data and was understood by the server, but the server is refusing action. This may be due to the user not having the necessary permissions for a resource or needing an account of 
        some sort, or attempting a prohibited action (e.g. creating a duplicate record where only one is allowed). This code is also typically used if the request provided authentication by answering the WWW-Authenticate 
        header field challenge, but the server did not accept that authentication. The request should not be repeated.
    404 Not Found
        The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible.


*/

void ask_both_sites_what_dst_flag_is(){

        //const char* read_dst = "read_dst_flag.php"; // look at plots to confirm this
        
        read_php_number(dream_host,read_dst,1,0); // the last two args: type 0 normal dir, type 1 x10 dir     log_flag = 1 to log result

        sprintf(rwb_buf,"dream says dst flag is %d",php_read_32);
        add_log(rwb_buf);
        
//        read_php_number(biz_host,read_dst,1,0);

  //      sprintf(rwb_buf,"biz says dst flag is %d",php_read_32);
    //    add_log(rwb_buf);
}


int get_both_server_millens(){

  int ret;

  int main_ret = 0;
    
  ret = ask_host_for_current_millenium(dream_id);
  
  if (ret == 1){

    dream_dead = 0; // this is for setup code which resets wifi for reasons I don't entirely understand (if online not enabled)
    save_a_uint32_t((char *) "/dream_status",dream_dead);

    locked_dream = php_read_32 + (uint32_t) utcOffsetInSeconds;;

    my_epoch = locked_dream; // 11-7-2021 if dream works then that's the time i want to use, always, so go ahead and sync my_epoch

    sync_rtc_to_epoch(&locked_dream); // 11-7-2021 why not

    main_ret = 1; // use dream to set my_epoch if rtc is dead
  } else {

    add_bad_news((char *) "failed to get dream epoch");
  }

 

  return main_ret;
}


int write_php_number(const char * host,const char * php, uint8_t type,int value){ // type 0 normal dir, type 1 x10 dir
    
//    if (in_charge_online != 1){
  //    return 0;
  //  }
    
    if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;
      WiFiClient client;

      if (type == 0){

        sprintf(rwb_buf,"%s%scutoff=%d",host,php,value);   // hmm, should unsigned use u instead of d. AND REMEMBER SPACES ARE NOT ALLOWED IN URL
      } else {

        sprintf(rwb_buf,"%s%s%scutoff=%d",host,x10,php,value);   // hmm, should unsigned use u instead of d. AND REMEMBER SPACES ARE NOT ALLOWED IN URL  
      }
       
      String httpRequestData = rwb_buf;           
      
      http.begin(client,httpRequestData);    
      
      int httpResponseCode = http.GET();

      http.end();

      if (type == 0){
        sprintf(rwb_buf,"%s %s  val %d ack %d",host,php,value,httpResponseCode);  
      } else {
        sprintf(rwb_buf,"%s %s  dir %s  val %d ack %d",host,php,x10,value,httpResponseCode);
      }

      add_log(rwb_buf);
      
              
      if (httpResponseCode == 200){

        return 1;  // good ACK DON'T CHANGE THIS UNLESS YOU CHANGE all the places that use this to re-send the packet
             
      } else {
        
        return 0; // bad ACK
      }
      
      
      
    }else {
       return 2; // no wifi
    } 
}


int read_php_number(const char * host,const char * php,uint8_t type, uint8_t log_flag){ // type 0 normal dir, type 1 x10 dir    i confirm various things by looking at web pages; dst and flow cutoff, and source select don't need to be read explicitly.
                                                                                        // log_flag = 1 to log result
    int ret = 0;
  
    if(WiFi.status()== WL_CONNECTED){
      
      HTTPClient http;
      WiFiClient client;
 
      char rwb_buf2[400];

      if (type == 1){ // in x10 dir
        
        sprintf(rwb_buf2,"%s%s%s",host,x10,php);   // SPACES ARE NOT ALLOWED IN URL  
      
      } else { // in normal dir
        
        sprintf(rwb_buf2,"%s%s",host,php);   // SPACES ARE NOT ALLOWED IN URL    
      }
      
      String httpRequestData = rwb_buf2;   

      latency_millis = millis();    // this was added for read_millenium to see how long it takes to get the reply
      
      http.begin(client,httpRequestData);    
      
      int httpResponseCode = http.GET();

      if (httpResponseCode == 200){
          
      //    int len = http.getSize(); this is always -1
    
          String echo = http.getString();     

          latency_millis = millis() - latency_millis;
    
          sprintf(rwb_buf,"GET %s",echo.c_str());
    
          if ((strlen(rwb_buf) > 0) && (strlen(rwb_buf) < 15)){ // be careful logging this string. i think it crashed esp once. also it can send html crap which is cool but no. limit used to be 10 chars but i need more to read millenium

            if (log_flag == 1){
    
              add_log(rwb_buf);
            }

            sprintf(rwb_buf,"%s",echo.c_str());
    
            php_read_32 = get_uint32(rwb_buf);

            ret = 1;
          }
             
          http.end();
     
          return ret;

      } else {

        http.end();

        return 0;
      }
    }

    return 0;

  
}


int send_flow_adc(int hostnum){

//    if (in_charge_online != 1){     // allow any device to do this, since it's always manually via button click
//      return 0;
//    }
    
    if(WiFi.status()== WL_CONNECTED){

      prepare_flow_adc(); // puts 1440 hex values in block[] separated by spaces
      
      HTTPClient http;
      WiFiClient client;
 
      char rwb_buf2[400];
     
      if (hostnum == dream_id){
        
        sprintf(rwb_buf2,"%s%s%s",dream_host,x10,"flow_adc_block.php");   // as of 5-20-2021 i only use dream for rpi temperature and adc flow plots
      
      }
      
      add_log(rwb_buf2);
           
      http.begin(client,rwb_buf2);
      
      http.addHeader("Content-Type", "text/plain"); // i think this applies to me, and i think it means just send it, and don't try to mess with name-value pairs
  
      String httpRequestData = block;           

      sprintf(rwb_buf,"http string len %d block len %d",strlen(httpRequestData.c_str()),strlen(block));
      add_log(rwb_buf);


      // it keeps logging the old block length when prepare block finds nothing, so...
      
      block[0] ='\0';
   
      int httpResponseCode = http.POST(httpRequestData);  

      http.end();

      sprintf(rwb_buf,"send flow adc ack %d",httpResponseCode);
      add_log(rwb_buf);
         
      
 
      if (httpResponseCode == 200){

         return 1;

      } else {

        return 0;
      }
    }

    return 0;
}


int send_rpi_temp(int hostnum){

//    if (in_charge_online != 1){     // allow any device to do this, since it's always manually via button click
//      return 0;
//    }
    
    if(WiFi.status()== WL_CONNECTED){

      prepare_rpi_temp(); // puts 366 hex values in block[] separated by spaces
      
      HTTPClient http;
      WiFiClient client;
 
      char rwb_buf2[400];
     
      if (hostnum == dream_id){
        
        sprintf(rwb_buf2,"%s%s%s",dream_host,x10,"rpi_temp_block.php");   // as of 5-20-2021 i only use dream for rpi temperature and adc flow plots
      



      }
      
      add_log(rwb_buf2);
           

      
      http.begin(client,rwb_buf2);
      
      http.addHeader("Content-Type", "text/plain"); // i think this applies to me, and i think it means just send it, and don't try to mess with name-value pairs
  
      String httpRequestData = block;           

      sprintf(rwb_buf,"http string len %d block len %d",strlen(httpRequestData.c_str()),strlen(block));
      add_log(rwb_buf);


      // it keeps logging the old block length when prepare block finds nothing, so...
      
      block[0] ='\0';
   
      int httpResponseCode = http.POST(httpRequestData);  

      http.end();

      sprintf(rwb_buf,"send rpi temperature ack %d",httpResponseCode);
      add_log(rwb_buf);
         
      
 
      if (httpResponseCode == 200){

         return 1;

      } else {

        return 0;
      }
    }

    return 0;
    
}


int send_xbee_RS(int hostnum){

//    if (in_charge_online != 1){     // allow any device to do this, since it's always manually via button click
//      return 0;
//    }
    
    if(WiFi.status()== WL_CONNECTED){

      prepare_xbee_RS(); // puts 366 hex values in block[] separated by spaces
      
      HTTPClient http;
      WiFiClient client;
 
      char rwb_buf2[400];
     
      if (hostnum == dream_id){
        
        sprintf(rwb_buf2,"%s%s%s",dream_host,x10,"xbee_RS_block.php");   // as of 5-20-2021 i only use dream for rpi temperature and adc flow plots and xbee_RS
      

      }
      
      add_log(rwb_buf2);
           
      http.begin(client,rwb_buf2);
      
      http.addHeader("Content-Type", "text/plain"); // i think this applies to me, and i think it means just send it, and don't try to mess with name-value pairs
  
      String httpRequestData = block;           

      sprintf(rwb_buf,"http string len %d block len %d",strlen(httpRequestData.c_str()),strlen(block));
      add_log(rwb_buf);

      // it keeps logging the old block length when prepare block finds nothing, so...
      
      block[0] ='\0';
   
      int httpResponseCode = http.POST(httpRequestData);  

      http.end();

      sprintf(rwb_buf,"send xbee RS ack %d",httpResponseCode);
      add_log(rwb_buf);
 
      if (httpResponseCode == 200){

         return 1;

      } else {

        return 0;
      }
    }

    return 0;
}



int send_block(const char * php,const char * host){

    if (in_charge_online != 1){
      return 0;
    }
    
  //add_log((char *) "send_block");

    if(WiFi.status()== WL_CONNECTED){
      
      HTTPClient http;
      WiFiClient client;

      char rwb_buf2[400];
      
      sprintf(rwb_buf2,"%s%s%s",host,x10,php);   // SPACES ARE NOT ALLOWED IN URL

      if (verbose1) add_log(rwb_buf2);
           
      http.begin(client,rwb_buf2);
      
      http.addHeader("Content-Type", "text/plain"); // i think this applies to me, and i think it means just send it, and don't try to mess with name-value pairs
  
      String httpRequestData = block;           

//      sprintf(rwb_buf,"http string len %d block len %d",strlen(httpRequestData.c_str()),strlen(block));
//      add_log(rwb_buf);


      // it keeps logging the old block length when prepare block finds nothing, so...
      
      // how can send_block repeat ever work with this? find some other solution block[0] ='\0';
   
      int httpResponseCode = http.POST(httpRequestData);  

      http.end();

      if (verbose1) {
        sprintf(rwb_buf,"send_block ack %d",httpResponseCode);
        add_log(rwb_buf);
      }
         
      
 
      if (httpResponseCode == 200){

         return 1;

      } else {

        return 0;
      }
    }

    return 0;
}




int send_gallons(const char * host){

    if (in_charge_online != 1){
      return 0;
    }
    
  char rwb_buf2[350]; // keep this one. rwb_buf is used here 
 
  if(WiFi.status()== WL_CONNECTED){

    add_fast_log((char *) "send_gallons");
      
      HTTPClient http;
      WiFiClient client;
    
      //read string from file. no need to add datecode here. that was done in prune pulse once per day, and this code might be called repeatedly if bad ack
      
      read_datecode_file(rwb_buf);  // the file should have been created before this function was called

      add_routine_log(rwb_buf);

      sprintf(rwb_buf2,"%s%sgallons_new.php?g=%s",host,x10,rwb_buf);   // hmm, should unsigned use u instead of d. AND REMEMBER SPACES ARE NOT ALLOWED IN URL
      
      String httpRequestData = rwb_buf2;           
      
      http.begin(client,httpRequestData);    
      
      int httpResponseCode = http.GET();

      http.end();
      
      sprintf(rwb_buf,"send gallons ack %d",httpResponseCode);   
      add_routine_log(rwb_buf);
      log_routine_time();
      
      if (httpResponseCode == 200){
      
        return 1;  // good ACK DON'T CHANGE THIS UNLESS YOU CHANGE all the places that use this to re-send the packet
      
      } else {
  
        sprintf(rwb_buf,"bad ack on send gallons %d",httpResponseCode);  
        add_bad_news(rwb_buf);
      
        return 0; // bad ACK
      }
      
      
      
  } else {
    
    sprintf(rwb_buf,"fail to send gallons, no wifi"); 
    add_log(rwb_buf);

    return 2; // no wifi
  } 
}







int send_change(int state,const char * php,const char * host){
    
    if (in_charge_online != 1){
      return 0;
    }
    
    if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;
      WiFiClient client;

      

      sprintf(rwb_buf,"%s%s%sstate=%d",host,x10,php,state);   // hmm, should unsigned use u instead of d. AND REMEMBER SPACES ARE NOT ALLOWED IN URL
       
      String httpRequestData = rwb_buf;           
      
      http.begin(client,httpRequestData);    
      
      int httpResponseCode = http.GET();

      http.end();
              
      if (httpResponseCode == 200){

//        ping_web = 0;
        return 1;  // good ACK DON'T CHANGE THIS UNLESS YOU CHANGE all the places that use this to re-send the packet

         
             
      } else {
        
        return 0; // bad ACK
      }
      
      
      
    }else {
       return 2; // no wifi
    } 
}





int new_ping(const char * host){

    if (in_charge_online != 1){
      return 0;
    }
    
    if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;
      WiFiClient client;

      sprintf(rwb_buf,"%s%snew_ping.php",host,x10);   
       
      String httpRequestData = rwb_buf;           
      
      http.begin(client,httpRequestData);    
      
      int new_ping_ack = http.GET();

      http.end();
              
      if (new_ping_ack == 200){
        
        
        return 1;  // good ACK
             
      } else {
        
        return 0; // bad ACK
      }
      
      
      
    } else {
      
       return 2; // no wifi
    }   
}

int ping_the_web(const char * host){

    if (in_charge_online != 1){
      return 0;
    }
    
    if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;
      WiFiClient client;

      sprintf(rwb_buf,"%s%sping.php",host,x10);   
       
      String httpRequestData = rwb_buf;           
      
      http.begin(client,httpRequestData);    
      
      int httpResponseCode = http.GET();

      http.end();
              
      if (httpResponseCode == 200){
        
        return 1;  // good ACK
             
      } else {
        
        return 0; // bad ACK
      }
      
      
      
    } else {
       return 2; // no wifi
    }   
}
