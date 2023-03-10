import serial

port = '/dev/cu.wchusbserial110'

ser = serial.Serial(port, 9600)
ser.readline()

