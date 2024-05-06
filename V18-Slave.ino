//Slave/Peripheral code
// Geen: Connected to master
// Orange: Code is running
// Peripherals must connect first and then phone can connect
// 

 // MOSI - pin 11
 // MISO - pin 12
 // CLK - pin 13
 // CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

#include <ArduinoBLE.h> //Bluetooth library
#include "Arduino_BMI270_BMM150.h" // IMU Library
#include "SdFat.h"

//values
#define OFFVAL 0
#define WALKINGVAL 1
#define RUNNINGVAL 2

//file name of file
char file_name[80];

SdFs sd;
FsFile myFile;

const int chipSelect = 4;


struct packet {
  unsigned long int t;
  int16_t a1[3];
  int16_t w1[3];
} data; // 22 bytes

//Bluetooth stuff
BLEService connectionService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energ connection Service

// Bluetooth® Low Energy Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

bool sensorRunning = false;

int incomingValue = 0;

int16_t xAcc, yAcc, zAcc; // accelerometer x, y, z
int16_t xGyr, yGyr, zGyr; // gyroscope x, y, z


void setup() {
  Serial.begin(9600);
  while (!Serial); //remove later

  //initialize LED
  initializeLED();     

  //check everything
  checkSensors();

  // Start Slave Connection
  startSlaveConnection(connectionService, switchCharacteristic);
}


void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to Slave/Peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address()); // print the central/master's MAC address:

      // while the central is still connected to peripheral:
    while (central.connected()) {
      //Led is green when connected
      digitalWrite(LEDR, HIGH);       
      digitalWrite(LEDG, LOW);        
      digitalWrite(LEDB, HIGH);  
    
      
      if (switchCharacteristic.written()) {
        //close file and save data 
        if (myFile.isOpen()) {myFile.close();}
        Serial.println("File Closed");
        
        //Receive and analyze incoming data
        incomingValue = switchCharacteristic.value();
        sensorRunning = analyzeBLE(incomingValue);
        
        if(sensorRunning){
          Serial.println("Opening file...");
          
          // create new named file
          //walking
          if(incomingValue == 1){
            int file_name_count = 0;
            sprintf(file_name, "walking%03d", file_name_count);
            while (sd.exists(file_name)){sprintf(file_name, "walking%03d", ++file_name_count);}
            Serial.print("initialization succeeded! \nSaving to ");
            Serial.println(file_name);
          }

          if(incomingValue == 2){
            int file_name_count = 0;
            sprintf(file_name, "running%03d", file_name_count);
            while (sd.exists(file_name)){sprintf(file_name, "running%03d", ++file_name_count);}
            Serial.print("initialization succeeded! \nSaving to ");
            Serial.println(file_name);
          }
         

          // open the file.
          if (!myFile.open(file_name, O_WRITE | O_CREAT | O_AT_END)) {
            Serial.println("Error opening file for writing");
            return;
          } 

          myFile.seekEnd();

          Serial.println("file created");
        }
      }

      if(sensorRunning) {startSensors();} 
    }

    // when the central disconnects, print it out and turn Led red:
    digitalWrite(LEDR, LOW);       
    digitalWrite(LEDG, HIGH);        
    digitalWrite(LEDB, HIGH);        

    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}




//start phone bluetooth connection
void startSlaveConnection(BLEService connectionService, BLEByteCharacteristic switchCharacteristic){
  BLE.setLocalName("SENSOR");
  BLE.setAdvertisedService(connectionService);

  // add the characteristic to the service
  connectionService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(connectionService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE - Peripheral/Slave");
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


void checkSensors(void) {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  if (!sd.begin(chipSelect)) {
    Serial.println("initialization failed!");    
      digitalWrite(LEDR, HIGH);       
      digitalWrite(LEDG, HIGH);        
      digitalWrite(LEDB, LOW);  
    while (1);
  }
}


void startSensors(void){


  IMU.readAcceleration(xAcc, yAcc, zAcc); 
  IMU.readGyroscope(xGyr, yGyr, zGyr); 

 // displaySensors(xAcc, yAcc, zAcc, xGyr, yGyr, zGyr);
  saveSD(xAcc, yAcc, zAcc, xGyr, yGyr, zGyr);
} 


bool analyzeBLE(int incomingValue){
  
  switch(incomingValue){
    case WALKINGVAL:
      sensorRunning = true;
    break;
    case RUNNINGVAL:
      sensorRunning = true;
    break;
    case OFFVAL:
      sensorRunning = false;
    break;
    default:
      sensorRunning = false;
    break;
  }

 return sensorRunning;
}

void saveSD(int16_t xAcc, int16_t yAcc, int16_t zAcc, int16_t xGyr, int16_t yGyr, int16_t zGyr){

  //set data to packet
  data.t = millis();
  data.a1[0] = xAcc;
  data.a1[1] = yAcc;
  data.a1[2] = zAcc;
  data.w1[0] = xGyr;
  data.w1[1] = yGyr;
  data.w1[2] = zGyr;

  //save data
  if (myFile.write((const uint8_t *)&data, sizeof(data)) != sizeof(data)) {Serial.println("Error writing to file");} 
}





// Process and print sensor values
void displaySensors(int16_t xAcc, int16_t yAcc, int16_t zAcc, int16_t xGyr, int16_t yGyr, int16_t zGyr){
  Serial.print("Accelerometer Values: ");
  Serial.print(xAcc);
  Serial.print(", ");
  Serial.print(yAcc);
  Serial.print(", ");
  Serial.println(zAcc);

  Serial.print("Gyroscope Values: ");
  Serial.print(xGyr);
  Serial.print(", ");
  Serial.print(yGyr);
  Serial.print(", ");
  Serial.println(zGyr);


}
