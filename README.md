# Nano-Master-Slave-IMU
Multiple Arduino Nanos taking commands from a phone to log IMU sensor data

### Project Description
Two Arduino Nano 33 BLE Sense Rev2 microcontrollers are connected together through a BLE master and slave protocol. The master MCU is also connected to a phone and recieves commands to then relay to the slave MCU. The app used is "nRF Connect".

The slave MCU is also conencted to an SD Card module and an Adafruit IMU sensor module. This project is used to log IMU sensor data to train a machine learning model to detect the stage in motion from IMU data during ankle and foot gait phase change. This project is used to allow for a more efficient and seamless testing period. The phone is used to send commands to start and end the project. The commands received also specify the action being done by the test subject "Running", "Walking", ...etc. 
