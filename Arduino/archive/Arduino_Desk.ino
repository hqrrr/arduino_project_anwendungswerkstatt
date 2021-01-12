#include <Wire.h>                  //Include I2C-Library
#include <Adafruit_GFX.h>          //Include Libraries
#include <Adafruit_SSD1306.h>      //for OLED Display
#include <DS3231.h>                //for Real Time Clock
#include <DHT.h>                   //for T & Humidity Sensor

//Declaration Temp/Hum-Sensor
#define DHTPIN 7                    // sensor pin number 7 (Digital Pin)
#define DHTTYPE DHT11               // sensor type: DHT 11 (DHT22 also possible)

// Declaration for SSD1306 Display (I2C)
#define SCREEN_WIDTH 128           //pixels display width
#define SCREEN_HEIGHT 64           //pixels display height
#define OLED_RESET -1              
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Declaration Real Time Clock
RTClib RTC;                        // Initialize real time clock

//Declaration DHT-Sensor
DHT dht(DHTPIN, DHTTYPE);          // Initialize DHT sensor for normal 16mhz Arduino

// Variables for Motion Sensor
const int PIN_TO_SENSOR = 4;   // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin

// Variables for Data-Logging and Controlling
// Sending to Slave
char heater_onOff = '0';      // default setting after turning on device
float T_set = 30;          // default setting after turning on device
float hum_room;       
float T_room;
//char time_stamp[16] = "010121 13:10:14"; // Evtl auslassen um Programmspeicher zu sparen

// Receiving from Slave
float T_is_chair;   // test temperature
// bool is_sitting;  // evtl. rauslassen

// Global Variables for practical reasons
int timerI2C=0; //to store the last second Information has been sent to Slave

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  //Serial.println("Setup Complete");

}


void loop() {
  // put your main code here, to run repeatedly:

HomeScreen();
ControlScreen();
SenseScreen();
PomodoroScreen();


}



// -----------------------functions to call-------------------------- 


void Master_communication(void) {
  
  DateTime now = RTC.now();

  // Get variables
  hum_room = dht.readHumidity();
  T_room = dht.readTemperature();

  String T_set_Str = String(T_set, DEC).substring(0,2);
  String hum_room_Str = String(hum_room, DEC).substring(0,5);
  String T_room_Str = String(T_room, DEC).substring(0,5);
  //String time_stamp_Str = String(time_stamp, DEC).substring(0,15);

  if ((abs((now.second() - timerI2C)) > 2))  {

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
    char data_received[5];
    int r = 0;
    Wire.requestFrom(8,4);
    while (Wire.available()) {
      data_received[r] = Wire.read();
      r++;
    }
    String data = String(data_received);
    T_is_chair = data.toFloat();

    Serial.print("T_chair ");
    Serial.println(data_received);
    
    timerI2C = now.second();

    
    
  }
}

void HomeScreen(void) {

  delay(150);
  
  for(;;) {

  DateTime now = RTC.now();

    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);         // Draw white text
    display.setCursor(0,0);                     // Start at top-left corner
    display.setTextSize(1);
    display.print("Home");
    
    // Show date
    display.setTextSize(2);    
    display.setCursor(0,19);
    display.print(now.day(), DEC);        
    display.print('.');
    display.print(now.month(), DEC);
    display.print('.');
    display.print(now.year(), DEC);

    // Show time
    display.setTextSize(3);
    display.setCursor(0,40);
    display.print(now.hour(), DEC);
    display.print(':');
    display.print(now.minute(), DEC);
    
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
    display.print(String(T_is_chair).substring(0,2));
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

void PomodoroScreen(void) {

  Master_communication();
  
}


void eventQuery(void) {

  // if(digitalRead(52) == LOW) break;
  //PROBLEM: Rückgabetyp ist void, es müsste aber "break" zurückgegeben werden
  
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
