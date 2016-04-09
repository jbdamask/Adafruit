# noble_uart
Simple code to interact with my Adafruit Flora over a connected Adafruit Flora Bluefruit LE component. 
         I have the controller programmed with other code from this repo that listens to UART.RX for "RED", which 
        changes the onboard NeoPixel to red, and writes GREEN to UART.TX when a sensor is touched. 
        This javascript code inverts TX and RX so it can write "RED" to UART.TX and listens to UART.RX via
        the characteristic's "notify" property, which simply means GREEN is written to the console when the
        sensor is touched.
## Install
$ npm install

(don't worry about the warnings and errors)
## Usage
$ node noble_uart.js
