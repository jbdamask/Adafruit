/*
 * Simple code to demonstrate using a button to trigger a NeoPixel color change. All managed by an Adafrtuit Flora board.
 * There's actually nothing specific about the Flora...this will work fine on other boards
 */

#include <Adafruit_NeoPixel.h>

/* Define our pins */
#define NEOPIXEL_PIN 6
#define BUTTON_PIN 10

/* Create our NeoPixel object */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/* Button debouncing */
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time; increase if the output flickers 

/* Set up our method of tracking button state*/ 
int buttonState;
int lastButtonState = LOW;  

/* We'll cycle through event states on button pushes */
uint8_t colorState = 0;

void setup(){
  /* Button setup */
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  /* NeoPixel setup */
  strip.begin();
  strip.setBrightness(20);
  strip.show(); // Initialize all pixels to 'off'
}

void loop(){
  int reading = digitalRead(BUTTON_PIN);
  if(reading != lastButtonState){
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if(reading != buttonState){
      buttonState = reading;
      if(buttonState == LOW){
        colorState < 3 ? colorState += 1 : colorState = 0;
        switch(colorState){
          case 0:
            colorWipe(strip.Color(0, 255, 0), 2000);
            break;
          case 1:
            colorWipe(strip.Color(255, 0, 0), 2000);
            break;
          case 2:
            colorWipe(strip.Color(0, 0, 255), 2000);
            break;
          default:
            colorWipe(strip.Color(0, 0, 0), 2000);
            break;
        }
      }
    }
  }
  lastButtonState = reading;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
