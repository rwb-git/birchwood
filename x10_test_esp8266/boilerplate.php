<?php 

// >>>>>>>>>>> REMEMBER THAT everything below accomplishes one thing: the text warning at the bottom of the page. everything here is local and is not shared with the php script above for img src which has to duplicate much of this work
// 
// and this block of code from here to the last line </html> will be the same in every page to be displayed by the MENU calls, I think.

include 'read_time_file.php';    // read ping.txt to get $millenium based on server time for most recent ping or update packet. 
                                 // calc $minutes2 = minutes past midnite based on durham time. 
                                 // get $current_millenium based on server time.
                                 // use all this to calculate $minutes_since_last_packet and $data_is_old.
                                 // if $data_is_old fix $minutes2 so the plots show a gap on the right end. also calculate $pivot_index for 288 style arrays, which might be obsolete with esp style


include 'main_file_functions.php';  // handle_time_and_IP_log() = calls show_elapsed(server millenium - $millenium from read_time_file). log IP of clicks. 
                                    // prune_log_file() = prune that file when it is large. 
                                    // show_elapsed() = use $minutes_since_last_packet from read_time_file.php to calculate seconds since last ping or packet and show warning if data is not current
handle_time_and_IP_log();

?>


