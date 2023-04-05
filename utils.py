import numpy as np

port = '/dev/cu.wchusbserial1110'
fieldnames = ["Timestamp", "Distance", "Category"]
outfile = "trainingdata.csv"

startSymb = "start"
stopSymb = "stop"
staySymb = "-"

timeseries_window = 10
max_dist = 200.0

def handle_training_data(raw_serial_line, isCat): 
    datapoint = read_serial(raw_serial_line)
    isCat = update_cat(datapoint["Symbol"], isCat)
    datapoint["Category"] = isCat
    return datapoint

def handle_inference_data(raw_serial_line):
    datapoint = read_serial(raw_serial_line)
    return datapoint

def read_serial(raw_serial_line):
    pieces = raw_serial_line.split(",")

    if len(pieces) < 2:
        print(pieces)
        return None

    dist = float(pieces[1])
    if dist == 0:
        # 0 actually means it's at the max distance
        dist = max_dist

    symbol = None
    if len(pieces) > 2:
        symbol = pieces[2]

    datapoint = dict(
        Timestamp = int(pieces[0]),
        Distance = dist, 
        Symbol = symbol
    )

    return datapoint

def update_cat(symbol, isCat):
    if symbol == startSymb:
        isCat = True
    elif symbol == stopSymb:
        isCat = False
    return isCat


def pad_sequence(X, step, default):
  return np.pad(X, (step-1,0), mode='constant', constant_values=(default,))
