# -*- coding: utf-8 -*-
# =================================
# Python 3
# Pycharm 2020.3
# Author: hqrrr
# Date of last update: Dez. 2020
# =================================
import matplotlib.pyplot as plt
from drawnow import *  # real time plot lib, pip install drawnow
import atexit
import time
import urllib  # get data from an URL

# =================================
#            settings
# =================================
# settings of urllib
link = "http://192.168.0.99/"

# activate interactive mode of matplotlib
plt.ion()

# font size of the point label
fz = 7

# 1

# =================================
values = []
temperature = []
pressure = []
humidity = []


# functions
# def doAtExit():
#    arduino.close() # close arduino serial port
#    print("Close serial")
#    print("arduino.isOpen() = " + str(arduino.isOpen())) # True: open, False: closed

def plotValues():
    plt.subplots_adjust(wspace=0, hspace=0.75)
    plt.rcParams['figure.figsize'] = [10, 10]
    plt.rcParams['figure.dpi'] = 150
    plt.subplot(311)
    plt.title("temperature")  # plot title
    plt.grid(True)  # show grid, True or False
    plt.ylabel("temperature [$^\circ$C]")  # label of axis y
    plt.plot(temperature, "-", label="bme280")
    plt.legend(loc="best")  # label location

    # label data points on plot
    for x, y in enumerate(temperature):
        plt.text(x, y + 0.001, '%.1f' % y, ha='center', va='bottom', fontsize=fz)

    plt.subplot(312)
    plt.title("pressure")  # plot title
    plt.grid(True)  # show grid, True or False
    plt.ylabel("pressure [hPa]")  # label of axis y
    plt.plot(pressure, "-", label="bme280")
    plt.legend(loc="best")  # label location

    # label data points on plot
    for x, y in enumerate(pressure):
        plt.text(x, y + 0.001, '%.0f' % y, ha='center', va='bottom', fontsize=fz)

    plt.subplot(313)
    plt.title("humidity")  # plot title
    plt.grid(True)  # show grid, True or False
    plt.ylabel("humidity [%]")  # label of axis y
    plt.plot(humidity, "-", label="bme280")
    plt.legend(loc="best")  # label location

    # label data points on plot
    for x, y in enumerate(humidity):
        plt.text(x, y + 0.001, '%.1f' % y, ha='center', va='bottom', fontsize=fz)


# atexit.register(doAtExit) # if program exit, do function doAtExit to close arduino serial port

# print("arduino.isOpen() = " + str(arduino.isOpen())) # True: open, False: closed

# pre-load some dummy data
for i in range(0, 50):
    temperature.append(20)
    pressure.append(900)
    humidity.append(30)

while True:
    time.sleep(5)  # 5-sec delay for each loop
    # reset all lists
    valueRead_list = []
    valueRead_list_1 = []
    valueRead_list_final = []
    # open link
    f = urllib.request.urlopen(link)
    # read data and processing
    valueRead = f.read()  # type: bytes
    valueRead_str = str(valueRead.decode("utf-8"))  # type bytes -> string
    valueRead_str = valueRead_str.strip('<!DOCTYPE HTML>/html').strip().strip(
        '<br />/html').strip()  # delete useless characters
    valueRead_list = valueRead_str.split("<br />")  # split into a list according to <br />
    valueRead_list_1 = [i.split(": ") for i in valueRead_list]  # split values and labels in a new list
    for i in range(0, len(valueRead_list_1)):
        valueRead_list_1[i].pop(0)  # delete labels
        valueRead_list_final.append(valueRead_list_1[i][0])  # import all values in the final list

    # check if value valid
    try:
        for i in range(0, len(valueRead_list_final)):
            value = float(valueRead_list_final[i])

            if value >= 0:
                if i == 0:
                    temperature.append(value)
                    temperature.pop(0)
                elif i == 1:
                    pressure.append(value)
                    pressure.pop(0)
                elif i == 2:
                    humidity.append(value)
                    humidity.pop(0)
                else:
                    print("Error! len(valueRead_list) incorrect")

            else:
                print("Invalid! negative number")

        drawnow(plotValues)


    except ValueError:
        print("Invalid! cannot cast")
        print(valueRead_str)