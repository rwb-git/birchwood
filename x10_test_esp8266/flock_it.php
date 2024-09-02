<?php    

   
/* 
   $dra33 = getdate();

   $lock_val = $dra33[0];


   $fp = fopen('lock_file.txt', 'w');    // 'a' for append

   fwrite($fp, $lock_val);
   fwrite($fp, " ");  // why do this. 
   
   fclose($fp);
*/
   // wait a bit. the issue here is another url trying to lock the server has also just read the zero value. so, how long does it take for this script to read zero and write a value? certainly not on the order of seconds, worst case?

   // read the file to see if it contains our lock value. if so, assume we have locked the server. if it contains another non-zero value then it appears that another url just locked it, so abort

/*

         $fp = fopen("/tmp/lock.txt", "r+");

         if (flock($fp, LOCK_EX)) {  // acquire an exclusive lock
             ftruncate($fp, 0);      // truncate file
             fwrite($fp, "Write something here\n");
             fflush($fp);            // flush output before releasing the lock
             flock($fp, LOCK_UN);    // release the lock
         } else {
             echo "Couldn't get the lock!";
         }

         fclose($fp);

*/


         $fp = fopen("lock_file.txt", "r+");

         if (flock($fp, LOCK_EX)) {  // acquire an exclusive lock
//             ftruncate($fp, 0);      // truncate file
  //           fwrite($fp, "Write something here\n");
    //         fflush($fp);            // flush output before releasing the lock


               http_response_code(400);

               sleep(4);      // seconds. when i sleep 10 seconds it takes about 5 seconds and the response code is -11
                              // 1 second works ok, as does 2


               flock($fp, LOCK_UN);    // release the lock


         } else {
      //       echo "Couldn't get the lock!";
               http_response_code(403);
         }

         fclose($fp);



?>
