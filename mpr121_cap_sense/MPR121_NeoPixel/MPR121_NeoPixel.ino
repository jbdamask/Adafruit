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
#include <FastLED.h>
#define PIN 9
#define NUMPIXELS 12
#define NUMTOUCH 12
//volatile byte interrupt = 0;
//volatile byte state = LOW;
//#define LED_PIN     5
//------- FAST LED PARAMETERS -------
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60
/* COOLING: How much does the air cool as it rises?
  Less cooling = taller flames.  More cooling = shorter flames.
  Default 55, suggested range 20-100 */
#define COOLING  75
/* SPARKING: What chance (out of 255) is there that a new spark will be lit?
  Higher chance = more roaring fire.  Lower chance = more flickery fire.
  Default 120, suggested range 50-200. */
#define SPARKING 190
bool gReverseDirection = false;
CRGB leds[NUMPIXELS];
// Fire
CRGBPalette16 gPal;

// MRP121 stuff
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN);

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int index = 0;
bool animation = false;
uint8_t state = 0;

void setup() {
  //while (!Serial);        // needed to keep leonardo/micro from starting too fast!
  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 

  // Fast led stuff
  FastLED.addLeds<CHIPSET, PIN, COLOR_ORDER>(leds, NUMPIXELS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  // MPR121 stuff
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
  // Fast led...
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());
  // Get the currently touched pads
  currtouched = cap.touched();
  for (uint8_t i=0; i<NUMTOUCH; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      if(i>8) {
        animation = true;
      } else {
        animation = false;
        strip.setPixelColor(i+1,255,0,0);              
      }
      state = i;
    }
/*        
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      strip.setPixelColor(i+1,0,0,0);
    } */
    strip.show();
  }
  
  // reset our state
  lasttouched = currtouched;
  //if(animation==true) {   
    switch (state){
      case 0:
        colorWipe(strip.Color(0,0,0));
        break;        
      case 1:
        colorWipe(strip.Color(113,40,180));
        break;      
      case 2:
        colorWipe(strip.Color(255,0,0));    
        break;
      case 3:
        colorWipe(strip.Color(0,255,0));      
        break;
      case 4:
        colorWipe(strip.Color(0,0,255));      
        break;
      case 5:
        colorWipe(strip.Color(255,255,0));      
        break;
      case 6:
        colorWipe(strip.Color(255,0,255));            
        break;
      case 7:
        colorWipe(strip.Color(255,255,0));            
        break;
      case 8:
        colorWipe(strip.Color(0,255,255));  
        break;
      case 9:
        rainbow();
        break;
      case 10:   // cold as ice
        gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
        Fire2012WithPalette(); // run simulation frame, using palette colors
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);
        break;
      case 11:   // FIRE!
        gPal = HeatColors_p;
        Fire2012WithPalette(); // run simulation frame, using palette colors
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);
        break;
    default:
      colorWipe(strip.Color(0,0,0));
      break;  
    }
    
  if(animation==true) {     
    index++;
  } else {
     // put a delay so it isn't overwhelming
    delay(100);
  }
}

// ------- ANIMATIONS -------
// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.setBrightness(30);
    strip.show();
    delay(5);
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

void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUMPIXELS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUMPIXELS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUMPIXELS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUMPIXELS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUMPIXELS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUMPIXELS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}
