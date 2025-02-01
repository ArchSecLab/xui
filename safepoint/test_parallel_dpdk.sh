python3 x86-ubuntu-run-with-kvm-uipi-template-processor-dpdk.py --int ${int_type} --p ${packet_rate}  --interval ${interrupt_interval} --pci ${pci} --alt ${alt} --m ${mode}
rm m5out/stats.txt
while [ ! -s m5out/stats.txt ]
do
./g x86-ubuntu-run-with-kvm-uipi-dpdk.py
done
cp m5out/stats.txt ../../stats/statNew${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${mode_type}.txt
cp m5out/board.pc.com_1.device ../../boards/boardNew${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${mode_type}
cp latency.txt ../../latencies/latencyNew${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${mode_type}.txt
touch ../../doneNew${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${mode_type}