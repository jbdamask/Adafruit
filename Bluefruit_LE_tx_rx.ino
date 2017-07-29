/*********************************************************************
/*********************************************************
Author: John B Damask & Adafruit folks (via their demos)
Created: July 27, 2017
Purpose: Using Adafruit's capacitive touch board or Bluefruit to control NeoPixels. 
Note: Using RGB neopixels, and MPR121 capacitive touch breakout, Feather Bluefruit 32u4.
      The Feather can both read and write from BLE. The idea is that colors can come in from
      a central device (e.g. a Pi) or you can touch a pad connected to a sensor and send that color
      to the device. Note that I've adopted the payload format from Adafruit for sending Colors 
      (as reconstructed from packetParser.cpp). Basically it's !C<red><green><blue><checksum>
Todo: Create classes for ble, touch and pixel functions.

*********************************************************************/
#include <Wire.h>
#include "Adafruit_MPR121.h"

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include <Adafruit_NeoPixel.h>

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    PIN                       Which pin on the Arduino is connected to the NeoPixels?
    NUMPIXELS                 How many NeoPixels are attached to the Arduino?
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE     1
    #define PIN                     9
    #define NUMPIXELS               24
    #define BRIGHTNESS              30
    #define MIN                     1
    #define MAX                     255
    #define NUMTOUCH                12
/*=========================================================================*/

uint8_t output = 0;
uint8_t len = 0;
//Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
uint8_t state = 0;
bool printOnceBle = false;

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];
uint8_t PAYLOAD_START = "!";
uint8_t COLOR_CODE = "C";
uint8_t BUTTON_CODE = "B";
uint8_t red;
uint8_t green;
uint8_t blue;

// the ble payload, set to max buffer size
uint8_t payload[21];

uint8_t crc(uint8_t input[]) {
  char checksum = 0; // could be an int if preferred
  int sizeOfArray = sizeof(input)/sizeof(uint8_t);
  //char buf[sizeOfArray];
  //strcpy(buf, inputStr);
  for(int x = 0; x < sizeOfArray; x++)
  {
      checksum += input[x];
  }
  return checksum;
}

void setColors(uint8_t r, uint8_t g, uint8_t b){
  red = r; green = g; blue = b;
}

void wipe(){
  colorWipe(pixel.Color(red, green, blue),10);
}


/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup()
{
  delay(1000);
  Serial.println("Setting up");
//  while (!Serial);  // required for Flora & Micro
  

  // turn off neopixel
  pixel.begin(); // This initializes the NeoPixel library.
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
  }
  pixel.show();

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Neopixel Color Picker Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  // We don't want to wait for the ble connection because we want touch to work regardless
  /*while (! ble.isConnected()) {
      Serial.println("Waiting for bluetooth connection");
      delay(500);
  }*/

  // MPR121 board
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  Serial.println("before cap");
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    //while (1);
  }else {
    Serial.println("MPR121 found!");    
  }
  Serial.println("After cap");



}

void bleSetupOnConnect(){
  if(ble.getMode() != 0) {
    Serial.println(F("***********************"));
    // Set Bluefruit to DATA mode
     Serial.println( F("Switching to DATA mode!") );
    ble.setMode(BLUEFRUIT_MODE_DATA);
    Serial.println(F("***********************"));
  }
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
 // Serial.println("In main loop");
  if(ble.isConnected()){
    if(!printOnceBle) {
      Serial.println("ble connected!");
      printOnceBle = true;
    }
    bleSetupOnConnect();
  // Check bluetooth input
  len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  }

  // Check touch pads input
  currtouched = cap.touched();
 
 // if (len == 0) return;
 // Check for bluetooth input
 if(len != 0) {
  Serial.println("Bluetooth event detected!");
  delay(2000);
  bl();
 } else if(currtouched != 0 && currtouched != lasttouched) {
  Serial.println("Touch event detected!");
  Serial.println(currtouched);
  touch();
 } else {
 // Serial.println("Nothing detected. :-(");
  return;
 }

  delay(100);
}

// Lights triggered by bluetooth
void bl(){
  
  /* Got a packet! */
   printHex(packetbuffer, len);

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      switch(buttnum){
        case 1:
          rainbow(10);
          break;
        case 2:
          rainbowCycle(10);
          break;
        case 3:
          output = MIN + (rand() % (int)(MAX - MIN + 1));
          Serial.print("Random color value is "); Serial.print(output); Serial.println("");
          theaterChase(output, 10);
          break;
        case 4:
          theaterChaseRainbow(10);
          break;
        default:
          colorWipe(pixel.Color(0,0,0), 10);          
          break;
      }
      Serial.println(" pressed");
    } else {
      Serial.println(" released");
    }
  }

  // Color
  if (packetbuffer[1] == 'C') {
    red = packetbuffer[2];
    green = packetbuffer[3];
    blue = packetbuffer[4];
    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue, HEX);

    colorWipe(pixel.Color(red,green,blue), 10);  

/*    for(uint8_t i=0; i<NUMPIXELS; i++) {
       Serial.print("Setting color for pixel: ");
       Serial.print(i);
       Serial.println("");
      pixel.setPixelColor(i, pixel.Color(red,green,blue));
       pixel.show(); // This sends the updated pixel color to the hardware.
    }*/
   
  }
}

// Lights triggered by touch pads
void touch(){
   for (uint8_t i=0; i<NUMTOUCH; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
      state = i;
    }
        // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  //  pixel.show();
  }
  Serial.print("Button selected: ");
  Serial.println(state);
    // reset our state
  lasttouched = currtouched;
  //if(animation==true) {   
    switch (state){
      case 0:
        setColors(255, 49, 236);
        wipe();
        break;        
      case 1:
        setColors(113, 40, 180);
        wipe();
        break;      
      case 2:
        setColors(255, 0, 0);
        wipe();
        break;
      case 3:
        setColors(0, 255, 0);
        wipe();
//        rainbow(10);          
        break;
      case 4: //OFF
        setColors(0, 0, 0);
        wipe();
        break;
      case 5: 
        setColors(53, 34, 120);
        wipe();
        break;
      case 6:
        setColors(255, 0, 255);
        wipe();
        break;
      case 7:
        setColors(255, 255, 0);
        wipe();
        break;
      case 8:
        setColors(0, 255, 255);
        wipe();
        break;
      case 9:
        setColors(76, 0, 48);
        wipe();
        break;
      case 10:   
        setColors(50, 50, 50);
        wipe();
//        theaterChaseRainbow(10);
        break;
      case 11:   
        setColors(200, 200, 200);
        wipe();
 //       rainbow(10);
        break;
      default:
       colorWipe(pixel.Color(0,0,0),10);
       break;  
    }

    payload[0] = 0x21;
    Serial.println(payload[0], HEX);
    payload[1] = 0x43;
    Serial.println(payload[1], HEX);
    payload[2] = red;
    Serial.println(payload[2], HEX);
    payload[3] = green;
    Serial.println(payload[3], HEX);
    payload[4] = blue;
    Serial.println(payload[4], HEX);
    payload[5] = crc(payload);
    Serial.println(payload[5], HEX);
    //ble.write(payload, 6);

     Serial.println( F("Switching to COMMAND mode!") );
    ble.write(payload,6);
    /*ble.setMode(BLUEFRUIT_MODE_COMMAND);    
    ble.print("AT+BLEUARTTX=");
    ble.print(1, 1);
    ble.print(",");
    ble.print(2, 1);
    ble.print(",");
    ble.print(3, 1);
    ble.println("|");*/
    ble.readline(200);

    Serial.println( F("Switching to DATA mode!") );
    ble.setMode(BLUEFRUIT_MODE_DATA);    
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixel.numPixels(); i++) { 
    pixel.setPixelColor(i, c);
    pixel.setBrightness(BRIGHTNESS);
    pixel.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixel.numPixels(); i++) {
      pixel.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixel.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixel.numPixels(); i++) {
      pixel.setPixelColor(i, Wheel(((i * 256 / pixel.numPixels()) + j) & 255));
    }
    pixel.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixel.show();

      delay(wait);

      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixel.show();

      delay(wait);

      for (uint16_t i=0; i < pixel.numPixels(); i=i+3) {
        pixel.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
