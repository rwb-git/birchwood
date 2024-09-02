The website code is messy due to the way it evolved over the years. Put all the top_level_directory code in your home directory, then make a directory in there named x10_test_esp8266, 
and put the files from x10_test_directory in there. You will not have a directory named "top_level_directory". If I had understood github properly I would have nested
the folders like that, but it's not that hard to sort it out on your website.

the box in the shed has an avr mega8535 which has the code from that directory. that box sends data to an esp8266 which sends it to your web site. you have to
put your wifi router SSID and password in that code, along with your website path in the variable dream_host. All the references to "dream" are because there used to
be a backup site on another url. All you have to change is that one variable, dream_host.

