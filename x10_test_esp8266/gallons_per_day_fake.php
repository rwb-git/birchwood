<?php

// more cool info is at this:   http://www.fork20.xyz/birchwood/x10_test_esp8266/server_info.php

   $fgname = "gal_per_day_fake_log.txt";

   $flog = fopen($fgname, 'w');   
   
   fwrite($flog,"<br>code update 5 <br> ");  // one way to indicate that the new file is actually in place

   $handle = fopen("gallons_new", "r");

      if ($handle) {

         $ra_index = 0;

         //$dy87 = 20;    // this is not used in this file; grep says not used anywhere else

         //$debug = 0;    // this is not used in this file. it's in a lot of others...

         $new_y = $tank_y8 + 20;
   
         date_default_timezone_set('UTC');
 
         $php_day = 1 + intval(date("z"));   // java day of year is 1... while php is 0...          as of 1-2-2024 none of these are used except php_hour, and it's just used for the pathetic case where the bar
                                             // colors are "wrong" for a few hours in the morning. goddamn am I actually autistic. 
         $php_hour = intval(date("G"));  // 0..23   does not have leading zeros
         
         $php_minute = intval(date("i"));  // 00..59   has leading zeros
         
         $php_date = intval(date("j"));  // 1..31
                
         $f7pcut = fopen("dst_flag.txt", "r");

         $dst_flag = fgets($f7pcut); 
         
         fclose($f7pcut);

         if ($dst_flag == 1){

            $dst_fix = 1;

         } else {
            
            $dst_fix = 0;     // it looks like dst_flag.txt is manually edited. all i know is that when dst is "on" the value is 1. 
         }

         $total_fix = 5 - $dst_fix; // in the winter, EST = UTC - 5     in the summer EDT = UTC - 4

         if ($php_hour >= $total_fix){
         
            $need_to_fix_server_day = 0;

         } else {
            
            $need_to_fix_server_day = 1;

         }

         //$oldest_year_in_file = 0;  // not used in this file. grep says not used anywhere else

         $pad_days = 0;    // pad days is required because ra_index is based on the day of the year, and at year end pad days fixes this: 364 365 1 2 to this 364 365 366 367 which is adjusted by first_day to 1 2 3 4 

         $last_year = 2017;
         
          $first_day = -1;

          $old_index = 0;

          $fake_day = 0;

          while (($line = fgets($handle)) !== false) {

            /*
               need to use day of year from file to insert in proper array slot so that missing days are shown as zero

               the final value read is for yesterday but the idea still works. just use the first day as zero.
            */
              
               $numbers = explode(" ", trim($line));

               $index=0;

               $code = 0;

               $gallons = 0;                       // clear this in case of a blank line

               foreach ($numbers as &$number){     // if there is a blank line the code below will use the data from the preceding line

                  $number = intval($number);

                  if ($index == 0){

                     $code = $number;
   
                     fwrite($flog,"code  $code   ");

                  } else {

                     $gallons = $number;
   
                     fwrite($flog,"gallons $gallons\n");
                  }

                  $index = $index + 1;
               }

               if ($gallons > 0){

                  $year = intval($code / 1000);

                  if ($last_year == 2017){

                     $last_year = $year; // added 1-2-2024. this might be a mistake, but it should get rid of the stupid 365 fake days added right off the bat. after i added this, missing days at the beginning
                                         // are just blank with no legend. the month name shows up on the first day with data along with the correct legend. the old code with the stupid 365 fake days put the legend
                                         // and month day all the way from the start. austistic rod wants to have that happen without the stupid 365 days, but if it means more un-maintainable code then skip it?
                                         //
                                         // NO WAIT. THE LEGEND IS WRONG, BUT THE MONTH IS CORRECT, so the new code is preferable. keep this change for now. (the legend should have said 03 04 05 06 but had 27 28 29 06
                                         //
                                         // if yesterday is missing then the thing is fucked and thinks "yesterday" is whatever the final entry in the file is. don't even think about trying to fix that shit.

                  } else if ($last_year < $year){

                     fwrite($flog,"<br>last_year $last_year is less than year in file $year ---------------------------------------------------------<br> ");
                     
                     fwrite($flog,"<br>before fix: pad $pad_days  last_year $last_year  year $year<br> ");

                     $last_year = $year;   // so today is jan 1 2020 and it's showing jan 1 twice. gallons_new file has 2019 for every line except the last 2020

                     $pad_days += 365;    // since last_year is initialized as 2017 this will add 365 here, and now last_year == 2019. when we get to the final line I think pad_days will be 365 * 2

                     fwrite($flog,"<br>after fix: pad $pad_days  last_year $last_year <br><br> ");

                     if ((($year-1) % 4) == 0){ // last year was leap year, and the year in the file just changed to the following year, so the last day of last year is 366

                        $pad_days ++;  
                     
                        fwrite($flog,"<br>last year was leap year, so add a pad day: $pad_days  year $year<br><br> ");
                     }
                  }

                  $day = intval($code - ($year * 1000));

                  if ($first_day == -1){
   
                     $first_day = $day;            // on january 1 this will be 1. maybe add 365 for 2018, and worry about 2019 later? or add 365/366 for each year beyond the earliest in the file?

                  }

                  $ra_index = $day - $first_day + $pad_days;   // on january 1, december 31 will be 365 - 1 = 364. ordinarily 

                  fwrite($flog,"<br>ra_index stuff: day $day  first day $first_day pad $pad_days  ra_index $ra_index old_index $old_index <br> ");
                  
                  $temp11 = $old_index + 1;
                  fwrite($flog,"<br>if ra_index $ra_index > $temp11 (old_index + 1) then fix missing days <br> ");


// actual bug. if i add missing days and the year changes in that group, it looks like i use the later year for all the days. 
// i might be able to catch this if i compare the year for the last good data to the year for the good data following the missing days. also, those day numbers will be something like 365 and 1 or similar
// but remember the earlier year might be a leap year. DO I NEED TO CARE ABOUT THIS. NOBODY LOOKS AT THIS SHIT, MUCH LESS gives a damn about wrong dates for missing days. the days that actually have data
// should have the correct numbers. who cares. nobody even looks at this bar chart but me. and this only applies to missing data at the year end.




                  if ($ra_index > ($old_index + 1)){   // example old 4, ra 6. loop will start at 5 and end at 5. this makes missing days appear as blanks instead of being skipped

if (($day < 200) && ($fake_day > 200)){ // the gap includes the end of the year

   $overlap = 1;

   $year --;   // so back up to last year

   if (($year % 4) == 0){  // last year was a leap year

      $eoy = 366;
   } else {
      $eoy = 365; // at this point day and fake day are 1..365 or 1..366. later on we deal with php being 0..364 and 0..365
   }
} else {

   $overlap = 0;
   $eoy = 0;
}
                     fwrite($flog,"<br>fake day $fake_day   day $day   overlap $overlap if overlap == 1 then the gap includes end of year $eoy<br> ");
                     
                     for ($r5= $old_index + 1; $r5 < $ra_index; $r5++){

                        $gallons_ra[$r5] = 1;

                        if ($overlap == 1){
                           
                           if ($fake_day > $eoy){

                              $fake_day = 1;
                              $overlap = 0; // it's been fixed so clear the flag
                           }
                        }

                        $year_ra[$r5] = $year;

                        $day_ra[$r5] = $fake_day; 

                        fwrite($flog,"<br>fix missing day at index $r5, and add a fake day $fake_day for year $year ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++<br> ");
                        fwrite($flog,"day_ra at this index was set to fake_day. this is the only place where fake day is used <br> ");
                        
                        $fake_day++;
                     }
                  }
                        
                  fwrite($flog,"<br>save year $year, day $day, gallons $gallons at this index $ra_index <<<<<<<<<<<<<<<<< <br> ");

                  $old_index = $ra_index;
 
                  $year_ra[$ra_index] = $year;

                  $day_ra[$ra_index] = $day;   // day is from datecode like 2017122 == 122 = day of year from java which uses 1..366 while php uses 0..365

                  $fake_day = $day + 1;   // fake day = the next day in case we have to fill in gap of missing days. this will be the first one and will be incremented if more are needed.
                        
                  fwrite($flog,"<br>fake day = $fake_day = day + 1 in case we need to add a missing day, this will be the day number  <br> ");

                  $gallons_ra[$ra_index] = $gallons;

               }  // if gallons on that line in the file is > 0

         } // read each line in gallons_new file

         $ra_index++; // old code had this extra inc, so it's easiest to use day for index by doing this 

         fclose($handle);











// everything above sorted out the file. the bottom section plots it
// everything above sorted out the file. the bottom section plots it
// everything above sorted out the file. the bottom section plots it
// everything above sorted out the file. the bottom section plots it
// everything above sorted out the file. the bottom section plots it











            $x = $plot_x2 - 2;

            if ($ra_index > 0){


               $day2 = intval(date("w")) - 1; // 0 is sunday. this is today and the plot starts at yesterday. the server is 5 or 6 hours ahead of me, so this will be wrong from about 7 pm until midnight

                  // I subract 1 because the plot starts at yesterday

                  if ($need_to_fix_server_day == 1){ // for a few hours each night, UTC has already gone to the next day, so this corrects the bar colors during those hours

                     $day2 -= 1;
                  }

               if ($day2 < 0){   // day2 is used for the colors that make sunday easy to identify

                  $day2 = $day2 + 7; //6;

               }
/*
               if ($php_date != $date2){ // germany has already moved to tomorrow == this should only happen late in the day after 8 or so; after midnight my date2 should be the same

// php 8 broke because date2 not defined. date2 is old-style code for yesterday from the small time file. day2 is used for weekday colors in bar chart, and this is why new is not same color as old for sunday, etc.


                  $day2--;

                  if ($day2 < 0){

                     $day2 = 6;

                  }


               }
*/
               $days = array("S", "M", "T", "W", "T", "F", "S");  // is this ever used? not in this file.


               // with 24 pixels per day, 288 x 3 / 24 = 36 days.

               $bars = 30; // show this many days

               $dx = 864 / $bars;

               if ($ra_index > $bars){  // 288 * 3 = 2^5 x 3^3 so I can use  48 36 24 18 12

                  $stopx = $ra_index - $bars;

               } else {

                  $stopx = 0;

               }
                        
               $s111 = $ra_index - 1;
               $f222 = $stopx;
               fwrite($flog,"<br>look at data for index = $s111 decreasing down to $stopx<br> ");
               
               $yy = count($year_ra);
               $dd = count ($day_ra);
               $gg = count($gallons_ra);

               fwrite($flog,"array sizes: year_ra $yy   day_ra $dd   gallons_ra $gg<br> ");


               for ($i=$ra_index - 1; $i >=$stopx; $i--){  // undo the odd ++ up there.

                  $year = $year_ra[$i]; 

                  $day = $day_ra[$i]; 

                  $gallons = $gallons_ra[$i];

                  fwrite($flog,"<br>massaged data ready to plot $year   $day   $gallons <br> ");

                  if ($gallons > 1){

                     $y = $tank_y8 - $gallons; // gallons is x 100 so the range is already 0..200
                 
                     imagefilledrectangle ( $my_img->img , (int)$x-20 , (int) $y ,  (int)$x-2 , $tank_y8-2 ,$colors[$day2]);
                  }
               
                  $x -= $dx;

                  $dumrod = $day-1;    // this will print 0 for the day when the next section will print 01 for dumrod format 'd'
                  fwrite($flog,"create using day $dumrod  year $year ");      // this will print day 364 for dec 31 non-leap year

                  $date33 = DateTime::createFromFormat('Y z',$year . ' ' . ($day - 1));  // fixed for php 8.0.10 8-27-2021    java day 1..366  php day 0..365 so subtract 1. 366 and 365 for leap year. 365 and 364 non-leap year
                     
                  $dumrod = $date33->format('d');     // d day of month 01..31
                  $dumrodyy = $date33->format('Y');   // Y full year 2023
                  $dumrodmm = $date33->format('F');   // F = month full name January
//                  $dumrodm2 = $date33->format('m'); // m = month number. jan = 01
//                  fwrite($flog,"initial date33  $dumrod $dumrodyy month $dumrodmm  $dumrodm2  <br>");  // no need to print month name and month number. 
                  fwrite($flog,"initial date33  $dumrod $dumrodyy month $dumrodmm<br>");

                  if (($day >= 0) && ($day < 367)){
                     $m87 = $date33->format('d');  // 'd' 01..31   'j' 1..31  d and j both start with first day of month = 1
                  } else {
                     $m87 = -1;
                  }

                  $date99 = intval($m87);

                  imagettftext($my_img->img, 12, 0, (int) $x + 7, $tank_y8 + 16 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text

                  if (($date99 == 1) || ($i == $stopx)){

                     $m87 = $date33->format('M');  // 'M' Jan   'F' January

                     imagettftext($my_img->img, 12, 0, (int) $x + 7, $tank_y8 + 32 ,  $text_colour, $lib_font, $m87);   // img, size, angle, x, y, color, font, text
                  }

//                  if ($debug < 6) {

  //                   $debug++;
    //              }

                  $day2--;

                  if ($day2 < 0){

                     $day2 = 6;

                  }

               }  // for ra_index -1 down to stopx

//               $debug++;

            }  // if ra_index > 0. I guess this handles empty files?


//            $debug++;

  //          $debug++;

         // watermark
            
         if ($need_to_fix_server_day == 1){ // for a few hours each night, UTC has already gone to the next day, so this corrects the bar colors during those hours

            imagettftext($my_img->img, 6, 0,  $left_pad * 1/4,  $tank_y2 + 10,  $text_colour, $lib_font, "fix");   // img, size, angle, x, y, color, font, text

         } else {
            
            imagettftext($my_img->img, 6, 0, (int)( $left_pad * 1/4),  $tank_y2 + 10 ,  $text_colour, $lib_font, "no_fix");   // img, size, angle, x, y, color, font, text
         }

      } else {
         
         imagettftext($my_img->img, 12, 0, (int)( $left_pad / 2), $new_y ,  $text_colour, $lib_font, "can't open gallons_new");   // img, size, angle, x, y, color, font, text
      }

   fclose($flog);

?>
