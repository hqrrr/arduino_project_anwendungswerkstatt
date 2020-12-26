// =================================
// Arduino Nano
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Nov. 2020
// =================================


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
//   Adafruit BME280 sensor
// ---------------------------
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

// current barrometric pressure of your city
#define SEALEVELPRESSURE_HPA (1023) // Aachen: 1023

// ---------------------------
//         Variables
// ---------------------------
// mosfet
int mosfetPin = 6;
int mosfetOn = 200;
int mosfetOff = 0;
// sensors
float ds00_temperature;
float ds01_temperature;
float ds02_temperature;
float pressure;
float temperature;
float altimeter;
float humidity;
int deviceCount = 0;
// other
float t;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(mosfetPin, OUTPUT);
  
  // start serial port 
  Serial.begin(9600); 
  // Start up the library 
  sensors.begin(); 

  // Error message
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // locate DS18B20 devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" DS18B20 sensors.");
  Serial.println("");
  
  // serial plotter labels
  //Serial.println("Temperature[degC]_Dallas_0,Temperature[degC]_Dallas_1,Temperature[degC]_Dallas_2,Temperature[degC]_bme280,Pressure[hPa],Approx.Altitude[m],Humidity[%]");
  Serial.println("Temperature[degC]_Dallas_0,Temperature[degC]_Dallas_1,Temperature[degC]_Dallas_2,Temperature[degC]_bme280,Humidity[%]");
}

// the loop function runs over and over again forever
void loop() {
  //fadeOn(1000,5);
  //fadeOff(1000,5);

  // Read values from bme 280 sensor
  pressure = bme.readPressure() / 100; // Pa -> hPa
  temperature = bme.readTemperature(); // degC
  humidity = bme.readHumidity(); // %
  altimeter = bme.readAltitude(SEALEVELPRESSURE_HPA); // m
  
  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  ds00_temperature = sensors.getTempCByIndex(0);
  ds01_temperature = sensors.getTempCByIndex(1);
  ds02_temperature = sensors.getTempCByIndex(2);

  // print the temperature in Celsius by using Dallas temperature sensor
  Serial.print(ds00_temperature);
  Serial.print(",");
  Serial.print(ds01_temperature);
  Serial.print(",");
  Serial.print(ds02_temperature);
  Serial.print(",");
  
  // bme280 sensor
  Serial.print(temperature);
  Serial.print(",");
  //Serial.print(pressure);
  //Serial.print(",");
  //Serial.print(altimeter);
  //Serial.print(",");
  Serial.println(humidity);

  if (ds00_temperature < 25 && ds01_temperature < 25 && ds02_temperature < 25 && temperature < 25){
    analogWrite(mosfetPin, mosfetOn);
    }
  else {
    analogWrite(mosfetPin, mosfetOff);
    }
  
  delay(3000); // update every 3 sec
}
