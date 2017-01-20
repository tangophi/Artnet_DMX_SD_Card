/*
Same as ArtnetNeoPixel.ino but with controls to record and playback sequences from an SD card. 
To record, send 255 to the first channel of universe 14. To stop, send 0 and to playback send 127.  
The limit of leds seems to be around 450 to get 44 fps. The playback routine is not optimzed yet.
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <Adafruit_NeoPixel.h>

#define PIN_START_BUTTON   4
#define PIN_STOP_BUTTON    5
#define PIN_SD_CS          15
#define PIN_LED            2

//Wifi settings
const char* ssid = "NiceMoose";
const char* password = "NetgearW";

// Neopixel settings
const int numLeds = 39; // change for your setup

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, PIN_LED, NEO_GRB + NEO_KHZ800);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels]; // Combined universes into a single array

// SD card
File datafile;

char fileNameFull[10] = "";
int  fileNameSuffix   = 0;

volatile bool record   = false;
volatile bool nextFile = false;


// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool storeFrame = 1;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

// When the start button is pressed, set 'record' to true.  Since the file is already opened
// for write, on receipt of full DMX frames, it will be written to the file.
void buttonHandlerStart()
{
  if (!record)
  {
    record = true;
    Serial.println("Start button pressed.");
  }
}

// When the stop button is pressed, set 'record' to false.  This will stop DMX frames from being
// written to the exisiting file in onDmxFrame() function.  Also as nextFile is set to true, in
// the loop() function, the existing file will be closed and the next file is opened for writing.
void buttonHandlerStop()
{
  if (record)
  {
    record = false;
    nextFile = true;
    Serial.println("Stop button pressed.");
  }
}

void setup()
{
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("initialization failed!");
  }
  else
    Serial.println("initialization done.");

  ConnectWifi();
  artnet.begin();
  leds.begin();
  initTest();
  
  attachInterrupt (PIN_START_BUTTON,   buttonHandlerStart,   RISING);
  attachInterrupt (PIN_STOP_BUTTON,    buttonHandlerStop,    RISING);

  sprintf(fileNameFull, "data%d", fileNameSuffix);
  datafile = SD.open(fileNameFull, FILE_WRITE);
  
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();

  // When the stop button is pressed, close the previous file and open the next file
  // to record.
  if (!record && nextFile)
  {
    nextFile = false;

    Serial.print("Closing ");
    Serial.print(fileNameFull);

    datafile.close();
    memset(fileNameFull, 0, 10);
    sprintf(fileNameFull, "data%d", ++fileNameSuffix);  // fileNameSuffix is incremened for the next filename.
    datafile = SD.open(fileNameFull, FILE_WRITE);
    
    Serial.print(" and opening ");
    Serial.print(fileNameFull);
    Serial.println(" for writing.");
  }
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  storeFrame = 1;

  // Store which universe has got in
  if (universe < maxUniverses)
    universesReceived[universe] = 1;

  // See if data for all universes is received.  If it is, then storeFrame will still be 1 and in the next
  // code block, the full DMX frame (containing data for all the universes) will be written to the file.
  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      storeFrame = 0;
      break;
    }
  }
  
  // read universe and put into the right part of the display buffer
  for (int i = 0 ; i < length ; i++)
  {
    int bufferIndex = i + ((universe - startUniverse) * length);
    if (bufferIndex < numberOfChannels) // to verify
      channelBuffer[bufferIndex] = byte(data[i]);
  }
  
  // Write data to the file if a full DMX frame containing data for all the universes is received and if we 
  // are still recording
  if (record && storeFrame)
  {    
    datafile.write(channelBuffer, numberOfChannels);
    memset(universesReceived, 0, maxUniverses);
  } 
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 127, 0, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 127, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 0, 127);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 0, 0);
  leds.show();
}
