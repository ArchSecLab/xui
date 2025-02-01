python3 x86-ubuntu-run-with-kvm-uipi-template-processor-accel.py --int ${int_type} --e ${max_error} --pci ${pci} --len ${request_length} --vmr ${vmr}
rm m5out/stats.txt
while [ ! -s m5out/stats.txt ]
do
./g x86-ubuntu-run-with-kvm-uipi-accel.py
done
cp m5out/stats.txt ../../stats/stat${request_length}us${int_type_full}${stddev}${pci_type}${max_error}.txt
cp m5out/board.pc.com_1.device ../../boards/board${request_length}us${int_type_full}${stddev}${pci_type}${max_error}
cp accel_latency ../../latencies/latency${request_length}us${int_type_full}${stddev}${pci_type}${max_error}.txt
touch ../../done${request_length}us${int_type_full}${stddev}${pci_type}${max_error}