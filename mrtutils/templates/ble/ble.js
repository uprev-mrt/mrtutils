var deviceName = 'Palm' 
var bleService = 'environmental_sensing'
var bleCharacteristic = 'humidity'
var bluetoothDeviceDetected
var gattCharacteristic

function connect() 
{
    if (navigator.bluetooth) 
    {
        console.log('Requesting Bluetooth device...');

        navigator.bluetooth.requestDevice({
        "acceptAllDevices": true
        })
        .then(device => {
        console.log(device);
        console.log('> Name:      ' + device.name);
        console.log('> Id:        ' + device.id);
        console.log('> Connected: ' + device.gatt.connected);
        return device.gatt.connect();
        })
        .then(server => {
            console.log('Getting Service...');
            console.log(server);
            return server.getPrimaryServices();
        })
        .then(services => {
            console.log('Getting Characteristics...');
            console.log(services);
            
            services[0].getCharacteristics().then( characteristics => {
                console.log(characteristics);
            })
            services[1].getCharacteristics().then( characteristics => {
                console.log(characteristics);
            })
            services[2].getCharacteristics().then( characteristics => {
                console.log(characteristics);
            })
            services[3].getCharacteristics().then( characteristics => {
                console.log(characteristics);
            })

        })
        .catch(error => {
        console.log("D'oh! " + error);
        });
    }
    else 
    {
        console.log('Web Bluetooth API is not available in this browser!')
    }
}