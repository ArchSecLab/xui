python3 x86-ubuntu-run-with-kvm-uipi-template-processor-l3.py --int ${int_type} --p ${packet_rate}  --interval ${interrupt_interval} --pci ${pci} --t ${trial} --nics ${nics} --alt ${alt} --m ${mode}
rm m5out/stats.txt
while [ ! -s m5out/stats.txt ]
do
./g x86-ubuntu-run-with-kvm-uipi-l3.py
done
cp m5out/stats.txt ../../stats/statl3${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${trial}Trials${nics}Nics.txt
cp m5out/board.pc.com_1.device ../../boards/boardl3${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${trial}Trials${nics}Nics
cp latency.txt ../../latencies/latencyl3${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${trial}Trials${nics}Nics.txt
touch ../../donel3${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${trial}Trials${nics}Nics