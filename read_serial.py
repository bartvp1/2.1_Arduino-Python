import serial

global ser
try:
    ser = serial.Serial('COM9', 9600)
    i = 0
    while True:
        x = ser.readline()
        if x:
            print(str(i) + " while: "+str(x.decode()))
            i+=1
        if i == 2:
            serOutput = serial.Serial("COM9", 9600)
            serOutput.flushInput()
            serOutput.write("{xd}")
except serial.serialutil.SerialException:
    ser.close()
