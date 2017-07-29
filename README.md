Bluefruit_LE_tx_rx
==================

Using RGB neopixels, and MPR121 capacitive touch breakout, Feather Bluefruit 32u4.
The Feather can both read and write from BLE. The idea is that colors can come in from a central device (e.g. a Pi) or you can touch a pad connected to a sensor and send that color to the device. Note that I've adopted the payload format from Adafruit for sending Colors  (as reconstructed from packetParser.cpp). Basically it's !C<red><green><blue><checksum>

