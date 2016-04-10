/*
 Note: This works great but I've run into situations when re-running it does not
        This seems to have to do with improper disconnection. While I can reconnect
        via gatttools, this code will never find the services and characteristics.
        I found I can fix this by reseting the Bluetooth adapter on the Pi
        $ sudo hciconfig hci0 reset
        I'd bet this has something to do with me not disconnecting to the peripheral
        correctly
*/

var noble = require('noble');
//var prompt = require('prompt');
const EventEmitter = require('events');
const util = require('util');
var stdin=process.stdin;

function MyEmitter() {
  EventEmitter.call(this);
}

util.inherits(MyEmitter, EventEmitter);
const myEmitter = new MyEmitter();

myEmitter.on('write', function() {
  console.log('Read from stdin. Writing RED to TX');
  tx.write(new Buffer('RED'), true, function(error){
    console.log('Write RED to TX');
  })
});

myEmitter.on('read', function(r) {
  console.log('An RX event occurred: ' + r);
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


/*
         tx.write(new Buffer('RED'),true, function(error){
           console.log('Wrote RED to TX');
         });
*/
      });


//      peripheral.disconnect();

    });
  });
}

noble.on('stateChange', scan);
noble.on('discover', findAndConnect);
stdin.resume();
stdin.on('data',function(){
  myEmitter.emit('write');
}).on('end',function(){ // called when stdin closes (via ^D)
  console.log('stdin:closed');
});


function onErr(err) {
  console.log(err);
  return 1;
}
