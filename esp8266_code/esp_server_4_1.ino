
#include "LittleFS.h" 

#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>

#include <ESP8266HTTPClient.h>

//#include <NTPClient.h>

#include <WiFiUdp.h>

#include <DS3231.h>  // this includes wire.h

#include <Wire.h> // rtc fails most of the time so put this back in. lol.


bool verbose1 = true; // original purpose was to continue logging all block sends except skip normal 0x15 which always sends a small block since i need to send epoch for accurate gpm 

int bad_ln,bad_actual_ln;

int check_server_millens_state = 2;


uint32_t php_read_32;

uint32_t gals2[10];
uint32_t years2[10];

#define dream_id 1
//#define biz_id 2

//int new_ping_ack;

uint32_t dream_new_ping_cnt,bad_dream_cnt, xbee_reset_cnt, restart_wifi_cnt,  the_millis, bad_log_cnt,last_master_rtc_sync;

time_t  last_good_dream_epoch, last_bad_dream_epoch, test_dst_epoch;

#define blocksize 4400  // flow adc is 1440 x 3 = 4320. maybe it could be smaller. air calc in prepare block says 4032 if air is saved every 5 mins BUT THAT FORGOT that i use delta after 
                        // the first data. so 4400 is huge for all those blocks.

char block[blocksize]; // used to send blocks to php. stupid waste of ram, but whatever. "normal" code crashed a million different ways when i tried to send this as an argument. smaller
                        // char arrays seem safe; 100 didnt crash like the big one.
                       // but why wont a pointer and malloc work? did I try that? that wouldnt be an argument.

#define log22_size 2000 // was 6500 and esp207 hung. so i put yields in the prep code, and reduced this a lot. im thinking about having several small logs instead of this huge one

#define bad_news_log_size 500
 // 11-26-2023 i got tired of scrolling every day

#define routine_log_size 2000   

//int secs_ntp_dead = 0;

uint8_t air_block = 0; 

char routine_log[routine_log_size]; // if name of size is changed, also fix show_a_log()

char log22[log22_size];  // if name of size is changed, also fix show_a_log()

char bad_news[bad_news_log_size];  // if name of size is changed, also fix show_a_log()

char fast_log[log22_size];  // if name of size is changed, also fix show_a_log()

//uint8_t first_ntp = 1;

#define xbee_histo_size 40

uint16_t xbee_histo[xbee_histo_size + 1]; // save a count of RS for values 15..54         this just saves 0xAA RS. no other packets are checked except 0x39

uint16_t xbee_histo_39[xbee_histo_size + 1]; // save a count of RS for values 15..54      0x39 is sewer

uint8_t my_ip;

uint8_t temporary_flow_cutoff,try_gallons_biz_already_acked,try_gallons_dream_already_acked,clock_state;

uint8_t routine_log_wrap = 0;
uint8_t log22_wrap = 0;
uint8_t bad_news_wrap = 0;
uint8_t fast_log_wrap = 0;



uint8_t in_charge_rtc = 0;  // if this is 1, this esp will handle master rtc when it drifts too much, setting master rtc for dst which is only needed because android does it that way.

uint8_t in_charge_0x15 = 0;  // if this is 1, this esp will ack 0x16 to 0x15 packets

uint8_t in_charge_0xAA = 0;  // if this is 1, this esp will ack 0xAB to 0xAA packets. this was added for nano_master and might never be in mega8535 code

uint8_t in_charge_online = 1; // use this to toggle_online(). if this is 0, NOTHING will be sent to web, and it will not save epoch to send block later on. this is to switch back and forth
                              // between two or more esps that have been monitoring xbee but are ignoring online. this assumes that one has been working correctly. i might want to add a button
                              // that says "send block" in case the active esp dies and some data is not online. NOTE THAT this flag also controls server dst flag
                            /*
                                5-19-2021 i decided to use the "online" file to store 32 flag bits
                             */

//uint8_t monitor_php_hour = 0;

#define in_charge_online_bit 0
#define in_charge_0x15_bit 2        // if this is 1, this esp will ack 0x16 to 0x15 pulse packets
#define in_charge_rtc_bit 3         // if this is 1, this esp will handle master rtc when it drifts too much, setting master rtc for dst which is only needed because android does it that way.
#define in_charge_0xAA_bit 4       // if this is 1, this esp will ack 0xAB to 0xAA packets. this was added for nano_master and might never be in mega8535 code


char * rwb_buf; // this will be set equal to msg to simplify changeover from all those dynamically allocated buffers
char msg[400]; // this wastes ram, but simplifies coding. i doubt if i'll ever send anything this large. whole screen is about this wide. later i increased the size because show_arrays was killing something
//resets32 140 todays_seconds() 34402 days 0 1:45:50 clock error at last ntp check 0 ntpday 0 ntp time 9:27:36 epoch 1614590856 clock fix 1.000000 ntp_doe 18687
//0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 = 190

int code_cnt,doy_day_changed,new_doy;

//bool rtc_looks_good,ntp_looks_good,time_is_known;

int restart_wifi_flag = 0;

DS3231 Clock;

DateTime dt;

uint32_t myerr, rtc_vs_dream;

uint8_t need_free_values,need_free_times,flow_adc,flow_cutoff,master_rtc_state,xbee_RS_counter,xbee_RS,web_page, min_xbee_RS, max_xbee_RS,xbee_RS_39, min_xbee_RS_39, max_xbee_RS_39;
//uint8_t xbee_RS_state;

uint32_t dream_dead; 

uint8_t stop_nagging_no_packets, fake_dream_is_dead;

uint32_t tank_writes,tr1_writes,tr2_writes,float_writes,flow_writes,pulse_writes,this_writes, seconds_since_last_pulse,routine_log_ptr,log22_ptr,bad_news_ptr,fast_log_ptr,air_writes;

time_t last_air_epoch,last_tank_epoch, my_epoch, locked_dream;

uint32_t npt_millis_at_last_poll, master_pulse_cnt,yesterday_master_cnt, gallons_per_master_cnt,offset_cnt;

uint32_t  master_err; // use int for +/- error  - nope, crazy math errors. try uint32_t

int http_ack = 0;

const char* tr_td ="<tr align='left'><td colspan='13'>";

const char* tr_td_half ="<tr align='left'><td colspan='6'>";

const char* dream_host = "http://www. your web site /birchwood/";   

const char* x10 = "x10_test_esp8266/";


const char* pulse_http = "pulse.php?";
const char* float_chg = "float_change.php?";
const char* flow_chg = "flow_change.php?";
const char* trans1_chg = "trans1_change.php?";
const char* trans2_chg = "trans2_change.php?";
const char* tank_chg = "tank_change.php?";

const char* air_chg = "air_change.php?";

const char* flow_cut = "flow_cutoff.php?";          // look at red line on flow plot to confirm this

//const char* write_ss = "write_source_select.php?";  // look at plots to confirm this. there's a tiny x10 to the left of tank and expanded pulse plots
const char* write_jss = "write_jay_source_select.php?";  // look at plots to confirm this. there's a tiny x10 to the left of tank and expanded pulse plots

const char* write_dst = "write_dst_flag.php?";      // look at legends to confirm this

//const char* read_ss = "read_source_select.php"; look at plots to confirm this

const char* read_dst = "read_dst_flag.php"; // look at plots to confirm this

//const char* read_dst = "read_cutoff.php";

const char* poll_server_day_of_week = "poll_server_date.php"; // returns day of week, not date. 0 = sunday, 6 = saturday
const char* poll_server_day_of_week_UTC = "poll_server_date_UTC.php"; // returns day of week, not date. 0 = sunday, 6 = saturday

const char* poll_server_hour = "poll_server_hour.php"; // returns hour 0..23
const char* poll_server_hour_UTC = "poll_server_hour_UTC.php"; 

const char* read_millenium = "read_millenium.php"; 


const char* flow_blk = "flow_block.php";      // flow_block does not have url args so no need for ? at the end. 
const char* float_blk = "float_block.php";    // float_block does not have url args so no need for ? at the end. 
const char* trans1_blk = "trans1_block.php";  // trans1_block does not have url args so no need for ? at the end. 
const char* trans2_blk = "trans2_block.php";  // trans2_block does not have url args so no need for ? at the end. 
const char* tank_blk = "tank_block.php";      // tank_block does not have url args so no need for ? at the end. 
const char* air_blk = "air_block.php";      
const char* pulse_blk = "pulse_block.php";    // pulse_block does not have url args so no need for ? at the end. 


//const long utcOffsetInSeconds = 0; // est and dst suck. just use utc everywhere except web page legends, and maybe info and log screens here. but this sucks because midnight doy change is at 7 or 8 pm
const long utcOffsetInSeconds = -18000; // wikipedia says est is utc - 5 so -5 x 3600

uint32_t   monthDay,currentMonth,   currentYear, yesterday_year, day_of_year,yesterday_day_of_year,master_seconds;

time_t boot_epoch;

uint8_t  master_resets;
uint32_t yesterday_gallons, latency_millis;


//WiFiUDP ntpUDP;

//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define filestringsize 18   // was 100  

#define maxfiles 50 // THIS INCLUDES THE file contents if the file is small enough to be read into filenames array. 

int file_tries=0; // weird error - it won't show files until a uart packet arrives. wtf.

char filenames[maxfiles][filestringsize]; // 288 bytes separated by spaces = 864

int strings_found;

uint8_t  last_rpi_not_3,rpi_temperature,rpi_today_max,rpi_today_min,rpi_yesterday_max,rpi_yesterday_min,rpi_max; 

uint32_t seconds_since_last_packet, rpi_ack_secs,rpi_3_secs,rpi_norm_secs,bad_rpi_ack;

time_t rtc_epoch,yesterday_midnight_epoch;

uint32_t ping_web_dream = 0;
uint32_t ping_web_biz = 0;

uint32_t secs_since_good_ping_dream, secs_since_bad_ping_dream, good_pings_today_dream,bad_pings_today_dream;

uint32_t secs_since_good_ping_biz, secs_since_bad_ping_biz, good_pings_today_biz,bad_pings_today_biz;

uint8_t show_tank_mode = 0;

uint8_t first_rpi = 1;

uint32_t * times;
/*
    as of today 11-24-2023 this sketch is updated to work with IDE 2.2.1 and board version 3.1.2

    declared here uint32_t * times;
    spiffs uses it like this:times = (uint32_t *)  malloc(filesize);

    it gave screwy date and time in the show_file button until I added a variable (time_t) ss, and used it to read one value, then pass ss to gmtime(&ss)

    go back a few versions to zzz4 and it's all the same. I guess gcc or arduino or the board or whatever changed how they work. I don't know if this is related, but 
    until I started using IDE 2.2.1 I never had problems with time_t, but now I have learned that esp8266 considers it 8 bytes while esp32 is 4 bytes. I wonder if it
    used to be 4 bytes with esp8266 but have no practical way to find out.

      there is discussion going back to 2017 about changing gcc time_t from 32 signed to 32 un-signed or to 64 signed, so i guess it happened with esp8266 somehow but not with esp32
      
*/

uint8_t * values;    // 8-27-2022 was uint8_t before adding air which can be greater than one byte


const char * ssid = "your ssid";
const char * password = "your password";

const char * new_ssid = "your ssid";  // new wifi router uses same password as old


ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80

uint32_t new_clock_millis,old_clock_millis;  // today's millis will zero out at midnight
uint32_t clock_error;
//uint32_t ntpday,ntphour,ntpminute,ntpsecond, ntp_doe;

uint32_t debug32;

uint8_t current_tank,current_status,old_status,last_air_write,last_tank_write,trans_plus_flow,last_new_style_tank,debug1,debug0,debug2,debug3,debug4,debug5,debug6,debug7,log_heap_counter; //,ignore_xbee;



uint8_t current_air, old_air, last_new_style_air, final_state;

uint32_t   spiffs_total,spiffs_used,spiffs_block,spiffs_page,last_pulse_seconds, final_time;

//double esp_clock_fix = 1.00; // 10.0.0.200 1.00122     207 1.0000000

//unsigned long int old_ntp_epoch,ntp_epoch;

uint32_t old_millis = 0;
uint32_t new_millis = 0;
uint32_t old_uart_millis = 0;

//#define my_millis_ra_size 22

//uint32_t my_millis_ra[22];
//int my_millis_ra_ptr = 0;

int first_packet = 1;
int force_status_write = 0;

uint16_t xbee_reset_seconds = 300; // start at 300 so it will reset immediately the first time packets are late

uint16_t xeep_slot = 2000; // actual range 0..1439 so this will force write on first packet after reset

uint16_t uart_ptr = 0;

int num_recs,filesize;   // was int before 8-28-2022; air file is large; stupid. file can't be that big or I can't prune it or send block due to heap which is never larger than signed int 32768

uint16_t start_bit_cnt = 0;

uint8_t save_ck = 0;
uint8_t ck = 0;
uint8_t done = 1;
uint8_t save_my_ck = 0;
uint8_t expected = 0;

#define uart_buff_size 20
uint8_t uart_buff[uart_buff_size];

#define big_buff_size 320    // use 320 since that's what gets printed.
uint8_t big_uart_buff[big_buff_size];  // before adding this at 10000 size, it said 30720 used, 51200 available. with this, it said same thing. wtf. later on it said 40856 used: 40856 - 30720 = 10136 which is ok i suppose. what a load of shit.

uint32_t big_uart_ptr = 0;

uint16_t uart_cnt = 0;

uint16_t reset_cnt = 0;

uint16_t restart_wifi_secs = 30000; // start large so it resets immediately first time web looks dead

uint16_t fast_log_0xAA_cnt = 0;

uint32_t unknown_packets = 0;
uint32_t total_unknown_packets = 0;
uint8_t unknown_packet_flood = 0;
time_t unknown_packet_epoch;

uint32_t xbee_reset_flood_cnt = 0;
time_t xbee_reset_epoch;
uint8_t xbee_reset_flood = 0;

uint32_t packets_0xAA = 0;
uint32_t packets_0x15 = 0;
uint32_t packets_0x16 = 0;


uint32_t reset_cnt_32 = 0;
uint32_t good_ck = 0;
uint32_t bad_ck = 0;

uint8_t LED1pin = D0;         // cpp data type. unsigned integers can be 8 16 32 or 64. i had trouble with Serial.println(rwb_cnt); when it was uint64_t
bool LED1status = LOW;
bool LED1 = LOW;

uint8_t LED2pin = D4;

uint8_t xbee_reset_pin = D5;


uint8_t wifi_select_pin = D7; // added 1-1-2022





void reset_xbee_RS(){

  
  min_xbee_RS = 99;

  max_xbee_RS = 0;

  
  min_xbee_RS_39 = 99;

  max_xbee_RS_39 = 0;
}






void setup() {  //----------------------------------------------------------------setup setup---------------------------------------------------------------------------

  //xbee_RS_state = 0;

  reset_xbee_RS();

  
  master_rtc_state = 0;

  last_master_rtc_sync = 0;

  clock_state = 0;

  pinMode(wifi_select_pin, INPUT_PULLUP);

  rwb_buf = msg; // same buffer will be used in lots of places

  day_of_year = -1; // flag to see if we get a valid setting in setup()
  
  //time_is_known = false; // if rtc or ntp ever works even once, this will be set to true forever. this is only to avoid storing bad events after a reset. otherwise ntp or rtc or millis will know the time 
  
  for (int i=0;i<routine_log_size;i++){
  
    routine_log[i] = 1; // get rid of the zeros which look like a pile of null terminated strings
  }

    
  
  for (int i=0;i<log22_size;i++){
  
    log22[i] = 1; // get rid of the zeros which look like a pile of null terminated strings

    fast_log[i] = 1;
  }


  
  for (int i=0;i<bad_news_log_size;i++){    

    bad_news[i] = 1;
  }
  
  
  for (int i=0;i<big_buff_size;i++){
    big_uart_buff[i] = 0x26; // this is so it can find packets before the buffer is full of stuff other than 00
  }

  //format_fs();
  
  check_spiffs_format();





/* from esp_heat_2 with wifi_select_pin added

  if (digitalRead(wifi_select_pin) == HIGH){  // high is no jumper = old wifi

    WiFi.begin(ssid, password);   // I have to enable this if I use a new device that does not have my router ssid/pwd already stored in flash.

  } else {

    WiFi.begin(new_ssid, password);   // I have to enable this if I use a new device that does not have my router ssid/pwd already stored in flash.
  }

  while (WiFi.status() != WL_CONNECTED) {
    
    delay(1000);

  }

 
 */

  
  start_wifi(0);

  
  while (WiFi.status() != WL_CONNECTED) {

    delay_half_second();
  }


/* old esp_server_4_1 before adding wifi_select_pin    apparently I don't need WIFI_STA ???
  

  
  WiFi.persistent(false); // according to readthedocs, the flash can wear out if I call WiFi.begin(ssid,pwd) too many times. setting persistent = false is supposed to make it use whatever is already there and not re-write it. so, I think
                          // that I can leave this alone in every sketch as long as I'm using the same router ssid and pwd. 

  WiFi.mode(WIFI_STA);

  
 WiFi.begin(); // this will use whatever is in flash and will not write it

  //WiFi.begin(ssid, password);   // I have to enable this if I use a new device that does not have my router ssid/pwd already stored in flash.

 
  while (WiFi.status() != WL_CONNECTED) {

    delay_half_second();
  }


*/

  if (WiFi.status() != WL_CONNECTED){

    sprintf(rwb_buf,"wifi failed, code is %d",WiFi.status());
    add_log(rwb_buf);
    add_log((char *) "wifi failed to connect <<<<<<<<<<<<<<<<<<<<<<");
    /*
        1 No SSID Available Unit is too far from the Wi-Fi access point, the SSID and/or password is incorrect, or the SSID is for a 5GHz-band access point.
        2 Scan Completed  Scanning for available networks is completed.
        3 Connected Success
        4 Connection Failed The opposite of success.
        5 Connection Lost 
        6 Disconnected  
        255 No Shield Used for compatibility with the Arduino WiFi Shield - not relevant to the ESP8266.
     */
  }
  
  
  
  day_of_year = read_a_uint32_t((char *) "/doy"); // DO NOT DELETE THIS or it's a mess trying to avoid pruning on reset since it looks like doy changed

  master_seconds = read_a_uint32_t((char *) "/m_secs"); // used for the unlikely event that esp resets while master is repeating a pulse. this is how i detect repeats. I could get rid of this and use the master pulse count file, i suppose

  master_resets = (uint8_t) read_a_uint32_t((char *) "/m_resets"); // this is only one byte, but go ahead and use the same routine to read and write
  
  try_to_read_reset_cnt_file();

  tank_writes = read_a_uint32_t((char *) "/tank_wr");

  
  sprintf(rwb_buf,"tk wr %d",tank_writes);   
  add_log(rwb_buf);


  tr1_writes = read_a_uint32_t((char *) "/tr1_wr");

  sprintf(rwb_buf,"tr1 wr %d",tr1_writes);   
  add_log(rwb_buf);

  tr2_writes = read_a_uint32_t((char *) "/tr2_wr");
  sprintf(rwb_buf,"tr2 wr %d",tr2_writes);   
  add_log(rwb_buf);

  float_writes = read_a_uint32_t((char *) "/float_wr");
  sprintf(rwb_buf,"float wr %d",float_writes);   
  add_log(rwb_buf);

  flow_writes = read_a_uint32_t((char *) "/flow_wr");
  sprintf(rwb_buf,"flow wr %d",flow_writes);   
  add_log(rwb_buf);

  pulse_writes = read_a_uint32_t((char *) "/pulse_wr");
  sprintf(rwb_buf,"pulse wr %d",pulse_writes);   
  add_log(rwb_buf);

  in_charge_online = read_flags_bit(in_charge_online_bit); // returns 0 if it fails to read a value, so new devices will not be online until i click the button.

  //monitor_php_hour = read_flags_bit(monitor_php_hour_bit);

  in_charge_rtc = read_flags_bit(in_charge_rtc_bit);
  
  in_charge_0x15 = read_flags_bit(in_charge_0x15_bit);

  in_charge_0xAA = read_flags_bit(in_charge_0xAA_bit);

  
  master_pulse_cnt = read_a_uint32_t((char *) "/today_cnt"); // returns 0 if it fails to read a value

  yesterday_master_cnt = read_a_uint32_t((char *) "/yday_cnt"); // returns 0 if it fails to read a value

  if (yesterday_master_cnt == 0){ // this is for new devices; don't delete this. it also is in 0x15 processing, which is probably where it belongs anyway

    yesterday_master_cnt = master_pulse_cnt; // on the first day this will give a low value instead of an enormously incorrect one
    
    save_a_uint32_t((char *) "/yday_cnt",yesterday_master_cnt);
  }

  offset_cnt = read_a_uint32_t((char *) "/offset_cnt"); // returns 0 if it fails to read a value

  gallons_per_master_cnt = master_pulse_cnt + offset_cnt - yesterday_master_cnt; // this is gallons since midnight today, which is not very useful except at the next midnight
  
  reset_cnt++;

  reset_cnt_32++;

  save_settings();
    
 
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  pinMode(xbee_reset_pin, OUTPUT);

  //timeClient.begin();     // ntp  


  server.on("/", hhandle_OnConnect);                     // Call the 'handle_OnConnect' function when a client requests URI "/"
  server.on("/button0", hhandle_but0);                 
  //server.on("/button0", handle_led1off);                
  server.on("/button1", hhandle_but1);                 // but1
  server.on("/button2", hhandle_but2);          // but2
  server.on("/button3", hhandle_but3);          // but3
  server.on("/button4", hhandle_but4);      // but4
  server.on("/button5", hhandle_but5);      // but5
  server.on("/button6", hhandle_but6);      // but6
  server.on("/button7", hhandle_but7);      // but7
  server.on("/button8", hhandle_but8);      // but8
  server.on("/button9", hhandle_but9);      // but9
  server.on("/button10", hhandle_but10);      // but10
  server.on("/button11", hhandle_but11);      // but11
  server.on("/button12", hhandle_but12);      // but12
  
  


  

  
  server.onNotFound(handle_NotFound);

  server.begin();  

  get_spiffs_info();

  init_uart();

  Wire.begin();

  scan_i2c();
  
  delay(50); // milliseconds. 
  yield();

  my_epoch = 0; // if it still is zero after setup finishes i can't save events in files

  if (get_rtc_epoch() == 1){

    my_epoch = rtc_epoch; // if rtc dies, esp dies. simple as that. ntp is such a clusterfuck (or esp's handling of ntp is) it's just easier to do this. 

    add_log((char *) "rtc ok, use it for my_epoch");
  }


  

  uint32_t checkit = read_a_uint32_t((char *) "/dream_status");

  if (checkit > 100){ // if it's dead, this file has the epoch of death time, so i use it to determine which events to block send
    dream_dead = checkit;

    dream_new_ping_cnt = 1;
  } else {
    dream_dead = 0;  // 0 is good. if it's dead, this is the epoch when it died
  }


  int ret = get_both_server_millens(); // returns 1 if dream acks, 2 if dream fails and biz acks, 0 if both fail. this also fixes dream dead and biz dead if the server acks

  if (my_epoch == 0){

    if (ret == 1){

      my_epoch = locked_dream;    // 11-7-2021 this has already been done in the millens routines if dream was good

      add_log((char *) "rtc dead so use dream to set my_epoch");
      
      
    }
  }

  //ntp_epoch = 0;
  /*
  if (simple_ntp(123)){
  
    if (my_epoch == 0){
  
      my_epoch = ntp_epoch;
  
      add_log((char *) "rtc and dream and biz failed, so use ntp to set my_epoch");
    }
  }
*/
  if (my_epoch == 0){

    add_log((char *) "THE WORLD IS COMING TO AN END. rtc, dream, biz, and ntp all failed, so my_epoch is fucked");
  }

  last_air_epoch = my_epoch;
  last_tank_epoch = my_epoch;
  //locked_ntp_epoch = ntp_epoch; // after this, locked is only synced via manual button click, and is used to manually sync rtc and my_epoch via button click. ntp/esp is too flakey to fuck with

  

  old_clock_millis = millis(); // this was in the ntp success block above. why not always do this?
 
  boot_epoch = my_epoch;

  sprintf(rwb_buf,"my_epoch %lld dream_dead %d ",my_epoch,dream_dead);
  add_log(rwb_buf);

  
  test_read_new_file((char *) "/tank3_new",8,2);  // most recent value will be at values[num_recs - 1]   8-20-2022 was it a mistake to use two bytes for tank. range is 0.100d, and I think the file is written in hex. at any rate air is two bytes
                                                                                                // sprintf(str1,"%08X%02X",my_epoch,current_tank); <- this is how tank is written, so it only needed one byte.

  current_tank = values[num_recs - 1]; // load this so it won't send php or add to local file on first packet after boot

  
  test_read_new_file((char *) "/air_new",8,2);  // most recent value will be at values[num_recs - 1]

  current_air = values[num_recs - 1]; // load this so it won't send php or add to local file on first packet after boot
  old_air = current_air; // this is what actually stops the extra writes
  last_air_write = current_air; // old_air is obsolete

/*
    if (LittleFS.exists("/air_new")){
    
      LittleFS.remove("/air_new"); 
                        
    }

    
    if (LittleFS.exists("/air_wr")){
    
      LittleFS.remove("/air_wr"); 
                        
    }
*/

  handle_malloc();
  
  //old_tank = current_tank; // this is what actually stops the extra writes
  last_tank_write = current_tank;

  sprintf(rwb_buf,"current tank %d  old %d",current_tank,last_tank_write);
  add_log(rwb_buf);

  // 0x01     0x02        0x04        0x08      0x10    0x20    0x40      0x80
  // 0 na     1 trans2    2 flow34    3 float   4 na    5 na    6 trans1  7 na
  
  test_read_new_file((char *) "/flowa",8,1);  
  old_status |= (final_state << 2);

  handle_malloc();
  
  sprintf(rwb_buf,"flowa final %02X",final_state);   
              
  add_log(rwb_buf);
 
  test_read_new_file((char *) "/trans_1a",8,1);  
  old_status |= (final_state << 6);
  sprintf(rwb_buf,"tr1 final %02X",final_state);  
  add_log(rwb_buf);

  handle_malloc();
  
  test_read_new_file((char *) "/trans_2a",8,1);  
  old_status |= (final_state << 1);
  sprintf(rwb_buf,"tr2 final %02X",final_state);  
  add_log(rwb_buf);

  handle_malloc();
  
  test_read_new_file((char *) "/float",8,1);
  old_status |= (final_state << 3);  
  sprintf(rwb_buf,"float final %02X",final_state);
  add_log(rwb_buf);

  handle_malloc();
  
  sprintf(rwb_buf,"old_status %02X",old_status);
  add_log(rwb_buf);

   

 rpi_today_min = 255;
 rpi_yesterday_min = 255;



  sprintf(rwb_buf,"my_epoch %lld",my_epoch);
  add_log(rwb_buf);

  
  read_datecode_file(rwb_buf);
  add_log(rwb_buf);





  if ((fake_dream_is_dead == 1) && (dream_dead == 0)){ // if it was already dead leave it alone

    dream_died(); // this uses my_epoch
  }
  


  IPAddress ipa = WiFi.localIP();

  my_ip = ipa[3];

  sprintf(rwb_buf,"IP %d",my_ip);
  add_log(rwb_buf);
  
  /*
   IPAddress::IPAddress  (   uint8_t   first_octet,
    uint8_t   second_octet,
    uint8_t   third_octet,
    uint8_t   fourth_octet 
  )     
   */

  sprintf(rwb_buf,"end of setup");
  add_routine_log(rwb_buf);

}  //-------------------------------------------------------------------------------------------------------------------------------------------




uint8_t button_pressed = 250;









void toggle_leds(){

  LED1 = !LED1;
  
  if(LED1){
    
    digitalWrite(LED1pin, HIGH);
    digitalWrite(LED2pin, LOW);
    
  } else {
    
    digitalWrite(LED1pin, LOW);
    digitalWrite(LED2pin, HIGH);
  }
  
}

void do_the_button(){
  
  switch (button_pressed){

    case 100:

      handle_OnConnect();
      break;
    case 0:

      handle_but0();
      
      break;
      case 1:

      handle_but1();
      break;
      case 2:

      handle_but2();
      break;
      case 3:

      handle_but3();
      break;
      case 4:

      handle_but4();
      break;
      case 5:

      handle_but5();
      break;
      case 6:

      handle_but6();
      break;
      case 7:

      handle_but7();
      break;
      case 8:

      handle_but8();
      break;
      case 9:

      handle_but9();
      break;
      case 10:

      handle_but10();
      break;
      case 11:

      handle_but11();
      break;
      case 12:

      handle_but12();
      break;
  }
}










void loop() {  //------ main loop main_loop -------------------------------------------------------------------------------------------------------------------------------------

  if (detect_second()){

    //test_air_file_and_send_to_dream(); // this will send once per minute

    //uint32_t secs = (new_clock_millis - old_clock_millis) / 1000; // the half second delays can skip seconds under really fucked situations
    
    handle_seconds(the_millis / 1000);
  }
  
  server.handleClient();               // Listen for HTTP requests from clients

  handle_malloc(); // try to catch disasters that should never occur. i hope this never creates one. times[] and values[] are intermittently used and should not cross this point while still being used, i hope.
  
  poll_uart();

/*  
  if (xbee_RS_state == 5){

    get_RS();

    save_RS_to_xeep();

    xbee_RS_state = 0;  // see notes in uart get_RS()
  }
*/
  if (master_rtc_state >= 14){

    master_rtc_state = 0;

    send_master_rtc_packet();
    log_time();

    last_master_rtc_sync = my_epoch;
  }
  

  yield(); // see yield discussion in asus esp8266/rwb.txt. apparently yield is way more important on things with wifi and it's sort of like interrupts and task switching, in that data can be changed and cause corruption, so
           // wifi information that is received (does that happen?) should be buffered and then dealt with here. since I call server.handleClient in this loop, does that even make sense? does anything happen behind the scenes
           // that could corrupt data here?
  if (button_pressed < 200){
    

    // 10-5-2021 add this. I had synched rtc to dream and my_epoch to rtc on both esps, but the one at 204 was doing the bcd test that fills the log, so I added this to try to catch the error. delete this if it seems to add weirdness
    /*

          tl,dr the script repeat_both sends button presses.
          
          
          
          
          
          it seems to be sending phantom mouse clicks, or the esp seems to be creating phantom http requests that look like repeats of the last mouse click. if it only happens on 204 then maybe that one is screwy. if it happens on both
          it could be my code or my pc or my mouse. it might always be a fake button 1 even if i clicked some other button on the web page.

            >>> IS IT THE SCRIPT repeat_both, which sends button 1? probably, and I need to deal with that. either stop running the script, or make it safe to send button 1 no matter what page the esp is on. try sending just the IP, with no button. nope,
            seems to send button 100, so just ditch the script for now. I think the rationale for it anyway was to see if i found the crash bug, by polling both esps all day long at 20 second intervals, which worked and both stay up for days,
            so i don't need the script right now
          
          what is the difference between button_pressed and button_clicked. that looks ok. button_pressed is the global. button_clicked is an arg to the web pages and is only used to show which was pressed, except for the big page where it
          also selects what is shown, since that page (page 0) can show logs, files, info, etc. all the other pages only show one thing, usually just descriptions of what the buttons do, but they use the arg to show the last button that
          was pressed
          
          
          
          
          it seems that it can happen when I click button 11 on page 204 because in this log that's all i was doing: go to clock page and return without any sync, just return using button 11, but it adde a button 1:

          page 0 is the main page with all buttons having functions, like "info" and "show log"

          page 1 has the buttons to go to other pages, like clock, server stuff, source select, flow cutoff

            button 1 on page 1 goes to clock, which is page 204. 

          page 100 is flow cutoff

          page 200 dst

          page 201 source select

          page 202 is local

          page 203 server stuff

          page 204 is clock, and button 1 on that page is the screwy one that is calling test_bin_2bcd. is it somehow sending two button presses when i click button 1 on page 1, so that it also clicks button 1 on page 204

                    
          button 11 page 0
          button 1 page 1
          button 11 page 204
          button 11 page 0                
          button 1 page 1                 
          button 1 page 204
          but1 case 204 test_bin2bce
          in 0 00 out 00
          in 1 01 out 01
          in 2 02 out 02
          in 3 03 out 03
          in 4 04 out 04
          ...
          
          in 97 61 out 97
          in 98 62 out 98
          in 99 63 out 99
          button 11 page 204
          button 11 page 0
          button 1 page 1
          button 11 page 204
          button 3 page 0


          another example:

          
          button 11 page 0
          button 1 page 1
          button 10 page 204
          button 1 page 204
          but1 case 204 test_bin2bce
          in 0 00 out 00
          in 1 01 out 01
          ...
          in 98 62 out 98
          in 99 63 out 99
          button 3 page 204
          button 11 page 204
          button 3 page 0

          another with more logging:

                    
          button 11 page 0                      click 11 to see the menu that shows clock on button 1
          button 250 page 1 on loop() exit
          
          button 1 page 1                       click 1 to go to clock set menu, which has bcd test on button 1
          button 250 page 204 on loop() exit    clear the button 1 press to say 250
          
          button 1 page 204                     i did not click this button 1; my guess is that the mouse is sending multiple clicks or the esp is bizarrely somehow doing it outside of my code, in the http code. in either case I guess i need to
                                                look at every fucking page and see what malfunction could occur if a button is repeated. see that painful discussion below
          but1 case 204 test_bin2bce
          
          in 0 00 out 00
          in 1 01 out 01
          in 2 02 out 02
          ...
          in 98 62 out 98
          in 99 63 out 99
          
          button 250 page 204 on loop() exit
          
          button 11 page 204
          button 250 page 0 on loop() exit
          
          button 11 page 0
          button 250 page 1 on loop() exit
          
          button 1 page 1

          if the esp http code is somehow repeating clicks, i can only assume that it goes with the same button unless i see some other malfunction

            I clicked button 11 80 times and it did not screw up; it went to menu page 1 and returned to main page 0 perfectly

            if server_stuff screws up it will click force 24 dream - which happened in the log below this BUT NOTE THAT THE fake button was not a repeat of button 0 which was pressed, but was button 1

            if local screws up it will toggle in_charge_rtc page 202 on bogus click of button 

            if source_select screws up nothing happens

            if dst screws up it will click "march", no problem there

            if flow_cutoff screws up nothing happens

                        
            button 250 page 1 on loop() exit
            button 0 page 1
            button 250 page 203 on loop() exit
            button 1 page 203                                       <<< bogus button 1 on page 203 server stuff = toggle fake dead dream
            dream died
            force but1 fake dream 1
            button 250 page 203 on loop() exit
            button 11 page 203
            button 250 page 0 on loop() exit
            button 11 page 0
            button 250 page 1 on loop() exit
            button 2 page 1
            button 250 page 202 on loop() exit
            button 1 page 202                                       <<< bogus button 1 on page 202 = local, which does nothing
            button 250 page 202 on loop() exit
            button 11 page 202
            button 250 page 0 on loop() exit
            button 11 page 0
            button 250 page 1 on loop() exit
            button 4 page 1
            button 250 page 201 on loop() exit
            button 1 page 201                                       <<< bogus button 1 on page 201 source select does nothing
            button 250 page 201 on loop() exit
            button 11 page 201
            button 250 page 0 on loop() exit
            button 11 page 0
            button 250 page 1 on loop() exit
            button 5 page 1
            button 250 page 200 on loop() exit
            button 11 page 200
            button 250 page 0 on loop() exit
            button 11 page 0
            button 250 page 1 on loop() exit
            button 7 page 1
            button 250 page 100 on loop() exit
            button 1 page 100                                       <<< bogus button 1 on page 100 flow cutoff = ask master cutoff, but xbee was unpowered
            button 250 page 100 on loop() exit
            button 11 page 100
            button 250 page 0 on loop() exit
            button 3 page 0

            later, not logged, it might have sent a bogus button 0, since normal log shows rtc string, but i missed the fast log of buttons because another bcd test flooded it out.

    */
    
//      sprintf(rwb_buf,"button %d page %d",button_pressed, web_page);
//      add_fast_log(rwb_buf);
    
    // end of 10-5-2021 add this. 

  



    do_the_button();          // trying to stop crashing on button click. any delay() or yield() can process a button click, and maybe building a log page or whatever was interfering with whatever was going on that had a yield or delay
                              // so this new way simply sets a variable on button click, and the page is not built and sent until here, so presumably any xbee/php or file or whatever activity cannot be happening via task switching

    button_pressed = 250; // do nothing

    // 10-5-2021 added this block
    
//      sprintf(rwb_buf,"button %d page %d on loop() exit",button_pressed, web_page);
//      add_fast_log(rwb_buf);
      
    // end of 10-5-2021 added this block
    
  }
}  //-------------------------------------------------------------------------------------------------------------------------------------------




void hhandle_OnConnect(){     // all the hhandle_ are called when a client clicks a web page button. this sets button_pressed variable and the button is processed in loop(). I was trying to stop crashes, but I think the crashes were
                              // caused by a log buffer bug. at any rate this code complexity probaby does no harm

  button_pressed = 100;
}

void hhandle_but0(){
  button_pressed = 0;
}

void hhandle_but1(){
  button_pressed =1 ;
}
void hhandle_but2(){
  button_pressed =2 ;
}
void hhandle_but3(){
  button_pressed =3 ;
}
void hhandle_but4(){
  button_pressed =4 ;
}
void hhandle_but5(){
  button_pressed =5 ;
}
void hhandle_but6(){
  button_pressed =6 ;
}
void hhandle_but7(){
  button_pressed =7 ;
}
void hhandle_but8(){
  button_pressed =8 ;
}
void hhandle_but9(){
  button_pressed =9 ;
}
void hhandle_but10(){
  button_pressed =10 ;
}
void hhandle_but11(){
  button_pressed =11 ;
}
void hhandle_but12(){
  button_pressed =12 ;
}






void handle_OnConnect() {  //-------------------------------------------------------------------------------------------------------------------------------------------

  show_web_page(1); // open to info screen
  
  //server.send(200, "text/html", SendHTML(0));     // 200 means "OK", then this says it's going to send some html text, and then a long string of html which was prepared by SendHTML()

      // Note that I changed the content type of the response from "text/plain" to "text/html". If you send it as plain text, the browser will display it as text instead of interpreting it as HTML and showing it as a button.

}



void toggle_in_charge_rtc(){

  if (in_charge_rtc == 1){

    in_charge_rtc = 0;

    clear_flags_bit(in_charge_rtc_bit);
    
  } else {

    in_charge_rtc = 1;

    set_flags_bit(in_charge_rtc_bit);
  }
}


void toggle_in_charge_0x15(){

  if (in_charge_0x15 == 1){

    in_charge_0x15 = 0;

    clear_flags_bit(in_charge_0x15_bit);
    
  } else {

    in_charge_0x15 = 1;

    set_flags_bit(in_charge_0x15_bit);
  }
}



void toggle_in_charge_0xAA(){

  if (in_charge_0xAA == 1){

    in_charge_0xAA = 0;

    clear_flags_bit(in_charge_0xAA_bit);
    
  } else {

    in_charge_0xAA = 1;

    set_flags_bit(in_charge_0xAA_bit);
  }
}

void toggle_online(){

  if (in_charge_online == 1){

    in_charge_online = 0;

    clear_flags_bit(in_charge_online_bit);
    
  } else {

    in_charge_online = 1;

    set_flags_bit(in_charge_online_bit);
  }
}







void handle_but0() {  //--------- but0 ---------------------------------------------------------------------------------------------------------------------------------- 
  
  
  switch(web_page){

    case 1:
      web_page = 203; // server stuff like force 24, fake dead
      break;

    case 203:
      
      dream_dead = my_epoch - 86440; // move it back 24 hours plus a bit
    
      dream_new_ping_cnt = 1;
      
      save_a_uint32_t((char *) "/dream_status",dream_dead);
      
      sprintf(rwb_buf,"force but0 dream %d",dream_dead); 
      add_log(rwb_buf);

      break;

    case 204:

      send_master_rtc_packet();

      break;

    case 100:

      write_php_number(dream_host,flow_cut,1,flow_cutoff); // 1 is for x10 dir
      break;
      
    case 200:

      //test_403_404();
      
      
  /*                   
#define in_charge_online_bit 0
#define php_hour_monitor 1
#define test_bitwise_code 2
*/
      
      break;
  }

  show_web_page(0);
  

}




void handle_but1() {  //------ but1 -------------------------------------------------------------------------------------------------------------------------------------
  
  switch(web_page){

    case 1:

      web_page = 204; // clock, rtc
      break;

    case 203:

      if (fake_dream_is_dead == 1){
    
        fake_dream_is_dead = 0;
        
      } else {
    
        fake_dream_is_dead = 1;
    
        dream_died();
      }
    
      sprintf(rwb_buf,"force but1 fake dream %d",fake_dream_is_dead);
      add_log(rwb_buf);
      
    break;


    case 204:

      add_fast_log((char *) "but1 case 204 test_bin2bce");
      
      test_bin2bcd();

      break;

    case 200:

      ask_both_sites_what_dst_flag_is();
      
      break;

    case 201:

      

      break;
      
    case 100:

      ask_master_cutoff_values();

      break;
  }

  show_web_page(1);
  
}





void handle_but2(){   //----------------------------------- but2 ----------------------------------------------------------------------------------
  
  switch(web_page){

    case 0:

      get_spiffs_info();
    
      read_dir();
      
      break;

    case 1:

      web_page = 202; // local stuff like reset xbee, in_charge, online
      break;

    case 203:

    
    break;

    
    case 202:

      toggle_in_charge_0xAA();
      break;

    case 200:

      write_php_number(dream_host,write_dst,1,0); // 1 means x10 dir, 1 is dst flag

//      write_php_number(biz_host,write_dst,1,0); // 1 means x10 dir, 1 is dst flag
      
      break;

    case 201:

      

      break;
      
    case 100:

      temporary_flow_cutoff++;
      break;
  }

  show_web_page(2);
  
}

void handle_but3(){   //------- but3 --------------------------------------------------------------------------------------------------------------

 
  switch(web_page){

    case 0:

      if (fast_log_0xAA_cnt > 0){
        
        add_fast_log_0xAA();    // this logs the count and resets it
      }

      
      break;

    case 1:

      reset_air_new_style();

     
      
      //char str1[100];
      //sprintf(str1,"%08X%02X",my_epoch,current_air);
      
      //test_write_air_file();

      break;

    case 202:

      toggle_in_charge_0x15();
      break;
  
    case 204:

      sync_rtc_to_epoch(&locked_dream);
      break;

    case 200:

      write_php_number(dream_host,write_dst,1,1); // 1 means x10 dir, 1 is dst flag

      //write_php_number(biz_host,write_dst,1,1); // 1 means x10 dir, 1 is dst flag
      
      break;
    
    case 201:

      

      break;
      
    case 100:

      temporary_flow_cutoff--;
      break;
  }

  show_web_page(3);
}

void log_heap(){
  
        uint32_t heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 
            
        sprintf(rwb_buf,"heap %u", heap);
        add_fast_log(rwb_buf);  
}


void handle_but4(){   //--------  but4 -------------------------------------------------------------------------------------------------------------
  
  switch(web_page){
  
    case 0:
      show_tank_mode++;
      
      if (show_tank_mode > 6){
        
        show_tank_mode = 0;
      }
    
      switch(show_tank_mode){ // see show_arrays for comments about these values and related code
    
        case 0:
    
          test_read_new_file((char *) "/tank3_new",8,2);  
          break;
    
        case 1:
    
          test_read_pulse_file((char *) "/pulse");  
          break;
    
        case 2:
    
          test_read_new_file((char *) "/flowa",8,1);  
          break;
          
        case 3:
    
          test_read_new_file((char *) "/trans_1a",8,1);  
          break;
          
        case 4:
    
          test_read_new_file((char *) "/trans_2a",8,1);  
          break;      
    
          
        case 5:
    
          test_read_new_file((char *) "/float",8,1);  
          break;      

          
        case 6:
    
          test_read_new_file((char *) "/air_new",8,2);  
          break;      
          
      }


      break;

    case 1:
    
      web_page = 201; // source select
      break;

    case 202:

      toggle_in_charge_rtc();
      break;
    
    case 201:

      

      break;
      
    case 100:

      temporary_flow_cutoff += 10;
      break;

    case 200:

      november();
      break;
  }

  show_web_page(4);
}





void handle_but5(){   //---------------------------------------- but5 -----------------------------------------------------------------------------


  
  switch(web_page){

    case 0:

      break;

    case 1:
    
      test_dst_epoch = my_epoch;
      web_page = 200; // test dst
    
      break;
    
    case 202:

      send_control_byte(0x88); // off = sleep

      break;
      
    case 100:

      temporary_flow_cutoff -= 10;
      break;
      
    case 200:

      march();
      break;
      
    case 201:

      //write_php_number(biz_host,write_jss,0,1);  // 0,1: 0 = top dir, not x10   1 means source_select uses top dir data, not x10

      break;
  }

  show_web_page(5);
  
}




void handle_but6(){   //---------------------------------- but6 -----------------------------------------------------------------------------------
  
  switch(web_page){

    case 1:
      
      //test_write_air_file_and_send_to_dream();    

      break;

    case 202:
      toggle_online();

    break;
    
    case 204:


      break;
      
    case 100:

      send_master_cutoff_values();
      break;

   
    case 200:

      test_dst_epoch += 365 * 86400;
      break;
      
    case 201:

//      write_php_number(biz_host,write_jss,0,2);  // 0,1: 0 = top dir, not x10   2 means source_select uses x10 data

      break;
  }

  show_web_page(6);
}




void handle_but7(){   //---------------------------------- but7 -----------------------------------------------------------------------------------
  
  switch(web_page){

    case 0:
      
      send_rpi_temp(dream_id);
      break;

    case 1:

      web_page = 100; // flow cutoff edit page

      temporary_flow_cutoff = flow_cutoff;
      
      //ask_master_cutoff_values();

    break;
    
    case 200:

      break;
      
    case 202:

      send_control_byte(0x99); // on = awake

      break;
      
    case 201:

      write_php_number(dream_host,write_jss,0,1);  // 0,1: 0 = top dir, not x10   1 means source_select uses top dir data, not x10

      break;
  }

  show_web_page(7);
}




void handle_but8(){   //---------------------------------- but8 -----------------------------------------------------------------------------------


  switch(web_page){

    case 0:
      send_xbee_RS(dream_id);
      break;
    
    
    case 202:

       for (int i=0; i<xbee_histo_size; i++){

        xbee_histo[i] = 0;

        xbee_histo_39[i] = 0;
        
       }
        
/*
 
      if(debug2==1){
        
        digitalWrite(xbee_reset_pin, LOW);
        debug2 = 0;
    
        
      } else {
    
        digitalWrite(xbee_reset_pin, HIGH);
        debug2 = 1;
      }
      */
      
    break;

  case 204:

   

    break;

  case 200:

    
    
    break;
    
  case 201:

    write_php_number(dream_host,write_jss,0,2);  // 0,1: 0 = top dir, not x10   2 means source_select uses x10 data

    break;
    
  }

  show_web_page(8);
}





void handle_but9(){   //---------------------------------- but9 -----------------------------------------------------------------------------------

  switch(web_page){

    case 0:
      
      send_flow_adc(dream_id);    
      break;

    case 202:

      reset_xbee_RS();

      
      break;
      
    case 204:
    
      if (get_rtc_epoch() == 1){
    
        my_epoch = rtc_epoch; // if rtc dies, esp dies. simple as that. ntp is such a clusterfuck (or esp's handling of ntp is) it's just easier to do this. 
                              // 11-7-2021 my epoch is always synced to dream if dream millen works in doy change
        
      } else {

        add_bad_news((char *) "something failed, so my_epoch was not set to rtc_epoch");
      }
    

      break;
  }


  show_web_page(9);

}





void handle_but10(){   //---------------------------------- but10 -----------------------------------------------------------------------------------

  switch(web_page){

    case 0:
      
      get_both_server_millens();  // this also sets dream_dead = 0  if they ack; added because setup was resetting wifi if online was not enabled
      break;

    case 202:
    
      scan_i2c();
      
      break;
      
    case 204:

      //locked_ntp_epoch = ntp_epoch;

      //write_dst_flag(biz_host,0);
  //    write_php_number(biz_host,write_dst,1,0); // 1 means x10 dir, 0 is dst flag
      break;
  }

  show_web_page(10);
  
}





void handle_but11(){   //---------------------------------- but11 -----------------------------------------------------------------------------------

  /*
      but11 will cycle web pages so i get more buttons. i think i can put switch statements in each of the other buttons to handle different pages without having to add more linkages to server.on in setup

      to test: the old page is 0 and the new page is 1

      leave all the other buttons alone at first

      4-27-2021

        add some complexity:

          flow cutoff page = page number 100

          button sends me to that page

          if page == 100 then but11 goes back to page 0, otherwise but11 does what it used to do
   */

    switch(web_page){

      case 100:     // page 100 is flow cutoff page
      case 200:     // page 200 is for testing dst stuff
      
        web_page = 0;
        break;

      default:
    
        web_page++;
        
        if (web_page > 1){
    
          web_page = 0;
        }

        break;
    }

    show_web_page(11);
 
}

void show_web_page(int clicked_button){
    
    switch(web_page){

      case 0:

        server.send(200, "text/html", SendHTML(clicked_button));
        break;

      case 1:

        server.send(200, "text/html", SendHTML_1(clicked_button));
        break;
/*
      case 2:

        server.send(200, "text/html", SendHTML_2(clicked_button));
        break;
*/
     case 100:

        server.send(200, "text/html", SendHTML_100(clicked_button));
        break;
        
     case 200:

        server.send(200, "text/html", SendHTML_200(clicked_button));
        break;
        
     case 201:

        server.send(200, "text/html", SendHTML_201());
        break;

     case 202:

        server.send(200, "text/html", SendHTML_202());
        break;


     case 203:

        server.send(200, "text/html", SendHTML_203());
        break;


     case 204:

        server.send(200, "text/html", SendHTML_204());
        break;

        // web page range 0..255
        // web page range 0..255
        // web page range 0..255
        // web page range 0..255
    }
}


void handle_but12(){   //---------------------------------- but12 -----------------------------------------------------------------------------------

  
  switch(web_page){

    case 0:

      break;

    case 1:
      
    break;

    case 204:
   
    
      //sync_rtc_to_epoch(&locked_ntp_epoch);
      break;
  }

  show_web_page(12);
  
}




void test_disconnect_wifi(){

/*
They are different.
WiFi.dicsonnect() turns off the radio but also erases any saved access point credentials.
WiFi.mode(WIFI_OFF) merely turns off the radio but saves previously used credentials.

WiFi.mode(m): set mode to WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF.

*/
  //WiFi.disconnect();

  WiFi.mode(WIFI_OFF);
    
//  while (WiFi.status() == WL_CONNECTED) {
  //  
    //delay(1000);
 // }
}


/*

void connect_wifi(){

}








void clear_my_millis(){  //-------------------------------------------------------------------------------------------------------------------------------------------

  // this array is filled when a packet is received, and is cleared when main page is displayed

  int sz = sizeof(my_millis_ra)/sizeof(my_millis_ra[0]);

  for (int i = 0; i<sz; i++){

    my_millis_ra[i] = 0;
  }

  my_millis_ra_ptr = 0;
  
}
*/




void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}  //-------------------------------------------------------------------------------------------------------------------------------------------

/*
 * 
 * 
 
              char bigbuf[400];  // hope this is enough

              String full = ESP.getFullVersion();
              
              const char * sdk = ESP.getSdkVersion();
              String core = ESP.getCoreVersion();
              
              
              uint8_t bootver = ESP.getBootVersion();
              uint8_t bootmode = ESP.getBootMode();
              
              uint8_t cpu = ESP.getCpuFreqMHz();
              
              uint32_t sketchspace = ESP.getFreeSketchSpace();
              
              //String resetreason = ESP.getResetReason();
              //String resetinfo = ESP.getResetInfo();


              if (sizeof(full) < 370){
              
                sprintf(bigbuf,"%d %s %s %s %d %d %d %d \r\n",app_msg,full.c_str(),sdk,core.c_str(),cpu,sketchspace,bootver,bootmode); 
                
              } else {

                sprintf(bigbuf,"%d string too long \r\n",app_msg); 
              }
            
              client.print(bigbuf);
 * 
 */




 


void  show_files(String & ptr){  //- but2 ------------------------------------------------------------------------------------------------------------------------------------------

  ptr += "<br><br>";
  
  sprintf(rwb_buf,"log entries %d maximum capacity %d",strings_found,maxfiles);
  
  ptr += rwb_buf;

  //int off;
  
  int len;

  ptr += "<br><br>";
 
  for (int off2=0;off2<strings_found;off2++){
    
      ptr += tr_td; 

      len = strlen(filenames[off2]);

      if (len >= filestringsize){  

        sprintf(rwb_buf,"rwb error 22");
         
      } else {
      
        sprintf(rwb_buf,filenames[off2]);
      }
      
      ptr += rwb_buf;

      ptr += "<br><br>";
  }

  ptr += "<br><br>";
}






/*


void  show_result(String & ptr){  //-- ----------------------------------------------------------------------------------------------------------------------------------------


 

  ptr += "<br><br>";

  int int1 = -3;
  uint8_t u8,u7;

  u8 = 40;
  u7 = 50;

  sprintf(rwb_buf,"val %d  abs(val) %d",-3,abs(-3));

  
  ptr += rwb_buf;

  ptr += "<br><br>";
  
  sprintf(rwb_buf,"using ints val %d  abs(val) %d",int1,abs(int1));

  
  ptr += rwb_buf;

  ptr += "<br><br>";

  sprintf(rwb_buf,"using uint8_t val1 %d  val2 %d val1 - val2 %d abs(val1 - val2) %d, all format d",u8,u7,u8-u7,abs(u8-u7));
      
  ptr += rwb_buf;
  ptr += "<br><br>";

  sprintf(rwb_buf,"using uint8_t val1 %d  val2 %d val2 - val1 %d abs(val2 - val1) %d all format d",u8,u7,u7-u8,abs(u7-u8));
      
  ptr += rwb_buf;

  ptr += "<br><br>";


  
  ptr += rwb_buf;

  ptr += "<br><br>";

  sprintf(rwb_buf,"using uint8_t val1 %u  val2 %u val1 - val2 %u abs(val1 - val2) %u, all format u",u8,u7,u8-u7,abs(u8-u7));
      
  ptr += rwb_buf;
  ptr += "<br><br>";

  sprintf(rwb_buf,"using uint8_t val1 %u  val2 %u val2 - val1 %u abs(val2 - val1) %u all format u",u8,u7,u7-u8,abs(u7-u8));
      
  ptr += rwb_buf;

  ptr += "<br><br>";

}
*/






void  show_stub(String & ptr,int ii){  //-------------------------------------------------------------------------------------------------------------------------------------------

  

  sprintf(rwb_buf,"stub %d",ii);

  ptr += "<br><br>";
      
  ptr += rwb_buf;

  ptr += "<br><br>";
  
}






void  show_arrays(String & ptr){  // ----- but4 ---------------------------------------------------- show_file show_file() ----------------------------------------------------------------------------------
 
  //    file          show_arrays mode        file mode                               show_tank_mode
  //    ----------    ----------------        ----------------------------------      ------------------
  //    pulse         2                       5 bytes = seconds                       1
  //    tank3_new     1                       4 bytes = minutes, 2 bytes = value      0
  //
  //
  // 3-11-2024 if there was no activity in the last 24 hours (or whatever i use for pruning), this was showing garbage, so i tried to fix that
  // in spiffs test_read_new_file   num_recs = 0 was added


  int mode1 = 0;
  
  

  switch (show_tank_mode){

    case 0: // tank3_new

      mode1 = 1;
      sprintf(rwb_buf,"tank3_new");
      break;

    case 1: // pulse

      mode1 = 2;
      sprintf(rwb_buf,"pulse");

      break;
      
    case 2:

      mode1 = 1;
      sprintf(rwb_buf,"flow");

      break;
      
    case 3:

      mode1 = 1;
      sprintf(rwb_buf,"trans1");

      break;
      
    case 4:

      mode1 = 1;
      sprintf(rwb_buf,"trans2");

      break;

      
    case 5:

      mode1 = 1;
      sprintf(rwb_buf,"float");

      break;

      
    case 6:

      mode1 = 1;
      sprintf(rwb_buf,"air_new");

      break;
      
  }

  
  ptr += rwb_buf;


  //int rows=0;

  

  long items = num_recs;


  struct tm * ptm;

  float gpm;

  uint32_t old_secs = 0;

  int rows3 = (int)((float) items / 6.0 + 0.9999); // sparkfun says float sucks on arduino, and avoid it

  int cols3 = 6;

  sprintf(rwb_buf," ... items %ld  rows3 %d",items,rows3);

  // each item index = zero based row + zero based column * rows3

  ptr += rwb_buf;

  ptr += "<br>";

    
  char temp[300];

  int first = 1;

  int secs;
  //int final_cnt; // debugger

  int item_found = 0;

  time_t ss;

  int first_day_count = 0;
  int second_day_count = 0;
  byte first_day = 0;

  uint32_t today = my_epoch - 86400; // timestamps later than this happened in the last 24 hours

  int today_count = 0;

  //if (3==3){

            //if (items ==9000){ // force other code to run
            if (items > 25){
            
              int i = 0; // item index
              
                  for (int r=0;r<rows3;r++){
            
                    first = 1;
            
                    for (int c=0;c<cols3;c++){
            
                      i = r + c * rows3;
            
                      if (i<items){
            
                        item_found = 1;

                        ss = (time_t) times[i];
                  
                        ptm = gmtime ( &ss);
              
                    
                        if (mode1 == 1){ 
                           
                          sprintf(temp,"%02d-%02d %02d:%02d:%02d 0x%02X  %03d ..... ",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,values[i],values[i]);
                        
                        } else { // pulses?
            
                          if (times[i] > today){
            
                            today_count++;
                          }
            
                          if (first_day == 0){
            
                            first_day = ptm->tm_mday;
                          }
            
                          if (first_day == ptm->tm_mday){
            
                            first_day_count++;
                          
                          } else {
                            
                            second_day_count++;
                          }
                    
                          if (i == 0){
                    
                            gpm = 0;
                            
                          } else {
            
                            secs = times[i] - times[i-1];
            
                            if (secs > 0){
            
                              gpm = 6000.0 / (float)(times[i] - times[i-1]); // gallons / minute = 100 gallons / pulse * 1 pulse / seconds * 60 seconds / minute
                              
                            } else {
            
                              gpm = -1;
                            }
                          }
            
                          if (gpm < 10.0){    // html seems to collapse multiple spaces, so the columns don't add up. i suppose i could use a table instead. format %04.2f didn't do anything
            
                            sprintf(temp,"%02d-%02d %02d:%02d:%02d XXXX %.2f ... ",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,gpm);    // also, the XXXX kinda hilites new pulse block
                            
                          } else {
            
                            sprintf(temp,"%02d-%02d %02d:%02d:%02d gpm %.2f ... ",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,gpm);
                          }                
                        }
                        
                      } else {
            
                        item_found = 0; // end of list
                      }
            
                      if (item_found == 1){
                        
                        if (first == 1){
              
                          first = 0;
                
                          sprintf(rwb_buf,"%s",temp);
                          
                        } else {
                          
                          strcat(rwb_buf,temp);  
                        }
                      }
            
                      
                    }
            
            
                    
            
                     
                    ptr += rwb_buf;
            
            
                    sprintf(rwb_buf," row %d",r);          
                    ptr += rwb_buf;
                    
                    ptr += "<br>";
                  }
            
            
            /*
            
            } else if (debug == 1){
            
            
            
                if (items > 25){
            
                  debug_cnt = 5;
                  
                } else {
            
                  debug_cnt = items;
                }
            
              
                for (int i=0;i<debug_cnt;i++){
              
                  ptm = gmtime ((time_t *) &times[i]);
              
                  if (mode1 == 1){ 
                     
                    sprintf(rwb_buf,"%02d-%02d   %02d:%02d:%02d     0x%02X  %d",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,values[i],values[i]);
                  
                  } else { // pulses?
              
                    if (old_secs == 0){
              
                      gpm = 0;
                      
                    } else {
              
                      gpm = 6000.0 / (float)(times[i] - old_secs); // gallons / minute = 100 gallons / pulse * 1 pulse / seconds * 60 seconds / minute
                    }
              
                    old_secs = times[i];
              
                    sprintf(rwb_buf,"%02d-%02d   %02d:%02d:%02d   gpm %.2f",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,gpm);
                  }
                   
                  ptr += rwb_buf;
                  ptr += "<br>";
                }
            
            
            
            
            
            */
            
            }else {
            
            
              // old version
                // one column
/*
                if (items > 25){

                  final_cnt = 25;       // this is for debugging when the upper code fails
                } else {
                  final_cnt = items;
                }
            
  */            
                for (int i=0;i<items;i++){
              
                  ss = (time_t) times[i];
            
                  ptm = gmtime ( &ss);
              
                  if (mode1 == 1){ 
                     
                    sprintf(rwb_buf,"%02d-%02d   %02d:%02d:%02d     0x%02X  %03d",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,values[i],values[i]);

                    //sprintf(rwb_buf,"%02d-%02d   %02d:%02d:%02d     0x%02X  %d",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,values[i],values[i]);
                  
                  } else { // pulses?
              
                    if (old_secs == 0){
              
                      gpm = 0;
                      
                    } else {
              
                      gpm = 6000.0 / (float)(times[i] - old_secs); // gallons / minute = 100 gallons / pulse * 1 pulse / seconds * 60 seconds / minute
                    }
              
                    old_secs = times[i];
              
                    sprintf(rwb_buf,"%02d-%02d   %02d:%02d:%02d   gpm %.2f",ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,gpm);
                  }
                   
                  ptr += rwb_buf;
                  ptr += "<br>";
                }
            
            
            
            
            
            
            }
            
            
            
            if (mode1 != 1){
            
              ptr += "<br>";
              
              sprintf(rwb_buf,"first day %d second day %d pulses in last 24 hours %d",first_day_count, second_day_count,today_count);
              ptr += rwb_buf;
            }
  //}
   
  ptr += "<br><br>"; 

  handle_malloc();
}


void handle_malloc(){
  


  if (need_free_values == 1){
    
    free(values);
    need_free_values = 0;
  }

  
  if (need_free_times == 1){
    
    free(times);              // I don't know if it matters, but I swapped the order here 8-28-2022 because stackexchange says free the last allocated first
    need_free_times = 0;
  }
}






void  show_big_buffer(String & ptr){  //-------------------------------------------------------------------------------------------------------------------------------------------

  //int off;
   
  char str1[20*4];
  char str2[5];

  int cc = big_uart_ptr;
 

  for (int i=0;i< (big_buff_size / 20);i++){

    ptr += tr_td;
    
    for (int c = 0;c<20;c++){

        if (c == 0){
  
          sprintf(str1,"%02X ",big_uart_buff[cc]);
          
        } else {
  
          sprintf(str2,"%02X ",big_uart_buff[cc]);
          
          strcat(str1,str2);        
        }

        cc++;

        if (cc >= big_buff_size){

          cc = 0;
        }
    }
    
    ptr += str1;
  }
}








void  show_recent_packets(String & ptr){  //-------------------------------------------------------------------------------------------------------------------------------------------

  // start at ptr and show every packet, rolling over to do the entire buffer

  

  ptr +="<br><br>";

  int start1 = big_uart_ptr;
  int next1;

  uint8_t current,next;

  int cnt=0;

  while(cnt<big_buff_size){

    yield();
    
    current = big_uart_buff[start1];

    next1 = start1 + 1;

    if (next1 >= big_buff_size){
      
      next = big_uart_buff[0];
    
    } else {
      
      next = big_uart_buff[next1];
    }
  
    if ((current == 0x26) && (next == 0x26)){ // assume this is a string of start bytes

      ptr +="<br><br>";
       
      while ((current == 0x26) && (cnt < big_buff_size)){

        cnt++;
        
        start1++;
        
        if (start1 >= big_buff_size){
          start1 = 0;
        }
        
        current = big_uart_buff[start1];
      }
      
      if (cnt < big_buff_size){
        sprintf(rwb_buf,"%02X ",current);
        ptr += rwb_buf;
      }
      
    } else {
      
      sprintf(rwb_buf,"%02X ",current);
      ptr += rwb_buf;
      
    }

    start1++;
    
    if (start1 >= big_buff_size){
      start1 = 0;
    }
    cnt++;

   
  }
 ptr +="<br><br>";
 
}







void malloc_test(){
  /*
   * 
    heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. it varies with design file size but should be the same for 
                               // a particular file
  buffer = (char*) malloc (i+1);
  if (buffer==NULL) exit (1);

  for (n=0; n<i; n++)
    buffer[n]=rand()%26+'a';
  buffer[i]='\0';

  printf ("Random string: %s\n",buffer);
  free (buffer);
*/
}








void show_elapsed(String & ptr,int  msg,uint32_t epoch){ //"ntp bad",last_bad_ntp_epoch)

 char shit[100]; // i thought it was another char arg bug, but i had fucked up the string down there, so all the other approaches would probably have been ok

 switch (msg){

  case 1:
    sprintf(shit,"dream good");
    break;
  case 2:
    sprintf(shit,"dream bad");
    break;
  case 3:
    sprintf(shit,"biz good");
    break;
  case 4:
    sprintf(shit,"biz bad");
    break;
  
  
 }


  char buf[200];

  if (epoch < boot_epoch){
    
    sprintf(buf,"%s never happened",shit);
    
    ptr += buf;
    ptr += "<br>";
    
  } else {

    uint32_t secs = my_epoch - epoch;

    uint32_t hrs = secs / 3600;

    secs = secs - hrs * 3600;

    uint32_t mins = secs / 60;

    secs = secs - mins * 60;

    sprintf(buf,"%s was %d:%d:%d ago",shit, hrs,mins,secs);
    ptr += buf;

    ptr += "<br>";
  }
  
  
}


void parse_datecode_to_ptr(String & ptr){ 


  // <<<< this slows down the main page if i show it all the time <<<<<<<<

  
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

   */

//2021052_133_2021053_143_2021054_123_

  //const char * dc = "2021052_133_2021053_143_2021054_123_";

  char dc[200];

  read_datecode_file(dc);

  //ptr +="<br><br>";

  ptr +="datecodes: ";

  ptr += dc;

  ptr +="<br><br>";
/*
  int cnt = 0;
  
  char * paych = strdup(dc);// strdup(&dc[0]); // strdup copies dc, and needs to be freed when done
   
  char * save_paych = paych; // save so i can free, which requires original pointer value, and strsep alters paych

  char * token;

  byte index = 0;

  int year, gals, day;

  int gals2[10];
  int years2[10];

  int raptr = 0;

  char buf[100];

  if (strlen(paych) > 0){


    int len1 = strlen(paych);
    int codes = len1 / 12;

    ptr +="<br><br>";
    
    sprintf(buf,"length %d codes %d",len1,codes);

    ptr += buf;

    ptr +="<br><br>";
    
    token = strsep(&paych,"_");

    while (strlen(token) > 0){

      if (index == 0){

        index = 1;
        ptr += " date ";

        day = atoi(token); //.c_str());

        years2[raptr] = day;

        year = day / 1000;

        day = day - year * 1000;

        sprintf(buf," year is %d  day is %d ",year,day);

        ptr += buf;
        
      } else {

        ptr += " gals ";

        gals = atoi(token); //.c_str());

        gals2[raptr] = gals;

        raptr++;

        sprintf(buf," gals number  is %d ",gals);

        ptr += buf;
        index = 0;
      }
      
      ptr += token;
      ptr +="<br><br>";
      
      token = strsep(&paych,"_");

      if ((token) == NULL){ //"\0"){

        break;
      }
      
    }
  }

  free(save_paych);
  */
}







  //-------------------------------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------------------------------------



String SendHTML(uint16_t button_clicked){    // this prepares one long string that defines the html for the web page. it uses the two arguments to determine what to show on the buttons, and how to react when a button is clicked
                                                                      // but then I changed a bunch of stuff: the second arg led2stat is not used anywhere, but it's easier to leave it in place for now. delete it later since i use the third argument
                                                                      // for my stuff. the first arg just turns the first button color "on" and "off" in case i want to use a button as a toggle and then I'll have the code here, even though it serves
                                                                      // no purpose now. it does not turn the led on or off, and the button does the same thing whether you click it on or off

  
  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>LED Control</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  //ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  //ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";  this makes the titles above the buttons grey. delete this and they are black. not sure how the font size is controlled
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";

  
  /*
   *    <p> paragraph
   *    
   *    <a> is a hyperlink. href can be a url. in this case it seems that href connects to the button code. the OFF and ON between <a...> and </a> is the button label. the paragraph part, like "LED1 Status: On", is shown above the button
   *    
   *    

        The style attribute is used to add styles to an element, such as color, font, size, and more.
        Example   <p style="color:red;">This is a red paragraph.</p> 
   */

   
  ptr +="<TABLE>";


  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
  
  
    
  ptr +="<p>cycle display</p><a class=\"button button-off\" href=\"/button0\">0</a>\n";       // but0  page 0
  
  ptr +="<td>";
  ptr +="<p>info</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";                 // but1  page 0
        
  ptr +="<td>";
  ptr +="<p>show files</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 0
      
  ptr +="<td>";
  ptr +="<p>fast log</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";                  // but3  page 0
      
  ptr +="<td>";
  ptr +="<p>show file</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";            // but4  page 0
      
  ptr +="<td>";
  ptr +="<p>show log</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";             // but5  page 0
      
  ptr +="<td>";
  ptr +="<p>bad news log</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";         // but6  page 0
      
  ptr +="<td>";
  ptr +="<p>send rpi temps</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";                   // but7  page 0
          
  ptr +="<td>";
  ptr +="<p>send RS</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";                  // but8  page 0
      
  ptr +="<td>";
  ptr +="<p>send flow adc</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                  // but9  page 0
    
  ptr +="<td>";
  ptr +="<p>server millens</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";                // but10  page 0
    
  ptr +="<td>";
  ptr +="<p>page 0</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";           // but11  page 0
    
  ptr +="<td>";
  ptr +="<p>routine log</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";    // but12  page 0
    


//--------------------------------

  byte done44 = 0;

  ptr += tr_td;

  switch (button_clicked){

    case 0:                 // but0 
    
      show_recent_packets(ptr);
    
      show_big_buffer(ptr);

      done44 = 1; // don't put anything else on web page
      break;
      
    case 2: //     // but2 page 0 

      show_files(ptr);
      done44 = 1; // don't put anything else on web page

      break;

    case 3:

      //show_fast_log(ptr);
      show_a_log(ptr, fast_log_ptr, fast_log_wrap, fast_log, log22_size);
      done44 = 1;
      break;

    case 4:           // but4 page 0
      
      show_arrays(ptr); // this frees a lot of ram from malloc earlier for but 4. this has to follow all those test_read... calls or it will crash. or maybe not, i put a call to handle_malloc() at the end of the web page
      done44 = 1; // don't put anything else on web page
      break;
      
    case 5:             // but5 page 0
      
      
      //show_log(ptr);
      show_a_log(ptr, log22_ptr, log22_wrap, log22, log22_size);
      done44 = 1; // don't put anything else on web page
      break;
      
    case 6:             // but6 page 0
      
      
      //show_bad_news(ptr);
      show_a_log(ptr, bad_news_ptr, bad_news_wrap, bad_news, bad_news_log_size);
      done44 = 1; // don't put anything else on web page
      break;
      

    case 12:           //  but12 page 0
      
      //show_routine_log(ptr);
      show_a_log(ptr, routine_log_ptr, routine_log_wrap, routine_log, routine_log_size);
      done44 = 1; // don't put anything else on web page
      
      break;

    default:

      break;
  }


//--------------------------------


//--------------------------------
    uint32_t heap;

    if (done44 == 0){

        ptr += tr_td;
      
        uint32_t days,hrs,mins,secs;
      
        days = (my_epoch - boot_epoch) / 86400;
      
        secs = (my_epoch - boot_epoch) - days * 86400;
      
        hrs = secs / 3600;
      
        secs -= hrs * 3600;
      
        mins = secs / 60;
      
        secs -= mins * 60;
        
        sprintf(rwb_buf,"uptime days %d ... %d:%02d:%02d    ",days, hrs, mins, secs);        
        
        ptr += rwb_buf;

      
        days = (my_epoch - last_master_rtc_sync) / 86400;
      
        secs = (my_epoch - last_master_rtc_sync) - days * 86400;
      
        hrs = secs / 3600;
      
        secs -= hrs * 3600;
      
        mins = secs / 60;
      
        secs -= mins * 60;
        
        sprintf(rwb_buf,"    last master rtc sync days %d ... %d:%02d:%02d    ",days, hrs, mins, secs);        
        
        ptr += rwb_buf;
      
        ptr += tr_td;
        
      
        show_status_bits(ptr);
        ptr += "<br><br>";
      
        //--------------------------------
      
        
        heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 
      
      
        sprintf(rwb_buf,"heap start %d buffer %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X date %d month %d year %d doy %d",
        heap,
        uart_buff[0],uart_buff[1],uart_buff[2],uart_buff[3],
        uart_buff[4],uart_buff[5],uart_buff[6],uart_buff[7],
        uart_buff[8],uart_buff[9],uart_buff[10],uart_buff[11],
        uart_buff[12],uart_buff[13],uart_buff[14],uart_buff[15], // hmm, should unsigned use u instead of d
        uart_buff[16],uart_buff[17],uart_buff[18],uart_buff[19],
        monthDay,currentMonth,   currentYear,day_of_year); 
      
        ptr += tr_td;
        
        ptr += rwb_buf;
      
        //-------------------------------------

        sprintf(rwb_buf,"current tank %02X status %02X trans_plus_flow %d ",current_tank,current_status,trans_plus_flow);
      
        ptr += tr_td;
        
        ptr += rwb_buf;

      
      /*
        // show millis for a few uart bytes
      
        sprintf(rwb_buf,"uart millis %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d current tank %02X status %02X trans_plus_flow %d ",
        my_millis_ra[0],my_millis_ra[1],my_millis_ra[2],my_millis_ra[3],my_millis_ra[4],
        my_millis_ra[5],my_millis_ra[6],my_millis_ra[7],my_millis_ra[8],my_millis_ra[9],
        my_millis_ra[10],my_millis_ra[11],my_millis_ra[12],my_millis_ra[13],my_millis_ra[14],
        my_millis_ra[15],my_millis_ra[16],my_millis_ra[17],my_millis_ra[18],my_millis_ra[19],current_tank,current_status,trans_plus_flow);
      
      
        clear_my_millis();
        
        ptr += tr_td;
        
        ptr += rwb_buf;
      
      
        //show_status_bits(ptr);
      
      */
      
      //--------------------------------
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"resets32 %d todays_seconds() %d",reset_cnt_32,todays_seconds());
        
        
        ptr += rwb_buf;
      
      
        //--------------------------------
      
        
        ptr += tr_td;
      
        sprintf(rwb_buf,"packets 0xAA %d ... 0x15 %d ... 0x16 %d ... unknown %d ... yesterday gallons %d",
        packets_0xAA, packets_0x15, packets_0x16, total_unknown_packets,yesterday_gallons);
        
        
        ptr += rwb_buf;
      
      
      //--------------------------------
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"spiffs unused blocks %d fast_log_wrap %d ptr %d bad_log_cnt %d %d %d",(spiffs_total - spiffs_used) / spiffs_block,fast_log_wrap,fast_log_ptr,bad_log_cnt,bad_ln,bad_actual_ln);
      
        
        ptr += rwb_buf;
      
        
        ptr += tr_td;
      
        sprintf(rwb_buf,"checksums: good %d bad %d debugs %d %d %d %d %d %d  %d %d current tank %d last_tank_write %d, current air %d last air write %d",
        good_ck,bad_ck,debug0,debug1,debug2,debug3,debug4,debug5,debug6,debug7,current_tank, last_tank_write, current_air, last_air_write); 
        ptr += rwb_buf;
      
      //--------------------------------
      
        ptr += tr_td;
      
        sprintf(rwb_buf,"last packet %d ",seconds_since_last_packet);
      
        ptr += rwb_buf;
      
      
      
        ptr += tr_td;
        ptr += "<br>";      
        
        sprintf(rwb_buf,"dream: good pings today %d bad pings today %d",good_pings_today_dream,bad_pings_today_dream);
      
        ptr += rwb_buf;
      
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"biz: good pings today %d bad pings today %d<br>", good_pings_today_biz,bad_pings_today_biz);
      
        ptr += rwb_buf;
        

        calculate_clock_errors();

        ptr += "<br>";      

        // this block is two sets of text on one row
      
        ptr += tr_td_half;    // this has colspan 6, so the td colspan 7 fits on the same row

        show_elapsed(ptr,1,last_good_dream_epoch);

        // this block is two sets of text on one row

        ptr += tr_td_half;
        show_elapsed(ptr,2,last_bad_dream_epoch);

        
        ptr += "<td colspan='7'>";
        sprintf(rwb_buf,"my - rtc %d",myerr);
        ptr += rwb_buf;


        // this block is two sets of text on one row

        //ptr += tr_td;
        ptr += tr_td_half;
        
        
        ptr += "<td colspan='7'>";
        sprintf(rwb_buf,"rtc - dream %d",rtc_vs_dream);
        ptr += rwb_buf;

        



        ptr += tr_td;

        ptr += "<br>";
        
        sprintf(rwb_buf,"...... bad dream %d",bad_dream_cnt);
        ptr += rwb_buf;

        ptr += "<br>";
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"rpi_temperature %d rpi_today_max %d rpi_today_min %d rpi_yesterday_max %d rpi_yesterday_min %d rpi_max %d rpi_ack_secs %d",rpi_temperature,rpi_today_max,rpi_today_min,rpi_yesterday_max,rpi_yesterday_min,rpi_max,rpi_ack_secs);
      
        ptr += rwb_buf;
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"master_pulse_cnt %d ... yesterday_master_cnt %d ... gallons_per_master_cnt %d offset_cnt %d  offset + master %d",master_pulse_cnt,yesterday_master_cnt, gallons_per_master_cnt,offset_cnt, offset_cnt+master_pulse_cnt);
      
        ptr += rwb_buf;
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"rtc epoch %lld  %08X my_epoch now %lld %08X seconds_since_last_pulse %d",rtc_epoch,(unsigned int) rtc_epoch,my_epoch,(unsigned int) my_epoch,seconds_since_last_pulse);
      
        ptr += rwb_buf;
      
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"dream_dead %d  fake dream dead %d secs to next new ping %d ......  in charge: rtc %d 0x15 %d online %d  0xAA %d",
        dream_dead,fake_dream_is_dead,dream_new_ping_cnt,in_charge_rtc, in_charge_0x15,in_charge_online,in_charge_0xAA);
      
        ptr += rwb_buf;
      
        ptr += tr_td;
        
        sprintf(rwb_buf,"master clock error %d resets %d xbee RS %hu min %hu max %hu 0x39 %hu min %hu max %hu xbee_reset_cnt %d restart_wifi_cnt %d restart_wifi_secs %d",
        master_err,master_resets,xbee_RS,min_xbee_RS,max_xbee_RS, xbee_RS_39, min_xbee_RS_39, max_xbee_RS_39, xbee_reset_cnt,restart_wifi_cnt,restart_wifi_secs);
      
        ptr += rwb_buf;
      
            
        ptr += tr_td;
        sprintf(rwb_buf,"current wifi signal strength RSSI = %d. range is -0 strong to -100 weak. i get about -45 in front room to -80 in back room<br><br>",WiFi.RSSI());
        ptr += rwb_buf;

        /*
        ptr += tr_td;
        
        heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 
        sprintf(rwb_buf,"heap after building html %d",heap);
        
        ptr += rwb_buf;
        */
      
  }


  ptr += tr_td;
  
  heap = ESP.getFreeHeap();  // if this number drops over time it probably means memory is leaking. 
  sprintf(rwb_buf,"heap after building html %d",heap);
  
  ptr += rwb_buf;


  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
    
  return ptr;


}



















String SendHTML_1(uint16_t button_clicked){    

  
  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>LED Control</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
 // ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>server stuff</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";               // but0 page 1    
  
  ptr +="<td>";
  ptr +="<p>clock</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";              // but1  page 1
      
  ptr +="<td>";
  ptr +="<p>local</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";                 // but2  page 1
      
  ptr +="<td>";
  ptr +="<p>reset_air_file</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";                // but3  page 1
      
  ptr +="<td>";
  ptr +="<p>source select</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";                          // but4  page 1
      
  ptr +="<td>";
  ptr +="<p>dst</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";                          // but5  page 1
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";                // but6  page 1
      
  ptr +="<td>";
  ptr +="<p>flow cutoff</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";                  // but7  page 1
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";                   // but8  page 1
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                       // but9  page 1

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";                   // but10  page 1
    
  ptr +="<td>";
  ptr +="<p>page 1</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";                   // but11  page 1
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";                  // but12  page 1
    

  ptr += tr_td;
  sprintf(rwb_buf,"page %d button %d",web_page,button_clicked);
  ptr += rwb_buf;

  ptr += "<br><br><br>";


  ptr += tr_td;
  sprintf(rwb_buf,"server stuff can force send 24 hour block, or pretend a site is dead for a while<br><br>");
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"clock can sync various things to dream server millenium<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"local sets flag to ack 0x15, handle master rtc, send packets to web and handle php dst flag, and can reset xbee, read RS, scan I2C<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"source select handles x10 vs android data for online plots<br><br>");
  ptr += rwb_buf;

  ptr += tr_td;
  sprintf(rwb_buf,"dst calculates dst date in march and november for the next few years, and sets or clears server flag, which can be automatically handled by flag in local menu<br><br>");
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"flow cutoff page edits that value and sends to master and php. master value is more important because it determines when flow is considered on. php flag is just for the flow adc plot<br><br>");
  ptr += rwb_buf;
/*
  ptr += tr_td;
  sprintf(rwb_buf,"xbee reset will lower the reset line until you click this again. the red led should go out during reset<br><br>");
  ptr += rwb_buf;

  
  ptr += tr_td;
  sprintf(rwb_buf,"get RS will show the most recent RS on the info screen<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"scan I2C will look for all devices and will list those that are found in the log<br><br>");
  ptr += rwb_buf;
  */

  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;

}






















String SendHTML_100(uint16_t button_clicked){    // page 100 edits and sends flow cutoff

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>flow cutoff</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
 // ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>send 2 dream</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";       // but0 page 100
  
  ptr +="<td>";
  ptr +="<p>ask master</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";           // but1  page 100
      
  ptr +="<td>";
  ptr +="<p>+1</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 100
      
  ptr +="<td>";
  ptr +="<p>-1</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";         // but3  page 100
      
  ptr +="<td>";
  ptr +="<p>+10</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 100
      
  ptr +="<td>";
  ptr +="<p>-10</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";                // but5  page 100
      
  ptr +="<td>";
  ptr +="<p>send 2 mstr</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";         // but6  page 100
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 100
          
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";                // but8  page 100
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 100
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 100
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 100

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 100
    


  ptr += tr_td;
  sprintf(rwb_buf,"page %d button %d",web_page,button_clicked);
  ptr += rwb_buf;


  ptr += "<br><br><br>";


  ptr += tr_td;
  sprintf(rwb_buf,"temporary_flow_cutoff %d %02X is reset each time you come to this page. edit it here and send to master to make it permanent. after 0xAA packet returns new value it can be sent online<br><br>",temporary_flow_cutoff,
  temporary_flow_cutoff);
  ptr += rwb_buf;

  sprintf(rwb_buf,"current flow_cutoff %d %02X from the last 0xAA packet<br><br>",flow_cutoff,flow_cutoff);
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"send 2 dream sends the value for drawing the red threshold line on the flow ADC plot, and is the value from the most recent master packet %d %02X<br><br>",flow_cutoff,flow_cutoff);
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"ask master sends 0xDD packet to master who should respond  07 00 07 DE 66 66 00 3C E5<br>");
  ptr += rwb_buf;
  sprintf(rwb_buf,"those bytes are byte_count obsolete obsolete type=0xDE cut12 cut34 pause_seconds_msb_lsb ck<br><br>");
  ptr += rwb_buf;

  
  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;

}





String SendHTML_200(uint16_t button_clicked){    // page 200 tests dst code

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>testing</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
 // ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";       // but0 page 200
  
  ptr +="<td>";
  ptr +="<p>query servers</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";           // but1  page 200
      
  ptr +="<td>";
  ptr +="<p>both server 0</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 200
      
  ptr +="<td>";
  ptr +="<p>both server 1</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";         // but3  page 200
      
  ptr +="<td>";
  ptr +="<p>november</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 200
      
  ptr +="<td>";
  ptr +="<p>march</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";                // but5  page 200
      
  ptr +="<td>";
  ptr +="<p>next year</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";         // but6  page 200
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 200
          
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";                // but8  page 200
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 200
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 200
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 200

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 200
    




  ptr += tr_td;
  sprintf(rwb_buf,"page %d button %d",web_page,button_clicked);
  ptr += rwb_buf;


  ptr += "<br><br><br>";


  
  ptr += tr_td;
  sprintf(rwb_buf,"query servers will get dst flag from both and add to log<br><br>");
  ptr += rwb_buf;

  
  ptr += tr_td;
  sprintf(rwb_buf,"both server 1 or 0 sets the flags; this is done automatically if flag is set in local menu<br><br>");
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"server_info.php web page has lots of info about server date and time<br><br>");
  ptr += rwb_buf;

  
  ptr += tr_td;
  sprintf(rwb_buf,"the rest of this page is used to confirm the dst math for the next few years, via the november, march, and next_year buttons<br><br>");
  ptr += rwb_buf;

  struct tm *nptm = gmtime ((time_t *)&test_dst_epoch); 

  ptr += tr_td;
  sprintf(rwb_buf,"test_dst_epoch %lld<br><br> %d-%d-%d %d:%d:%d ..... day of week %d<br><br>",test_dst_epoch,nptm->tm_mon+1,nptm->tm_mday,nptm->tm_year+1900,nptm->tm_hour,nptm->tm_min,nptm->tm_sec,nptm->tm_wday);
  ptr += rwb_buf;

  
  
  ptr += tr_td;
  sprintf(rwb_buf,"is_dst_on_now() returned %d (1 means on, 0 means off. this is based on the current time, and is not a test of online dst_flags)<br><br>",is_dst_on_now());
  ptr += rwb_buf;
  
  
  

  ptr += tr_td;
  sprintf(rwb_buf,"2021 march 14 november 7<br><br>");
  ptr += rwb_buf;

  ptr += tr_td;
  sprintf(rwb_buf,"2022 march 13 november 6<br><br>");
  ptr += rwb_buf;

  ptr += tr_td;
  sprintf(rwb_buf,"2023 march 12 november 5<br><br>");
  ptr += rwb_buf;

  ptr += tr_td;
  sprintf(rwb_buf,"2024 march 10 november 3<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"2025 march 9 november 2<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"2026 march 8 november 1<br><br>");
  ptr += rwb_buf;
  
  ptr += tr_td;
  sprintf(rwb_buf,"2027 march 14 november 7<br><br>");
  ptr += rwb_buf;

  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}















String SendHTML_201(){ 

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>source select</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
//  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>---</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";       // but0 page 201
  
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";           // but1  page 201
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 201
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";         // but3  page 201
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 201


  ptr +="<td>";
  ptr +="<p>biz jss 1</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";           // but5  page 201
      
  ptr +="<td>";
  ptr +="<p>biz jss 2</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";           // but6  page 201
      
  ptr +="<td>";
  ptr +="<p>dream jss 1</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 201
      
  ptr +="<td>";
  ptr +="<p>dream jss 2</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";         // but8  page 201
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 201
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 201
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 201

  ptr +="<td>";
  ptr +="<p>---</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 201


  ptr += tr_td;
  sprintf(rwb_buf,"page %d",web_page);
  ptr += rwb_buf;


  ptr += "<br><br><br>";


    
  ptr += tr_td;
  ptr += "<br><br><br>";
  sprintf(rwb_buf,"jss 1 tells top dir to use data from the old top directory, android data. jss 2 uses esp data from the x10 directory. look for the tiny x10 to verify.<br><br>");
  ptr += rwb_buf;

  
  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}








String SendHTML_202(){ 

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>source select</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
//  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";       // but0 page 202
  
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";           // but1  page 202
      
  ptr +="<td>";
  ptr +="<p>in_charge_0xAA</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 202
      
  ptr +="<td>";
  ptr +="<p>in_charge_0x15</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";         // but3  page 202
      
  ptr +="<td>";
  ptr +="<p>in charge rtc</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 202


  ptr +="<td>";
  ptr +="<p>sleep</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";           // but5  page 202
      
  ptr +="<td>";
  ptr +="<p>in_charge_online</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";           // but6  page 202
      
  ptr +="<td>";
  ptr +="<p>awake</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 202
      
  ptr +="<td>";
  ptr +="<p>reset histo</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";         // but8  page 202
      
  ptr +="<td>";
  ptr +="<p>reset xbee</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 202
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 202
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 202

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 202


  ptr += tr_td;
  sprintf(rwb_buf,"page %d",web_page);
  ptr += rwb_buf;


  ptr += "<br><br><br>";


  
  ptr += tr_td;
  
  sprintf(rwb_buf,"in_charge_0xAA %d will ack 0xAB if this is 1<br><br>",in_charge_0xAA);

  ptr += rwb_buf;

  
  ptr += tr_td;
  
  sprintf(rwb_buf,"in_charge_0x15 %d will ack 0x16 if this is 1<br><br>",in_charge_0x15);

  ptr += rwb_buf;


  ptr += tr_td;
  
  sprintf(rwb_buf,"in_charge_rtc %d will handle master rtc if this is 1, including clock drift and dst<br><br>",in_charge_rtc);

  ptr += rwb_buf;
  
  
  ptr += tr_td;
  
  sprintf(rwb_buf,"in_charge_online %d will do all the web stuff, including server dst flags if this is 1<br><br>",in_charge_online);

  ptr += rwb_buf;


  ptr += tr_td;
  
  sprintf(rwb_buf,"reset histo will clear xbee RS history<br><br>");

  ptr += rwb_buf;
  
  
  ptr += tr_td;
  
  sprintf(rwb_buf,"reset xbee will set min to 99 and max to 0<br><br>");

  ptr += rwb_buf;

  
  /*
  ptr += tr_td;
  sprintf(rwb_buf,"current wifi signal strength RSSI = %d. range is -0 strong to -100 weak. i get about -45 in front room to -80 in back room<br><br>",WiFi.RSSI());
  ptr += rwb_buf;
*/

  ptr += tr_td;
  
  sprintf(rwb_buf,"xbee history 0xAA shed ..... 0x39 sewer<br><br>");

  ptr += rwb_buf;

  



  //#define xbee_histo_size 40

  //uint16_t xbee_histo[xbee_histo_size + 1]; // save a count of RS for values 15..54

  //int ind = (int)xbee_RS - 15; // 15 goes in slot 0, 54 - 15 goes in slot 39
  
  int indl = 0;

  for (int l=15; l<25; l++){  // 15 .. 24 will be in the first column. second column 25..34, then 35..44, 45..54

    ptr += tr_td;
    
    sprintf(rwb_buf,"%02d %05d %02d %05d %02d %05d %02d %05d ..... %02d %05d %02d %05d %02d %05d %02d %05d",
    
    l,xbee_histo[indl],  l+10,  xbee_histo[indl + 10],   l+20,  xbee_histo[indl + 20],   l+30,  xbee_histo[indl + 30],
    
    l,xbee_histo_39[indl],  l+10,  xbee_histo_39[indl + 10],   l+20,  xbee_histo_39[indl + 20],   l+30,  xbee_histo_39[indl + 30]);     
    
    ptr += rwb_buf;

    indl++; // indl = 0..9, so last column is 30..39 
        
  }


      
  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}







String SendHTML_203(){ 

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>source select</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
 // ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";



  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>force 24 dream</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";               // but0 page 203
  
  ptr +="<td>";
  ptr +="<p>fake dream dead</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";              // but1  page 203
      
  ptr +="<td>";
  ptr +="<p>force 24 biz</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";                 // but2  page 203
      
  ptr +="<td>";
  ptr +="<p>fake biz dead</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";                // but3  page 203
            
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 203


  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";           // but5  page 203
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";           // but6  page 203
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 203
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";         // but8  page 203
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 203
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 203
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 203

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 203


  ptr += tr_td;
  sprintf(rwb_buf,"page %d",web_page);
  ptr += rwb_buf;

  ptr += "<br><br><br>";

  ptr += tr_td;
  sprintf(rwb_buf,"force 24 means it will simulate that site having been dead for at least 24 hours, and as soon as it acks a ping, the esp will send 24 hour blocks of everything,");
  ptr += rwb_buf;

  ptr += tr_td;
  sprintf(rwb_buf,"and it will overwrite any data already there.<br><br>");
  ptr += rwb_buf;


  
  ptr += tr_td;
  sprintf(rwb_buf,"fake dead means the esp will not try to send anything to that site, and will not ping to see if it's alive.<br><br>");
  ptr += rwb_buf;
  
  
  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}







String SendHTML_204(){    

  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  
  ptr +="<title>LED Control</title>\n";
  
  ptr +="<style>html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}\n";
  
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  
  ptr +=".button-on {background-color: #1abc9c;}\n";

  ptr +=".button-page {background-color: #0066cc;}\n";
  
  ptr +=".button-on:active {background-color: #16a085;}\n";
  
 // ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  
  ptr +="<body>\n";
   
  ptr +="<TABLE>";
  
  //ptr += tr_td; don't use this here. wrecks everything
  ptr +="<tr>";
  ptr +="<td>";
    
  ptr +="<p>send mstr rtc</p><a class=\"button button-on\" href=\"/button0\">0</a>\n";       // but0 page 204
  
  ptr +="<td>";
  ptr +="<p>test bin2bcd</p><a class=\"button button-on\" href=\"/button1\">1</a>\n";           // but1  page 204
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button2\">2</a>\n";           // but2  page 204
      
  ptr +="<td>";
  ptr +="<p>sync rtc to drm</p><a class=\"button button-on\" href=\"/button3\">3</a>\n";         // but3  page 204
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button4\">4</a>\n";         // but4  page 204
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button5\">5</a>\n";                // but5  page 204
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button6\">6</a>\n";         // but6  page 204
      
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button7\">7</a>\n";         // but7  page 204
          
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button8\">8</a>\n";                // but8  page 204
      
  ptr +="<td>";
  ptr +="<p>sync my_epoch</p><a class=\"button button-on\" href=\"/button9\">9</a>\n";                // but9  page 204
    
  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button10\">10</a>\n";              // but10  page 204
    
  ptr +="<td>";
  ptr +="<p>return</p><a class=\"button button-page\" href=\"/button11\">11</a>\n";         // but11  page 204

  ptr +="<td>";
  ptr +="<p>-</p><a class=\"button button-on\" href=\"/button12\">12</a>\n";              // but12  page 204
    


  ptr += tr_td;
  sprintf(rwb_buf,"page %d",web_page);
  ptr += rwb_buf;


  ptr += "<br><br><br>";

  ptr += tr_td;
  sprintf(rwb_buf,"send mstr rtc sends xbee packet to master using my_epoch (in_charge_rtc must be 1)<br><br>");
  ptr += rwb_buf;

  
  ptr += tr_td;
  sprintf(rwb_buf,"sync rtc to dream<br><br>");
  ptr += rwb_buf;


  ptr += tr_td;
  sprintf(rwb_buf,"sync my_epoch to rtc.<br><br>");
  ptr += rwb_buf;

    

  ptr +="</TABLE>";
  
 
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;

}
















void start_wifi(int mode_start){ // added 1-1-2022 for wifi_select_pin. mode_start = 0 for normal, 1 for restart. I have no idea what it does, but my old restart used wifi_station_connect() but normal start did not

       WiFi.mode(WIFI_STA);
       
       if (mode_start == 1){
          wifi_station_connect();
       }
     
      if (digitalRead(wifi_select_pin) == HIGH){  // high is no jumper = old wifi
    
        WiFi.begin(ssid, password);   // I have to enable this if I use a new device that does not have my router ssid/pwd already stored in flash.
    
      } else {
    
        WiFi.begin(new_ssid, password);   // I have to enable this if I use a new device that does not have my router ssid/pwd already stored in flash.
      }

  
}







void restart_wifi(){    // reset_wifi()

  // this seems to have issues if i do it from a web page button before the web page is done. if i close the web page just after i click the esp seems to be ok. so, for testing i need to set a flag to 
  // call this after a few seconds.

  add_bad_news((char *) "restart wifi");

    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();


    for (int i=0;i<20;i++){ // pause 10 seconds
      
      delay_half_second();
    }



     WiFi.forceSleepWake();

     start_wifi(1);
     /*
     WiFi.mode(WIFI_STA);
     wifi_station_connect();
     WiFi.begin(ssid, password); 
*/

  /*  
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1); //Needed, at least in my tests WiFi doesn't power off without this for some reason

  
    guy said this will not re-connect:
  
             WiFi.forceSleepWake();
             delay(1);
             WiFi.mode(WIFI_STA);
             WiFi.begin(ssid, password);
  
  
     but said this works:
  
  
       WiFi.forceSleepWake();
       WiFi.mode(WIFI_STA);
       wifi_station_connect();
       WiFi.begin(ssid, password); 


   more stuff to try

      wifi
   close()

   stop()

  //Turn off WiFi
  WiFi.mode(WIFI_OFF);    //This also works
  //WiFi.forceSleepBegin(); //This also works

--Resume wifi from timed or indefinite sleep
wifi.resume()



*/
}














void lf(void){  //-------------------------------------------------------------------------------------------------------------------------------------------

#ifdef serial1
  Serial.print("\n");
  yield();  // needed this for file 326 orangered (url has 324).   363 to get 365 also wdt

#endif
}
