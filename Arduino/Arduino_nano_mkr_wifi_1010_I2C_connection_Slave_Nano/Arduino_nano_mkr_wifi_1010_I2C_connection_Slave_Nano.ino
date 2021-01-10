// =================================
// Arduino Nano
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Jan. 2021
// =================================

// ---------------------------
//     I2C communication
// ---------------------------
#include<Wire.h>

// ---------------------------
//  Dallas Temperature sensor
// ---------------------------
#include <OneWire.h> 
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 2 

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);




// ---------------------------
//         Variables
// ---------------------------
// mosfet
int mosfetPin = 6;
int mosfetOn = 200;
int mosfetOff = 0;
// sensors
float ds00_temperature = 33.3333;
float ds01_temperature = 33.3333;
float ds02_temperature = 33.3333;
int deviceCount = 0;
// settings
float T_set = 25;
// receiving
bool heater_onOff;
float hum_room;
//bool is_sitting;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(mosfetPin, OUTPUT);
  
  // start serial port 
  Serial.begin(9600);

  // specify the slave
  Wire.begin(8);                // join i2c bus with address #8
  // call the function when Slave receives value from master and when Master request value from Slave
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  
  // Start up the library 
  sensors.begin(); 

  // locate DS18B20 devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" DS18B20 sensors.");
  Serial.println("");

    // serial plotter labels
  Serial.println("Temperature[degC]_Dallas_0,Temperature[degC]_Dallas_1,Temperature[degC]_Dallas_2");
}

// the loop function runs over and over again forever
void loop() {

  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  ds00_temperature = sensors.getTempCByIndex(0);
  ds01_temperature = sensors.getTempCByIndex(1);
  ds02_temperature = sensors.getTempCByIndex(2);

  // Mosfet controll
  if (ds00_temperature < T_set && ds01_temperature < T_set && ds02_temperature < T_set){
    analogWrite(mosfetPin, mosfetOn);
    }
  else {
    analogWrite(mosfetPin, mosfetOff);
    }
  

  // print the temperature in Celsius by using Dallas temperature sensor
  Serial.print(ds00_temperature);
  Serial.print(",");
  Serial.print(ds01_temperature);
  Serial.print(",");
  Serial.println(ds02_temperature);

  delay(5000); // update every 5 sec
}

// ---------------------------
//         Functions
// ---------------------------
void requestEvent() {
  String T00_is_chair_Str = String(ds00_temperature, DEC).substring(0,4);
  String T01_is_chair_Str = String(ds01_temperature, DEC).substring(0,4);
  String T02_is_chair_Str = String(ds02_temperature, DEC).substring(0,4);
  Wire.write(T00_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T00_chair = " + String(ds00_temperature));
  Wire.write(T01_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T01_chair = " + String(ds01_temperature));
  Wire.write(T02_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T02_chair = " + String(ds02_temperature));           
                                             
}

void receiveEvent(int howMany) {

  char data_received[31];
  int i=0;

  Serial.print("Wire.available() = ");
  Serial.println(Wire.available());
  
  while (0 < Wire.available()) {            // loop through all bytes
    data_received[i] = Wire.read();         // receive byte as a character
    i++;
}

  // Handling the received data
  String data = (String)data_received;

  Serial.println("Data_received = ");
  Serial.println(data_received);
  
  if (data.substring(0,1) == "0") {bool heater_onOff = false;}
  else if (data.substring(0,1) == "1") {bool heater_onOff = true;}
  else {heater_onOff = false; Serial.println("heater_onOff ERROR: Wrong format");}
  
  T_set = data.substring(1,3).toFloat();
  hum_room = data.substring(3,5).toFloat();

  Serial.println(heater_onOff);
  Serial.println(T_set);
  Serial.println(hum_room);
  
}
