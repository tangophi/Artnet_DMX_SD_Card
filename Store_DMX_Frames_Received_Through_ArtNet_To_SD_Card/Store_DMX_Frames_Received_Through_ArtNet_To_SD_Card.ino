/*
 *  This sketch is used to store DMX frames received through ArtNet protocol to a SD card on a nodemcu
 *  or a ESP8266 module.  'Start' button is to start recording frames while 'Stop' button is to stop 
 *  recording.  Multiple effects can be stored in individual files with incremental names.
 *  
 *  LED strip need not be connected for this sketch to work.  However the strip need to be defined
 *  in the sketch so that the sketch can calculate the number of pixels, channels and universes.
 *  
 *  Change numLeds and startUniverse according to your setup.
 *  
 *  In Madrix, in Preferences-> Device Manager-> DMX Devices, make sure "Send full frames" box is not checked.
 *  
 * https://github.com/tangophi/Artnet_DMX_SD_Card 
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
const int numLeds = 39; // CHANGE FOR YOUR SETUP

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, PIN_LED, NEO_GRB + NEO_KHZ800);

// Artnet settings
ArtnetWifi artnet;
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels];     // Combined universes into a single array

// SD card
File datafile;

char fileNameFull[10] = "";
int  fileNameSuffix   = 0;

volatile bool startRecord = false;
volatile bool stopRecord  = false;
volatile bool recording   = false;

int bufferIndex = 0;

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


void buttonHandlerStart()
{
  if (!recording && !startRecord)
  {
    startRecord = true;
    Serial.println("Start button pressed.");
  }
}

void buttonHandlerStop()
{
  if (recording && !stopRecord)
  {
    stopRecord = true;
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
    Serial.println("Initialization failed!");
  }
  else
    Serial.println("Initialization done.");

  ConnectWifi();
  artnet.begin();
  leds.begin();
//  initTest();
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 0, 0);
  leds.show();
  
  attachInterrupt (PIN_START_BUTTON,   buttonHandlerStart,   RISING);
  attachInterrupt (PIN_STOP_BUTTON,    buttonHandlerStop,    RISING);

  sprintf(fileNameFull, "data%d", fileNameSuffix);
  
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();

  // Open a file for writing when the start button is pressed
  // and also set recording to true so that incoming DMX frames
  // are written to the file.
  if (startRecord && !recording)
  {
    datafile = SD.open(fileNameFull, FILE_WRITE);
    
    Serial.print("Opening ");
    Serial.println(fileNameFull);

    startRecord = false;
    recording = true;
  }

  // Stop the recording when the stop button is pressed and close 
  // the current file which was earlier opened for writing.  Also 
  // increment the fileNameFull variable.
  if (recording && stopRecord)
  {
    recording = false;
    delay(30);
    Serial.print("Closing ");
    Serial.println(fileNameFull);

    datafile.close();
    sprintf(fileNameFull, "data%d", ++fileNameSuffix);  // fileNameSuffix is incremened for the next filename.
    stopRecord = false;
  }
}

// This function is called for every packet received.  It will contain data for only
// one universe.  Hence, wait till data for all universes are received before 
// writing a full frame to the file.
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
  
  // Read universe data and put into the right part of the display buffer
  for (int i = 0 ; i < length ; i++)
  {
    if (bufferIndex < numberOfChannels) // to verify
      channelBuffer[bufferIndex++] = byte(data[i]);
  }
  
  // Write data to the file after DMX frames containing data for all the universes 
  // is received and if we are still recording
  if (recording && storeFrame)
  {    
    datafile.write(channelBuffer, numberOfChannels);
    memset(universesReceived, 0, maxUniverses);
    bufferIndex = 0;
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
