import csv
from pprint import pprint

import serial

port = '/dev/cu.wchusbserial1110'
fieldnames = ["Timestamp", "Distance", "Category"]
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

    with open(outfile, 'a') as out:

        ser = serial.Serial(port, 9600)

        isCat = False

        while True:
            line = ser.readline().decode("utf-8").strip()
            pieces = line.split(",")

            if len(pieces) < 3:
                continue

            dist = float(pieces[1])
            if dist == 0:
                # 0 actually means it's at the max distance
                dist = 200.0

            symbol = pieces[2]
            if symbol == startSymb:
                isCat = True
            elif symbol == stopSymb:
                isCat = False

            new_datapoint = dict(
                Timestamp = int(pieces[0]),
                Distance = dist, 
                Category = int(isCat)
            )

            #write_to_csv(new_datapoint)
            csvwriter = csv.DictWriter(out, fieldnames=fieldnames)
            csvwriter.writerow(new_datapoint)
            print(new_datapoint)