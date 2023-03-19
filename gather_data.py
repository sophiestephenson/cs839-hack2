import csv
from pprint import pprint

import serial

port = '/dev/cu.wchusbserial1110'
fieldnames = ["ts", "dist", "cat"]
outfile = "trainingdata.csv"

startSymb = "start"
stopSymb = "stop"

def write_to_csv(data):
    with open(outfile, 'a') as out:
        csvwriter = csv.DictWriter(out, fieldnames=fieldnames)
        csvwriter.writerow(data)

if __name__ == "__main__":

    with open(outfile, 'w') as out:
        csvwriter = csv.DictWriter(out, fieldnames=fieldnames)
        csvwriter.writeheader()

    ser = serial.Serial(port, 9600)

    isCat = False

    while True:
        line = ser.readline().decode("utf-8").strip()
        pieces = line.split(",")

        if len(pieces) < 3:
            continue

        symbol = pieces[2]
        if symbol == startSymb:
            isCat = True
        elif symbol == stopSymb:
            isCat = False

        new_datapoint = dict(
            ts = int(pieces[0]),
            dist = float(pieces[1]), 
            cat = int(isCat)
        )

        write_to_csv(new_datapoint)
        print(new_datapoint)