/*
Author: John B Damask
Created: April 2016
Purpose: Two way communication between microcontroller (Adafruit Flora w/ Bluefruit LE)
         and Adafruit.io.
 Note: This works great but I've run into situations when re-running it does not
        This seems to have to do with improper disconnection. While I can reconnect
        via gatttools, this code will never find the services and characteristics.
        I found I can fix this by reseting the Bluetooth adapter on the Pi
        $ sudo hciconfig hci0 reset
        I'd bet this has something to do with me not disconnecting to the peripheral
        correctly
*/

var noble = require('noble');
const EventEmitter = require('events');
const util = require('util');
var mqtt = require('mqtt');

const ADAFRUIT_IO_KEY = '<yourkey>';
const ADAFRUIT_IO_USERNAME = '<youru>';
const FEED_PUB = '<youru>/feeds/<yourfeed1>'; // dunno why but full path to feed required
const FEED_SUB = '<youru>/feeds/<yourfeed2>';

// Connect to adafruit io
var client  = mqtt.connect({host: 'io.adafruit.com',
                            port: 1883,
                            protocol: 'mqtt',
                            username: ADAFRUIT_IO_USERNAME,
                            password: ADAFRUIT_IO_KEY,
                            connectTimeout: 60 * 1000,
                            keepalive: 10000
                          });

client.on('error', function(err) {onErr(err)});

// Set up emitter
function MyEmitter() {
  EventEmitter.call(this);
}
util.inherits(MyEmitter, EventEmitter);
const myEmitter = new MyEmitter();

myEmitter.on('read', function(r) {
  console.log('Read value ' + r + ' from RX');
  console.log('Publishing to MQTT feed: ' + FEED_PUB);
  client.publish(FEED_PUB, 'RED\n');
});

myEmitter.on('message', function(message){
  console.log("myEmitter -> Received message from MQTT: " + message.toString());
  if(tx == null) {return};
  tx.write(new Buffer('RED'), true, function(error){
    console.log('Write RED to TX');
  })
});

const UART_SERVICE_UUID = "6e400001b5a3f393e0a9e50e24dcca9e";
const TX_CHAR_UUID      = "6e400002b5a3f393e0a9e50e24dcca9e";
const RX_CHAR_UUID      = "6e400003b5a3f393e0a9e50e24dcca9e";
var characteristicUUIDs = [TX_CHAR_UUID, RX_CHAR_UUID];

var tx;
var rx;

function scan(state){
  if (state ==='poweredOn') {
    noble.startScanning(UART_SERVICE_UUID);
  } else {
    noble.stopScanning();
  }
}


// Connect to first peripheral that runs the UART service
function findAndConnect(peripheral) {
  peripheral.connect(function(error) {
    console.log('connected to peripheral: ' + peripheral.toString());
    peripheral.discoverServices(UART_SERVICE_UUID, function(error, services) {
      var deviceInformationService = services[0];
      console.log('Discovered UART Service');

      deviceInformationService.discoverCharacteristics(characteristicUUIDs, function(error, characteristics) {
         for(var i in characteristics){
           if(characteristics[i].uuid === TX_CHAR_UUID){
             tx = characteristics[i];
             console.log("TX object: " + tx.toString());
           } else {
             rx = characteristics[i];
             console.log("RX object: " + rx.toString());
           }
         }

         rx.on('read', function(data, isNotification){
           if(data != null){
             console.log('RX value is: ', data);
             myEmitter.emit('read', data);
           }
         });

         rx.notify(true, function(error){
           console.log('RX notification ON')
         });

      });


//      peripheral.disconnect();

    });
  });
}

// Setup Bluetooth LE listeners
noble.on('stateChange', scan);
noble.on('discover', findAndConnect);

// Subscribe to adafruit.io feeds
client.on('connect', function (err) {
  console.log('Connecting to MQTT');
  client.subscribe(FEED_SUB);
});

client.on('message', function (topic, message) {
  // message is Buffer
  console.log("Received message from MQTT: " + message.toString());
  myEmitter.emit('message', message);
  //client.end();
});


function onErr(err) {
  console.log(err);
  return 1;
}
