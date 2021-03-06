/*********************************************************************
  Author: Adafruit folks, John B Damask
  Created: Februafy 2016
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
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_CC3000.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT_FONA.h>

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
    #define PIN                     8
    #define NUMPIXELS               1
/*=========================================================================*/

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
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
  long start = millis();
  long total1 =  cs_9_10.capacitiveSensor(30);
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
   Serial.print(millis() - start);        // check on performance in milliseconds
    Serial.print("\t");                    // tab character for debug windown spacing
    Serial.print(total1);                  // print sensor output 1
    Serial.print("\t");
    delay(10);                             // arbitrary delay to limit data to serial port   
  if(len > 0){
    // Color
    if (packetbuffer[1] == 'C') {
      uint8_t red = packetbuffer[2];
      uint8_t green = packetbuffer[3];
      uint8_t blue = packetbuffer[4];
      Serial.print ("RGB #");
      if (red < 0x10) Serial.print("0");
      Serial.print(red, HEX);
      if (green < 0x10) Serial.print("0");
      Serial.print(green, HEX);
      if (blue < 0x10) Serial.print("0");
      Serial.println(blue, HEX);
      strip.setPixelColor(0, strip.Color(red,green,blue));
      strip.setBrightness(50);
      strip.show();
    }
  }else if(total1 > 1000) {
    digitalWrite(7, HIGH);
    strip.setPixelColor(0, strip.Color(0,255,0));
    strip.setBrightness(30);
    strip.show();
  }else{    
    return;
  }
 
}
