

<html>
 <head>
  <title>IP log</title>
 </head>
 <body>

<?php       



$oldmins = 0;
$oldmil = 0;
$lines = 0;
$oldslot = 0;

$dmilmin = 1111111;
$dmilmax = 0;

$handle = fopen("gal_per_day_fake_log.txt", "r");


$contents = array();
$my_ips = 0;
$d_ips = 0;
$j_ips = 0;

$olddate = 0;
$views = 0;
$mine = 0;

if ($handle) {
    while (($line = fgets($handle)) !== false) {

      echo "$line <br>";
    }



} else {
    // error opening the file.
}
?>
 </body>
</html>
