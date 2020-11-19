// =================================
// Arduino MKR wifi 1010
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Nov. 2020
// =================================

// ---------------------------
// BME280 sensor Bosch library
// ---------------------------
#include <Bme280BoschWrapper.h>

Bme280BoschWrapper bme(true);

// current barrometric pressure of your city
#define SEALEVELPRESSURE_HPA (1023) // Aachen: 1023


// ---------------------------
//        Wifi Module
// ---------------------------
#include <WiFiNINA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


// ---------------------------
//         Variables
// ---------------------------
int ledPin = 6;
float pressure;
float temperature;
float altimeter;
float humidity;


// ---------------------------
//       Setup & Loop
// ---------------------------

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);
  
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
    Serial.println("connecting...");
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");
  
  // Error message
  if (!bme.beginI2C(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  } 
  
  // serial plotter labels
  Serial.println("Temperature[degC],Pressure[hPa],Humidity[%]");
}

// the loop function runs over and over again forever
void loop() {
  fadeOn(1000,5);
  fadeOff(1000,5);
  
  // Read values from bme 280 sensor
  bool ok = bme.measure();

  if (!ok) {
    Serial.println("Measuring failed.");
    while (1);
  } 

  pressure = bme.getPressure() / 100.0; // Pa -> hPa
  temperature = bme.getTemperature() / 100.0; // degC
  humidity = bme.getHumidity() / 1024.0; // %

  // bme280 sensor
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.println(humidity);
  
  delay(3000); // update every 3 sec
}



// ---------------------------
//         Functions
// ---------------------------

void fadeOn(unsigned int time,int increament){
  for (byte value = 0; value < 255; value+=increament){
    analogWrite(ledPin, value);
    delay(time/(255/5));
    }
  
}

void fadeOff(unsigned int time,int increament){
  for (byte value = 255; value > 0; value-=increament){
    analogWrite(ledPin, value);
    delay(time/(255/5));
    }
}

void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

}
