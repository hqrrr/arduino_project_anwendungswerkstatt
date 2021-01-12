# arduino project anwendungswerkstatt

## 1 Diagram

### 1.1 Circuit diagram

#### 1.1.1 Arduino Desk (Master) {Arduino MKR WiFi 1010}

#### 1.1.2 Arduino Chair (Slave) {Arduino Nano ATmega328P (Old Bootloader)}

### 1.2 Sketch

![image](pictures/sketch.png)

## 2. python code

### 2.1 function to get data from Arduino via WiFi

in this version, used matplotlib + drawnow libraries to create 
live plot automatically

#### 2.1.1 for Jupyter Notebook users:
[get_data.ipynb](/Python/get_data.ipynb)

#### 2.1.2 for other python IDE users:
[get_data.py](/Python/get_data.py)

### 2.2 GUI

in this version, used pyqtgraph to create live plot and PyQt5 to create
GUI

[datenlogger.py](/Python/datenlogger.py)

Environmental:

![image](/pictures/Pycharm_environmental_GUI.png)

#### 2.2.1 UI

[gui.ui](/Python/gui.ui)

used QtDesigner to build GUI

[gui.py](/Python/gui.py)

used PyUIC5 to convert .ui in .py

## 3. Arduino code:

### 3.1 Arduino Desk (Master) {Arduino MKR WiFi 1010}

[Arduino_nano_mkr_wifi_1010_I2C_connection_Master_mkr_1010.ino](/Arduino/Arduino_nano_mkr_wifi_1010_I2C_connection_Master_mkr_1010/Arduino_nano_mkr_wifi_1010_I2C_connection_Master_mkr_1010.ino)

Change WiFi username & password in:

[arduino_secrets.h](/Arduino/Arduino_nano_mkr_wifi_1010_I2C_connection_Master_mkr_1010/arduino_secrets.h)

Circuit diagram --> **1.1.1**

### 3.2 Arduino Chair (Slave) {Arduino Nano ATmega328P (Old Bootloader)}

[Arduino_nano_mkr_wifi_1010_I2C_connection_Slave_Nano.ino](/Arduino/Arduino_nano_mkr_wifi_1010_I2C_connection_Slave_Nano/Arduino_nano_mkr_wifi_1010_I2C_connection_Slave_Nano.ino)

Circuit diagram --> **1.1.2**

## 4. list of hardware:

Arduino MKR WiFi 1010

Arduino Nano (ATmega328P (Old Bootloader)) 

DHT 11 sensor (temperature/humidity)

Dallas temperature sensor DS18B20

Grove - MOSFET

PIR sensor 

## 5. Demo:
### 5.1 Datenlogger (jupyter notebook + matplotlib)
![image](pictures/plot_demo.gif)
