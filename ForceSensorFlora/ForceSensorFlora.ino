#include <Adafruit_NeoPixel.h>
/* Force Sensing Resistor testing sketch. 

I'm using a Flora board from Adafruit for this.
Connect one end of FSR to 3.3V, the other end to one end of a 100K resistor, then connect the same end of the resitor to D10
and the other end of the resistor to ground..

I made a force sensor using a kit. The kit doesn't exist anymore but video here https://youtu.be/M2tzLSo_kzM and information 
from Kickstarter here https://www.kickstarter.com/projects/1340651422/sensor-film-kit-custom-force-sensors-and-switches.
I can probably just use velostat, tin foil and scotch tape.
 
For more information see www.ladyada.net/learn/sensors/fsr.html */
 
int fsrAnalogPin = 10; // FSR is connected to analog 0
#define PIN 8
int fsrReading;      // the analog reading from the FSR resistor divider
int LEDbrightness;

#define FROM_LOW 0
#define FROM_HIGH 1023
#define TO_LOW 0
#define TO_HIGH 100

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

void setup(void) {
  Serial.begin(9600);   // We'll send debugging information via the Serial monitor
  strip.begin();
  strip.setPixelColor(1, strip.Color(0, 255, 0));
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'on'
}
 
void loop(void) {
  fsrReading = analogRead(fsrAnalogPin);
  Serial.print("Analog reading = ");
  Serial.println(fsrReading);
  
  // we'll need to change the range from the analog reading (0-1023) down to the range
  // used by analogWrite (0-100) with map!
  // map(value, fromLow, fromHigh, toLow, toHigh)
  // Map From-To ranges
//  int fromLow = 0;
//  int fromHigh = 1023;
//  int toLow = 0;
//  int toHigh = 50;
  LEDbrightness = map(fsrReading, FROM_LOW, FROM_HIGH, TO_LOW, TO_HIGH);
  // LED gets brighter the harder you press
  Serial.println(LEDbrightness);
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setBrightness(LEDbrightness);
  strip.show();
  delay(100);
}

