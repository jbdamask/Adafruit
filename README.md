# ESP32_tests

Testing basic functionality of Adafruit Feather HUZZAH ESP32 boards

## What it does
State machine for NeoPixel animations. All animations are non-blocking so you can always hear the next button click

* 0 Off (press and hold button)
* 1 Sparkle (single click)
* 2 Breathe cyan (double click)

## What it needs
- RGB NeoPixel ring
- Button
- Libraries
	- AceButton (nice lib for managing button pushes)
	- Adafruit's NeoPixel library