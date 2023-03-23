import pickle
from pprint import pprint

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import serial
from sklearn.linear_model import LogisticRegression

from utils import *


def send_prediction(prediction, ser):
    data = "{}\r\n".format(prediction)
    ser.write(data.encode())
    return

if __name__ == "__main__":

    print("Loading model...")

    with open('model.pth', 'rb') as file:
        model = pickle.load(file)
    
    print("Model loaded.")
    
    ser = serial.Serial(port, 9600) 

    curr_window = []

    while True:
        line = ser.readline().decode("utf-8").strip()

        datapoint = handle_inference_data(line)
        if not datapoint:
            continue
    
        curr_window.append(datapoint["Distance"])
        if len(curr_window) > timeseries_window:
            # keep at the specified window size
            curr_window.pop(0)
        np_window = pad_sequence(curr_window, timeseries_window-len(curr_window)+1, max_dist)

        prediction = model.predict(np_window.reshape(1,-1))
        send_prediction(prediction[0], ser)
        print(np_window, prediction)









