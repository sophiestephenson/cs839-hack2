import csv
from pprint import pprint

import serial

from utils import *


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
            datapoint = handle_data(line)
            if not datapoint:
                continue

            #write_to_csv(new_datapoint)
            csvwriter = csv.DictWriter(out, fieldnames=datapoint)
            csvwriter.writerow(datapoint)
            print(datapoint)