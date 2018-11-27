import serial
import numpy
import matplotlib.pyplot as plt
from drawnow import *

tempC = []
lightL = []

arduinoData = serial.Serial('/dev/ttyACM3', 115200)

plt.ion()
cunt=0
file = open("Data.txt", "a")


def fieldPlot():
    plt.ylim(0, 30) #ranges limited on y axis
    plt.title('lilipad stuff')
    plt.grid(True)
    plt.ylabel('Temp C')
    plt.plot(tempC, 'ro-', label='Degrees C')
    plt.legend(loc='upper left')
    plt2=plt.twinx()
    plt.ylim(0,10)
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

    file.write("Temperature C= ")
    file.write(str(temp))
    file.write("Light L =")
    file.write(str(l))
    
file.close()
        
    
    
    
'''
ser = serial.Serial(
   port='/dev/ttyACM4',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS
)

while 1:
    serial_line = ser.readline()
    print (str(serial_line))
    if len(serial_line) == 0:
      break
'''


