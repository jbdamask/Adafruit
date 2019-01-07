## NeoPixel StrandTest NonBlocking
This is code from Mike Cook posted on Arduino message board back in 2016. 
### URL
https://forum.arduino.cc/index.php?topic=412232.0
### Why?
The NeoPixel strandtest example that comes with the library blocks input. This means if you use the examples in your own script that has an input, the animations block the input. The way around this is to take advantage of the main loop() function. Mike does a great job demonstrating this.

### Note
Bill Earl has a [nice Adafruit post](https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all#overview) from 2015 that elegantly puts non-blocking code into classes. 