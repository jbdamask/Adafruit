# noble_uart_mqtt
Bluetooth LE, IoT gateway server. 
Extends my noble_uart code to interact with Adafruit.io over MQTT. 
I've installed this code on a Raspberry Pi 2 running Raspbian GNU/Linux 8 (jessie). 
It requires you have an account on Adafruit.
Put account and feed info in noble_uart_mqtt.js.

The hardware consists of Adafruit Flora, Flora Bluefruit LE and conductive fabric (See the project's jpg for setup). I have the controller programmed with other code from this repo that listens to UART.RX for "RED", which changes the onboard NeoPixel to red, and writes GREEN to UART.TX when a sensor is touched. This javascript code inverts TX and RX so it can write "RED" to UART.TX and listens to UART.RX viathe characteristic's "notify" property, which simply means GREEN is written to the console when the sensor is touched.

## Install
$ npm install

(don't worry about the warnings and errors)
## Usage
$ node noble_uart_mqtt.js
