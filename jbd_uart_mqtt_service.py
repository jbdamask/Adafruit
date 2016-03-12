# Author: John B Damask
# Created: March 2016
# Purpose: Gateway between a single, local ble device and Adafruit.io MQTT broker.
#          The program simulates the Adafruit LE Connect iOS app.
#          Setup, two sets of devices consisting of:
#          Flora, Flora Bluefruit LE, Capacitive touch sensor
#          This code runs from my Raspberry Pi 2 and connects to one
#          device while the Adafruit LE Connect app connects to the other
#          This two gateway approach is for my PoC and simulates the devices
#          being in different geographical locations.
#          The respective gateways are configured to pub/sub to my Adafruit.IO
#          feeds, thing1 and thing2 (of course the pub/sub channels are swapped
#          so that Device 1 publishes to feed1 and Device 2 subscribes to feed1, etc
#
# Prereqs: Make sure the BLE devices have already been paired/connected
#          to the gateways. This is easy with iOS but takes a bit of configuring
#          on the Pi using bluez (see http://www.elinux.org/RPi_Bluetooth_LE).
#
# Notes:   The code is clunky and sometimes produces unintended results. I
#          assume this is due to some sync issues between MQTT and my handlers.
#          An important piece is the use of a global variable, "msg" to
#          handle the payload transfer between MQTT and UART. While I don't like
#          this, I couldn't figure out how to get the MQTT and UART background
#          threads to share data.
#
# Credits: Original code for UART and MQTT by Tony DiCola from Adafruit
#          BLE: Adafruit_Python_BluefruitLE/examples/uart_service.py
#          MQTT: io-client-python/examples/mqtt_client.py
#   
# Run:   $ python jbd_uart_mqtt_service.py

import sys
from Adafruit_IO import MQTTClient
import Adafruit_BluefruitLE
from Adafruit_BluefruitLE.services import UART

ADAFRUIT_IO_KEY      = <KEY>
ADAFRUIT_IO_USERNAME = <USERNAME>
FEED_PUB = 'thing2'
FEED_SUB = 'thing1'

ble = Adafruit_BluefruitLE.get_provider()
client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
msg = None

def ble_main():
    global msg
    ble.clear_cached_data()
    adapter = ble.get_default_adapter()
    adapter.power_on()
    print('Using adapter: {0}'.format(adapter.name))
    print('Disconnecting any connected UART devices...')
    UART.disconnect_devices()
    print('Searching for UART device...')

    try:
        adapter.start_scan()
        # Search for the first UART device found (will time out after 60 seconds
        # but you can specify an optional timeout_sec parameter to change it).
        device = UART.find_device()
        if device is None:
            raise RuntimeError('Failed to find UART device!')
    finally:
        # Make sure scanning is stopped before exiting.
        adapter.stop_scan()
    # Connect to first device 
    print('Connecting to device...')
    device.connect()  # Will time out after 60 seconds, specify timeout_sec parameter
                      # to change the timeout.

    # Once connected do everything else in a try/finally to make sure the device
    # is disconnected when done.
    try:
        # Wait for service discovery to complete for the UART service.  Will
        # time out after 60 seconds (specify timeout_sec parameter to override).
        print('Discovering services...')
        UART.discover(device)

        # Once service discovery is complete create an instance of the service
        # and start interacting with it.
        uart = UART(device)
        print('Connected')

        client.loop_background()
        while True:
            received = uart.read(3)
            if received is not None:
                # Received data, print it out.
                print('UART received: {0} Writing RED to MQTT'.format(received))
                client.publish(FEED_PUB, 'RED')
                #uart.write('GREEN\r\n')
#            else:
#                print('UART got nothing')
                
            if msg is not None:
                print('MQTT received: {0} Writing RED to UART'.format(msg))
                uart.write('RED\r\n')
                msg = None
#            else:
#                print('MQTT got nothing')
                    
    finally:
        # Make sure device is disconnected on exit.
        device.disconnect()


# Define callback functions which will be called when certain events happen.
def connected(client):
    # Connected function will be called when the client is connected to Adafruit IO.
    # This is a good place to subscribe to feed changes.  The client parameter
    # passed to this function is the Adafruit IO MQTT client so you can make
    # calls against it easily.
    print 'Connected to Adafruit IO!  Listening for {0} changes...'.format(FEED_SUB)
    # Subscribe to changes on a feed named DemoFeed.
    client.subscribe(FEED_SUB)

def disconnected(client):
    # Disconnected function will be called when the client disconnects.
    print 'Disconnected from Adafruit IO!'
    sys.exit(1)

def message(client, feed_id, payload):
    # Message function will be called when a subscribed feed has a new value.
    # The feed_id parameter identifies the feed, and the payload parameter has
    # the new value.
    print 'Feed {0} received new value: {1}'.format(feed_id, payload)
    # Set a global var with the payload then check for it above
    global msg
    msg = payload

# Create an MQTT client instance
client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)

# Setup the callback functions defined above
client.on_connect = connected
client.on_disconnect = disconnected
client.on_message = message

# Connect to the Adafruit IO server
client.connect()

ble.initialize()

ble.run_mainloop_with(ble_main)

