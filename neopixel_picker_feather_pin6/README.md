# neopixel_picker_feather_pin6
## Description
Adafruit NeoPixel strip and Feather BluefruitLE.
I use a 5V 2A plug and soldered it into the feather

Plug power -> Feather BAT
           -> NeoPixel power
           
Plug ground -> Feather GND
            -> neopixel ground
            
Neopixel data -> feather pin 6
Neopixel data ground -> feather ground

(I don't know why the neopixel strip has two ground wires coming out of the end)

The software is a combination of Adafruit examples from the Adafruit BluefruitLE nRF51 and Adafruit NeoPixel libraries (these can be installed in your Arduino IDE): neopixel_picker, bluefruit controller and neopixel strand test.

The strip is controlled using the [Adafruit Bluefruit LE iPhone](https://itunes.apple.com/us/app/adafruit-bluefruit-le-connect/id830125974?mt=8) app.
Colors set from color picker and effects from controller. For the latter, we have unresponsive button clicks because the effects run in a loop and it's [not possible to interrupt via ble on the feather with the current firmware](http://forums.adafruit.com/viewtopic.php?f=22&t=94685&p=475626&hilit=feather+ble+interrupt#p475626).

### Note
At 3:45 in the video [NeoPixel Bluetooth Control Coding with Becky Stern](https://www.youtube.com/watch?v=Kym6crZF1Pg) she gives the secret to running the Adafruit BluefruitLE nRF51 neopixel_picker code w/out being connected to a computer. You need to comment out line 110 that says:

           -> while (!Serial);

