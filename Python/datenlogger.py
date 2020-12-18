# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'connect_me.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!


import time
import urllib.request  # get data from an URL
import sys
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import os

from gui import Ui_MainWindow



class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setupUi(self)
        # styling plots
        self.graphWidget.setBackground('w')  # background
        pen = pg.mkPen(color=(255, 0, 0))  # line color
        self.graphWidget.setTitle("Temperature")  # plot titles

        # =================================
        self.values = []
        self.temperature = []
        self.pressure = []
        self.humidity = []
        self.current_time = []
        self.valueRead_list = []
        self.valueRead_list_1 = []
        self.valueRead_list_final = []

        # preload some fake data
        for i in range(0, 50):
            self.temperature.append(20)
            self.pressure.append(900)
            self.humidity.append(30)
            self.current_time.append(i + 1)

        # create plot
        self.data_line = self.graphWidget.plot(self.current_time, self.temperature, pen=pen, symbol='x', symbolSize=10)

        # get data from Arduino
        self.get_data()

        # update plot
        self.timer = QtCore.QTimer()
        self.timer.setInterval(5000)  # 5000ms = 5s
        self.timer.timeout.connect(self.get_data)
        self.timer.start()


    def get_data(self):
        # reset all lists
        self.valueRead_list = []
        self.valueRead_list_1 = []
        self.valueRead_list_final = []
        # open link
        f = urllib.request.urlopen('http://192.168.0.99/')

        # read data and processing
        self.valueRead = f.read()  # type bytes
        self.valueRead_str = str(self.valueRead.decode("utf-8"))  # type bytes -> string
        self.valueRead_str = self.valueRead_str.strip('<!DOCTYPE HTML>/html').strip().strip(
            '<br />/html').strip()  # delete useless characters
        self.valueRead_list = self.valueRead_str.split("<br />")  # split into a list according to <br />
        self.valueRead_list_1 = [i.split(": ") for i in self.valueRead_list]  # split values and labels in a new list
        for i in range(0, len(self.valueRead_list_1)):
            self.valueRead_list_1[i].pop(0)  # delete labels
            self.valueRead_list_final.append(self.valueRead_list_1[i][0])  # import all values in the final list

    #def update_data(self):
        for i in range(0, len(self.valueRead_list_final)):
            self.value = float(self.valueRead_list_final[i])

            if self.value >= 0:
                if i == 0:
                    self.temperature.append(self.value)
                    #self.temperature.pop(0)
                elif i == 1:
                    self.pressure.append(self.value)
                    #self.pressure.pop(0)
                elif i == 2:
                    self.humidity.append(self.value)
                    #self.humidity.pop(0)
                else:
                    print("Error! len(valueRead_list) incorrect")

            else:
                print("Invalid! negative number")
        self.current_time.append(self.current_time[-1] + 1) # Add a new value 1 higher than the last.
        self.data_line.setData(self.current_time, self.temperature)




def main():
    app = QtWidgets.QApplication(sys.argv)
    main = MainWindow()
    main.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()

