

<html>
 <head>
  <title>IP log</title>
 </head>
 <body>

<?php       

/*

   

*/


$oldmins = 0;
$oldmil = 0;
$lines = 0;
$oldslot = 0;

$dmilmin = 1111111;
$dmilmax = 0;

$handle = fopen("ip_log.txt", "r");

/*

   file format ip_log.txt

   xx.xx.xx.xx hour minute second date
*/

$contents = array();
$my_ips = 0;
$d_ips = 0;
$j_ips = 0;

$olddate = 0;
$views = 0;
$mine = 0;

if ($handle) {
    while (($line = fgets($handle)) !== false) {

        $contents[] = $line;

         $numbers = explode(" ", trim($line));

         $index=0;

         foreach ($numbers as &$number){

            if ($index == 0){    // IP
               
               if (strcmp($number, "107.15.54.92") == 0) {

                  $my_ips = $my_ips + 1;

  //                $name = "   <--- me ---> ";

                  $mine = 1;
         
               } else {

                  $j_ips = $j_ips + 1;
//                  $name = "J";

                  $mine = 0;

               }

               $ip = $number;          // this is actually a string

            } elseif ($index == 1){

               $hour = (int) $number;

            } elseif ($index == 2){

               $minute = (int) $number;

            } elseif ($index == 3){

               $second = (int) $number;

            } elseif ($index == 4){

               $date = (int) $number;

               if ($date == $olddate){

                  $views = $views + 1;

               } else {

                  if ($views > 0){
                     echo "$views views on this date: $olddate <br> <br>";
                  }

                  $views = 1;
                  $olddate = $date;

               }
            }
       
            $index = $index + 1;
         }
         
         $lines = $lines + 1;

         $local_hour = $hour - 4;

         if ($local_hour < 0){

            $local_hour = $local_hour + 24;

         }

         $total_minutes = $local_hour * 60 + $minute;

         if ($total_minutes > (12 * 60)) {

            $ap = " PM ";

         } else {

            $ap = " AM ";

         }

         if ($local_hour > 12){

            $local_hour = $local_hour - 12;
         }

         if ($mine == 0){

            $s = sprintf("%16s at %02d:%02d:%02d (%02d:%02d:%02d %s) on %2d",$ip,$hour,$minute,$second,$local_hour,$minute,$second,$ap,$date); // you can pad with spaces: % 2d, but html seems to strip extra whitespace.

            echo "$s <br>";

         }

    }


    echo "$views views on this date: $olddate <br> <br>";

    $count = count($contents);

    echo " lines in file = $count <br>";

    fclose($handle);


   echo "<br>server time during DST is my time plus 4 hours; minutes seem to be off by 5 or so, as well <br>";
   echo "<br>my IP showed up $my_ips times. <br>";
   echo "<br>J  IP showed up $j_ips times. <br>";
//   echo "<br>D  IP showed up $d_ips times. <br>";


} else {
    // error opening the file.
}
?>
 </body>
</html>
