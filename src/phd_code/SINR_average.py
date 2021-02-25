import os
import pandas
import numpy as np


def main():
    directory = '/home/natasha/repos/ns-3-dev-git'
    filename = 'SignalNoiseInterference_5G.csv'
    signalNoiseInterferenceSNR = pandas.read_csv(
        os.path.join(directory, filename), 
        dtype={'signal(W)': float, 'noise(W)': float, 'interference(W)': float, 'snr(dB)': float},
        names=['signal(W)', 'noise(W)', 'interference(W)', 'snr(dB)'])
    
    snravg =signalNoiseInterferenceSNR['snr(dB)'].mean()
    print("mean SINR = {}".format(snravg))

    with open('SINRaverage2.csv', 'a') as f:
        f.write(str(snravg))
        f.write('\n')
    return snravg


if __name__ == "__main__":
    main()