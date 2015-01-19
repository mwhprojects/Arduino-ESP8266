# About Arduino-ESP8266
These are experiments in getting an Arduino to send data to the internet using the ESP8266 wifi module. These experiments use the data collected from a DHT22 temperature and humidity sensor and a photo resistor.

# Files
### ThingSpeak_noLCD_public
Initial code to get the ESP8266 online, sending data to Thing Speak.

### ThingSpeak_LCD_public
Adds LCD to base code.

### PHPpage_LCD_public
Changes ThingSpeak_LCD_public to send data to a webhost where data is inserted into a table in a MySQL database.

### PHPpage_ThingSpeak_LCD_public
Combines ThingSpeak_LCD_public and PHPpage_LCD_public so that data is sent to both the MySQL database and Thing Speak.

# Guides
I used these guides to get started:<br/>
http://www.instructables.com/id/ESP8266-Wifi-Temperature-Logger/?ALLSTEPS<br/>
http://randomnerdtutorials.com/getting-started-with-esp8266-wifi-transceiver-review/<br/>
http://mcuoneclipse.com/2014/12/14/tutorial-iot-datalogger-with-esp8266-wifi-module-and-frdm-kl25z/<br/>

# My Links
### PHP Page Demo
http://mwhprojects.com/esp8266

### Thing Speak Channel
http://thingspeak.com/channels/22770

### Blog
I cover the hardware aspect of this project on my blog:
https://mwhprojects.wordpress.com/category/projects-2/arduino/esp8266-module/