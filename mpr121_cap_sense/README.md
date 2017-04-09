# MPR121_NeoPixel
### Author: John B Damask & Adafruit folks (via their demos)
### Purpose: Capacitive touch for NeoPixel control
### Hardware: Adafruit BlueFruit Feather 32u4; Adafruit MPR121 capacitive touch breakout board; Adafruit NeoPixel ring
### Notes: 
* This sketch doesn't include any Bluetooth but I'll add it soon (easy)
* I want some pads to trigger animations and the examples I had were in FOR loops. These tie up the processor so I tried to get hardware interrupts working. But I couldn't figure it out...sigh. But I get to the same point by rigging the animation FOR loops to use the chip's loop() function and a global variable.

## Wiring
* Feather:Scl - MPR:Scl
* Feather:Sda - MPR:Sda
* Feather:9 - NexPixel:In