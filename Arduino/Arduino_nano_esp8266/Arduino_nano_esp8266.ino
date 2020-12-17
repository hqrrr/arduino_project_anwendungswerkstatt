// =================================
// Arduino Nano
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Nov. 2020
// =================================

#include <SoftwareSerial.h>

// use D3 as RX and D2 as TX
SoftwareSerial softSerial(3, 2); // RX, TX

// ---------------------------
//        Wifi Module
// ---------------------------
#include "arduino_esp8266_secrets.h"
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)




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
float pressure;
float temperature;
float altimeter;
float humidity;
int deviceCount = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);
  
  // Error message
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // serial plotter labels
  Serial.println("Temperature[degC],Pressure[hPa],Approx.Altitude[m],Humidity[%]");
}

// the loop function runs over and over again forever
void loop() {
  
  // Read values from bme 280 sensor
  pressure = bme.readPressure() / 100; // Pa -> hPa
  temperature = bme.readTemperature(); // degC
  humidity = bme.readHumidity(); // %
  altimeter = bme.readAltitude(SEALEVELPRESSURE_HPA); // m
  
  
  // bme280 sensor
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.print(altimeter);
  Serial.print(",");
  Serial.println(humidity);
  
  delay(3000); // update every 3 sec
}
