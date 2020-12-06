import os
import pandas
import numpy as np


def main():
    directory = '/home/natasha/repos/ns-3-dev-git'
    filename = 'SNRtrace.tr'
    snr = pandas.read_csv(os.path.join(directory, filename), names=["node", "SNR"])
    snr_agg = snr.groupby('node').agg({'SNR': [max, min, np.mean, np.median, np.std], 'node': len})
    snr_agg.to_csv(path_or_buf=filename, mode='a')
    print(snr_agg)


if __name__ == "__main__":
    main()