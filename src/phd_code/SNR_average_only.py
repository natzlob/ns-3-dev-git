import os
import pandas
import numpy as np


def main():
    directory = '/home/natasha/repos/ns-3-dev-git'
    filename = 'SNR_5G.csv'
    signalNoiseInterferenceSNR = pandas.read_csv(
        os.path.join(directory, filename),
        # low_memory=False,
        #dtype={'signal(W)': float, 'noise(W)': float, 'interference(W)': float, 'snr(dB)': float},
        names=['snr(dB)'])
    
    snravg =signalNoiseInterferenceSNR['snr(dB)'].mean()
    print("mean SINR = {}".format(snravg))

    with open('SINRaverage4.csv', 'a') as f:
        f.write(str(snravg))
        f.write('\n')

    print("wrote value {} to SINRaverage4.csv".format(snravg))
    return snravg


if __name__ == "__main__":
    main()