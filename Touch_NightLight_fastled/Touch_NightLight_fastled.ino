/*
 * Name: Touch_NightLight_fastled
 * Created: June 2016
 * Authors: John B Damask, liberally borrowing code from several Adafruit tutorials
 * Purpose: I have a NeoPixel strand, a Feather and three sensors for a total of 7 possible states
 * Notes: As of June 26, 2016 this works nicely but isn't perfect. Issues:
 *        1. Capacitive sensors are funky about grounding so I have a very low threshold for triggering the event.
 *           It works in my house when the device is plugged into the wall (with no ground) but I don't know if
 *           it would behave the same way on battery.
 *        2. To run long animations and listen for new events without an interrupt, I use the controller loop 
 *            instead of nested FOR loops. The animations aren't smooth yet but they are decent approximations.
 *        3. theaterChaseRainbow method needs to be re-written as it currently has a blocking FOR loop.
 *        4.  Clean up
 * 
 * Please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 */

#include <Adafruit_NeoPixel.h>
#include <CapacitiveSensor.h>
#include <FastLED.h>
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define PIN       6
#define NUMPIXELS 23
#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 30
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  75

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 190

bool gReverseDirection = false;
CRGB leds[NUMPIXELS];

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN);
// Fire
CRGBPalette16 gPal;

class TouchLight{
  CapacitiveSensor cs;  // Pin to monitor. 1M resistor between send and receive pin
  long total = 0;           // Sensor value
  int threshold = 100;        // Touch threshold at which we are ON 
  int id;
  
  public:
  TouchLight(uint8_t sendPin, uint8_t receivePin, int mID) : cs(CapacitiveSensor(sendPin, receivePin)), id(mID){}

  boolean isTouched(){
    total = cs.capacitiveSensor(30);
    if(total > threshold){
      Serial.print(id);
      Serial.print("\tCS Value: ");
      Serial.println(total);
      return true;
    } else {
      return false;
    }
  }
};

uint8_t currentState = 0; // Keeps track of current state 
uint8_t prevState = 0;    // Keeps track of previous state
long rSeed = 0;        // Holds random seed
uint8_t rRed = 0;         // Random red
uint8_t rGreen = 0;       // Random green
uint8_t rBlue = 0;        // Random blue
long previousMillis = 0;        // will store last time LED was updated
long duration = 500;           // Duration for timer
//uint8_t prevPixel = 0; // Keeps track of pixels for rainbow effect
//uint8_t prevPixelColor = 0;
uint16_t pixelCount = 0;
uint16_t colorCount = 0; 
uint16_t pixelOffset = 0;
uint16_t loopCounter = 0;
boolean onOff = false;
TouchLight tRed = TouchLight(23,19,1);
TouchLight tGreen = TouchLight(23,20,2);
TouchLight tBlue = TouchLight(23,21,3);

void setup() {
  // turn off neopixel
  pixel.begin(); // This initializes the NeoPixel library.
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
  }
  pixel.show();

  // Set a random seed so our random colors are different on each run
  // Per Arduino idiom, use an unconnected pin to generate the seed
  rSeed = analogRead(18);

  // Fast led stuff
  FastLED.addLeds<CHIPSET, PIN, COLOR_ORDER>(leds, NUMPIXELS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

}

void loop() {
  // Fast led...
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());
  
  prevState = currentState;   // Keep track of previous state
  long now = millis();
  if(now - previousMillis > duration){
    previousMillis = now;    
  } else {
    stateCondition();
    return;
  }
  // Touch
  if (tRed.isTouched() && tGreen.isTouched() && tBlue.isTouched()){
    currentState = 7;
    ////delay(20);
    Serial.println("All touched!");
  } else if (tRed.isTouched() && tGreen.isTouched()){
    currentState = 6;
    //delay(20);
  } else if (tRed.isTouched() && tBlue.isTouched()) {
    currentState = 5;
    //delay(20);
  } else if (tGreen.isTouched() && tBlue.isTouched()) {      
    rRed = random(50,225);
    rGreen = random(50,225);
    rBlue = random(50,225);    
    currentState = 4;
    //delay(20);
    } else if (tRed.isTouched()) {
    // If red sensor touched twice in succession, turn strip off
    if(prevState != 1 ){
      currentState = 1;
    } else {
      currentState = 0;
    }
  } else if (tGreen.isTouched()) {    
    currentState = 2;
  } else if (tBlue.isTouched()) {    
    currentState = 3;
  }
  stateCondition();
}

void stateCondition(){

  switch(currentState){    
    case 1:   // RED
      colorWipe(pixel.Color(255,0,0));
      Serial.println("Red");
      break;
    case 2:  // GREEN
      colorWipe(pixel.Color(0,255,0));
      Serial.println("Green");
      break;  
    case 3:  // BLUE
      colorWipe(pixel.Color(0,0,255));
      Serial.println("Blue");
      break;  
    case 4:   // RANDOM COLOR
      Serial.println("Random");
      colorWipe(pixel.Color(rRed,rGreen,rBlue));
      break;
    case 5:   // RAINBOW
      Serial.println("Rainbow");
      rainbowCycle();
      break;
    case 6:   // cold as ice
      gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
      Fire2012WithPalette(); // run simulation frame, using palette colors
      FastLED.show(); // display this frame
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      break;
    case 7:   // FIRE!
      gPal = HeatColors_p;
      Fire2012WithPalette(); // run simulation frame, using palette colors
      FastLED.show(); // display this frame
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      break;
    default:
      colorWipe(pixel.Color(0,0,0));
      break;            
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<pixel.numPixels(); i++) {
    pixel.setPixelColor(i, c);
    pixel.setBrightness(30);
    pixel.show();
    delay(5);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
// JBD - I changed the original rainbow() code from strandtest so we can receive inputs w/out interrupts
// It's not as smooth as the original
void rainbowCycle() {  
    pixelCount =  (pixelCount > pixel.numPixels()) ? 0 : pixelCount + 1;
    colorCount = (colorCount > 255*5) ? 0 : colorCount + 1;
    pixel.setPixelColor(pixelCount, Wheel(((pixelCount * 256 / pixel.numPixels()) + colorCount) & 255));    
    pixel.setBrightness(100);
    pixel.show();    
    delay(10);  
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

