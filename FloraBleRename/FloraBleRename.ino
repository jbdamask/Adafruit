// Basic sketch showing how to change the advertised name of an Adafruit Bluefruit LE board
#include "Adafruit_BluefruitLE_UART.h"
#define BLE_NAME "AT+GAPDEVNAME=I changed the name"
Adafruit_BluefruitLE_UART ble(Serial1, -1);

void setup() {
  ble.begin(true);   // Start it up  
  ble.println(BLE_NAME); // Change advertised name 
}

void loop() {
  /* nothing to do */
}

