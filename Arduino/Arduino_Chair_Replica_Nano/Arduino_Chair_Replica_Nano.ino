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
//     PID controller
// ---------------------------
/*
 * by Dorian
 * Concept:
 * 1) We read the temperature (Input) in the center of the chair
 * 2) We compare the Input with Setpoint and compute Output (heaterPID.Compute())
 * 3) We translate output into Power with upper and lower limit (0-100%)
 * 4) We translate Power into a pulse to control the heater
 *
 * Reglerparameter: Kp, Ki, Kd, delay zwischen Messungen
 * (je kürzer das delay, desto schneller wächst der Integratoranteil im Regler)
 */
#include <PID_v1.h>
// PI controller
double Setpoint;  // T_set
double Input;     // T_room
double Output;    // Power
double Kp = 1, Ki = 1, Kd = 0; // Controller Parameters
// Create PID instance
PID heaterPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// ---------------------------
//         Variables
// ---------------------------
// mosfet
int mosfetPin = 6;
int mosfetOn = 255; // 0 - 255
int mosfetOff = 0; // 0 - 255
int heatingOn_time;
int heatingOff_time;
// sensors
float T_chair1 = 33.3333; // DS18B20 temperature sensor on the chair
float T_chair2 = 33.3333; // DS18B20 temperature sensor on the chair
float T_chair3 = 33.3333; // DS18B20 temperature sensor on the chair
int deviceCount = 0; // count how many DS18B20 temperature sensor on the chair, by default = 0
int sitting_pressure; // pressure sensor on the chair
// settings
float T_set = 25;
// receiving
int heater_onOff;
float hum_room;
float T_room;
int is_sitting = 1; // 0:True, 1: False, by default = 1
int sit_p_setpoint = 50; // 0-1023, 0: no pressure, 1023: max. 10kg.



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(mosfetPin, OUTPUT);

  // use pin A0 for pressure sensor
  pinMode(A0, INPUT);
  
  // start serial port 
  Serial.begin(9600);

  // specify the slave
  Wire.begin(8);                // join i2c bus with address #8
  // call the function when Slave receives value from master and when Master request value from Slave
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  
  // Start up the library 
  sensors.begin(); 

  // PID controller
  Input = T_room;
  Setpoint = T_set;
  // Turn PID on
  heaterPID.SetMode(AUTOMATIC);
  // Adjust PID values
  heaterPID.SetTunings(Kp, Ki, Kd);

  // locate DS18B20 devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" DS18B20 sensors.");
  Serial.println("");

}

// the loop function runs over and over again forever
void loop() {

  // Send the command to get temperatures
  sensors.requestTemperatures(); 
  // read temperature
  T_chair1 = sensors.getTempCByIndex(0);
  T_chair2 = sensors.getTempCByIndex(1);
  T_chair3 = sensors.getTempCByIndex(2);
  
  // read sitting pressure value
  sitting_pressure = analogRead(A0);
  if (sitting_pressure > sit_p_setpoint){
    is_sitting = 0;
  }
  else {
    is_sitting = 1;
  }

  // print siiting pressure between 0 and 1023 and the temperature in Celsius
  Serial.println("Sitting pressure[0-1023],Temperature[degC]_Dallas_0,Temperature[degC]_Dallas_1,Temperature[degC]_Dallas_2");
  Serial.print(sitting_pressure);
  Serial.print(",");
  Serial.print(T_chair1);
  Serial.print(",");
  Serial.print(T_chair2);
  Serial.print(",");
  Serial.println(T_chair3);

  // PID controller incl. delay
  PID_control();

}

// ---------------------------
//         Functions
// ---------------------------
void PID_control() {
  // PID controller, read Input from sensor(received from Master), compute, write Output to actuator
  Input = (T_chair1 + T_chair2 + T_chair3) / 3;  // T mean readed from chair as input
  Setpoint = T_set;
  heaterPID.Compute();
  // Set Power (Output) within 0 and 100 %
  Output = constrain(Output, 0, 100);
  //analogWrite(3, Output);
  
  // Output for serial plotter
  Serial.print("PID Input: ");
  Serial.print(Input);
  Serial.print("   ");
  Serial.print("PID Output: ");       
  Serial.print(Output);
  Serial.print("   ");
  Serial.print("PID Setpoint: ");
  Serial.println(Setpoint);  

  // Prinzip PI Regler ->
  // - On        ______         _______           ________
  // - Off _____|      |_______|       |_________|        |______
  //            /------/               /---------/
  //         heatingOn_time          heatingOff_time
  // z.B. PID Output = 0 -> heatingOn_time = 0 und heatingOff_time = 5000 (Periode: 5 sec.)
  //      PID Output = 100 -> heatingOn_time = 5000 und heatingOff_time = 0.
  
  heatingOn_time = 5000 * Output / 100;
  heatingOff_time = 5000 - heatingOn_time;

  if (T_chair1 < T_set && T_chair2 < T_set && T_chair3 < T_set && is_sitting == 0 && heater_onOff == 0){
    
    analogWrite(mosfetPin, mosfetOn); // Heizmatte anschalten
    delay(heatingOn_time); // die Zeit ist abhängig von PID Output
    analogWrite(mosfetPin, mosfetOff);
    delay(heatingOff_time);
  }
  else {
    analogWrite(mosfetPin, mosfetOff);
    delay(5000);
  }
}


void requestEvent() {
  String T00_is_chair_Str = String(T_chair1, DEC).substring(0,4);
  String T01_is_chair_Str = String(T_chair2, DEC).substring(0,4);
  String T02_is_chair_Str = String(T_chair3, DEC).substring(0,4);
  String is_sitting_Str = String(is_sitting, DEC).substring(0,1);
  String PID_Output_Str = String(Output, DEC).substring(0,4);
  Wire.write(T00_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T00_chair = " + String(T_chair1));
  Wire.write(T01_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T01_chair = " + String(T_chair2));
  Wire.write(T02_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T02_chair = " + String(T_chair3));
  Wire.write(is_sitting_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("is_sitting = " + String(is_sitting));
  Wire.write(PID_Output_Str.c_str());        // respond with message of 1 bytes as expected by master
  Serial.println("PID Output = " + String(Output));
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
  
  if (data.substring(0,1) == "0") {heater_onOff = 0;}
  else if (data.substring(0,1) == "1") {heater_onOff = 1;}
  else {heater_onOff = 1; Serial.println("heater_onOff ERROR: Wrong format");}
  
  T_set = data.substring(1,3).toFloat();
  hum_room = data.substring(3,7).toFloat();
  T_room = data.substring(7,11).toFloat();

  Serial.print("heater_onOff: ");
  Serial.println(heater_onOff);
  Serial.print("T_set: ");
  Serial.println(T_set);
  Serial.print("humidity room: ");
  Serial.println(hum_room);
  Serial.print("temperature room: ");
  Serial.println(T_room);
  
}
