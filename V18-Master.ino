// Master/Central Code
//Red: not connected
// Green: Connected to Slave
// Orange: Connected to phone
// Peripherals must connect first and then phone can connect
// Off: 0
// Walking: 1
// Running: 2

#include <ArduinoBLE.h>
#include <SPI.h>
#include <SD.h>

#define offVal 0
#define walkingVal 1
#define runningVal 2

BLEService connectionService("180A"); // BLE advertized/connected Service

// BLE Switch Characteristic - custom 128-bit UUID, read and writable by central(what service can do)
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  while (!Serial); //Remove later

  initializeLED();

  Serial.println("BLE - Central/Master");

  // initialize the Bluetooth Low Energy hardware
  BLE.begin();

  //Start phone bluetooth connection
  startPhoneConnection(connectionService, switchCharacteristic);

  // start scanning for peripherals
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "SENSOR") { return; }

    BLE.stopScan(); // stop scanning

    controlled(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
  }
}




void controlled(BLEDevice peripheral) {

   // connect to the peripheral
  Serial.print("Connecting... ");  
  if (peripheral.connect()) { Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.print("Discovering attributes... ");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }
  
  // retrieve the characteristic
  BLECharacteristic writeCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  // while the peripheral is connected
  while (peripheral.connected()) {
    BLEDevice central = BLE.central();

    //Led is green when connected
      digitalWrite(LEDR, HIGH);       
      digitalWrite(LEDG, LOW);        
      digitalWrite(LEDB, HIGH);  

    // check if there is any data available from phone
    if (central){
      Serial.println("Connected To Phone!");
      while (central.connected()) {

        digitalWrite(LED_BUILTIN, HIGH);

        if (switchCharacteristic.written()) { 
          // read the incoming data from phone
          int incomingValue = switchCharacteristic.value();

          switch(incomingValue){
            case walkingVal:
              Serial.println("Walking...");
              writeCharacteristic.writeValue((byte)0x01); //send 1 to slave
            break;
            case runningVal:
              Serial.println("Running...");
              writeCharacteristic.writeValue((byte)0x02); //send 1 to slave
            break;
            case offVal:
              Serial.println("Off");
              writeCharacteristic.writeValue((byte)0x00); //send 0 to slave
            break;
            default:
              Serial.println("Off");
              writeCharacteristic.writeValue((byte)0x00); //send 0 to slave
            break;
          }
        }
      }

      digitalWrite(LED_BUILTIN, LOW);

    }
  }

  digitalWrite(LEDR, LOW);       
  digitalWrite(LEDG, HIGH);        
  digitalWrite(LEDB, HIGH); 
  Serial.println("Peripheral disconnected");
}

//setup LEDs and set to red
void initializeLED(void){
  // set pins to output mode
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  //Led is red until connected
  digitalWrite(LEDR, LOW);       
  digitalWrite(LEDG, HIGH);        
  digitalWrite(LEDB, HIGH);   
}


//start phone bluetooth connection
void startPhoneConnection(BLEService connectionService, BLEByteCharacteristic switchCharacteristic){
  // set advertised local name and service UUID:
  BLE.setLocalName("Master");
  BLE.setAdvertisedService(connectionService);

  // add the characteristic to the service
  connectionService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(connectionService);

  // set the initial value for the characteristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();
  Serial.println("BLE Connection to Phone Started");
}

