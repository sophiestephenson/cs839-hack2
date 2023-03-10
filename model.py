import serial

ser = serial.Serial('/dev/cu.wchusbserial110', 9600)
ser.readline()

