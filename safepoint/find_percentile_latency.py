import os
import glob
import numpy as np
import argparse

def find_percentile_latency(load, trials, nics, percentile):
    types = ["SPIN", "PCI", "UIPI", "APIC", "TIMER"]
    latencies = {type: [] for type in types}
    percentiles ={type: 0 for type in types}
    max_latencies = {type: 0 for type in types}
    for t in types:
        if round(load) == load:
            load = int(load)
        filename = f"latencies/latencyl3{load}GbpsTrack0{t}{trials}Trials{nics}Nics.txt"
        if os.path.exists(filename):
            with open(filename, 'r') as file:
                for line in file:
                    try:
                        latencies[t].append(float(line.strip()))
                    except ValueError:
                        continue
            percentiles[t] = np.percentile(latencies[t], percentile) 
            max_latencies[t] = np.max(latencies[t])

    if not latencies:
        raise ValueError("No valid latency data found.")
    return percentiles, max_latencies

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Find percentile latency.')
    parser.add_argument('load', type=float, help='Load in Gbps')
    parser.add_argument('trials', type=int, help='Number of trials')
    parser.add_argument('nics', type=int, help='Number of NICs')
    parser.add_argument('percentile', type=float, help='Percentile to find (0-100)')

    args = parser.parse_args()

    try:
        result = find_percentile_latency(args.load, args.trials, args.nics, args.percentile)
        print(result)
    except ValueError as e:
        print(e)