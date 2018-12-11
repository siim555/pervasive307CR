import serial
import warnings
import serial.tools.list_ports
import numpy
import matplotlib.pyplot as plt
from drawnow import *
import datetime
import os.path


tempC = []
lightL = []

exists = os.path.isfile('DataTemp.txt')
exists2 = os.path.isfile('DataLight.txt')


arduino_ports = [
    p.device
    for p in serial.tools.list_ports.comports()
    if 'Arduino' in p.description
]
if not arduino_ports:
    raise IOError("No Arduino found")
if len(arduino_ports) > 1:
    warnings.warn('Multiple Arduinos found - using the first')
    
    
arduinoData = serial.Serial(arduino_ports[0], 115200)


plt.ion()
cunt=0


def fieldPlot():
    plt.ylim(0, 30) #ranges limited on y axis
    plt.title('lilipad stuff')
    plt.grid(True)
    plt.ylabel('Temp C')
    plt.plot(tempC, 'ro-', label='Degrees C')
    plt.legend(loc='upper left')
    plt2=plt.twinx()
    plt.ylim(0,200)
    plt2.plot(lightL, 'bo-', label = 'Light L')
    plt.ylabel('Light level')
    plt2.ticklabel_format(useOffset=False)      #Force matplot to Not autoscale y axis
    plt2.legend(loc='upper right')

while True:
    while (arduinoData.inWaiting() == 0):
        pass
    arduinoString = arduinoData.readline()
    dataArray = arduinoString.split(',')
    temp = float(dataArray[0])
    l =    float(dataArray[1])
    tempC.append(temp)
    lightL.append(l)
    drawnow(fieldPlot)
    plt.pause(.000001)
    cunt=cunt+1
    if(cunt>50):
        tempC.pop(0)
        lightL.pop(0)
    if exists and exists2:
        file = open("DataTemp.txt", "a")
        file2 = open("DataLight.txt", "a")
        file.write(str(temp) +'\n')
        file2.write(str(l) + '\n')
   
file.close()
file2.close()

