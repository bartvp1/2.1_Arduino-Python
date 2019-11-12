#import serial

while 1==0:
    global ser
    try:
        #ser = serial.Serial("COM3", 9600)
        print(ser.readline().decode())
    except:
        #ser.close()
        continue
import serial.tools.list_ports
print([p for p in serial.tools.list_ports.comports()])