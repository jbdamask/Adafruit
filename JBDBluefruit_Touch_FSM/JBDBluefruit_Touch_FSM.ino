/*********************************************************************
  Author: John B Damask, Adafruit folks
  Created: May 2016
  Purpose: Prototyping
  Function: This code configures various tech from Adafruit including, Flora
            Bluefruit LE, NeoPixel and the capacitive sensor library to allow
            two devices (made of these tech) to communicate over the web.
            A central Bluetooth device is required, such as Adafruit's Bluefruit LE 
            smart phone app (in fact this code waits until our BLE peripheral 
            connects). 
            To make two devices communicate you'll need a broker, such as adafruit.io.
            The smart phone app lets you pub/sub to feeds.
 
  Notes: This liberally uses code from various Adafruit tutorials:
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
//#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
#include <Adafruit_NeoPixel.h>
#include "JBDBluefruit_Touch_FSM.h"


// ###################################################
//    Initialize variables for breathe() function
// ###################################################

int fadeControl = 0;//will hold the current brightness level
int maxBright = 30; // Maximum brightness of pixels
int fadeDirection = -1;//change sigen to fade up or down
int fadeStep = 10;//delay between updates

// ###################################################
//                States
// ###################################################
int _OFF = 1;
int _CALLING = 2;
int _IS_CALLED = 3;
int _CONNECTED = 4;
int _CONNECTED_LOW_ENERGY = 5;
int currentState = 1;
int previousState = 1;

// ###################################################
//                Timer
// ###################################################
long start = 0;
long maxTime = 10000;

// ###################################################
//                Touch Sensor
// ###################################################
int touchTrigger = 1000;

// ###################################################
//                Initialize Libraries, etc
// ###################################################
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
CapacitiveSensor   cs_9_10 = CapacitiveSensor(9,10);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
// Create the bluefruit object, either software serial...uncomment these lines
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);
// the packet buffer for ble
extern uint8_t packetbuffer[];
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


// ###################################################
//                Initialize BLE module
// ###################################################
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

// ###################################################
//                Logic Flow
// ###################################################
void loop(void)
{
  long touchValue =  cs_9_10.capacitiveSensor(30);
  ble.readline();
  Serial.print("State: ");
  Serial.print(currentState);
  Serial.println("");
  Serial.print("\tTouch: ");
  Serial.print(touchValue);
  Serial.println("");
  uint8_t len = strcmp(ble.buffer, "");
  if(len != 0){
    Serial.println(len);
    //delay(100);
    Serial.println(millis() - start);
  }
  /*****************/
  /* State control */
  /*****************/
  switch (currentState) {
    case 1 :
      if (touchValue > touchTrigger) {
        start = millis();
        currentState = 2;
      } else if (len > 0) {
        start = millis();
        currentState = 3; 
      }
      delay(100); 
      break;

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
      } else if (touchValue > touchTrigger && millis() - start > 2) {
        previousState = currentState;
        currentState = 4;
        start = millis();  // probably don't need this
      } else {
        theaterChase(strip.Color(0, 0, 127), 50); // Blue
      }      
      break;
      
    case 4 :
      Serial.print("\t\t Time diff: ");
      Serial.print(millis() - start);
      Serial.println("");
      if (millis() - start > maxTime * 5) {
        previousState = currentState;
        currentState = 5;
        start = millis();
      } else if ( (touchValue < touchTrigger || len < 1) && millis() - start > maxTime * 2) {
       /* Potential BUG - this condition will fail unless ble.x->gateway->broker->gateway->ble.y 
          is constantly sending messages from touching. */ 
       currentState = previousState;
       start = millis();
      } else {
        rainbowCycle(50);
      }
      break;
      
    case 5 :
      if ((touchValue < touchTrigger || len < 1) && millis() - start > maxTime * 2) {
       /* Potential BUG - this condition will fail unless ble.x->gateway->broker->gateway->ble.y 
          is constantly sending messages from touching. */ 
        currentState = previousState;
        start = millis();
      } else {
        colorWipe(strip.Color(100, 100, 100), 50); // 
      }
      break;
    default :
      // Should never happen
      currentState = 1;
      break;
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
    strip.setBrightness(BRIGHTNESS);
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
      strip.setBrightness(BRIGHTNESS);
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.setBrightness(BRIGHTNESS);
    strip.show();
    //delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
