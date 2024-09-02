The website structure is your.site/birchwood/. This is where you put everything that is in the top_level_directory here. Nothing in your website is actually
named "top level directory"; I should have called that "birchwood" here. Inside your.site/birchwood/ is a directory named x10_test_esp8266; everything in the 
x10_test_directory here goes into that directory.

The grey box in the shed has an avr mega8535 which has the code in directory mega8535_code. That box sends data to an esp8266 in someone's trailer which uses wifi to
send it to your web site. You have to put your wifi router SSID and password in that device using code in directory esp8266_code, along with your website path in the 
variable dream_host. All the references to "dream" are because there used to be a backup site on another url. All you have to change is that one variable, dream_host.

I used Arduino to program the esp8266 and avrdude to program the mega8535. 

