#JBDBluefruit_Touch_FSM
##Overview
Microcontroller code that connects Flora and Flora Bluefruit LE together with a capacitive sensor and NeoPixels. 

A finite state machine is used to move through several states:

1. Off
2. Calling
3. IsCalled
4. Connected
5. ConnectedLowEnergy

The onboard NeoPixel's color is controlled by the touch sensor and by UART BLE commands (for example, those sent by Adafruit's Bluefruit LE Connect iOS app)

Two of these devices can communicate using adafruit.io as the broker. Each device needs it's own gateway (at least now they do). Use the Adafruit app and/or the noble_uart_mqtt code in this repo.