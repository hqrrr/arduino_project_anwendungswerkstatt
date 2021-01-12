// =================================
// Arduino MKR wifi 1010
// Arduino 1.8.13
// Author: hqrrr
// Date of last update: Jan. 2021
// =================================

#include<Wire.h>                //for I2C communication

//#include <DS3231.h>                //for Real Time Clock
#include <RTCZero.h>
// Declaration Real Time Clock
//RTClib RTC;                        // Initialize real time clock
RTCZero rtc;

const int GMT = 2; //change this to adapt it to your time zone

// ---------------------------
//   SSD1306 Display (I2C)
// ---------------------------
#include <Adafruit_GFX.h>          //Include Libraries
#include <Adafruit_SSD1306.h>      //for OLED Display
// Declaration for SSD1306 Display (I2C)
#define SCREEN_WIDTH 128           //pixels display width
#define SCREEN_HEIGHT 64           //pixels display height
#define OLED_RESET -1              
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------------------
//      DHT 11 sensor
// ---------------------------
#include <DHT.h>
//Declaration Temp/Hum-Sensor
#define DHTPIN 7                    // sensor pin number 7 (Digital Pin)
#define DHTTYPE DHT11               // sensor type: DHT 11 (DHT22 also possible)

//Declaration DHT-Sensor
DHT dht(DHTPIN, DHTTYPE);          // Initialize DHT sensor for normal 16mhz Arduino

// ---------------------------
//    PIR sensor hc-sr-501
// ---------------------------
#include<Wire.h>

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

// Motion Sensor
const int PIN_TO_SENSOR = 4;   // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

// Variables for Data-Logging and Controlling
// Sending to Slave
char heater_onOff = '0';      // default setting after turning on device, 0: on, 1: off
float T_set = 30;          // default setting after turning on device
float hum_room;       
float T_room;

// Receiving from Slave
float T00_is_chair;   // temperature
float T01_is_chair;
float T02_is_chair;
int is_sitting = 1;   // 0: true, 1:false, by default = 1
float PID_Output;
// bool is_sitting;  // evtl. rauslassen

// Global Variables for practical reasons
int timerI2C=0; //to store the last second Information has been sent to Slave

// the setup function runs once when you press reset or power the board
void setup() {
  // start serial port 
  Serial.begin(9600); 

  // Begins I2C communication at pin (A4,A5)
  Wire.begin();

  // start read data from dht sensor
  dht.begin();

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

  // real time clock
  rtc.begin();
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 15; // try maxTries times to get real time from server.
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
    // if NTP unreachable, try again after 2 sec.
    if(epoch == 0){
      Serial.print("WiFi.getTime try again...");
      Serial.print(numberOfTries);
      Serial.print(" / ");
      Serial.println(maxTries);
      delay(2000);
    };
  }
  while ((epoch == 0) && (numberOfTries < maxTries));
  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    //while (1);
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    Serial.println();
  }

  // button
  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  
  // Error message
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // serial plotter labels
  Serial.println("Temperature[degC],Humidity[%],T00_is_chair[degC],T01_is_chair[degC],T02_is_chair[degC]");
  
}

// the loop function runs over and over again forever
void loop() {

  HomeScreen();
  ControlScreen();
  SenseScreen();
  
}

// ---------------------------
//         Functions
// ---------------------------
void Master_communication(void) {
  
  //DateTime now = RTC.now();
  // Read values from dht 11 sensor
  T_room = dht.readTemperature(); // degC
  hum_room = dht.readHumidity(); // %
  
  String T_set_Str = String(T_set, DEC).substring(0,2);
  String hum_room_Str = String(hum_room, DEC).substring(0,5);
  String T_room_Str = String(T_room, DEC).substring(0,5);
  //String time_stamp_Str = String(time_stamp, DEC).substring(0,15);

  // If it is more than 5 seconds away from the last communication -> start new communication and print Web
  if ((abs((rtc.getSeconds() - timerI2C)) > 5))  {
    // WRITE / SEND
    //Serial.println("Master Communication Started");
    Wire.beginTransmission(8);                  // transmit to device #8
    Wire.write(heater_onOff);                     // 1 byte
    Serial.print("heater_onOff: ");
    Serial.println(heater_onOff);
    //Serial.println(heater_onOff);
    Wire.write(T_set_Str.c_str());                // 2 bytes (z.B. 20)
    Serial.print("T_set_Str: ");
    Serial.println(T_set_Str);
    //Serial.println(T_set_Str.c_str());
    Wire.write(hum_room_Str.c_str());             // 5 bytes
    Serial.print("hum_room_Str: ");
    Serial.println(hum_room_Str);
    //Serial.println(hum_room_Str.c_str());
    Wire.write(T_room_Str.c_str());               // 5 bytes
    Serial.print("T_room_Str: ");
    Serial.println(T_room_Str);
    //Serial.println(T_room_Str.c_str());
    //Wire.write(time_stamp);                       // 15 bytes (DDMMYY HH:MM:SS)
    Wire.endTransmission();                     // stop transmitting
  
  
    // READ / REQUEST
    char data_received[31];
    int r = 0;
    Wire.requestFrom(8,18);
    while (Wire.available()) {
      data_received[r] = Wire.read();
      r++;
    }
    String data = String(data_received);
    
    T00_is_chair = data.substring(0,4).toFloat();
    T01_is_chair = data.substring(4,8).toFloat();
    T02_is_chair = data.substring(8,12).toFloat();
    is_sitting = data.substring(12,13).toInt();
    PID_Output = data.substring(13,17).toFloat();
  
    Serial.print("data received ");
    Serial.println(data_received);
    Serial.print("T00_is_chair: ");
    Serial.println(T00_is_chair);
    Serial.print("T01_is_chair: ");
    Serial.println(T01_is_chair);
    Serial.print("T02_is_chair: ");
    Serial.println(T02_is_chair);
    Serial.print("is_sitting[0:true,1:false]: ");
    Serial.println(is_sitting);
    Serial.print("PID_Output: ");
    Serial.println(PID_Output);

    // use "client" to check if the server is available
    WiFiClient client = server.available();
    // If it is, execute the printWEB() function.
    if (client) {
      printWEB();
    }
    
    timerI2C = rtc.getSeconds();
  }  
}

void HomeScreen(void) {

  delay(150);
  
  for(;;) {

    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);         // Draw white text
    display.setCursor(0,0);                     // Start at top-left corner
    display.setTextSize(1);
    display.print("Home");
    
    // Show date
    display.setTextSize(2);    
    display.setCursor(0,19);
    display.print(rtc.getDay(), DEC);        
    display.print('.');
    display.print(rtc.getMonth(), DEC);
    display.print('.');
    display.print(rtc.getYear(), DEC);

    // Show time
    display.setTextSize(3);
    display.setCursor(0,40);
    display.print(rtc.getHours() + GMT, DEC);
    display.print(':');
    display.print(rtc.getMinutes(), DEC);
    
    display.display();
    coronaAlarm();

    Master_communication();

    if(digitalRead(6) == LOW) break;
    
  }
 return; 
}

void ControlScreen(void) {
 delay(150);

while(1) {

    // Get T_set
    int potiValue = analogRead(A2);
    potiValue += 1;
    T_set = float(potiValue)/1024*41 + 10;

    //Turn heater_onOff
    if (digitalRead(5) == LOW) {
      if (heater_onOff == '0') heater_onOff='1'; 
      else heater_onOff = '0';
      delay(150);
    }

    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);        // Draw white text

    //Show header                       
    display.setTextSize(1);
    display.setCursor(0,0);                   
    display.print("Chair ");
    if (heater_onOff == '0') {display.print("On");}
    else if (heater_onOff == '1') {display.print("Off");}
    else {display.print("err");}
    
    // Show T_set
    display.setTextSize(2);
    display.setCursor(0,20); 
    display.print("Set ");
    display.print(String(T_set).substring(0,2));
    display.println(" C");

    // Show T_is_chair           
    display.print("Now ");
    display.print(String(T00_is_chair).substring(0,2));
    display.print(" C");

    display.display();

    coronaAlarm();

    Master_communication();
    
    if(digitalRead(6) == LOW) break;
    
  }
}

void SenseScreen(void) {

  delay(150);
  
  display.setTextSize(3);             // Normal 1:1 pixel scale
  for(;;) {

    // Read humidity (%) and temperature (°C)
    hum_room = dht.readHumidity();
    T_room = dht.readTemperature();

    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.setTextSize(1);
    display.print("Sensors");
        
    display.setCursor(0,18);
    display.setTextSize(2);
    display.print(T_room);
    display.print(" C");

    display.setCursor(0,40);
    display.setTextSize(2);
    display.print(hum_room);
    display.print(" %");
    
    display.display();

    coronaAlarm();

    Master_communication();

    if(digitalRead(6) == LOW) break;
    
  }
 return; 
}

void coronaAlarm(void) {
  
  pinStatePrevious = pinStateCurrent; // store old state
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // pin state change: LOW -> HIGH
    
    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);        // Draw white text
    
    display.setCursor(0,0); 
    display.setTextSize(1);
    display.print("Corona Warning");

    display.setCursor(0,22);        
    display.setTextSize(2);
    display.print("Abstand");
    
    display.setCursor(0,44);            
    display.print("halten!");

    display.display();

    Master_communication();
    delay(1000);
    Master_communication();
    delay(1000);
    
  }
  else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
    
  }
  return;
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
          client.print(T_room);
          client.print("<br />");
          client.print("Humidity[%]: ");
          client.print(hum_room);
          client.print("<br />");
          client.print("heater_onOff[0:on,1:off]: ");
          client.print(heater_onOff);
          client.print("<br />");
          client.print("T_set: ");
          client.print(T_set);
          client.print("<br />");
          client.print("is_sitting[0:true,1:false]: ");
          client.print(is_sitting);
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
          client.print("PID Output: ");
          client.print(PID_Output);
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
