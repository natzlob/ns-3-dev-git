import os
import pandas
import numpy as np


def main():
    directory = '/home/natasha/repos/ns-3-dev-git'
    filename = 'SNRtrace_3.tr'
    snr = pandas.read_csv(os.path.join(directory, filename), names=["node", "SNR"])
    # snr_agg = snr.groupby('node').agg({'SNR': [max, min, np.mean, np.median, np.std], 'node': len})
    snr_agg = snr.groupby('node').agg({'SNR': [np.mean], 'node': len})

    mean_of_means_snr = snr_agg['SNR']['mean'].mean()
    print("mean of means = {}".format(mean_of_means_snr))

    with open('SNRaverage.txt', 'a') as f:
        f.write(str(mean_of_means_snr)+'\n')

    print("wrote mean of means to SNRaverage.csv")
    #return mean_of_means_snr


if __name__ == "__main__":
    main()