// =================================
// Arduino MKR wifi 1010
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Jan. 2021
// =================================

// ---------------------------
//     I2C communication
// ---------------------------
#include<Wire.h>

// ---------------------------
//   Adafruit BME280 sensor
// ---------------------------
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;;

// current barrometric pressure of your city
#define SEALEVELPRESSURE_HPA (1023) // Aachen: 1023

// ---------------------------
//        Wifi Module
// ---------------------------
#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

// WiFi ID & PW -> arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiServer server(80);            //server socket


// ---------------------------
//         Variables
// ---------------------------
// sensors
float pressure = 33.3333;
float temperature = 33.3333;
float altimeter = 33.3333;
float humidity = 33.3333;
float ds00_temperature = 33.3333;
float ds01_temperature = 33.3333;
float ds02_temperature = 33.3333;
// Variables for Data-Logging and Controlling
// Sending to Slave
char heater_onOff = '0';      // default setting after turning on device
float T_set = 30;          // default setting after turning on device
float hum_room;       
float T_room;

// Receiving from Slave
float T00_is_chair;   // test temperature
float T01_is_chair;
float T02_is_chair;
// bool is_sitting;  // evtl. rauslassen


// the setup function runs once when you press reset or power the board
void setup() {
  // start serial port 
  Serial.begin(9600); 

  // Begins I2C communication at pin (A4,A5)
  Wire.begin();
  
  // Error message
  //if (!bme.begin(0x76)) {
  //  Serial.println("Could not find a valid BME280 sensor, check wiring!");
  //  while (1);
  //}

  // setup IP address - 192.168.0.99
  WiFi.config(IPAddress(192, 168, 0, 99));

  enable_WiFi();
  connect_WiFi();
  
  server.begin();
  
  // you're connected now, so print out the WiFi data:
  Serial.println("You're connected to the network");
  Serial.println("----------------------------------------");
  printWifiStatus();
  Serial.println("----------------------------------------");

  // serial plotter labels
  Serial.println("Temperature[degC],Pressure[hPa],Approx.Altitude[m],Humidity[%],T00_is_chair[degC],T01_is_chair[degC],T02_is_chair[degC]");
  
}

// the loop function runs over and over again forever
void loop() {
  // Read values from bme 280 sensor
  //pressure = bme.readPressure() / 100; // Pa -> hPa
  //temperature = bme.readTemperature(); // degC
  //humidity = bme.readHumidity(); // %
  //altimeter = bme.readAltitude(SEALEVELPRESSURE_HPA); // m

  // start I2C communication between Master and Slave
  Master_communication();

  // bme280 sensor
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.print(altimeter);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(T00_is_chair);
  Serial.print(",");
  Serial.print(T01_is_chair);
  Serial.print(",");
  Serial.println(T02_is_chair);

  // use "client" to check if the server is available
  WiFiClient client = server.available();
  // If it is, execute the printWEB() function.
  if (client) {
    printWEB();
  }

  
  
  delay(5000); // update every 5 sec
}

// ---------------------------
//         Functions
// ---------------------------
void Master_communication(void) {
  
  //DateTime now = RTC.now();

  String T_set_Str = String(T_set, DEC).substring(0,2);
  String hum_room_Str = String(humidity, DEC).substring(0,5);
  String T_room_Str = String(temperature, DEC).substring(0,5);
  //String time_stamp_Str = String(time_stamp, DEC).substring(0,15);

  // WRITE / SEND
  //Serial.println("Master Communication Started");
  Wire.beginTransmission(8);                  // transmit to device #8
  Wire.write(heater_onOff);                     // 1 byte
  Serial.println(heater_onOff);
  //Serial.println(heater_onOff);
  Wire.write(T_set_Str.c_str());                // 2 bytes (z.B. 20)
  Serial.println(T_set_Str);
  //Serial.println(T_set_Str.c_str());
  Wire.write(hum_room_Str.c_str());             // 5 bytes
  Serial.println(hum_room_Str);
  //Serial.println(hum_room_Str.c_str());
  Wire.write(T_room_Str.c_str());               // 5 bytes
  Serial.println(T_room_Str);
  //Serial.println(T_room_Str.c_str());
  //Wire.write(time_stamp);                       // 15 bytes (DDMMYY HH:MM:SS)
  Wire.endTransmission();                     // stop transmitting


  // READ / REQUEST
  char data_received[13];
  int r = 0;
  Wire.requestFrom(8,12);
  while (Wire.available()) {
    data_received[r] = Wire.read();
    r++;
  }
  String data = String(data_received);
  
  T00_is_chair = data.substring(0,4).toFloat();
  T01_is_chair = data.substring(4,8).toFloat();
  T02_is_chair = data.substring(8,12).toFloat();

  Serial.print("data received ");
  Serial.println(data_received);
  
  //timerI2C = now.second();
}

void printWifiStatus() {
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

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
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
}

void printWEB() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          // output
          client.print("Temperature[degC]: ");
          client.print(temperature);
          client.print("<br />");
          client.print("Humidity[%]: ");
          client.print(humidity);
          client.print("<br />");
          client.print("Pressure[hPa]: ");
          client.print(pressure);
          client.print("<br />");
          client.print("Approx.Altitude[m]: ");
          client.print(altimeter);
          client.print("<br />");
          client.print("heater_onOff: ");
          client.print(heater_onOff);
          client.print("<br />");
          client.print("T_set: ");
          client.print(T_set);
          client.print("<br />");
          client.print("T00_is_chair: ");
          client.print(T00_is_chair);
          client.print("<br />");
          client.print("T01_is_chair: ");
          client.print(T01_is_chair);
          client.print("<br />");
          client.print("T02_is_chair: ");
          client.print(T02_is_chair);
          client.print("<br />");
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

}
