# Under cabinet lighting for under $60
## Description
Under the counter lights made from Adafruit NeoPixel strip controlled via Feather BluefruitLE.

## Parts
* [Feather BluefruitLE](https://www.adafruit.com/products/2829)
* [NeoPixel Strip](https://www.adafruit.com/products/1376) cut to about 1 meter (actually 42 pixels)
* [5Volt 2Amp plug](https://www.adafruit.com/products/276)
* [Terminal block](https://www.adafruit.com/products/368)
* [2-pin JST SM plug + Receptacle Cable Set](https://www.adafruit.com/products/2880)

## Circuit
[See photo](https://github.com/jbdamask/Adafruit/blob/master/neopixel_picker_feather_pin6/feather_ble_neopixel_circuit.jpg)

Plug power -> Feather BAT
           & NeoPixel power
           
Plug ground -> Feather GND
            & NeoPixel ground
            
NeoPixel data -> Feather pin 6
NeoPixel data ground -> Feather ground

(I don't know why the neopixel strip has two ground wires coming out of the end)

## Other stuff
The software is a combination of Adafruit examples from the Adafruit BluefruitLE nRF51 and Adafruit NeoPixel libraries (these can be installed in your Arduino IDE): neopixel_picker, bluefruit controller and neopixel strand test.

The strip is controlled using the [Adafruit Bluefruit LE iPhone](https://itunes.apple.com/us/app/adafruit-bluefruit-le-connect/id830125974?mt=8) app.
Colors set from color picker and effects from controller. For the latter, button clicks can produce sketchy results because the effects take a bit to run and additional clicks may not be heard. It's [not possible to interrupt via ble on the feather with the current firmware](http://forums.adafruit.com/viewtopic.php?f=22&t=94685&p=475626&hilit=feather+ble+interrupt#p475626).

### Note
At 3:45 in the video [NeoPixel Bluetooth Control Coding with Becky Stern](https://www.youtube.com/watch?v=Kym6crZF1Pg) she gives the secret to running the Adafruit BluefruitLE nRF51 neopixel_picker code w/out being connected to a computer. You need to comment out line 110 that says:

           while (!Serial);

