import os
import pandas
import numpy as np


def main():
    directory = '/home/natasha/repos/ns-3-dev-git'
    filename = 'SNRtrace_3.csv'
    snr = pandas.read_csv(os.path.join(directory, filename), names=["node", "SNR"])
    # snr_agg = snr.groupby('node').agg({'SNR': [max, min, np.mean, np.median, np.std], 'node': len})
    snr_agg = snr.groupby('node').agg({'SNR': [np.mean], 'node': len})
    print("average of means = {}".format(snr_agg.SNR.mean()))
    # snr_agg.to_csv('SNRaverage.csv', mode='a')
    # print("SNR aggregate: {}".format(snr_agg))
    mean_of_means_snr = snr_agg['SNR']['mean'][1].mean()
    print("mean of means = {}".format(mean_of_means_snr))

    with open('SNRaverage.csv', 'wa') as f:
        f.write(str(mean_of_means_snr))
    return mean_of_means_snr


if __name__ == "__main__":
    main()