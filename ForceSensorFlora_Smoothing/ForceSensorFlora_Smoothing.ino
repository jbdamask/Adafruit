#include <AnalogSmooth.h>
#include <Adafruit_NeoPixel.h>

/* Force Sensing Resistor testing sketch. 

I'm using a Flora board from Adafruit for this.
Connect one end of FSR to 3.3V, the other end to one end of a 100K resistor, then connect the same end of the resitor to D10
and the other end of the resistor to ground. See https://github.com/jbdamask/Adafruit/blob/master/ForceSensorFlora/ForceSensorFlora_wiring.jpg

I made a force sensor using a kit. The kit doesn't exist anymore but video here https://youtu.be/M2tzLSo_kzM and information 
from Kickstarter here https://www.kickstarter.com/projects/1340651422/sensor-film-kit-custom-force-sensors-and-switches.
I can probably just use velostat, tin foil and scotch tape.
 
For more information see www.ladyada.net/learn/sensors/fsr.html */

/* ########################################
            CONFIGURATION
   ######################################## */
#define FSR_ANALOG_PIN 10 // FSR is connected to analog 0
#define NUMBER_PIXELS 1   // Only one on the board
#define PIN 8             // Flora's onboard NeoPixel pin
#define MAX_FSR_VALUE 1023  // This is the max value of the fsr sensor using standard analog input
int fsrReading;      // the analog reading from the FSR resistor divider
int threshold;       // trigger point
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
AnalogSmooth as = AnalogSmooth(); // Defaults to window size 10

/* ########################################
            ACTION JACKSON
   ######################################## */
void setup(void) {
  threshold = 0.10 * MAX_FSR_VALUE;   // Configure our sensor threshold for "ON"
  strip.begin();
}

void loop(void) {
  float analogSmooth = as.analogReadSmooth(FSR_ANALOG_PIN);
  strip.setBrightness((analogSmooth > threshold) ? 100 : 0);
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  delay(100);
}

