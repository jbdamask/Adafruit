#include <CapacitiveSensor.h>
#include <Adafruit_NeoPixel.h>

#define PIN 8


/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 * Modified by Becky Stern 2013 to change the color of one RGB Neo Pixel based on touch input
 * 
 * Modified by John Damask 2016 to make Flora's on-board NeoPixel fade in and out when capacative sensor is touched/released
 * Note the original code by Paul and Becky uses the FloraPixel library.
 */


CapacitiveSensor   cs_9_10 = CapacitiveSensor(9,10);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
uint8_t brightness = 0; //global variable

void setup()                    
{
   cs_9_10.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(9600);
    strip.begin();
    strip.show();

}

void loop()                    
{
    long start = millis();
    long total1 =  cs_9_10.capacitiveSensor(30);
    //long total2 =  cs_9_2.capacitiveSensor(30);
    //long total3 =  cs_4_8.capacitiveSensor(30);

if (total1 > 1000){
   colorWipe(strip.Color(255,0,200), 50, true);
} else {
   colorWipe(strip.Color(255,0,200), 50, false);
}

    
    Serial.print(millis() - start);        // check on performance in milliseconds
    Serial.print("\t");                    // tab character for debug windown spacing

    Serial.println(total1);                  // print sensor output 1
    delay(10);                             // arbitrary delay to limit data to serial port 
}



// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, bool on) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    if( on && brightness < 256) brightness++;
    else if ( !on && brightness > 0 ) brightness--;
    strip.setBrightness( brightness );
    strip.show();
    delay(wait);
  }
}


