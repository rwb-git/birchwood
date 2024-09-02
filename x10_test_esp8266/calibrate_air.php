<html>
<body>
     


      <font size="50">
      
      <?php
      
      
      $type =  $_GET["type"]; 
      
      $val =  $_GET["val"]; 

   include 'read_offsets.php';
  


   if ($type == 0){


      if ($val == 0){

        $offset = $offset - 1;

      } else {

         $offset = $offset + 1;

      }
   }


   if ($type == 1){

      if ($val == 0){

        $offset_H = $offset_H - 1;

      } else {

         $offset_H = $offset_H + 1;

      }
   }


   if ($type == 2){

      if ($val == 0){

        $offset_L = $offset_L - 1;

      } else {

         $offset_L = $offset_L + 1;

      }
   }
//   $offset = 0;
//   $offset_H = 0;
//   $offset_L = 0;


   $fp = fopen($filename, "w");



      fwrite($fp, $offset);
      fwrite($fp, " ");

      fwrite($fp, $offset_H);
      fwrite($fp, " ");

      fwrite($fp, $offset_L);
      fwrite($fp, " ");


   fclose($fp);

   echo " $offset  $offset_H  $offset_L  <br> <br>";

      ?>
      
 
      

      <br>
      <br>
      <br>

      </font>

<!--  My page got send multiple times because of ELEMENTS IN THE HTML CODE THAT COULD NOT BE FOUND. << this message on web saved my ass after hours of
      bizarre behavior because this was running twice each time with 0 0 the second time. I don' know why I had to put the full urls here; maybe because of the
      url parameters that require the trailing / in the url that calls this script? I know I had confusion before with the urls when some of them 
      began with / and some didn't, and php was doing different directory stuff-->


<!-- this fails      <a href="fork20.xyz/birchwood/x10_test_esp8266/air.php"><img src="fork20.xyz/birchwood/x10_test_esp8266/return.png"></a>  -->

<!--      <a href="http://fork20.xyz/birchwood/x10_test_esp8266/air.php"><img src="http://fork20.xyz/birchwood/x10_test_esp8266/return.png"></a>  
          this sucks because it says fork20 -->
      
      
   <a href="../../air2.php"><img src="../../return.png"></a> 

<!--      <a href="air.php"><img src="return.png"></a>  I can't get this to work. I think I might need switcher scripts to send the two arguments down, so html never leaves the top dir, or
i need to have separate files for birch and biz. fuck that. make the switcher-->

</body>
</html>


