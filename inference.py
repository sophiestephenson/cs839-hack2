import serial

from utils import *


def send_prediction(prediction, ser):
    data += "{}\r\n".format(prediction)
    ser.write(data.encode())
    return

if __name__ == "__main__":
    
    ser = serial.Serial(port, 9600) 

    curr_window = []

    model = None

    while True:
        line = ser.readline().decode("utf-8").strip()

        datapoint = handle_data(line)
        if not datapoint:
            continue
    
        curr_window.append(datapoint)
        if len(curr_window) >= timeseries_window:
            # keep at the specified window size
            curr_window.pop(0)

            prediction = model.predict(datapoint)
            send_prediction(prediction, ser)









