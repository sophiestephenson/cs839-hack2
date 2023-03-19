import serial

port = '/dev/cu.wchusbserial110'

ser = serial.Serial(port, 9600)

while True:
    line = ser.readline()
    print(line)

# todo: loop, read lines, dump to file