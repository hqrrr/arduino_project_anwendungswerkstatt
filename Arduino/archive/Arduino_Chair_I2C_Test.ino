#include <Wire.h>

// Variables
// receiving
bool heater_onOff;
float T_set;
float hum_room;
// sending
float T_is_chair = 33.3333;
//bool is_sitting;


void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  Serial.begin(9600);           // start serial for output
  Serial.println("Setup complete");
  pinMode(A0, INPUT);           // for Temperature Sensor
}

void loop() {
  delay(100);
  T_is_chair = ((analogRead(A0))*(5.0/1023.00)-0.5)*100.00;
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
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

void requestEvent() {
  String T_is_chair_Str = String(T_is_chair, DEC).substring(0,4);
  Wire.write(T_is_chair_Str.c_str());        // respond with message of 4 bytes as expected by master
  Serial.println("T_chair = " + String(T_is_chair));
  Serial.println(String(analogRead(A0)));                               
                                             
}
