import serial

global ser
try:
    ser = serial.Serial('COM9', 9600)
    i = 0
    while ser.read():
        print(str(i) + " while: "+str(ser.readline().lower().decode()))
        i+=1
except serial.serialutil.SerialException:
    ser.close()
