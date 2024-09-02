<?php          

/*
   $cutoff=$_GET["cutoff"];
   
   $handle= fopen("flow_cutoff.txt", "w");  // @fopen as seen in some examples - the @ supresses error messages
   
   fwrite($handle,$cutoff);

   fclose($handle);


   $dra33 = getdate();
   
   $current_millenium = $dra33[0];

   $fp = fopen('ping.txt', 'w');    // 'a' for append

   fwrite($fp, $dra33[0]);
   fwrite($fp, " ");
   
   fclose($fp);


          random_int ( int $min , int $max ) : int

         Generates cryptographic random integers that are suitable for use where unbiased results are critical, such as when shuffling a deck of cards for a poker game.

         The sources of randomness used for this function are as follows:

             On Windows, » CryptGenRandom() will always be used. As of PHP 7.2.0, the » CNG-API will always be used instead.
             On Linux, the » getrandom(2) syscall will be used if available.
             On other platforms, /dev/urandom will be used.
             If none of the aforementioned sources are available, then an Exception will be thrown.

             Note: Although this function was added to PHP in PHP 7.0, a » userland implementation is available for PHP 5.2 to 5.6, inclusive. 

         Parameters ¶

         min

             The lowest value to be returned, which must be PHP_INT_MIN or higher.
         max

             The highest value to be returned, which must be less than or equal to PHP_INT_MAX.

*/


   // read lock_file to see if it's zero



   // if lock_file is zero then try to lock it

   $dra33 = getdate();

   $lock_val = $dra33[0];


   $fp = fopen('lock_file.txt', 'w');    // 'a' for append

   fwrite($fp, $lock_val);
   fwrite($fp, " ");  // why do this. 
   
   fclose($fp);





   
?> 
