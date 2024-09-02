

<?php          


//-------------------------------- main body -------------------------------------------------------------------------------------------------------------





$inp = file_get_contents('php://input');

// delete cuz error $size = count($inp);

//echo "count $size...";

file_put_contents('post_288.txt',$inp);


// 11-11-2017 add stuff from get_no_test.php so I can delete that url


$tank=77; //$_GET["tank"];
$status=1; // $_GET["status"];

$minutes=$_GET["minutes"];
$new_date=$_GET["date"];       
$new_yesterday_code=$_GET["yes"];       


   $dra = getdate();

   $fp = fopen('small_test.txt', 'w');    // 'a' for append

   //$fp = fopen('small_test_new33.txt', 'w');    // 'a' for append

   fwrite($fp, $tank);
   fwrite($fp, " ");

   fwrite($fp, $status);
   fwrite($fp, " ");

   fwrite($fp, $minutes);
   fwrite($fp, " ");

   fwrite($fp, $new_date);        
   fwrite($fp, " ");

   fwrite($fp, $dra[0]);
   fwrite($fp, " ");
   
   fwrite($fp, $new_yesterday_code);


   fclose($fp);



/*

      3-18-2019 delete all this shit because collisions if pulses at same time. different failures on fork and biz, so fuck it and fix the other files...

         // 11-12-2017 add code to create new pulse and float files shifted if midnight has occurred, otherwise leave them alone. also make backups of the original files so I can compare to the shifted ones.
         //
         // get the shift code from pulse_new and float_new
         //
         // add code to handle zero elements in both yesterday and today since it malfunctions if no data is there. see pulse_stuff_33_fake.php for this code



         // from pulse_new, read the existing file
         //
         // ------------ read the old data ------------------

            $final_old_yesterday = 0; // newest data already in php list

            $final_old_today = 0;
            
            $f99gname = "erase_new_fake.txt";

            $_fp = fopen($f99gname, "r");

            $data = explode(",", trim(fgets($_fp)));   // yesterday string , today string , yesterdays_code = 2017233

            fclose($_fp);



         // test for midnight in pulse file


            //if (22 != $new_yesterday_code){
            if ($data[2] != $new_yesterday_code){


               // write a backup file

               // now write to file: old today , new today, code

               $fgname = "erase_new_fake_bak.txt";
               
               $fpm = fopen($fgname, 'w');    // 'a' for append

               fwrite($fpm,$data[0] . "," . $data[1] . "," . $data[2]);
            
               fclose($fpm);



               // write new shifted fake file = leave old style file alone

               $fgname = "erase_new_fake.txt";
               
               $fpm = fopen($fgname, 'w');    // 'a' for append

               fwrite($fpm,$data[1] . ",," . $new_yesterday_code);

               fclose($fpm);

            }




         // now handle float file


            $f77gname = "float_new_fake.txt";

            $_fp = fopen($f77gname, "r");

            $data = explode(",", trim(fgets($_fp)));   // yesterday string , today string , yesterdays date

            fclose($_fp);


            //if (22 != $data[2]){
            if ($new_date != $data[2]){

               // write a backup file

               $fgname = "float_new_fake_bak.txt";
               
               $fpm = fopen($fgname, 'w');    // 'a' for append

               fwrite($fpm,$data[0] . "," . $data[1] . "," . $data[2]);
            
               fclose($fpm);



               // write new shifted fake file = leave old style file alone

               $fgname = "float_new_fake.txt";
               
               $fpm = fopen($fgname, 'w');    // 'a' for append

               fwrite($fpm,$data[1] . ",," . $new_date);

               fclose($fpm);

            }


*/



?>



