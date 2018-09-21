/* 
  Basic sketch showing how to change the advertised name of an Adafruit Bluefruit LE board
  Check with any BLE sniffer like Adafruit Bluefruit LE Connect app 
*/
#include "Adafruit_BluefruitLE_UART.h"

// If using a Flora Bluefruit LE board, connect MODE pin to Flora D12 and change this to 12
#define MODE_PIN    12
#define BLE_NAME "AT+GAPDEVNAME=I changed the name"

Adafruit_BluefruitLE_UART ble(Serial1, MODE_PIN);

void setup() {
  // Uncomment for Flora Bluefruit LE boards.
  // This makes the board start in CMD mode. Starting in DATA mode will result in the name change not working
  digitalWrite(MODE_PIN, HIGH);
  ble.begin(true);   // Start it up  
  ble.println(BLE_NAME); // Change advertised name
}

void loop() {
  /* nothing to do */
}

