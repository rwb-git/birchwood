<html>
 <head>
  <title>air pressure</title>
 </head>
 <body>

<img src="x10_test_esp8266/air_mk_img7.php" alt="Image created by a PHP script">

<?php 

include 'boilerplate.php';

?>

<a href="menu.html"><center><img src="menu.png"></center></a>

      <font size="50">
<!--

<form action="welcome.php" method="post">
enter new value: <input type="number" name="value"><br>
<input type="image" src="submit.png" alt="Submit" width="228" height="60">
</form>
     --> 

<table style="width:100%">

  <tr>
    
    <td>
      <p style="font-size:110px">

<!--      <img src="x10_test_esp8266/mk_new_expanded_fake.php/?p=1">   if p value > 100 then debug is turned on and p - 100 = plot hours -->
      
      <a href="x10_test_esp8266/calibrate_air.php/?type=0&val=1">raise all</a>
   </td>

  </tr>

  <tr>

   <td>
      <p style="font-size:110px">
    
      <a href="x10_test_esp8266/calibrate_air.php/?type=0&val=0">lower all</a>
   </td>

  </tr>

  <tr>

   <td>
      <p style="font-size:110px">
    
      <a href="x10_test_esp8266/calibrate_air.php/?type=1&val=1">raise upper</a>
   </td>

  </tr>


  <tr>
    
    <td>
      <p style="font-size:110px">
      <a href="x10_test_esp8266/calibrate_air.php/?type=1&val=0">lower upper</a>
   </td>

  </tr>

  <tr>

   <td>
      <p style="font-size:110px">
    
      <a href="x10_test_esp8266/calibrate_air.php/?type=2&val=1">raise lower</a>
   </td>

  </tr>

  <tr>

   <td>
      <p style="font-size:110px">
      <a href="x10_test_esp8266/calibrate_air.php/?type=2&val=0">lower lower</a>
    
   </td>

  </tr>

</table>




 </body>
</html>
