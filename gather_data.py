import serial

port = '/dev/cu.wchusbserial110'

ser = serial.Serial(port, 9600)
ser.readline()

# todo: loop, read lines, dump to file