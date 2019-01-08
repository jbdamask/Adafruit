
// StrandTest from AdaFruit implemented as a state machine
// pattern change by push button
// By Mike Cook Jan 2016

#define PINforControl   6 // pin connected to the small NeoPixels strip
#define NUMPIXELS1      13 // number of LEDs on strip
#define BRIGHTNESS      30 // Max brightness of NeoPixels

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS1, PINforControl, NEO_GRB + NEO_KHZ800);

unsigned long patternInterval = 20 ; // time between steps in the pattern
unsigned long lastUpdate = 0 ; // for millis() when last update occoured
unsigned long intervals [] = { 20, 20, 50, 100, 2, 50 } ; // speed for each pattern
const byte button = 10; // pin to connect button switch to between pin and ground

// Colors for flashRandom
uint8_t myFavoriteColors[][3] = {{200,   0, 200},   // purple
                                 {200,   0,   0},   // red 
                                 {200, 200, 200},   // white
                               };
#define FAVCOLORS sizeof(myFavoriteColors) / 3

void setup() {
  strip.setBrightness(BRIGHTNESS); // These things are bright!
  strip.begin(); // This initializes the NeoPixel library.
  wipe(); // wipes the LED buffers
  pinMode(button, INPUT_PULLUP); // change pattern button
}

void loop() {
  static int pattern = 0, lastReading;
  int reading = digitalRead(button);
  if(lastReading == HIGH && reading == LOW){
    pattern++ ; // change pattern number
    if(pattern > 5) pattern = 0; // wrap round if too big
    patternInterval = intervals[pattern]; // set speed for this pattern
    wipe(); // clear out the buffer 
    resetBrightness();
    delay(50); // debounce delay
  }
  lastReading = reading; // save for next time

if(millis() - lastUpdate > patternInterval) updatePattern(pattern);
}

void  updatePattern(int pat){ // call the pattern currently being created
  switch(pat) {
    case 0:
        rainbow(); 
        break;
    case 1: 
        rainbowCycle();
        break;
    case 2:
        theaterChaseRainbow(); 
        break;
    case 3:
        colorWipe(strip.Color(255, 0, 0)); // red
        break;  
    case 4:
        breatheBlue();
        break;   
    case 5:
        wipe();
        flashRandom(3);
        break;
  }  
}

void rainbow() { // modified from Adafruit example to make it a state machine
  static uint16_t j=0;
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
     j++;
  if(j >= 256) j=0;
  lastUpdate = millis(); // time for next change to the display
  
}
void rainbowCycle() { // modified from Adafruit example to make it a state machine
  static uint16_t j=0;
    for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
  j++;
  if(j >= 256*5) j=0;
  lastUpdate = millis(); // time for next change to the display
}

void theaterChaseRainbow() { // modified from Adafruit example to make it a state machine
  static int j=0, q = 0;
  static boolean on = true;
     if(on){
            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
             }
     }
      else {
           for (int i=0; i < strip.numPixels(); i=i+3) {
               strip.setPixelColor(i+q, 0);        //turn every third pixel off
                 }
      }
     on = !on; // toggel pixelse on or off for next time
      strip.show(); // display
      q++; // update the q variable
      if(q >=3 ){ // if it overflows reset it and update the J variable
        q=0;
        j++;
        if(j >= 256) j = 0;
      }
  lastUpdate = millis(); // time for next change to the display    
}

void colorWipe(uint32_t c) { // modified from Adafruit example to make it a state machine
  static int i =0;
    strip.setPixelColor(i, c);
    strip.show();
  i++;
  if(i >= strip.numPixels()){
    i = 0;
    wipe(); // blank out strip
  }
  lastUpdate = millis(); // time for next change to the display
}

void breatheBlue() { // modified from Adafruit example to make it a state machine
  float MaximumBrightness = 30;
  float SpeedFactor = 0.008; // I don't actually know what would look good
  static int i = 0;
  // Make the lights breathe
  float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
  strip.setBrightness(intensity);
  for (int j=0; j<strip.numPixels(); j++) {
    strip.setPixelColor(j, 0, 127, 127);
  }
  strip.show();
  i++;
  if(i >= 65535){
    i = 0;
  }
  lastUpdate = millis();
}

void flashRandom(uint8_t howmany) {

  static int x = 0;
  static bool goingUp = true;
  
  for(uint16_t i=0; i<howmany; i++) {
    // pick a random favorite color!
    int c = random(FAVCOLORS);
    int red = myFavoriteColors[c][0];
    int green = myFavoriteColors[c][1];
    int blue = myFavoriteColors[c][2]; 

    // get a random pixel from the list
    int j = random(strip.numPixels());
    
    // now we will 'fade' it in 5 steps
    if(goingUp){
      if(x < 5) {
        x++;
      } else {
        goingUp = false;
      }
    } else {
      if(x>0){
        x--;
      } else {
        goingUp = true;
      }     
    }

    int r = red * (x+1); r /= 5;
    int g = green * (x+1); g /= 5;
    int b = blue * (x+1); b /= 5;      
    strip.setPixelColor(j, strip.Color(r,g,b));      
    strip.show();
  }
  lastUpdate = millis();
}



void wipe(){ // clear all LEDs
     for(int i=0;i<strip.numPixels();i++){
       strip.setPixelColor(i, strip.Color(0,0,0)); 
       }
}


void resetBrightness(){
  strip.setBrightness(BRIGHTNESS);
}


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
