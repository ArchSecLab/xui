import os
import re
import statistics
import sys

def parse_stat_file(filepath):
    sent_packets = {}
    recv_packets = 0

    with open(filepath, 'r') as file:
        for line in file:
            sent_match = re.match(r'board\.loadgens(\d*)\.LoadGenerator\.sentPackets\s+(\d+)\s', line)
            recv_match = re.match(r'board\.loadgens(\d*)\.LoadGenerator\.recvPackets\s+(\d+)', line)
            
            if sent_match:
                try:
                    nic = int(sent_match.group(1))
                except:
                    nic = 0
                sent = int(sent_match.group(2))
                if nic in sent_packets:
                    sent_packets[nic] += sent
                else:
                    sent_packets[nic] = sent

            if recv_match:
                recv_packets += int(recv_match.group(2))

    total_sent_packets = sum(sent_packets.values())
    return total_sent_packets, recv_packets

def main(nic_count, trial_count):
    stats_dir = 'stats'
    stat_files = [f for f in os.listdir(stats_dir) if f.startswith('statl3') and f.endswith('.txt')]

    # Filter stat_files by nic count and trial count
    stat_files = [f for f in stat_files if int(re.search(r'(\d+)Nics', f).group(1)) == nic_count and int(re.search(r'(\d+)Trials', f).group(1)) == trial_count]

    # we assume the miss rate in overload is linearly correlated with the load-max_load and get max_load
    estimated_max_loads_from_overloads = []
    estimated_max_load_from = []
    # we need the background cycles to estimate how much we can 
    estimated_max_loads_from_underloads = []

    for stat_file in stat_files:
        filepath = os.path.join(stats_dir, stat_file)
        total_sent_packets, recv_packets = parse_stat_file(filepath)
        stat_file_mod = stat_file[len('statl3'):]
        # print(f'Stat File: {stat_file_mod}')
        # load = re.match(r'(\d+)\s', stat_file_mod).group(1)
        load = float(re.match(r'(\d+(\.\d+)?)Gbps', stat_file_mod).group(1))
        # print(f'Load: {load}')
        #print(f'File: {stat_file}')
        #print(f'Total Sent Packets: {total_sent_packets}')
        #print(f'Received Packets: {recv_packets}')
        #print(f'Difference: {total_sent_packets - recv_packets}')
        # recv packets count for 3.4% less time so readjust the recv_packets
        recv_packets = recv_packets * 1.034
        if recv_packets < total_sent_packets:
            missed_packets = total_sent_packets - recv_packets
            # load -> total_sent_packets
            # max_load -> recv_packets
            # example: load = 100, and we have 1000 packets sent, and 900 packets received
            # load = 1000/100 = 10 packets per load
            # max_load = 900/10 = 90
            load_per_packet = load/total_sent_packets
            estimated_max_load = load_per_packet*recv_packets
            estimated_max_loads_from_overloads.append(estimated_max_load)
            estimated_max_load_from.append((load,stat_file))
        
    if estimated_max_loads_from_overloads:
        median_max_load = statistics.median(estimated_max_loads_from_overloads)
        print(f'Median Estimated Max Load: {median_max_load}')
    else:
        print('No estimated max loads to calculate median.')

    print(f'Estimated Max Loads: {estimated_max_loads_from_overloads}')
    overloads_above_median_that_are_sustained = []
    for stat_file in stat_files:
        filepath = os.path.join(stats_dir, stat_file)
        total_sent_packets, recv_packets = parse_stat_file(filepath)
        stat_file_mod = stat_file[len('statl3'):]
        load = float(re.match(r'(\d+(\.\d+)?)Gbps', stat_file_mod).group(1))
        recv_packets = recv_packets * 1.034
        # all loads that are above the median max load and could be sustained
        if recv_packets > total_sent_packets and load > median_max_load:
            overloads_above_median_that_are_sustained.append((load,stat_file))

    pci_overloads = []
    non_pci_overloads = []
    pci_max = 0
    non_pci_max = 0

    for load, stat_file in overloads_above_median_that_are_sustained:
        if 'PCI' in stat_file:
            pci_overloads.append((load, stat_file))
            pci_max = max(pci_max, load)
        else:
            non_pci_overloads.append((load, stat_file))
            non_pci_max = max(non_pci_max, load)

    #print(f'PCI Overloads: {pci_overloads}')
    #print(f'Non-PCI Overloads: {non_pci_overloads}')
    print(f'PCI Max: {pci_max}')
    print(f'Non-PCI Max: {non_pci_max}')
    # print(f'Overloads above median that are sustained: {overloads_above_median_that_are_sustained}')
            
        

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python find_sent_vs_recv.py <nic_count> <trial_count>")
        sys.exit(1)
    nic_count = int(sys.argv[1])
    trial_count = int(sys.argv[2])
    main(nic_count, trial_count)

