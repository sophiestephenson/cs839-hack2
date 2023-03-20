
port = '/dev/cu.wchusbserial1110'
fieldnames = ["Timestamp", "Distance", "Category"]
outfile = "trainingdata.csv"

startSymb = "start"
stopSymb = "stop"

timeseries_window = 10

def handle_data(raw_serial_line):
    pieces = raw_serial_line.split(",")

    if len(pieces) < 3:
        return None

    dist = float(pieces[1])
    if dist == 0:
        # 0 actually means it's at the max distance
        dist = 200.0

    symbol = pieces[2]
    if symbol == startSymb:
        isCat = True
    elif symbol == stopSymb:
        isCat = False

    datapoint = dict(
        Timestamp = int(pieces[0]),
        Distance = dist, 
        Category = int(isCat)
    )

    return datapoint