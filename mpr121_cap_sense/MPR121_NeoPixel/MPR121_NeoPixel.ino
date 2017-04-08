/*********************************************************
Author: John B Damask & Adafruit folks (via their demos)
Created: April 8, 2017
Purpose: Using Adafruit's capacitive touch board to control NeoPixels. 

This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_NeoPixel.h>
#define PIN 9
#define NUMPIXELS 12
#define NUMTOUCH 12
volatile byte interrupt = 0;
volatile byte state = LOW;

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN);

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int index = 0;
bool animation = false;

void setup() {
  //while (!Serial);        // needed to keep leonardo/micro from starting too fast!
  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  strip.begin();
    for(uint8_t i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0,0,0)); // off
  }
  strip.show();
}

void loop() {
    // Get the currently touched pads
  currtouched = cap.touched();
  for (uint8_t i=0; i<NUMTOUCH; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
      if(i==8) {
         animation = true;
      } else {
        animation = false;
        strip.setPixelColor(i+1,255,0,0);              
      }
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      strip.setPixelColor(i+1,0,0,0);
    }
    strip.show();
  }
  
  // reset our state
  lasttouched = currtouched;
  if(animation==true) {   
    rainbow();
    index++;
  } else {
     // put a delay so it isn't overwhelming
    delay(100);
  }
}

// Rainbow animation
void rainbow() {
  //for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
  if(index < 256){
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        //strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third strip on
        strip.setPixelColor(i+q, Wheel( (i+index) % 255));    //turn every third strip on
      }
      strip.show();

      delay(2);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third strip off
      }
    }
  } else {
    index = 0;
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
