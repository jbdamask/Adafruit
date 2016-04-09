var async = require('async');
var noble = require('noble');

const UART_SERVICE_UUID = "6e400001b5a3f393e0a9e50e24dcca9e";
const TX_CHAR_UUID      = "6e400002b5a3f393e0a9e50e24dcca9e";
const RX_CHAR_UUID      = "6e400003b5a3f393e0a9e50e24dcca9e";
var characteristicUUIDs = [TX_CHAR_UUID, RX_CHAR_UUID];

noble.on('stateChange', function(state) {
  if (state ==='poweredOn') {
    noble.startScanning(UART_SERVICE_UUID);
  } else {
    noble.stopScanning();
  }
});

// Connect to first peripheral that runs the UART service
noble.on('discover', function(peripheral) {
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
             console.log('RX value is: ', data + '%');
           }
         });

         rx.notify(true, function(error){
           console.log('RX notification ON')
         });

         tx.write(new Buffer('RED'),true, function(error){
           console.log('Wrote RED to TX');
         });
      });


//      peripheral.disconnect();

    });
  });
});
