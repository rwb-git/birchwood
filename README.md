The website code is messy due to the way it evolved over the years. Put all the top_level_directory code in your home directory, then make a directory in there named x10_test_esp8266, 
and put the files from x10_test_directory in there. You will not have a directory named "top_level_directory". If I had understood github properly I would have nested
the folders like that, but it's not that hard to sort it out on your website. (Actually, if you are ambitious you can get rid of that sub-directory along with all the
switch crap, but I'd avoid that. The switching was back when an android tablet handled the web site while I was developing the esp8266 code, so I had parallel code for it
in the sub directory. Once I switched the old system off there was no need to clean the mess up other than deleting most of the files in the top level directory that 
the old system used.)

the box in the shed has an avr mega8535 which has the code in directory mega8535_code. that box sends data to an esp8266 which sends it to your web site. you have to
put your wifi router SSID and password in that device using code in directory esp8266_code, along with your website path in the variable dream_host. All the references to "dream" are because there used to
be a backup site on another url. All you have to change is that one variable, dream_host.

I used Arduino to program the esp8266 and avrdude to program the mega8535. 

