import matplotlib.pyplot as plt
import csv

x=[]
y=[]
print ("What data you want to see:\n")
print ("1.Temperature Data\n")
print ("2.Light Data\n")

filename =raw_input()

if filename == str(1): #or "Temperature Data" or "temperature data" or "temp data" or "Temp Data" or "temp" or "Temp":
    with open('DataTemp.txt', 'r') as csvfile:
        plots = csv.reader(csvfile)
        for row in plots:
            #x.append(int(row[0]))
            #print (x)
            y.append(float(row[0]))
            print(y)
    plt.figure(1)
    plt.plot(y,label="Temperature")
    #plt.xlabel('x')
    plt.ylabel('Temperature')
    plt.title('TempData')
    plt.legend()
    plt.show()

if filename == str(2): #or "Light Data" or "light data" or "Light" or "light":
    #with open('Data.txt', 'r') as csvfile2:
    with open('DataLight.txt', 'r') as csvfile2:
        plots2 = csv.reader(csvfile2)
        for row2 in plots2:
            x.append(float(row2[0]))
            print (x)
            #y.append(int(row[0]))
            #print(y)
    plt.figure(2)
    plt.plot(x,label="Light")
    #plt.xlabel('x')
    plt.ylabel('Light')
    plt.title('LightData')
    plt.legend()
    plt.show()
