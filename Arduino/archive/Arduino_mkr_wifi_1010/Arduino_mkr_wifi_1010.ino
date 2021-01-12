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
// LED
int ledPin = 6;

// bme280 sensor
float pressure;
float temperature;
float altimeter;
float humidity;

// Dieser ky018-Sensor hat nichts mit unserem Projekt zu tun, nur zum Testzweck
int ky018_sensorPin = 2; //define analog pin 2
int ohm_value = 0;
int lux_value = 0;


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

  // check bme280 sensor
  check_bme280();
   
  
}

// the loop function runs over and over again forever
void loop() {
  // Read values from bme 280 sensor
  bool ok = bme.measure();
  // If read values from bme280 failed -> Error
  if (!ok) {
    Serial.println("Measuring failed.");
    while (1);
  } 
  
  // Dieser ky018-Sensor hat nichts mit unserem Projekt zu tun, nur zum Testzweck
  // KY-018 PHOTORESISTOR MODULE
  ohm_value = analogRead(ky018_sensorPin); // Ohm
  // y=4747*e^(-0.0164x) ; function, Ohm (x) -> Lux (y), according to https://docs.google.com/spreadsheets/d/116wSlU34jTDTN5_HFJ2kBTdmWWZi-rcdivWq9slRP4A/edit?usp=sharing
  lux_value = 4092 * exp(-0.00977 * ohm_value); // lux
  
  pressure = bme.getPressure() / 100.0; // Pa -> hPa
  temperature = bme.getTemperature() / 100.0; // degC
  humidity = bme.getHumidity() / 1024.0; // %
  
  // data output via Serial Monitor
  // bme280 sensor
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.println(lux_value);

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

void check_bme280() {
  // Check if bme280 sensor at 0x76 can be found, if not -> Error message 
  if (!bme.beginI2C(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
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
          client.println("<br />");
          client.print("Pressure[hPa]: ");
          client.print(pressure);
          client.println("<br />");
          client.print("Humidity[%]: ");
          client.print(humidity);
          client.println("<br />");
          
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
