/*********************************************************************
  Author: Adafruit folks, John B Damask
  Created: May 2016
  Purpose: Prototyping
  Function: This code configures various tech from Adafruit including, Flora
            Bluefruit LE, NeoPixel and the capacitive sensor library to do the following:
            1 - Conductive fabric touched, onboard NeoPixel turns GREEN
            2 - Pick whatever color using Adafruit Bluefruit LE iPhone app, NeoPixel turns whatever
 
  Notes: This is basically a mash up of various Adafruit tutorials:
  Reference: https://learn.adafruit.com/capacitive-touch-with-conductive-fabric-and-flora/overview
  Reference: https://learn.adafruit.com/adafruit-flora-bluefruit-le
  Reference: https://learn.adafruit.com/flora-rgb-smart-pixels/run-pixel-test-code
 
 *********************************************************************
 *Adafruit text
 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <CapacitiveSensor.h>
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
//    #define PIN                     12
    // On board NeoPixel
    #define PIN                     12
    #define NUMPIXELS               60
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"    
/*=========================================================================*/


int fadeControl = 255;//will hold the current brightness level
int maxBright = 30; // Maximum brightness of pixels
int fadeDirection = -1;//change sigen to fade up or down
int fadeStep = 10;//delay between updates

// #################
//    States
// #################
int _OFF = 1;
int _CALLING = 2;
int _IS_CALLED = 3;
int _CONNECTED = 4;
int _CONNECTED_LOW_ENERGY = 5;
int currentState = 1;
int previousState = 1;

// #################
//    Timer
// #################
long start = 0;
long maxTime = 5000;

// #################
//    Touch
// #################
int touchTrigger = 1000;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
CapacitiveSensor   cs_9_10 = CapacitiveSensor(9,10);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired

// Create the bluefruit object, either software serial...uncomment these lines
 Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

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


/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
 // while (!Serial);  // required for Flora & Micro
  delay(500);
  cs_9_10.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  strip.begin();
  strip.setPixelColor(0, strip.Color(0,0,0)); //off
  strip.show();

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
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));

}

/**************************************************************************/
/*!
    @brief  This is the meat. Constantly poll for new command from either
    the iPhone app controller or the fabric controller.
*/
/**************************************************************************/
void loop(void)
{
  long touchValue =  cs_9_10.capacitiveSensor(30);
  /* Wait for new data to arrive */
//  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  ble.readline();
  Serial.println(currentState);
  Serial.println(ble.buffer);
  uint8_t len = strcmp(ble.buffer, "");
  Serial.println(len);
  /* State control */
  switch (currentState) {
    case 1 :
      if (touchValue > touchTrigger) {
        start = millis();
        currentState = 2;
      } else if (len > 0) {
        start = millis();
        currentState = 3; 
      } else {
        break;
      }

    case 2 :
      if (millis() - start > maxTime) {
        previousState = currentState;
        currentState = 1;
        colorWipe(strip.Color(0,0,0), 10);
        start = millis();  // probably don't need this
      } else if (len > 0) {
        previousState = currentState;
        currentState = 4;
        start = millis();  // probably don't need this
      } else {
        breathe();
      }
      break;
    case 3 :
      if (millis() - start > maxTime) {
        previousState = currentState;
        currentState = 1;
        colorWipe(strip.Color(0,0,0), 10);
        start = millis();  // probably don't need this
      } else if (touchValue > touchTrigger) {
        previousState = currentState;
        currentState = 4;
        start = millis();  // probably don't need this
      } else {
        theaterChase(strip.Color(0, 0, 127), 50); // Blue
      }      
      break;
    case 4 :
      if (millis() - start > maxTime) {
        previousState = currentState;
        currentState = 5;
        start = millis();
      } else if (touchValue < touchTrigger || len < 1) {
       currentState = previousState;
       start = millis();
      } else {
        colorWipe(strip.Color(0, 255, 0), 50); // Green
      }
      break;
    case 5 :
      if (touchValue < touchTrigger || len < 1) {
       currentState = previousState;
       start = millis();
      } else {
        colorWipe(strip.Color(100, 100, 100), 50); // 
      }
      break;
    default :
      // Should never happen
      currentState = 1;
  }
}


void breathe(){
  for(uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 255, 24, 0);//set the pixel colour
    strip.setBrightness(fadeControl);//set the pixel brightness
    strip.show();
   // Serial.println(fadeControl);
    fadeControl = fadeControl + fadeDirection;//increment the brightness value
    if (fadeControl <-255 || fadeControl > 255)
    //If the brightness value has gone past its limits...
    {
      fadeDirection = fadeDirection * -1;//change the direction...
      fadeControl = fadeControl + fadeDirection;//...and start back.
    }
      delay(10);
  }

  //delay(fadeStep);//wait a bit before doing it again.
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.setBrightness(30);
    strip.show();
    //delay(wait);
  }
}


//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
