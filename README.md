# ArtNet_DMX_SD_Card

A project used to store DMX frames received through Artnet on a nodemcu to a SD card with one sketch.  With the other sketch, playback the stored effects.  With this, a standalone way to playback LED effects is possible without the need to have a software sending DMX frames or even a WiFi connection.

Using two buttons, we can store several effects in individual files (with incremental names) when the recording sketch is active.   Using the same buttons, we can play the previous or next effect from the various files when the playback sketch is active.


## Hardware

nodemcu  
SD card reader with microSD card  
Two push buttons  
Two 10K resistors  
WS2812B LED strip  
Power supply  


## Connections

![alt tag](https://github.com/tangophi/Artnet_DMX_SD_Card/blob/master/nodemcu_sd_card_ws2812b_push_buttons_bb.png)

### SD Card reader

nodemcu		   SD card reader  
  
   D5             SCK  
   D6             MISO  
   D7             MOSI  
   D8              CS  
  3.3v            3.3v  
   G              GND  

### Push buttons

Connect D2 of nodemcu to one leg of push button.  Also connect that leg to GND through a 10K resistor.  Connect the other leg of the button to 3.3v.
Connect D1 of nodemcu to one leg of another push button.  Also connect that leg to GND through another 10K reistor.  Connect the other leg of the button to 3.3v.


The above connections will remain the same for both recording and playback.   

For recording, push button connected to D2 will be 'Start' button.  Push button connected to D1 will be 'Stop' button.
For playback, push button connected to D2 will be 'Previous' button.  Push button connected to D1 will be 'Next' button.

### LED Strip

The DIN of the WS2812B strip is connected to D4 pin of the nodemcu.


## Uploading the sketch

Arduino IDE 1.8.0 has been used in this project.  Before uploading either of the recording or playback sketches, make sure the following libraries are installed.

SPI  
SD  
ESP8266WiFi  
WiFiUdp  
ArtnetWifi  
Adafruit_NeoPixel  



## For recording effects

1.  Upload Store_DMX_Frames_Received_Through_ArtNet_To_SD_Card.ino sketch to the nodemcu.

2.  Configure Madrix to send DMX frames to the nodemcu.

3.  Play the first effect on the Madrix.  It will send DMX frames to the nodemcu.

4.  Press the first (Start) button.  The nodemcu will start to store the received DMX frames to the SD card.  The first filename will always be 'data0'.

5.  To stop storing the effect, press the second (Stop) button.  This will close the current file and automatically will open the next file, like 'data1' for writing.

7.  Stop the first effect on the Madrix.

8.  Play the next effect on the Madrix. 

Repeat steps 4 to 8 to record the second effect (to file 'data1') and more effects (and more files).


# *** NOTE *** IMPORTANT ***

If you reset the nodemcu or poweroff then power on the nodemcu when the recording sketch is active, then it will start overwriting the data files
from the beginning, that is from data0.



## For replaying effects

1.  Upload Play_Stored_DMX_Frames_From_SD_Card.ino sketch to the nodemcu.

2.  Madrix is not needed for this.  A wifi connection is also not needed.  Wifi wont be started on the nodemcu.

3.  As soon as the nodemcu starts, it will start playing the effects stored in the first data file - data0.   Once it reaches the end of the file, it will start again from the beginning of the same file and the process repeats. 

4.  To go to the next effect (file), press the second (Next) button.  If its playing the last file, then the next file will be the first file.

5.  To go to the previous effect (file), press the first (Previous) button.  If its playing the first file, then the previous file will be the last file.



## Links


### Used in this project

Library to use Madrix with ESP8266 - Example works!!!
https://github.com/rstephan/ArtnetWifi

Original Artnet library - one example is to capture DMX frames, store it in sd card and play them back - Example ArtnetNeoPixelSD.ino works !!!
https://github.com/natcl/Artnet
https://github.com/natcl/Artnet/tree/master/examples/ArtnetNeoPixelSD



### Other Links not used in this project

Create lighting effects using Vixen, export it to a file, copy file to sd card and use Arduino to read the data and send it to LEDs
https://www.dorkbotpdx.org/blog/paul/dmx_lighting_sequence_player

Read DMX data from a SD card and apply to FastLED
http://stackoverflow.com/questions/36896918/reading-bytes-from-arduino-sd-card-to-dmx-lighting
http://pastebin.com/wHAT6dZB

FastLED library - works with WS2812B
http://fastled.io/
